///////////////////////////////////////////////////////////////////////////////////////////////////
//	DialogAccountNew.cpp
//
//	Dialog to add an existing XMPP account or create a new XMPP account.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "DialogAccountNew.h"
#include "DialogInvitations.h"
#include "WNavigationTree.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogInvitationCore::DDialogInvitationCore(CInvitation * pInvitationInit, PSZAC pszCaption, EMenuAction eMenuIcon) : DDialogOkCancelWithLayouts(pszCaption, eMenuIcon)
	{
	m_pInvitationInit = pInvitationInit;
	m_paInvitationEdit = NULL;
	m_pwEditInvitation = NULL;
	m_pwLabelInvitationFrom = NULL;
	m_pwEditPassword = NULL;
	}

DDialogInvitationCore::~DDialogInvitationCore()
	{
	delete m_paInvitationEdit;
	}

void
DDialogInvitationCore::CreateEditInvitation()
	{
	Assert(m_paInvitationEdit == NULL);
	Assert(m_pwEditInvitation == NULL);
	Dialog_SetSizeFixed(600, 350);
	m_pwEditInvitation = new WEditTextArea;
	m_pwEditInvitation->Edit_SetWatermark("Paste your invitation here");
	//m_pwEditInvitation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pwEditInvitation->Edit_SetHeightRecommended(200);
	m_paInvitationEdit = Invitation_PazAllocateFromTextOfClipboard();
	if (m_paInvitationEdit != NULL)
		m_pwEditInvitation->Edit_SetText(m_paInvitationEdit->m_strInvitationText);
	DialogBody_AddRowWidget_PA("Invitation Message:|Copy and paste your invitation here", m_pwEditInvitation);
	//m_poLayoutDialog->Layout_PoAddRowLabelsAndWidgets_VEZA("Invitation Message:|Copy and paste your invitation here", m_pwEditInvitation, NULL);
	connect(m_pwEditInvitation, SIGNAL(textChanged()), this, SLOT(SL_EditInvitation_textChanged()));
	}

void
DDialogInvitationCore::CreateLabelInvitationFrom()
	{
	Assert(m_pwLabelInvitationFrom == NULL);
	m_pwLabelInvitationFrom = DialogBody_PwAddRowLabel("|Peer who invited you\n\nBy creating an acount, this peer will go to your peer list");
	UpdateLabelInvitationFrom();
	}

void
DDialogInvitationCore::UpdateLabelInvitationFrom()
	{
	CInvitation * pInvitation = PGetInvitation();
	if (pInvitation != NULL)
		{
		m_strInvitationBase64 = pInvitation->m_strInvitationBase64;
		m_strJidInvitationFrom = pInvitation->m_strJidInvitationFrom;
		}
	else
		{
		m_strJidInvitationFrom.Empty();
		}
	Assert(m_pwLabelInvitationFrom != NULL);
	if (m_pwLabelInvitationFrom == NULL)
		return;	// Just in case, to avoid a crash
	PSZAC pszaFmtTemplateLabelInvitationFrom;
	if (m_pwEditInvitation != NULL || m_pwEditPassword == NULL)
		{
		pszaFmtTemplateLabelInvitationFrom = m_strJidInvitationFrom.FIsEmptyString() ? c_szaEmpty : "Invitation from $S";
		if (m_pwEditPassword != NULL)
			m_pwLabelInvitationFrom->Label_SetTextColorGray();
		}
	else
		pszaFmtTemplateLabelInvitationFrom = "To accept your invitation from <b>^S</b>, you need an account.  Please select a username and a password to create your account.";
	m_pwLabelInvitationFrom->Label_SetTextFormat_VE_Gsb(pszaFmtTemplateLabelInvitationFrom, &m_strJidInvitationFrom);
	}

//	Return the pointer to the invitation (if any)
CInvitation *
DDialogInvitationCore::PGetInvitation() const
	{
	return (m_pInvitationInit != NULL) ? m_pInvitationInit : m_paInvitationEdit;
	}

//	The user typed (or pasted) a new invitation text
void
DDialogInvitationCore::SL_EditInvitation_textChanged()
	{
	delete m_paInvitationEdit;
	m_paInvitationEdit = Invitation_PazAllocateFromEdit(m_pwEditInvitation);	// Try to see if the new text is indeed a valid invitation
	Endorse(m_paInvitationEdit == NULL);	// The text entered in the edit control is not a valid invitation
	UpdateLabelInvitationFrom();
	}

