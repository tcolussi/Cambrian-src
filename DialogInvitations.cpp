///////////////////////////////////////////////////////////////////////////////////////////////////
//	DialogInvitations.cpp
//
//	Dialogs to create and accept invitations.
//
//	INVITATION FORMAT
//
//		"<i
//			u='username'		// Username of the new contact
//			f='from'			// JID of the sender
//			p='port'			// Port of the server
//			h='hash of certificate of server'
//			n='note'
//		/>"
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "DialogInvitations.h"

/*
#define d_cxWidthDialogsInvitation	600
DDialogInvitationCreate::DDialogInvitationCreate(TAccountXmpp * pAccount) : DDialogOkCancelWithLayouts("Create Invitation", eMenuAction_ContactInvite)
	{
	Assert(pAccount != NULL);
	m_pAccount = pAccount;
	m_pwEditUsername = new WEditUsername;
//	m_pwEditNote = new WEdit;
	m_pwEditInvitationBase64 = new WEditTextAreaReadOnly;
	//m_pwEditInvitationBase64->setMaximumHeight(100);
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_ButtonCreate(OK), "Copy|Copy the invitation to the clipboard");
	DialogBody_AddRowWidget_PA("Username:|Enter the name of the person you wish to invite.\n\nExample: Jon", m_pwEditUsername);	// TODO: Add 'Secret Code'
//	DialogBody_AddRowWidget_PA("Note:|Enter a comment for the person you wish to invite.\n\nFor instance, you may wish to give additional instructions and/or introduce yourself", m_pwEditNote);
//	DialogBody_AddRowWidget_PA("Server:|Leave this field as is, unless you wish to invite someone to register his/her account on a different server than you are currently using.", m_pwEditServerName);

	Dialog_AddRowWidget_PA("Invitation to Send:|Copy and paste this invitation and send it to the person you wish to invite.\n\nYou may use email or other messaging system to send your invitation.", m_pwEditInvitationBase64);
	m_pwLabelInvitationXml = m_poLayoutDialog->Layout_PwAddRowLabel("|The invitation above is the content of this XML encoded in base64.");
	m_pwLabelInvitationXml->Label_SetTextColorGray();
	//m_poLayoutDialog->Layout_AddWidgetSpacer();

//	Dialog_SetSizeFixed(400, 200);

	setMinimumWidth(d_cxWidthDialogsInvitation);
//	setFixedHeight(250);

	//setMinimumSize(350, 250);
	connect(m_pwEditUsername, SIGNAL(textChanged(QString)), this, SLOT(SL_Edit_textChanged(QString)));
//	connect(m_pwEditNote, SIGNAL(textEdited(QString)), this, SLOT(SL_Edit_textChanged(QString)));
	m_pwEditUsername->setFocus();
	}

#if 1
DDialogInvitationCreate::~DDialogInvitationCreate()
	{
	}
#endif

void
DDialogInvitationCreate::UpdateUI()
	{
	CStr strUsername = *m_pwEditUsername;
//	CStr strNote = *m_pwEditNote;
	CStr strInvitationXml;
	strInvitationXml.Format("<i u='^S' f='^S' p='$i' h='{B/}'/>", &strUsername, &m_pAccount->m_strJID, m_pAccount->m_uServerPort, m_pAccount->Certificate_PGetBinaryFingerprint());
	m_pwLabelInvitationXml->Label_SetTextPlain(strInvitationXml);
	//m_strInvitationBase64.InitFromTextEncodedInBase64(strInvitationXml);
	m_strInvitationBase64.Format("Hi $S,\nThis is an invitation to create a chat account to communicate with $S.\n\n"
	"If you do not have SocietyPro installed, please click on this download link:\n"
	d_szUrlBaseDowloadInvitation "?Invitation={S/}\n\n"
	"Otherwise, just copy & paste the invitation below into SocietyPro:\n\n" d_szInvitationPrefix "{S/}" d_szInvitationPostfix "\n", &strUsername, &m_pAccount->m_strJID, &strInvitationXml, &strInvitationXml);

	m_pwEditInvitationBase64->setPlainText(m_strInvitationBase64);
	}

void
DDialogInvitationCreate::SL_ButtonOK_clicked()
	{
	m_strInvitationBase64.CopyStringToClipboard();
	DDialogOkCancel::SL_ButtonOK_clicked();
	}

void
DDialogInvitationCreate::SL_Edit_textChanged(const QString &)
	{
	UpdateUI();
	}
*/
/*
///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogInvitationAccept::DDialogInvitationAccept() : DDialogOkCancelWithLayouts("Accept Invitation", eMenuIconXmpp)
	{
	m_pwEditInvitation = new WEditTextArea;
	m_pwEditUsername = new WEdit;
	m_pwEditPassword = new WEditPassword;
	m_pwEditFrom = new WEditReadOnly;
	m_pwEditServerName = new WEditReadOnly;
	m_pwEditServerPort = new WEditNumberReadOnly;
	m_pwEditFingerprint = new WEditFingerprint;
	m_pwEditFingerprint->setReadOnly(true);

	setMinimumWidth(d_cxWidthDialogsInvitation);
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_ButtonAccept(OK), "Create Account|Register a new account on the server");
	DialogBody_AddRowWidget_PA("Invitation Message:|Copy and paste your invitation here", m_pwEditInvitation);
	DialogBody_AddRowWidgets_PA("Username:|Pick a username to create your account.\n\nThis username was entered by the person who created your invitation, however you are free to pick any username.", m_pwEditUsername, "Password:|Pick a password to create your account", m_pwEditPassword);
	DialogBody_AddRowWidget_PA("From:|This is the person who created the invitation, and to whoom you will connect when you create your chat account", m_pwEditFrom);
	DialogBody_AddRowWidgets_PA("Server:|Server where the account will be created", m_pwEditServerName, "Port:|Port to connect to the server", m_pwEditServerPort);
	DialogBody_AddRowWidget_PA("Certificate Fingerprint:|Fingerprint of the certificate used by the server", m_pwEditFingerprint);

	// Attempt to check if there is an invitation stored in the clipboard
	m_strInvitationText = QApplication::clipboard()->text();
	UpdateUI();
	m_pwEditInvitation->Edit_SetText(m_strInvitationText);

	connect(m_pwEditInvitation, SIGNAL(textChanged()), this, SLOT(SL_EditInvitation_textChanged()));
	m_pwEditPassword->setFocus();
	} // DDialogInvitationAccept()

void
DDialogInvitationAccept::UpdateUI()
	{
	InitToZeroes(OUT &m_invitation, sizeof(m_invitation));
	m_binFingerprint.Empty();
	m_invitation.pbinFingerprint = &m_binFingerprint;
	CXmlNode * pXmlNodeRoot = NULL;
	PSZUC pszInvitationBase64 = m_strInvitationBase64.PszSetTextBetweenPrefixAndPostfix(m_strInvitationText, d_szInvitationPrefix, d_szInvitationPostfix);
	if (pszInvitationBase64 != NULL)
		{
		// Decode the invitation into XML
		CErrorMessage error;
		m_oXmlTree.m_binXmlFileData.BinAppendBinaryDataFromBase64Szv(pszInvitationBase64);
		if (m_oXmlTree.EParseFileDataToXmlNodes(INOUT_F_VALID &error) == errSuccess)
			{
			pXmlNodeRoot = m_oXmlTree.PGetRootNodeValidate("i");
			if (pXmlNodeRoot != NULL)
				{
				m_invitation.pszUsername = pXmlNodeRoot->PszuFindAttributeValue_NZ("u");
				m_invitation.pszFrom = pXmlNodeRoot->PszuFindAttributeValue_NZ("f");
				m_invitation.pszServerName = PszGetServerNameFromJid(m_invitation.pszFrom);
				m_invitation.uServerPort = pXmlNodeRoot->UFindAttributeValueDecimal_ZZR("p");
				m_binFingerprint.BinAppendBinaryDataFromBase64Szv(pXmlNodeRoot->PszuFindAttributeValue_NZ("h"));
				}
			}
		} // if

	m_pwEditUsername->Edit_SetTextU(m_invitation.pszUsername);
	m_pwEditFrom->Edit_SetTextU(m_invitation.pszFrom);
	m_pwEditServerName->Edit_SetTextU(m_invitation.pszServerName);
	m_pwEditServerPort->Edit_SetTextFromNumber(m_invitation.uServerPort);
	m_pwEditFingerprint->EditFingerprint_Set(m_binFingerprint);
	if (pXmlNodeRoot == NULL)
		m_strInvitationText.Empty();	// There is no valid invitation, so empty the text
	} // UpdateUI()

//	The user pasted a new invitation text
void
DDialogInvitationAccept::SL_EditInvitation_textChanged()
	{
	m_strInvitationText = *m_pwEditInvitation;
	UpdateUI();
	} // SL_EditInvitation_textChanged()

void
DDialogInvitationAccept::SL_ButtonOK_clicked()
	{
	accept();
	}

void
DisplayDialogInvitationAccept()
	{
	DDialogInvitationAccept dialog;
	if (dialog.FuExec())
		{
		DisplayDialogAccountNew(TRUE, &dialog.m_invitation);
		}
	}
*/
/*
///////////////////////////////////////////////////////////////////////////////////////////////////
void
DisplayDialogInvitationCreate(TAccountXmpp * pAccount)
	{
	if (pAccount == NULL)
		pAccount = (TAccountXmpp *)g_oConfiguration.m_arraypaAccountsXmpp.PvGetElementUnique_YZ();
	if (pAccount == NULL)
		return;
	DDialogInvitationCreate dialog(pAccount);
	dialog.exec();
	}
*/


