#include "DialogContactNew.h"

#define d_chSeparatorContacts	';'		// Use the semicolon to separate the contacts (this would be nice, however spaces are not allowed in a JID)

void
DisplayDialogContactNew()
	{
	TAccountXmpp * pAccount = Configuration_PGetAccountSelectedOrFirstAccount();
	if (pAccount == NULL)
		return;
//	pAccount->Contacts_RosterDisplayDebug("DisplayDialogContactNew() before");
	DialogContactNew dlg(pAccount);
	dlg.exec();
	}

#define	SL_DialogContactNew(_pfmName)		SL_DDialog(_pfmName, DialogContactNew)

DialogContactNew::DialogContactNew(TAccountXmpp * pAccount) : DDialogOkCancelWithLayouts("Add New Contact", eMenuAction_ContactAdd)
	{
	Assert(pAccount != NULL);
	m_pAccount = pAccount;

	DialogBody_PwAddRowLabel((PSZAC)g_strScratchBufferStatusBar.Format("Adding a new contact to your account <b>^S</b>", &pAccount->m_strJID))->setWordWrap(false);
	m_pwEditUsername = new WEdit;
	m_pwEditUsername->setPlaceholderText("Enter multiple contact usernames separated with a space");
	DialogBody_AddRowWidget_PA("Contact Username:|Enter the usernames of the contacts you wish to add to your list.\n\nTo add multiple contacts at once, separate each username with a space", m_pwEditUsername);
	m_pwEditHtmlUsernames = new WEditTextAreaReadOnlyGray;
	m_pwEditHtmlUsernames->Edit_SetWatermark("New Contacts");
	DialogBody_AddRowWidget_PA(m_pwEditHtmlUsernames);
	connect(m_pwEditUsername, SIGNAL(textChanged(QString)), this, SLOT(SL_EditUsername_textChanged(QString)));
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DialogContactNew(SL_ButtonOK_clicked), "Add Contact", eMenuAction_ContactAdd);
	m_pwEditUsername->setFocus();
	}

DialogContactNew::~DialogContactNew()
	{
	}

void
DialogContactNew::SL_EditUsername_textChanged(const QString & sUsernames)
	{
	m_arraypaszContacts.DeleteAllStrings();	// Delete any previous strings
	CStr strUsernamesHtml;
	CStr strUsernameJID;

	// Parse the text from sUsernames to determine if there are several of of them
	CStr strUsernames(sUsernames);
	CHU * pchu = strUsernames.PbGetData();
	if (pchu != NULL)
		{
		PSZUC pszUsername = pchu;
		while (TRUE)
			{
			UINT ch = *pchu++;
			if (Ch_FIsWhiteSpaceOrNullTerminator(ch))
				{
				pchu[-1] = '\0';	// Insert a null-terminator instead of the whitespace
				if (pszUsername[0] != '\0')
					{
					if (strchrU(pszUsername, '@') == NULL)
						{
						// The username does not have a server name, so use same server name as its parent account
						pszUsername = strUsernameJID.Format("$s@$S", pszUsername, &m_pAccount->m_strServerName);
						}
					m_arraypaszContacts.AddStringAllocate(pszUsername);
					QRGB coUsername = d_coBlack;
					if (m_pAccount->Contact_PFindByJID(pszUsername) != NULL)
						{
						// There is already a contact sharing the same username
						coUsername = d_coGrayLight;
						}
					if (m_pAccount->m_strJID.FCompareStringsNoCase(pszUsername))
						{
						// The contact JID is the same as the account
						coUsername = d_coRed;
						}
					strUsernamesHtml.BinAppendTextSzv_VE("<font color='$c'>$s</font><br/>", coUsername, pszUsername);
					}
				if (ch == '\0')
					break;	// We are done
				pszUsername = pchu;
				}
			} // while
		} // if
	m_pwEditHtmlUsernames->setHtml(strUsernamesHtml);
	} // SL_EditUsername_textChanged()