//	Add the invitation (if any) to the account's navigation tree.
void
DDialogInvitationCore::AddContactInvitationToNavigationTree_MB(TAccountXmpp * pAccount)
	{
	Assert(pAccount != NULL);
	PSZUC pszContact = m_strJidInvitationFrom.PszuGetStringNullIfEmpty();
	if (pszContact != NULL)
		{
		// We have an invitation, so create the contact
		TContact * pContactInvitation = pAccount->Contact_PFindByJID(pszContact);
		if (pContactInvitation == NULL)
			{
			pContactInvitation = pAccount->TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(pszContact);
			#ifdef TODO
			pContactInvitation->Invitation_ReplyWithBase64(m_strInvitationBase64);
			#endif
			}
		else
			EMessageBoxInformation("The peer <b>^s</b> is already in your list", pszContact);
		pContactInvitation->TreeItemW_SelectWithinNavigationTree();		// Make the new contact has the focus and/or is visible to the user
		}
	pAccount->TreeItemW_Expand();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#define SL_DDialogInvitationContactAdd(pfmSlot)		SL_DDialog(pfmSlot, DDialogInvitationContactAdd)
DDialogInvitationContactAdd::DDialogInvitationContactAdd(CInvitation * pInvitationInit) : DDialogInvitationCore(pInvitationInit, "Add Peer From Invitation", eMenuAction_AccountAcceptInvitation)
	{
	if (pInvitationInit == NULL)
		CreateEditInvitation();
	CreateLabelInvitationFrom();
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DDialogInvitationContactAdd(SL_ButtonOK_clicked), "Add Peer", eMenuAction_ContactAdd);
	}

void
DDialogInvitationContactAdd::SL_ButtonOK_clicked()
	{
	if (!FValidateInvitationNotFromSelf_MB())
		return;
	AddContactInvitationToNavigationTree_MB(Configuration_PGetAccountSelectedOrFirstAccount());
	DDialogInvitationCore::SL_ButtonOK_clicked();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#define SL_DDialogAccountAdd(pfmSlot)	SL_DDialog(pfmSlot, DDialogAccountAdd)
DDialogAccountAdd::DDialogAccountAdd(TProfile * pProfile, EMenuAction eMenuAction, CInvitation * pInvitationInit) : DDialogInvitationCore(pInvitationInit, (eMenuAction == eMenuAction_AccountRegister) ? "Register New Account" : (eMenuAction == eMenuAction_AccountLogin) ? "Account Login" : "One-Time Sign Up", eMenuIconXmpp)
	{
	Assert(pProfile != NULL);
	m_pProfile = pProfile;
	m_uServerPort = 0;
	m_paAccount = NULL;

	QWidget * pwFocus = m_pwEditUsername = new WEdit;
	m_pwEditPassword = new WEditPassword;
	m_eMenuAction = eMenuAction;
	if (eMenuAction == eMenuAction_AccountAcceptInvitation)
		{
		if (pInvitationInit == NULL)
			{
			CreateEditInvitation();	// No invitation provided, so display a big text area so the user may paste the invitation
			if (m_paInvitationEdit != NULL)
				pwFocus = m_pwEditPassword;	// We have a valid invitation from the clipboard, so set the focus to the password
			}
		CreateLabelInvitationFrom();
		}
	/*
	Dialog_AddButtonsOkCancel_VEZ(SL_ButtonClicked(OK), SL_ButtonClicked(Cancel), new WSpacer, m_pwButtonAdvanced, NULL);
	Dialog_RenameButtonOk((eMenuAction != eMenuAction_AccountLogin) ? "Create Account|Register a new account on the server" : "Login|Login to your server", eMenuIconXmpp);
	*/
//	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DDialogAccountAdd(SL_ButtonOK_clicked), (eMenuAction != eMenuAction_AccountLogin) ? "Create Account|Register a new account on the server" : "Login|Login to your server", eMenuIconXmpp);
	DialogBody_AddRowWidgets_PA("Username:|Enter the username you wish to add the account", m_pwEditUsername, "Password:|Enter the password for your account", m_pwEditPassword);
	m_pwButtonAdvanced = PwAllocateButton("Advanced...|Advanced options to connect to the server, including certificate pinning.", SL_DDialogAccountAdd(SL_ButtonAdvanced_clicked));
	m_pwButtonAdvanced->Button_SetIcon(eMenuIconServerSecure);
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DDialogAccountAdd(SL_ButtonOK_clicked), (eMenuAction != eMenuAction_AccountLogin) ? "Create Account|Register a new account on the server" : "Login|Login to your server", eMenuIconXmpp);
	if (m_pwEditInvitation != NULL)
		{
		m_poLayoutButtons->addStretch(1);
		m_poLayoutButtons->addWidget(m_pwButtonAdvanced);
		}
	else
		DialogBody_AddRowWidget_PA(m_pwButtonAdvanced);
	m_pwEditSocketMessageLog_YZ = new WEditMessageLog;
//	m_pwEditLog->Edit_SetWatermark("Message Log");
	m_poLayoutDialog->addWidget(PA_CHILD m_pwEditSocketMessageLog_YZ);
	//setMinimumSize(400, minimumSizeHint().height());	// Make the dialog is never smaller than its minimum recommended size (SetSizeMinimum(
	Dialog_SetSizeMinimum(400);
	UpdateInvitation();
	connect(m_pwEditUsername, SIGNAL(textEdited(QString)), this, SLOT(SL_EditUsername_textChanged(QString)));
	pwFocus->setFocus();
	} // DDialogAccountAdd


