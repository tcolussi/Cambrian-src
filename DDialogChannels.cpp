//	DDialogChannels.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "DDialogChannels.h"

//	Search all groups find the first matching channel name.
//	Eventually this should be implemented with a hash table
TGroup *
TProfile::PFindChannelByName(PSZUC pszChannelName) CONST_MCC
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TGroup * pChannel = pAccount->GroupChannel_PFindByName_YZ(pszChannelName);
		if (pChannel != NULL)
			return pChannel;
		}
	return NULL;
	}
/*
BOOL
TProfile::FIsChannelNameUnused(PSZUC pszChannelName) CONST_MCC
	{

	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TGroup * pGroup = pAccount->GroupChannel_PFindByName_YZ(pszChannelName);
		if (pGroup != NULL)
			{
			if (pGroup->Group_FuIsChannelUsed())
				return FALSE;
			break;
			}
		}
	return TRUE;
	}
*/

void
DisplayDialogChannelsBrowse(TProfile * pProfile)
	{
	if (pProfile == NULL)
		return;	// This happens when no profile is selected
	Assert(pProfile != NULL);
	DDialogChannels dialog(pProfile);
	dialog.FuExec();
	}

BOOL
DisplayDialogChannelNewFu(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	DDialogChannelNew dialog(pProfile);
	return dialog.FuExec();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogChannels::DDialogChannels(TProfile * pProfile) : DDialogOkCancelWithLayouts("Join Channel", eMenuIcon_HashtagBlack)
	{
	Assert(pProfile != NULL);
	m_pProfile = pProfile;

	setMinimumWidth(600);
	m_pwTreeRecommendations = new WTreeWidget;
	m_poLayoutBody->addWidget(m_pwTreeRecommendations);
	m_pwTreeRecommendations->setFocus(Qt::TabFocusReason);
	m_pwTreeRecommendations->setRootIsDecorated(false);
	m_pwTreeRecommendations->setColumnCount(2);
	m_pwTreeRecommendations->setHeaderLabels((QStringList) "Channel Name" << "Channel Purpose" << "Popularity" << "Also Joined By");
	QHeaderView * pwHeader = m_pwTreeRecommendations->header();
	pwHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	pwHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	OLayoutHorizontal * poLayoutHorizontal = new OLayoutHorizontal(m_poLayoutBody);
	WButtonText * pwButtonChannelNew = new WButtonTextWithIcon("New Channel...", eMenuIcon_Add);
	WButtonText * pwButtonOK = new WButtonTextWithIcon("Join Channels ", eMenuIcon_HashtagBlack);
	connect(pwButtonOK, SIGNAL(clicked()), this, SLOT(SL_ButtonOK()));
	connect(pwButtonChannelNew, SIGNAL(clicked()), this, SLOT(SL_ButtonNew()));
	poLayoutHorizontal->addWidget(pwButtonOK);
	poLayoutHorizontal->addWidget(pwButtonChannelNew, d_zNA, Qt::AlignRight);

	QIcon oIconChannel = OGetIcon(eMenuIcon_HashtagBlack);
	CChannelName ** ppChannelNameStop;
	CChannelName ** ppChannelName = pProfile->m_arraypaChannelNamesAvailables.PrgpGetChannelsStop(OUT &ppChannelNameStop);
	while (ppChannelName != ppChannelNameStop)
		{
		CChannelName * pChannelName = *ppChannelName++;
		TGroup * pGroupChannel = pProfile->PFindChannelByName(pChannelName->m_strName);
		if (pGroupChannel != NULL && pGroupChannel->Group_FuIsChannelUsed())
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Channel '$S' is in use\n", &pChannelName->m_strName);
			continue;
			}
		CTreeWidgetItem * paRecommendation = new CTreeWidgetItem;
		paRecommendation->setCheckState(0, Qt::Unchecked);
		paRecommendation->setIcon(0, oIconChannel);
		paRecommendation->setText(0, pChannelName->m_strName);
	//	paRecommendation->setText(1, Timestamp_ToQDateTime(pChannel->m_tsFirstRecommended).toString());
		m_pwTreeRecommendations->addTopLevelItem(paRecommendation);
		}
	//m_pwTreeRecommendations->setFocus(Qt::ActiveWindowFocusReason); // Does not work

	/*
	CHashTableChannels * pHashTableChannels = new CHashTableChannels;
	pHashTableChannels->PFindChannelOrAllocate((PSCUC)"#bitcoin", NULL);
	*/
	}