void
DialogContactNew::SL_ButtonOK_clicked()
	{
//	m_pAccount->Contacts_RosterDisplayDebug("DialogContactNew::SL_ButtonOK_clicked() begin");
	TContact * pContactSelect = NULL;
	TContact * pContactDuplicate = NULL;
	CStr strContactsDuplicate;
	// The user clicked on the OK button, so add the contacts to the account
	PSZUC * ppszContactStop;
	PSZUC * ppszContact = m_arraypaszContacts.PrgpszGetStringsStop(OUT &ppszContactStop);
	while (ppszContact != ppszContactStop)
		{
		PSZUC pszContact = *ppszContact++;
		Assert(pszContact != NULL);
		Assert(pszContact[0] != '\0');
		if (m_pAccount->m_strJID.FCompareStringsJIDs(pszContact))
			continue;	// Skip the contact, since its JID is the same as its parent account JID
		TContact * pContact = m_pAccount->Contact_PFindByJID(pszContact);
		if (pContact == NULL)
			{
			// This is a new JID which is not already in the contact list
			pContactSelect = m_pAccount->TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(IN pszContact);
			}
		else
			{
			// The JID is already in the contact list
			pContactDuplicate = pContact;
			strContactsDuplicate.AppendSeparatorAndTextU("\n", pszContact);
			}
		} // while
	if (!strContactsDuplicate.FIsEmptyString())
		EMessageBoxInformation("The following contacts have been skipped because they are already present in the account $S:\n\n$S", &m_pAccount->m_strJID, &strContactsDuplicate);
	if (pContactSelect == NULL)
		{
		// No contact were added, so select one of the duplicate contact.  This dialog "Add Contacts" can be used to search for a contact.
		pContactSelect = pContactDuplicate;
		}
	if (pContactSelect != NULL)
		pContactSelect->TreeItem_SelectWithinNavigationTree();
	DDialogOkCancelWithLayouts::SL_ButtonOK_clicked();
//	m_pAccount->Contacts_RosterDisplayDebug("DialogContactNew::SL_ButtonOK_clicked() end");
	} // SL_ButtonOK_clicked()


///////////////////////////////////////////////////////////////////////////////////////////////////
CInvitationFromUrlQueryString::CInvitationFromUrlQueryString(PSZUC pszJID) : m_strJID(pszJID)
	{
	}

//	This function is the reverse of ChatLog_PszGetInvitationLink()
CInvitationFromUrlQueryString *
Invitation_PazAllocateFromUrlQueryString(PSZUC pszUrl)
	{
	PSZUC pszContact = g_strScratchBufferStatusBar.InitFromQueryStringPsz(pszUrl, "c");
	if (pszContact != NULL && *pszContact != '\0')
		{
		CInvitationFromUrlQueryString * paInvitation = new CInvitationFromUrlQueryString(pszContact);
		paInvitation->m_strNameDisplay.InitFromQueryStringPsz(pszUrl, "n");
		return paInvitation;
		}
	return NULL;
	}

PSZUC
TAccountXmpp::ChatLog_PszGetInvitationLink(OUT_IGNORE CStr * pstrInvitationLink) const
	{
	return pstrInvitationLink->Format(d_szUrlBaseDowloadInvitation "?c={S%}&n={S%}", m_strServerName.FCompareStringsNoCase(c_szXmppServerDefault) ? &m_strUsername : &m_strJID, &m_pProfileParent->m_strNameProfile);
	}