DDialogAccountAdd::~DDialogAccountAdd()
	{
	if (m_paAccount != NULL)
		m_paAccount->TreeItemAccount_DeleteFromNavigationTree_MB(PA_DELETING);
	}

void
DDialogAccountAdd::UpdateInvitation()
	{
//	MessageLog_AppendTextFormatCo(d_coGray, "DDialogAccountAdd::UpdateInvitation()\n");
	m_strUsername.Empty();
	m_strServerName.Empty();
	m_binFingerprint.Empty();

	CInvitation * pInvitation = PGetInvitation();
	if (pInvitation != NULL)
		{
		m_strUsername = pInvitation->m_strUsername;
		m_strServerName = pInvitation->m_strServerName;
		m_uServerPort = pInvitation->m_uServerPort;
		m_binFingerprint = pInvitation->m_binFingerprintCertificate;
		}
	m_pwEditUsername->Edit_SetText(m_strUsername);
	if (m_strServerName.FIsEmptyString())
		m_strServerName.BinInitFromStringWithNullTerminator(d_szXmppServerDefault);
	UpdateJID();
	} // UpdateInvitation()

void
DDialogAccountAdd::UpdateJID()
	{
//	MessageLog_AppendTextFormatCo(d_coGray, "DDialogAccountAdd::UpdateJID()\n");
	CHU * pchAt = m_strUsername.PchFindCharacter('@');
	if (pchAt != NULL)
		{
		// The user entered a username containing '@' (eg: user@cambrian.org)
		m_strJID = m_strUsername;	// The JID is the username
		m_strUsername.StringTruncateAt(pchAt);
		m_strServerName.InitFromStringU(pchAt + 1);
		}
	else
		{
		// Format the JID from the username and servername
		m_strJID.Empty();
		if (!m_strUsername.FIsEmptyString() && !m_strServerName.FIsEmptyString())
			m_strJID.Format("$S@$S", &m_strUsername, &m_strServerName);
		}
	m_pwEditSocketMessageLog_YZ->clear();	// Clear any previous error
	SetCursorRestoreDefault();	// Remove the wait cursor if present
	} // UpdateJID()

void
DDialogAccountAdd::GetJID()
	{
	m_strUsername = m_pwEditUsername;
	UpdateJID();
	}

void
DDialogAccountAdd::SetFocusToUsername()
	{
	m_pwEditUsername->setFocus();
	}
void
DDialogAccountAdd::SL_EditUsername_textChanged(const QString &)
	{
	GetJID();
	}

void
DDialogAccountAdd::SL_EditInvitation_textChanged()
	{
	DDialogInvitationCore::SL_EditInvitation_textChanged();
	UpdateInvitation();
	}