void
DDialogChannels::SL_ButtonOK()
	{
	CStr strChannelName;
	QTreeWidgetItemIterator oIterator(m_pwTreeRecommendations);
	while (TRUE)
		{
		CTreeWidgetItem * pRecommendation = (CTreeWidgetItem *)*oIterator++;
		if (pRecommendation == NULL)
			break;
		if (pRecommendation->checkState(0) == Qt::Checked)
			{
			strChannelName = pRecommendation->text(0);
			m_pProfile->GroupChannel_PCreateAndSelectWithinNavigationTree_NZ(strChannelName);
			}
		}
	SL_ButtonOK_clicked();	// Close the dialog
	}

void
DDialogChannels::SL_ButtonNew()
	{
	if (DisplayDialogChannelNewFu(m_pProfile))
		SL_ButtonOK_clicked();	// If a new channel was created, close the the channels dialog
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogChannelNew::DDialogChannelNew(TProfile * pProfile) : DDialogOkCancelWithLayouts("Create a Channel", eMenuIcon_HashtagBlack)
	{
	Assert(pProfile != NULL);
	m_pProfile = pProfile;
	#if 0
	setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
	#else
	//setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	//setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
	#endif

	setStyleSheet(
		"QDialog {"
		"background-color: white;"
		//"border: 2px solid grey;  border-radius: 5px;"
		"}"
		"QPushButton {"
		"	color: white;"
		"	background-color: #2A80B9;"
		"	border-color: #2A80B9;"
		"	border-style: solid;"
		"	border-width: 2px;"
		"	border-radius: 5px;"
		"	font: bold 14px;"
		"	padding: 6 12 6 12;"
		" }"
		"QPushButton:hover { background-color: #439FE0; }"
		);

	//m_poLayoutDialog->setContentsMargins(0, 0, 0, 0);
	//m_poLayoutBody->Layout_PwAddRowLabel("Create a Channel")->setStyleSheet("background-color: #F2F2F5; color: #555459; font: bold 20px; padding: 5px");
	OLayoutForm * poLayout = new OLayoutForm(m_poLayoutBody);
	//poLayout->setContentsMargins(5, 5, 5, 5);
	poLayout->Layout_PwAddRowLabelFormat_VE_Gsb("This will create a new public channel that anyone on your team can join.")->setStyleSheet("color: #555459; font-weight: bold;");
	m_pwEditChannelName = new WEdit;
	poLayout->Layout_PwAddRowLabelAndWidget("Name", PA_CHILD m_pwEditChannelName);
	poLayout->Layout_PwAddRowBlankAndLabelDescription("Names must be lower case and cannot contain spaces or periods.");
	m_pwEditChannelPurpose = new WEditTextArea(5);
	poLayout->Layout_PwAddRowLabelAndWidget("Purpose", PA_CHILD m_pwEditChannelPurpose);
	poLayout->Layout_PwAddRowBlankAndLabelDescription("Give your channel a purpose that describes what it will be used for.");
	poLayout->Layout_PwAddRowBlankAndLabelDescription("");
	OLayoutHorizontalAlignRight * poLayoutButtons = new OLayoutHorizontalAlignRight(m_poLayoutBody);
	QPushButton * pwButtonCancel = new QPushButton("Cancel");
	pwButtonCancel->setStyleSheet("color: #9E9EA6; border-color: #9E9EA6; background-color: white;");
	poLayoutButtons->addWidget(pwButtonCancel);
	QPushButton * pwButtonOK = new QPushButton("Create Channel");
	pwButtonOK->setDefault(true);
	poLayoutButtons->addWidget(pwButtonOK);
	connect(pwButtonOK, SIGNAL(clicked()), this, SLOT(SL_ButtonOK()));
	connect(pwButtonCancel, SIGNAL(clicked()), this, SLOT(SL_ButtonCancel()));
	}

void
DDialogChannelNew::SL_ButtonOK()
	{
	CStr strChannelName = *m_pwEditChannelName;
	if (!strChannelName.FIsEmptyString())
		{
		TGroup * pGroupChannel = m_pProfile->GroupChannel_PCreateAndSelectWithinNavigationTree_NZ(strChannelName);
		pGroupChannel->m_strPurpose = m_pwEditChannelPurpose;
		}
	SL_ButtonOK_clicked();
	}