void
TAccountXmpp::Contact_AllocateNewFromGlobalInvitation()
	{
	if (g_paInvitation == NULL)
		return;
	g_paInvitation->m_strJID.AppendServerNameToJidIfMissing(m_strServerName);
	PSZUC pszJID = g_paInvitation->m_strJID;
	MessageLog_AppendTextFormatSev(eSeverityComment, "Adding $s ($S) from invitation...\n", pszJID, &g_paInvitation->m_strNameDisplay);
	if (Contact_PFindByJID(pszJID) == NULL)
		(void)TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(IN pszJID, g_paInvitation->m_strNameDisplay);
	g_paInvitation = NULL;
	Invitation_EraseFile();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutContactNew::WLayoutContactNew(TAccountXmpp * pAccount)
	{
	m_pAccount = pAccount;

	OLayoutVerticalAlignTop * poLayoutVertical = Splitter_PoAddGroupBoxAndLayoutVertical_VE("Add New Contacts");
	poLayoutVertical->Layout_PwAddRowLabelWrap("To add a contact that may not have Cambrian yet, please send them this personalized download link by whatever means you prefer. Once they install Cambrian you will be connected automatically. If they have Cambrian they can enter this same link in the bottom field to connect.");

	m_pwEditInvitation= new WEditReadOnly;
	m_pwEditInvitation->Edit_SetToolTip((PSZUC)"Personalized invlitation link to download Cambrian.\nPlease send this link to your friends by email or by instant messaging.");
	m_pwEditInvitation->Edit_SetTextU(pAccount->ChatLog_PszGetInvitationLink(OUT_IGNORED &g_strScratchBufferStatusBar));
	m_pwButtonCopyInvitation = new WButtonTextWithIcon("Copy|Copy the invitation link into the clipboard", eMenuAction_Copy);
	poLayoutVertical->Layout_PwAddRowLabelEditButton("Invitation Link:", m_pwEditInvitation, m_pwButtonCopyInvitation);
	connect(m_pwButtonCopyInvitation, SIGNAL(clicked()), this, SLOT(SL_CopyInvitation()));

	poLayoutVertical->Layout_PwAddRowHorizonalLine();
	poLayoutVertical->Layout_PwAddRowLabelWrap("If you have received a download invitation link or know your contact's username please enter either below to connect.");

	m_pwEditUsername = new WEdit;
	m_pwEditUsername->setPlaceholderText("Paste received invitation link, or enter multiple usernames separated with a space");
	WButtonTextWithIcon * pwButtonAddContacts = new WButtonTextWithIcon("Add|Add the new contacts", eMenuAction_ContactAdd);
	poLayoutVertical->Layout_PwAddRowLabelEditButton("New Contacts:", m_pwEditUsername, pwButtonAddContacts);

	m_pwEditHtmlUsernames = new WEditTextAreaReadOnlyGray;
	m_pwEditHtmlUsernames->hide();
	poLayoutVertical->addWidget(m_pwEditHtmlUsernames);
	connect(m_pwEditUsername, SIGNAL(textChanged(QString)), this, SLOT(SL_EditUsername_textChanged(QString)));
	connect(m_pwEditUsername, SIGNAL(returnPressed()), this, SLOT(SL_ButtonAddContacts()));
	connect(pwButtonAddContacts, SIGNAL(clicked()), this, SLOT(SL_ButtonAddContacts()));
	}

//	WLayoutContactNew::WLayout::Layout_SetFocus()
void
WLayoutContactNew::Layout_SetFocus()
	{
	m_pwEditUsername->setFocus();
	}

void
WLayoutContactNew::SL_CopyInvitation()
	{
	Clipboard_SetText(m_pwEditInvitation);
	}

void
WLayoutContactNew::SL_EditUsername_textChanged(const QString & sUsernames)
	{
	bool fHideContacts = true;
	m_arraypaszContacts.DeleteAllStrings();	// Delete any previous strings
	CStr strUsernamesHtml;
	CStr strUsernameJID;

	// Parse the text from sUsernames to determine if there are several of of them
	CStr strUsernames(sUsernames);
	CHU * pchu = strUsernames.PbGetData();
	if (pchu != NULL)
		{
		PSZUC pszUsername = pchu;
		while (TRUE)
			{
			UINT ch = *pchu++;
			if (Ch_FIsWhiteSpaceOrNullTerminator(ch))
				{
				pchu[-1] = '\0';	// Insert a null-terminator instead of the whitespace
				if (pszUsername[0] != '\0')
					{
					//	Check if there an invitation
					PSZUC pszUsernameAllocate = pszUsername;
					CInvitationFromUrlQueryString * paInvitation = Invitation_PazAllocateFromUrlQueryString(pszUsername);
					if (paInvitation != NULL)
						pszUsername = paInvitation->m_strJID;
					if (strchrU(pszUsername, '@') == NULL)
						{
						// The username does not have a server name, so use same server name as its parent account
						pszUsername = strUsernameJID.Format("$s@$S", pszUsername, &m_pAccount->m_strServerName);
						if (paInvitation == NULL)
							pszUsernameAllocate = pszUsername;
						}
					fHideContacts = false;
					QRGB coUsername = d_coBlack;
					PSZAC pszUsernameIntro = "New Contact";
					if (m_pAccount->m_strJID.FCompareStringsNoCase(pszUsername))
						{
						// The username is the same as the account JID
						coUsername = d_coRed;
						pszUsernameIntro = "Your Contact";
						pszUsernameAllocate = NULL;
						}
					else if (m_pAccount->Contact_PFindByJID(pszUsername) != NULL)
						{
						// There is already an existing contact sharing this username
						coUsername = d_coGrayLight;
						pszUsernameIntro = "Duplicate Contact";
						}
					else if (PcheValidateJID(pszUsername) != NULL)
						{
						coUsername = d_coRed;
						pszUsernameIntro = "Invalid Contact JID";
						pszUsernameAllocate = NULL;
						}
					#ifdef DEBUG_
					if (paInvitation != NULL)
						strUsernamesHtml.BinAppendTextSzv_VE("DisplayName: $S,", &paInvitation->m_strNameDisplay);
					#endif
					strUsernamesHtml.BinAppendTextSzv_VE("$s: <b><font color='$c'>^s</font></b><br/>", pszUsernameIntro, coUsername, pszUsername);
					if (pszUsernameAllocate != NULL)
						m_arraypaszContacts.AddStringAllocate(pszUsernameAllocate);
					delete paInvitation;
					} // if
				if (ch == '\0')
					break;	// We are done
				pszUsername = pchu;
				}
			} // while
		} // if
	m_pwEditHtmlUsernames->setHtml(strUsernamesHtml);
	m_pwEditHtmlUsernames->setHidden(fHideContacts);
	} // SL_EditUsername_textChanged()

void
WLayoutContactNew::SL_ButtonAddContacts()
	{
	TContact * pContactSelect = NULL;
	TContact * pContactDuplicate = NULL;
	CStr strContactsDuplicate;
	// The user clicked on the OK button, so add the contacts to the account
	PSZUC * ppszContactStop;
	PSZUC * ppszContact = m_arraypaszContacts.PrgpszGetStringsStop(OUT &ppszContactStop);
	while (ppszContact != ppszContactStop)
		{
		PSZUC pszContact = *ppszContact++;
		Assert(pszContact != NULL);
		Assert(pszContact[0] != '\0');
		PSZUC pszContactName = NULL;
		CInvitationFromUrlQueryString * paInvitation = Invitation_PazAllocateFromUrlQueryString(pszContact);
		if (paInvitation != NULL)
			{
			pszContact = paInvitation->m_strJID;
			pszContactName = paInvitation->m_strNameDisplay;
			}
		TContact * pContact = m_pAccount->Contact_PFindByJID(pszContact);
		if (pContact == NULL)
			{
			// This is a new JID which is not already in the contact list
			pContactSelect = m_pAccount->TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(IN pszContact, pszContactName);
			}
		else
			{
			// The JID is already in the contact list
			pContactDuplicate = pContact;
			strContactsDuplicate.AppendSeparatorAndTextU("\n", pszContact);
			}
		delete paInvitation;
		} // while
	if (!strContactsDuplicate.FIsEmptyString())
		EMessageBoxInformation("The following contacts have been skipped because they are already present in the account $S:\n\n$S", &m_pAccount->m_strJID, &strContactsDuplicate);
	if (pContactSelect == NULL)
		pContactSelect = pContactDuplicate;	// No contact were added, so select one of the duplicate contact (if any).  This feature may be used to find a contact.
	if (pContactSelect != NULL)
		pContactSelect->TreeItem_SelectWithinNavigationTree();
	} // SL_ButtonAddContacts()

//	TContactNew::ITreeItem::TreeItem_GotFocus()
void
TContactNew::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutContactNew(m_pAccount));
	MainWindow_SetFocusToCurrentLayout();
	}

//	TAccountXmpp::ITreeItem::TreeItem_GotFocus()
void
TAccountXmpp::TreeItem_GotFocus()
	{
	#if 0
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutAccount(this));
	#else
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutContactNew(this));
	#endif
	}