// Before creating a new account, make sure the invitation (if any) is from someone else than the accounts of the user
BOOL
DDialogInvitationCore::FValidateInvitationNotFromSelf_MB()
	{
	TAccountXmpp * pAccountExisting = g_arraypAccounts.PFindAccountByJID(m_strJidInvitationFrom);
	if (pAccountExisting == NULL)
		return TRUE;
	CInvitation * pInvitation = PGetInvitation();
	Assert(pInvitation != NULL);	// If there is no invitation, then m_strJidInvitationFrom should be empty, and consequently pAccountExisting should be NULL
	EMessageBoxWarning("This invitation message is for <b>^S</b>!<br/><br/>"
		"You cannot paste your own invitation as <b>^S</b> belongs to you.", &pInvitation->m_strUsername, &m_strJidInvitationFrom);
	if (m_pwEditInvitation != NULL)
		m_pwEditInvitation->setFocus();
	return FALSE;
	}

void
DDialogAccountAdd::SL_ButtonOK_clicked()
	{
	GetJID();
	if (!FValidateInvitationNotFromSelf_MB())
		return;
	TAccountXmpp * pAccountExisting = g_arraypAccounts.PFindAccountByJID(m_strJID);
	if (pAccountExisting != NULL)
		{
		CInvitation * pInvitation = PGetInvitation();
		if (pInvitation != NULL)
			{
			// The account already exists, so just add the contact
			AddContactInvitationToNavigationTree_MB(pAccountExisting);
			close();	// Close the dialog
			return;
			}
		EMessageBoxWarning("You already have an XMPP account named <b>^S</b>.<br/><br/>Please pick a different username or server name.", &m_strJID);
		SetFocusToUsername();
		return;
		}
	if (!m_pwEditUsername->EditUsername_FGet_MB(OUT &m_strUsername))
		return;
	CStr strPassword;
	if (!m_pwEditPassword->EditPassword_FGet_MB(OUT_F_UNCH &strPassword))
		return;
	if (!m_strServerName.FValidateServerName_MB())
		{
		SL_ButtonAdvanced_clicked();	// The button "Advances" allows the editing of the server
		return;
		}
	SetCursorWait();
	#ifdef DEBUG
	MessageLog_Show();		// Display the Message Log to help debugging
	#endif

	// Create a temporary chat account and attempt to connect (if successful, this chat account will be stored in the CChatConfiguration)
	BOOL fAddAccountToNavigationTree = FALSE;
	if (m_paAccount == NULL)
		{
		m_paAccount = m_pProfile->PAllocateAccount();
		fAddAccountToNavigationTree = TRUE;
		}
	m_paAccount->m_strJIDwithResource = m_paAccount->m_strJID = m_strJID;
	m_paAccount->m_strJID.TransformContentToLowercase();				// The JID is always stored in lowercase
	/*
	if (g_arraypAccounts.PFindChatAccountByNameDisplay(m_strUsername) == NULL)
		m_paAccount->m_strNameDisplay = m_strUsername;	// Make the display name as short as possible, unless there is already a similar username
	*/
	m_paAccount->m_strUsername = m_strUsername;
	if (m_eMenuAction != eMenuAction_AccountLogin)
		m_paAccount->m_strUsername.TransformContentToLowercase();		// Since we are registering/creating an account, make sure the username is in lowercase.  This will avoid future problem where the user types his/her password in a different case causing an error
	m_paAccount->m_strPassword = strPassword;
	m_paAccount->m_strServerName = m_strServerName;
	if (m_uServerPort == 0)
		m_uServerPort = d_uPortXmppDefault;
	m_paAccount->m_uServerPort = m_uServerPort;
	m_paAccount->Certificate_PinFingerprint(m_binFingerprint);
	if (fAddAccountToNavigationTree)
		{
		(void)m_paAccount->TreeItemAccount_DisplayWithinNavigationTree();	// Add the new account to the GUI.  This is important to do it before connecting to the socket because the socket will determine which icon to display if the account is valid
		m_paAccount->TreeItemW_SelectWithinNavigationTree();
		}
	else
		{
		m_paAccount->TreeItemW_UpdateText();  // Update the display text
		}
	m_paAccount->Socket_ConnectUI(this, TRUE);
	} // SL_ButtonOK_clicked()
/*
void
TContact::Invitation_InitFromXml(const CStr & strInvitationXml)
	{
	AssertValidStr(strInvitationXml);
	m_uFlagsContact |= FC_kfCommentContainsInvitation;
	m_strComment.InitFromTextEncodedInBase64(strInvitationXml);
	}
*/
#ifdef TODO	// We need to revise the invitation process with the new event/tasks interface
void
TContact::Invitation_ReplyWithBase64(const CStr & strInvitationBase64)
	{
	AssertValidStr(strInvitationBase64);
	m_uFlagsContact = (m_uFlagsContact & ~FC_kfContactNeedsInvitation) | FC_kfCommentContainsInvitation;
	m_strComment = strInvitationBase64;	// I am not sure if this line is necessary
	XmppMessageDispatchToServer(strInvitationBase64);	// Send the invitation back to the contact who sent the invitation (this is a form of handshake)
	TreeItemContact_UpdateIcon();
	}