///////////////////////////////////////////////////////////////////////////////////////////////////
CInvitation::CInvitation()
	{
	m_uServerPort = 0;
	}
CInvitation::~CInvitation()
	{
	}

BOOL
CInvitation::FInitFromBase64(const CStr & strInvitationBase64)
	{
	AssertValidStr(strInvitationBase64);
	Assert(m_strInvitationXml.FIsEmptyString());
	PSZUC pszInvitationXml = m_strInvitationXml.InitFromBase64Sz(strInvitationBase64);
	AssertValidStr(m_strInvitationXml);
	if (*pszInvitationXml != '\0')
		{
		CXmlTree oXmlTree;
		oXmlTree.m_binXmlFileData = m_strInvitationXml;
		if (oXmlTree.EParseFileDataToXmlNodes_ML() == errSuccess)
			{
			CXmlNode * pXmlNodeRoot = oXmlTree.PGetRootNodeValidate("i");
			if (pXmlNodeRoot != NULL)
				{
				m_strUsername = pXmlNodeRoot->PszuFindAttributeValue_NZ("u");
				PSZUC pszFromJid;
				m_strJidInvitationFrom = pszFromJid = pXmlNodeRoot->PszuFindAttributeValue_NZ("f");
				m_strServerName = PszroGetServerNameFromJid(pszFromJid);
				m_uServerPort = pXmlNodeRoot->UFindAttributeValueDecimal_ZZR("p");
				m_binFingerprintCertificate.BinAppendBinaryDataFromBase64Szv(pXmlNodeRoot->PszuFindAttributeValue_NZ("h"));	// Get the hash value of the fingerprint
				m_strInvitationBase64 = strInvitationBase64;	// Keep a copy of the original Base64 invitation
				return TRUE;
				}
			}
		}
	return FALSE;
	}

