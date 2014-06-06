#include "DDialogProperties.h"

DDialogPropertyPage::DDialogPropertyPage(PSZAC pszName) : QListWidgetItem(pszName), WWidget(NULL)
	{
	//new QLabel(pszName, this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#define SL_DDialogProperties(pfmSlot)		SL_DDialog(pfmSlot, DDialogProperties)

DDialogProperties::DDialogProperties()
	{
	m_pwPagesList = new QListWidget(this);
	m_pwPagesStacked = new QStackedWidget(this);
	OLayoutHorizontal * poLayoutHorizontal = new OLayoutHorizontal(this);
	poLayoutHorizontal->addWidget(m_pwPagesList);
	OLayoutVertical * poLayoutVertical = new OLayoutVertical(poLayoutHorizontal);
	poLayoutVertical->addWidget(m_pwPagesStacked);
	connect(m_pwPagesList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(SL_PageChanged(QListWidgetItem*,QListWidgetItem*)));

	Dialog_CreateButtonsOkCancel(SL_DDialogProperties(SL_ButtonOK_clicked));
	m_poLayoutButtons = new OLayoutHorizontal(poLayoutVertical);
	m_poLayoutButtons->setAlignment(Qt::AlignRight);
	m_poLayoutButtons->addWidget(m_pwButtonOK);
	m_poLayoutButtons->addWidget(m_pwButtonCancel);
	}

void
DDialogProperties::PageAdd(DDialogPropertyPage * pawPage)
	{
	m_pwPagesList->addItem(pawPage); // pawPage->windowTitle());
	m_pwPagesStacked->addWidget(pawPage);
	}

//	DDialogProperties::QDialog::exec()
int
DDialogProperties::exec()
	{
	if (m_pwPagesList->currentItem() == NULL)
		m_pwPagesList->setCurrentRow(0);	// If no page is selected, select the first one
	return QDialog::exec();
	}

void
DDialogProperties::SL_PageChanged(QListWidgetItem * pPageCurrent, QListWidgetItem * pPagePrevious)
	{
	Endorse(pPagePrevious == NULL);
	DDialogPropertyPage * pwPage = (DDialogPropertyPage *)pPageCurrent;
	m_pwPagesStacked->setCurrentWidget(pwPage);
	}

void
DDialogProperties::SL_ButtonOK_clicked()
	{
	EMessageBoxInformation("OK");
	}

void
DDialogProperties::SL_ButtonCancel_clicked()
	{

	}


///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogPropertiyPageAccountGeneral::DDialogPropertiyPageAccountGeneral(TAccountXmpp * pAccount) : DDialogPropertyPage("General")
	{
	m_pAccount = pAccount;
	/*
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(this);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	//poLayout->setSizeConstraint(QLayout::SetMaximumSize);
	poLayout->Layout_AddLabelAndWidgetH_PA("Username", new WEditUsername);
	poLayout->Layout_AddLabelAndWidgetH_PA("Password", new WEditPassword);
	poLayout->addWidget(PA_CHILD new QWidget, 1);
	*/
	OLayoutForm * pLayout = new OLayoutForm(this);
	pLayout->Layout_PwAddRowLabelEditReadOnly("Username:", pAccount->m_strUsername);
	m_pwEditPassword = pLayout->Layout_PwAddRowLabelEditPassword("Password:");
	pLayout->Layout_PwAddRowLabelEditReadOnly("Server:", pAccount->m_strServerName);
	pLayout->Layout_PwAddRowLabelEditReadOnlyInt("Port:", pAccount->m_uServerPort);
	pLayout->Layout_PwAddRowLabelEditReadOnly("Resource:", pAccount->m_strJIDwithResource.PchFindCharacter('/'));
	pLayout->Layout_PwAddRowLabelEditReadOnlyFingerprint("Certificate Fingerprint:", *pAccount->Certificate_PGetBinaryFingerprint());
	setMinimumWidth(500);
	}

DDialogPropertiyPageAccountTest::DDialogPropertiyPageAccountTest(PSZAC pszName) : DDialogPropertyPage(pszName)
	{
	new QLabel(pszName, this);
	}

DDialogPropertiesAccount::DDialogPropertiesAccount(TAccountXmpp * pAccount)
	{
	m_pAccount = pAccount;
	Dialog_SetCaptionFormat_VE("Account - ^j", pAccount);
	PageAdd(new DDialogPropertiyPageAccountGeneral(pAccount));
	PageAdd(new DDialogPropertiyPageAccountTest("RootID"));
	PageAdd(new DDialogPropertiyPageAccountTest("PGP"));
	PageAdd(new DDialogPropertiyPageAccountTest("Bitcoin"));
	PageAdd(new DDialogPropertiyPageAccountTest("Notes"));
	m_pwPagesList->setFixedWidth(100);
	}


void
TAccountXmpp::DisplayDialogProperties()
	{
	DDialogPropertiesAccount dialog(this);
	dialog.FuExec();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogPropertiyPageContactGeneral::DDialogPropertiyPageContactGeneral(TContact * pContact) : DDialogPropertyPage("General")
	{
	m_pContact = pContact;
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(this);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	poLayout->Layout_AddLabelAndWidgetH_PA("Name", new WEditUsername);
	poLayout->addWidget(PA_CHILD new QWidget, 1);
	}

DDialogPropertiesContact::DDialogPropertiesContact(TContact * pContact)
	{
	m_pContact = pContact;
	Dialog_SetCaptionFormat_VE("Contact - ^j", pContact);
	PageAdd(new DDialogPropertiyPageContactGeneral(pContact));
	PageAdd(new DDialogPropertiyPageAccountTest("RootID"));
	PageAdd(new DDialogPropertiyPageAccountTest("PGP"));
	PageAdd(new DDialogPropertiyPageAccountTest("Bitcoin"));
	PageAdd(new DDialogPropertiyPageAccountTest("Notes"));
	m_pwPagesList->setFixedWidth(100);
	}

void
TContact::DisplayDialogProperties()
	{
	DDialogPropertiesContact dialog(this);
	dialog.FuExec();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TGroup::DisplayDialogProperties()
	{
	DDialogPropertiesGroup dialog(this);
	dialog.FuExec();
	}

DDialogPropertiyPageGroupGeneral::DDialogPropertiyPageGroupGeneral(TGroup * pGroup) : DDialogPropertyPage("General")
	{
	m_pGroup = pGroup;
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(this);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	poLayout->Layout_AddLabelAndWidgetH_PA("Group ID", new WEditReadOnly(g_strScratchBufferStatusBar.Format("{h|}", &pGroup->m_hashGroupIdentifier)));
	poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	poLayout->Layout_AddLabelAndWidgetH_PA("File Name", new WEditReadOnly(pGroup->Vault_SGetPath()));
	//poLayout->addWidget(PA_CHILD new QWidget, 1);
	}

DDialogPropertiesGroup::DDialogPropertiesGroup(TGroup * pGroup)
	{
	m_pGroup = pGroup;
	Dialog_SetCaptionFormat_VE("Group - $s", pGroup->TreeItem_PszGetNameDisplay());
	PageAdd(new DDialogPropertiyPageGroupGeneral(pGroup));
	m_pwPagesList->setFixedWidth(100);
	setMinimumWidth(600);
	}