#endif
/*
//	This method is being called by the object CSocketXmpp when either condition:
//		1. The login was successful (the user provided the correct username and password).
//		2. The new account was successfully created on the server.
void
DDialogAccountAdd::DoActionAddAccountNavigationTree()
	{
	Assert(m_paAccount != NULL);

	m_paAccount->m_pProfileParent->m_arraypaAccountsXmpp.Add(PA_CHILD m_paAccount); // Add the account to its parent profile
	g_arraypAccounts.Add(m_paAccount);
	m_paAccount->TreeItemAccount_SetIconOnline();
	AddContactInvitationToNavigationTree_MB(m_paAccount);

	m_paAccount = PA_CAST_TO_P NULL;		// Complete the transfer of ownership (to prevent the destructor of DDialogAccountAdd to delete it)
	Configuration_Save();		// Save the configuration after adding a new account (just in case the application crashes)
//	g_oConfiguration.NavigationTree_AddCertificates();	// Just make sure all certificates are visible
	close();	// Close the dialog
	}
*/
/*
//	DDialogAccountAdd::QDialog::reject()
//	The user clicked on the Cancel button or press the Esc key.
void
DDialogAccountAdd::reject()
	{
	RemoveStatusBarDisplayFunction();
	QDialog::reject();	// This will close the dialog which will call the destructor of DDialogAccountAdd which will delete m_paAccount
	}
*/

void
DDialogAccountAdd::SocketUI_OnSuccess()
	{
	Assert(m_pSocket_YZ != NULL);
	Assert(m_paAccount != NULL);
	#if FIX_THIS
	g_pTreeItemCommunication->TreeItemWidget_EnsureVisible();	// Make sure the 'Inbox' is always visible
	#endif
	AddContactInvitationToNavigationTree_MB(m_paAccount);
	m_paAccount->Socket_DisconnectUI();
	m_paAccount = PA_CAST_TO_P NULL;		// Complete the transfer of ownership (to prevent the destructor of DDialogAccountAdd to delete it)
	Configuration_Save();		// Save the configuration after adding a new account (just in case the application crashes)
//	g_oConfiguration.NavigationTree_AddCertificates();	// Just make sure all certificates are visible
	close();	// Close the dialog
	}

void
DDialogAccountAdd::SocketUI_OnError()
	{
	SetFocusToUsername();
	}

/*
void
DDialogAccountAdd::S_StatusBarDisplayFunction(ESeverity eSeverity, PSZUC pszuTextStatusBar)
	{
	Report(s_pThis != NULL && "The DDialogAccountAdd should be visible");
	PSZAC pszErrorContext = "Unable to login to chat account";
	DDialogAccountAdd * pThis = s_pThis;
	if (pThis != NULL && s_pSocket != NULL)
		{
		if (pThis->m_eMenuAction != eMenuAction_AccountLogin)
			pszErrorContext = "Unable to register new chat account";
		CString sStatusMessage;
		WEditTextArea * pwStatus = pThis->m_pwEditLog;
		pwStatus->setTextColor(CoFromSeverity(eSeverity));
		if (eSeverity >= eSeverityErrorWarning)
			{
			pwStatus->setFontWeight(QFont::Bold);
			sStatusMessage = (PSZUC)pszErrorContext;
			}
		pwStatus->setText(sStatusMessage);
		sStatusMessage = pszuTextStatusBar;
		pwStatus->setFontWeight(QFont::Normal);
		pwStatus->append(sStatusMessage);
		pwStatus->repaint();		// Force the widget to redraw itself
		}
	else
		{
		// This is a rare case, however it is better to display the message in the StatusBar than nothing at all.
		StatusBar_DisplayFunctionDefault(eSeverity, pszuTextStatusBar);
		}
	if (eSeverity >= eSeverityErrorWarning)
		{
		SetCursorRestoreDefault();	// When there is an error, remove the wait cursor otherwise the user will be waiting for the UI to complete
		ErrorLog_AddNewMessage(pszErrorContext, pszuTextStatusBar);
		}
	MessageLog_AppendTextFormatSev(eSeverity, "DDialogAccountAdd::S_StatusBarDisplayFunction()\n$s\n", pszuTextStatusBar);
	} // S_StatusBarDisplayFunction()
*/