CInvitation *
Invitation_PazAllocateFromText(const CStr & strInvitationText)
	{
	PSZUC pszInvitationText = strInvitationText;	// Typecast
	CStr strInvitationBase64;						// Temporary buffer to hold the invitation
	PSZUC pszInvitationBase64 = strInvitationBase64.InitFromTextBetweenPrefixAndPostfixPsz(pszInvitationText, d_szInvitationPrefix, d_szInvitationPostfix);
	if (pszInvitationBase64 == NULL)
		{
		pszInvitationBase64 = strInvitationBase64.InitFromQueryStringPsz(pszInvitationText, "invitation");
		if (pszInvitationBase64 != NULL)
			StringReplaceAnyCharacterByCharacter(INOUT (PSZU)pszInvitationBase64, ' ', '+');	// Since the invitation is base64-encoded, any + sign is interpreted as a space, so we have to replace all spaces by the original value of a +
		else
			{
			// Check if there is a contract
			PSZUC pszContact = strInvitationBase64.InitFromQueryStringPsz(pszInvitationText, "contact");
			if (pszContact != NULL && *pszContact != '\0')
				{
				CInvitation * paInvitation = new CInvitation;
				paInvitation->m_strJidInvitationFrom = strInvitationBase64;
				return paInvitation;
				}

			}
		//MessageLog_AppendTextFormatCo(d_coBlack, "Invitation Query String = '$s'\n", pszInvitationBase64);
		}
	if (pszInvitationBase64 != NULL && *pszInvitationBase64 != '\0')
		{
		CInvitation * paInvitation = new CInvitation;
		if (paInvitation->FInitFromBase64(strInvitationBase64))
			{
			paInvitation->m_strInvitationText = strInvitationText;
			return paInvitation;
			}
		delete paInvitation;
		}
	return NULL;
	}


CInvitation *
Invitation_PazAllocateFromEdit(WEditTextArea * pwEdit)
	{
	Assert(pwEdit != NULL);
	return Invitation_PazAllocateFromText(*pwEdit);
	}

CInvitation *
Invitation_PazAllocateFromTextOfClipboard()
	{
	CStr strInvitationText;
	strInvitationText.InitFromClipboard();
	return Invitation_PazAllocateFromText(strInvitationText);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Search for an account or contact matching the JID
ITreeItem *
Configuration_PFindAccountOrContactByJID(PSZUC pszuContactJID)
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		if (pAccount->m_strJID.FCompareStringsNoCase(pszuContactJID))
			return pAccount;
		TContact * pContact = pAccount->Contact_PFindByJID(pszuContactJID, eFindContact_kfMakeVisible);
		if (pContact != NULL)
			return pContact;
		}
	return NULL;
	}

void
Invitation_NoticeDisplay(PA_DELETING CInvitation * pazInvitation)
	{
	if (pazInvitation == NULL)
		return;
	// Before allocating a notice, check if the invitation was already accepted (or if the JID is already in use)
	if (Configuration_PFindAccountOrContactByJID(pazInvitation->m_strJidInvitationFrom) != NULL)
		{
		// We already have a contact with the same JID, so delete the invitation
		MessageLog_AppendTextFormatSev(eSeverityNoise, "The invitation containing peer $S is already in the list.\n", &pazInvitation->m_strJidInvitationFrom);
		delete pazInvitation;
		Invitation_EraseFile();
		return;
		}
	NoticeListRoaming_AddNotice(PA_DELETING new WNoticeInvitationPending(pazInvitation));
	}

#define SL_WNoticeInvitation(_pfmName)		SL_INotice(_pfmName, WNoticeInvitationPending)