void
DDialogAccountAdd::SL_ButtonAdvanced_clicked()
	{
	if (m_uServerPort == 0)
		m_uServerPort = d_uPortXmppDefault;
	DDialogAccountAddAdvanced dialogAdvanced(this);
	if (dialogAdvanced.FuExec())
		{
		m_strUsername.StringTruncateAtCharacter('@');	// Make sure the Username no longer contains any server component
		m_pwEditUsername->Edit_SetText(m_strUsername);
		UpdateJID();
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

const char c_szToolTipPort[] = "<html>Enter the port to connect to the server.<br/><br/>"
"Here are some recommended values:"
"<div style='margin:-25px;'><ul>"
	"<li>5222 Default XMPP port with SSL/TLS</li>"
	"<li><nobr/>5223 Alternative XMPP port (typically always SSL/TLS)<nobr/></li>"
	"<li>443 Secure HTTP port</li>"
"</ul></div></html>";

#define SL_DDialogAccountAddAdvanced(pfmSlot)	SL_DDialog(pfmSlot, DDialogAccountAddAdvanced)
DDialogAccountAddAdvanced::DDialogAccountAddAdvanced(DDialogAccountAdd * pDialogParent) : DDialogOkCancelWithLayouts("Advanced Options", eMenuIconXmpp)
	{
	Assert(pDialogParent != NULL);
	m_pDialogParent = pDialogParent;
	m_pwEditServerName = new WEdit((PSZUC)m_pDialogParent->m_strServerName);
	m_pwEditServerPort = new WEditNumber(pDialogParent->m_uServerPort);
	m_pwEditServerPort->setToolTip(c_szToolTipPort);
	DialogBody_AddRowWidgets_PA("Server:|Enter the name or IP address of the XMPP server", m_pwEditServerName, "Server Port:|Port to connect to the server", m_pwEditServerPort);
	m_pwEditFingerprint = new WEditFingerprint(pDialogParent->m_binFingerprint, NULL);
	DialogBody_AddRowWidget_PA("Certificate Fingerprint:|"
		"Enter the fingerprint of the certificate encrypting the connection.\n\n"
		"If the entered fingerprint does not match any of the certificate's fingerprint (MD5, SHA-1, SHA-256 or SHA-512)\n"
		"the connection will fail rather establishing an unsecure connection.", m_pwEditFingerprint);
	m_pwEditFingerprint->Edit_SetWatermark("MD5, SHA-1, SHA-256 or SHA-512 of the certificate");
	Dialog_AddButtonsOkCancel(SL_DDialogAccountAddAdvanced(SL_ButtonOK_clicked));
	}

void
DDialogAccountAddAdvanced::SL_ButtonOK_clicked()
	{
	if (!m_pwEditServerName->EditServerName_FGet_MB(OUT_F_UNCH &m_pDialogParent->m_strServerName))
		return;
	m_pDialogParent->m_uServerPort = m_pwEditServerPort->text().toInt();
	if (!m_pwEditFingerprint->EditFingerprint_FGetMB(OUT_F_UNCH &m_pDialogParent->m_binFingerprint))
		return;
	DDialogOkCancelWithLayouts::SL_ButtonOK_clicked();
	}

void
DisplayDialogAccountNew(EMenuAction eMenuAction, CInvitation * pInvitation)
	{
	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();	// This is a bit of a hack, however this dialog should appear only when a profile is selected
	if (pProfile == NULL)
		return;
	if (eMenuAction == eMenuAction_AccountAcceptInvitation && !Configuration_FIsWithoutAccounts())
		{
		DDialogInvitationContactAdd dialog(pInvitation);
		dialog.FuExec();
		}
	else
		{
		DDialogAccountAdd dialog(pProfile, eMenuAction, pInvitation);
		dialog.FuExec();
		}
	}

void
DisplayDialogAccountNew(EMenuAction eMenuAction)
	{
	DisplayDialogAccountNew(eMenuAction, NULL);
	}

void
DisplayDialogAccountAcceptInvitation(CInvitation * pInvitation)
	{
	DisplayDialogAccountNew(eMenuAction_AccountAcceptInvitation, pInvitation);
	}