WNoticeInvitationPending::WNoticeInvitationPending(PA_CHILD CInvitation * paInvitation)  : INoticeWithIcon("Accept Invitation?", eMenuIcon_Question)
	{
	Assert(paInvitation != NULL);
	m_paInvitation = paInvitation;
	MessageLog_AppendTextFormatCo(d_coGreen, "constructing WNoticeInvitationPending()\n");
	NoticeMessage_SetText_VE("You have received an invitation from <b>^S</b>.  If you wish to add <b>^S</b> to your peer list, click on the <b>Accept</b> button.", &m_paInvitation->m_strJidInvitationFrom, &m_paInvitation->m_strJidInvitationFrom);
	Notice_AddButton(PA_CHILD new WButtonTextWithIcon("Accept...|Create a new peer to communicate with the person who created the invitation", eMenuIcon_AcceptOrApprove), SL_WNoticeInvitation(SL_ButtonInvitationAccept_clicked));
	Notice_AddButton(PA_CHILD new WButtonTextWithIcon("Reject|Ignore the invitation", eMenuIcon_Deny), SL_WNoticeInvitation(SL_ButtonInvitationReject_clicked));
	}

WNoticeInvitationPending::~WNoticeInvitationPending()
	{
	delete m_paInvitation;
	}

//	WNoticeInvitationPending::INotice::ETreeItemGotFocus()
//
ENoticeAction
WNoticeInvitationPending::ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus)
	{
	Assert(piTreeItemFocus != NULL);
	if (Configuration_PFindAccountOrContactByJID(m_paInvitation->m_strJidInvitationFrom) != NULL)
		return eNoticeActionDelete;	// If the contact is already added, then delete the invitation
	return eNoticeActionDisplay;
	}

void DisplayDialogAccountAcceptInvitation(CInvitation * pInvitation);

void
WNoticeInvitationPending::SL_ButtonInvitationAccept_clicked()
	{
	TAccountXmpp * pAccount = Configuration_PGetAccountSelectedOrFirstAccount();
	if (pAccount != NULL)
		pAccount->TreeItemAccount_PContactAllocateNewToNavigationTreeInvited_NZ(m_paInvitation->m_strJidInvitationFrom);
	else
		DisplayDialogAccountAcceptInvitation(m_paInvitation);
	}

void
WNoticeInvitationPending::SL_ButtonInvitationReject_clicked()
	{
	if (m_paInvitation->m_strInvitationText.FIsEmptyString())
		Invitation_EraseFile();	// The invitation does not have a text, meaning it was created from the file "Invitation.txt"
	delete this;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#define SL_WNoticeInviteContactsByEmail(_pfmName)			SL_INotice(_pfmName, WNoticeInviteContactsByEmail)
WNoticeInviteContactsByEmail::WNoticeInviteContactsByEmail(TAccountXmpp * pAccount) : INoticeWithIcon("Invite Peers?", eMenuIcon_Question)
	{
	Assert(pAccount != NULL);
	mu_sponsor.pAccount = pAccount;	// This line is necessary for the notice to delete itself if the account is deleted
	setMinimumHeight(200);

	NoticeMessage_SetText_VE("You may invite multiple peers by email");

	WButtonTextWithIcon * pwButtonSend = new WButtonTextWithIcon("Send|Send the invitation by email", eMenuIcon_MessageEnvelopeForward);
	OLayoutHorizontalAlignLeft * poLayoutEmailAddresses = m_poLayoutNotice->Layout_PoAddRowLabelsAndWidgets_VEZA("Email Addresses", new WEdit, NULL);
	poLayoutEmailAddresses->addWidget(pwButtonSend, 0, Qt::AlignBottom);
	ConnectButton(pwButtonSend, SL_WNoticeInviteContactsByEmail(SL_ButtonSend_clicked));
	CStr strInvitation;
	strInvitation.Format("Hi,\nThis is an invitation to create a chat account to communicate with $S.\n\n"
	"If you do not have SocietyPro installed, please click on this download link:\n"
	d_szUrlBaseDowloadInvitation "?Contact=^j", &pAccount->m_pProfileParent->m_strNameProfile, pAccount);
	m_pwEditInvitation = new WEditTextArea(strInvitation);
	m_poLayoutNotice->Layout_PoAddRowLabelsAndWidgets_VEZA("Suggested Invitation Text", m_pwEditInvitation, NULL);
	}

void
WNoticeInviteContactsByEmail::SL_ButtonSend_clicked()
	{
	EMessageBoxInformation("Send Email");
	}
