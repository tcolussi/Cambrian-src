///////////////////////////////////////////////////////////////////////////////////////////////////
//	TContact.cpp
//
//	Class holding information regarding a contact.
//	The contact may be a user of another instant messaging, an email account, or something else.
//	Each TAccountXmpp may contain multiple contacts
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

IXmlExchange *
TContact::S_PaAllocateContact(POBJECT pAccountParent)
	{
	Assert(pAccountParent != NULL);
	return new TContact((TAccountXmpp *)pAccountParent);
	}

TContact::TContact(TAccountXmpp * pAccount) : ITreeItemChatLogEvents(pAccount)
	{
	m_uFlagsContact = FC_kfContactNeedsInvitation | FC_kfNoCambrianProtocol;	// Until proven otherwise, any new contact needs an invitation and is assumed to not understand the Cambrian Protocol <xcp>
	m_uFlagsContactSerialized = 0;
	m_plistAliases = NULL;
	}

TContact::~TContact()
	{
//	NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(IN this);		// This line is necessary, so the notices are deleted before the layout, otherwise the layout will delete
	}

CChatConfiguration *
TContact::PGetConfiguration() const
	{
	return m_pAccount->PGetConfiguration();
	}

BOOL
TContact::Contact_FuCommunicateViaXcp() const
	{
	return FALSE;
	}

//	Upload a file to the contact.
//	The file name is expected to be a local path.
void
ITreeItemChatLogEvents::XmppUploadFile(PSZUC pszFileUpload)
	{
	Assert(m_pawLayoutChatLog != NULL);
	if (pszFileUpload == NULL || pszFileUpload[0] == '\0')
		return;
	CEventFileSent * pEvent = new CEventFileSent(this, pszFileUpload);
	Vault_AddEventToVault(PA_CHILD pEvent);
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->ChatLog_EventAppend(pEvent);
	}

//	Upload multiple files.
//	This method expect files to be in the 'URL' with the prefix "file://"
void
ITreeItemChatLogEvents::XmppUploadFiles(IN_MOD_INV PSZU pszmFilesUpload)
	{
	Assert(pszmFilesUpload != NULL);
	PSZUC pszFileUpload = pszmFilesUpload;
	while (TRUE)
		{
		UINT ch = *pszmFilesUpload++;
		if (ch == '\n' || ch == '\0')
			{
			pszmFilesUpload[-1] = '\0';	// Insert a null-terminator
			CStr strFile = QUrl::fromUserInput(CString(pszFileUpload)).toLocalFile();	// This code is grossly inefficient, however necessary because Qt uses triple slashes (///) after the schema, such as: "file:///c:/folder/file.txt".  I could manually skip the extra slash, however it may break under other platrorms.
			MessageLog_AppendTextFormatCo(d_coBlack, "XmppUploadFiles() - $S\n", &strFile);
			XmppUploadFile(strFile);
			if (ch == '\0')
				return;	// We are done
			pszFileUpload = pszmFilesUpload;
			}
		}
	}

/*
PSZUC
TContact::PszGetNicknameChatWindow()
	{
	if (m_strNicknameChatWindow.FIsEmptyString())
		m_strNicknameChatWindow.AppendTextUsernameFromJidPszr(m_strJidBare);
	return m_strNicknameChatWindow;
	}
*/

QString
ITreeItemChatLogEvents::Vault_SGetPath() const
	{
	SHashSha1 hashFileNameEvents;
	Vault_GetHashFileName(OUT &hashFileNameEvents);
	return PGetConfiguration()->SGetPathOfFileName(IN &hashFileNameEvents);
	}

//	TContact::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TContact object to respond to the interface of its parent, the TAccountXmpp
//
//	See also the identical method TGroup::PGetRuntimeInterface()
void *
TContact::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	switch (rti)
		{
	case RTI(TProfile):
	case RTI(TAccountXmpp):
	case RTI(TCertificate):
	case RTI(TCertificateServerName):
		return m_pAccount->PGetRuntimeInterface(rti);
	default:
		return ITreeItemChatLogEvents::PGetRuntimeInterface(rti);
		} // switch
	} // PGetRuntimeInterface()


//	TContact::IXmlExchange::XmlExchange()
//
//	IMPLEMENTATION NOTES
//	The Chat Log is serialized (saved) however is not read by this method.  Instead, the Chat Log
//	is read on-demand when the user clicks on the contact.
void
TContact::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	if (pXmlExchanger->m_fSerializing)
		Vault_WriteEventsToDiskIfModified();		// This line is important to be first because saving the events may modify some variables which may be serialized by ITreeItemChatLogEvents::XmlExchange()
	ITreeItemChatLogEvents::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("JID", INOUT_F_UNCH_S &m_strJidBare);
	pXmlExchanger->XmlExchangeFlags("Flags", INOUT &m_uFlagsContact, FC_kmFlagsSerializeMask);
	pXmlExchanger->XmlExchangeUIntHex("F", INOUT &m_uFlagsContactSerialized);
	pXmlExchanger->XmlExchangeStr("Comment", INOUT_F_UNCH_S &m_strComment);
	m_strJidBare.StringTruncateAtCharacter('/');	// Remove the resource from the JID. In earlier version of the chat, the serialized JID could contain the resource.  Eventually this line should go away.
	} // XmlExchange()

// Unique file name to store the message history.  This hash is computed from the JIDs of the account and the contact, so if both are deleted and later added, the chat history is preserved.
void
TContact::Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const
	{
	Assert(pHashFileNameVault != NULL);
	// Calculate the hash from the following: the bare JID of the contact + the bare JID of the account + the salt of Config.xml
	g_strScratchBufferStatusBar.Empty();
	g_strScratchBufferStatusBar.BinAppendCBinLowercase(m_strJidBare);
	g_strScratchBufferStatusBar.BinAppendCBinLowercase(m_pAccount->m_strJID);
	g_strScratchBufferStatusBar.BinAppendBinaryData(m_pAccount->PGetSaltOfConfiguration(), CChatConfiguration::c_cbSalt);
	HashSha1_CalculateFromCBin(OUT pHashFileNameVault, IN g_strScratchBufferStatusBar);
	}

const QBrush &
TContact::ChatLog_OGetBrushForNewMessageReceived()
	{
	m_uFlagsTreeItem ^= FTI_kfChatLogBrushColor;	// Alternate the color for every message received
	return (m_uFlagsTreeItem & FTI_kfChatLogBrushColor) ? c_brushGreenSuperPale : c_brushGreenSuperSuperPale;
	}

//	TContact::ITreeItem::TreeItem_FContainsMatchingText()
BOOL
TContact::TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC
	{
	//MessageLog_AppendTextFormatCo(d_coYellowDark, "'$S'.FStringContains('$S')=$i\n", &m_strJID, &strText, m_strJID.FStringContains(strText));
	return (ITreeItem::TreeItem_FContainsMatchingText(pszTextSearchLowercase) || m_strJidBare.FStringContainsSubStringNoCase(pszTextSearchLowercase));
	}

//	TContact::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TContact::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return _PszGetDisplayNameOr(m_strJidBare);
	}

EMenuAction
TContact::Contact_EGetMenuActionPresence() const
	{
	if (g_fIsConnectedToInternet)
		{
		const BOOL fuInsecure = (m_uFlagsContact & FC_kfNoCambrianProtocol);
		const UINT uFlagsPresence = (m_uFlagsContact & FC_kmPresenceMask);
		if (uFlagsPresence)
			{
			if (uFlagsPresence == FC_kePresenceOnline)
				return fuInsecure ? eMenuIcon_PresenceInsecureOnline : eMenuAction_PresenceAccountOnline;
			else if (uFlagsPresence == FC_kePresenceAway)
				return eMenuAction_PresenceAway;
			else if (uFlagsPresence == FC_kePresenceAwayExtended)
				return eMenuAction_PresenceAwayExtended;
			else if (uFlagsPresence == FC_kePresenceBusy)
				return eMenuAction_PresenceBusy;
			}
		}
	return eMenuAction_PresenceAccountOffline;
	}

//	TContact::ITreeItem::TreeItem_IconUpdate()
void
TContact::TreeItem_IconUpdate()
	{
	QRGB coText = d_coTreeItem_Default;
	EMenuAction eMenuIconPresence = eMenuAction_PresenceAccountOffline;
	EMenuAction eMenuIconDisplay = eMenuAction_Contact;
	/*
	if (m_uFlagsContact & FC_kfContactNeedsInvitation)
		eMenuIconDisplay = eMenuAction_Contact;
	*/
	if (m_uFlagsContact & FC_kmRosterSubscriptionBoth)
		eMenuIconDisplay = eMenuAction_PresenceAccountOffline;	// If there is a form of subscription, display the icon offline
	if (g_fIsConnectedToInternet)
		{
		const BOOL fuInsecure = (m_uFlagsContact & FC_kfNoCambrianProtocol);
		const UINT uFlagsPresence = (m_uFlagsContact & FC_kmPresenceMask);
		if (uFlagsPresence)
			{
			if (uFlagsPresence == FC_kePresenceOnline)
				eMenuIconPresence = fuInsecure ? eMenuIcon_PresenceInsecureOnline : eMenuAction_PresenceAccountOnline;
			else if (uFlagsPresence == FC_kePresenceAway)
				eMenuIconPresence = eMenuAction_PresenceAway;
			else if (uFlagsPresence == FC_kePresenceAwayExtended)
				eMenuIconPresence = eMenuAction_PresenceAwayExtended;
			else if (uFlagsPresence == FC_kePresenceBusy)
				eMenuIconPresence = eMenuAction_PresenceBusy;
			eMenuIconDisplay = eMenuIconPresence;
			}
		}
	if (m_cMessagesUnread > 0)
		{
		coText = d_coTreeItem_UnreadMessages;
		eMenuIconDisplay = eMenuAction_MessageNew;
		}
	if (m_uFlagsContact & FC_kfContactUnsolicited)
		coText = d_coGray;
	TreeItem_SetTextColorAndIcon(coText, eMenuIconDisplay);

	// Update the icon for every alias
	IContactAlias * pAlias = m_plistAliases;
	while (pAlias != NULL)
		{
		pAlias->ContactAlias_IconChanged(eMenuIconDisplay, eMenuIconPresence);
		pAlias = pAlias->m_pNextAlias;
		}
	} // TreeItem_IconUpdate()

const EMenuActionByte c_rgzeActionsMenuContact[] =
	{
	eMenuAction_ContactSendFile,
	eMenuActionSeparator,
	eMenuAction_ContactRename,
	eMenuAction_ContactRemove,
	eMenuAction_ContactInvite,
	eMenuAction_ContactProperties,
	eMenuAction_ContactTransactions,
	eMenuAction_ContactPing,
	eMenuIconOutOfSync,
	ezMenuActionNone
	};

//	TContact::ITreeItem::TreeItem_MenuAppendActions()
void
TContact::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	if (m_uFlagsContact & FC_kfContactUnsolicited)
		pMenu->ActionAdd(eMenuAction_ContactApprove);
	//pMenu->ActionAdd((m_uFlagsContact & mskfRosterSubscribed) ? eMenuAction_ContactUnsubscribe : eMenuAction_ContactSubscribe);
	/*
	pMenu->ActionAdd(eMenuAction_ContactSubscribe);
	pMenu->ActionAdd(eMenuAction_ContactUnsubscribe);
	*/
	WMenu * pMenuGroup = pMenu->PMenuAdd("Add Contact to Group", eMenuAction_GroupAddContact);
	int eMenuActionGroup = eMenuSpecialAction_GroupFirst;
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		pMenuGroup->ActionAddFromText(pGroup->TreeItem_PszGetNameDisplay(), eMenuActionGroup++, eMenuAction_Group);
		}
	pMenuGroup->ActionAddFromText((PSZUC)"<New Group...>", eMenuAction_GroupNew, eMenuAction_GroupNew);
	pMenu->ActionsAdd(c_rgzeActionsMenuContact);
	} // TreeItem_MenuAppendActions()


//	TContact::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TContact::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	const int iGroup = eMenuAction - eMenuSpecialAction_GroupFirst;
	if (iGroup >= 0)
		{
		Contact_AddToGroup(iGroup);
		return ezMenuActionNone;
		}
	switch (eMenuAction)
		{
	case eMenuAction_GroupNew:
		Contact_AddToGroup(iGroup);	// Any out of range value will create a new group
		return ezMenuActionNone;
	case eMenuAction_ContactInvite:
		ChatLog_PwGetLayout_NZ()->WidgetContactInvitation_Show();
		//MessageLog_AppendTextFormatCo(d_coBlack, "eMenuAction_ContactInvite\n");
		return ezMenuActionNone;
	case eMenuAction_ContactSendFile:
		DisplayDialogSendFile();
		return ezMenuActionNone;
	case eMenuAction_ContactRemove:
		TreeItemContact_DeleteFromNavigationTree_MB();
		return ezMenuActionNone;
	case eMenuAction_ContactApprove:
		m_uFlagsContact &= ~FC_kfContactUnsolicited;
		TreeItemContact_UpdateIcon();		// Remove the gray color
		return ezMenuActionNone;
	case eMenuAction_ContactSubscribe:
		m_pAccount->Contact_RosterSubscribe(this);
		return ezMenuActionNone;
	case eMenuAction_ContactUnsubscribe:
		m_pAccount->Contact_RosterUnsubscribe(this);
		return ezMenuActionNone;
	case eMenuAction_ContactPing:
		Xmpp_Ping();
		return ezMenuActionNone;
	case eMenuIconOutOfSync:
		Xcp_Synchronize();
		return ezMenuActionNone;
	case eMenuAction_ContactProperties:
		DisplayDialogProperties();
		return ezMenuActionNone;
	case eMenuAction_ContactTransactions:
		TreeItem_DisplayTransactionsBitcoin();
		return ezMenuActionNone;
	case eMenuSpecialAction_ITreeItemRenamed:
		TreeItemContact_GenerateDisplayNameFromJid();
		TreeItemContact_UpdateNameDisplayOfAliases();
		goto Default;
	default:
		Default:
		return ITreeItemChatLogEvents::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()


//	Attempt to generate the best (shortest) display name from the JID, making sure no other sibling contact share the same name
void
TContact::TreeItemContact_GenerateDisplayNameFromJid()
	{
	if (!m_strNameDisplayTyped.FIsEmptyString())
		return;	// Don't generate a display name if there is already one

	// First, attempt use a display name without the digits
	PSZUC pszJid = m_strJidBare;
	PCHUC pchAt = pszJid;
	while (TRUE)
		{
		CHS ch = *pchAt;
		if (ch == '@')
			{
			BOOL fDigitsRemoved = FALSE;
			PCHUC pchTemp = pchAt;
			while (--pchTemp > pszJid)
				{
				if (Ch_FIsDigit(*pchTemp))
					fDigitsRemoved = TRUE;
				else if (fDigitsRemoved)
					{
					if (TreeItemContact_FSetDisplayNameUnique(pszJid, pchTemp + 1))
						return;
					break;
					}
				} // while
			if (TreeItemContact_FSetDisplayNameUnique(pszJid, pchAt))
				return;
			}
		else if (ch == '\0')
			break;
		pchAt++;
		} // while
	Verify(TreeItemContact_FSetDisplayNameUnique(pszJid, pchAt));	// At this point, use the full JID as the display name
	} // TreeItemContact_GenerateDisplayNameFromJid()

void
TContact::TreeItemContact_UpdateNameDisplayOfAliases()
	{
	IContactAlias * pAlias = m_plistAliases;
	while (pAlias != NULL)
		{
		pAlias->TreeItem_SetTextToDisplayNameTyped();
		if (pAlias->EGetRuntimeClass() == RTI(TGroupMember))
			{
			TGroupMember * pMember = (TGroupMember *)pAlias;
			pMember->m_pGroup->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEventsRepopulateAll;	// Refresh the Chat Log of each group where the contact participates, so the new name is updated there.
			}
		pAlias = pAlias->m_pNextAlias;
		}
	}

BOOL
TContact::TreeItemContact_FSetDisplayNameUnique(PSZUC pszBegin, PCHUC pchCopyUntil)
	{
	m_strNameDisplayTyped.InitFromStringCopiedUntilPch(pszBegin, pchCopyUntil);
	return (m_pAccount->m_arraypaContacts.PFindContactByNameDisplay(m_strNameDisplayTyped, this) == NULL);
	}


//	Add the contact to a group.  If iGroup is out of range, then create a new group
void
TContact::Contact_AddToGroup(int iGroup)
	{
	m_pAccount->Group_AddNewMember_UI(this, iGroup);
	}

void
TContact::TreeItemContact_DeleteFromNavigationTree_MB(PA_DELETING)
	{
	EAnswer eAnswer = EMessageBoxQuestion("Are you sure you want to remove the contact $S?", &m_strJidBare);
	if (eAnswer != eAnswerYes)
		return;
	Vault_WriteEventsToDiskIfModified();	// Always save the Chat Log. This is important because if the user wants to add the contact again, he/she will recover the entire Chat Log.
	m_pAccount->Contact_DeleteSafely(PA_DELETING this);
	Configuration_Save();		// Save the configuration to make sure the contact does not re-appear in case of a power failure or if the application crashes
	}

void
ITreeItemChatLogEvents::DisplayDialogSendFile()
	{
	CStr strCaption;
	strCaption.Format("Send file to $s", TreeItem_PszGetNameDisplay());
	strCaption = QFileDialog::getOpenFileName(g_pwMainWindow, IN strCaption);
	XmppUploadFile(strCaption);	// An empty filename will be ignored by XmppUploadFile()
	}

void
TContact::TreeItemContact_DisplayWithinNavigationTree()
	{
	Assert(m_pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	TreeItem_DisplayWithinNavigationTree(m_pAccount);
	TreeItemChatLog_UpdateTextAndIcon();
	}

void
ITreeItemChatLogEvents::TreeItemChatLog_UpdateTextToDisplayMessagesUnread()
	{
	TreeItem_SetTextToDisplayMessagesUnread(m_cMessagesUnread);
	}

void
ITreeItemChatLogEvents::TreeItemChatLog_UpdateTextAndIcon()
	{
	TreeItemChatLog_UpdateTextToDisplayMessagesUnread();
	TreeItem_IconUpdate();
	}

void
TContact::TreeItemContact_UpdateIconComposing()
	{
	m_pAccount->m_arraypContactsComposing.AddUniqueF(this);
	TreeItem_SetIcon(eMenuIconPencil);
	// Update the icon for every alias
	IContactAlias * pAlias = m_plistAliases;
	while (pAlias != NULL)
		{
		pAlias->TreeItem_SetIcon(eMenuIconPencil);	// This line needs to be fixed to handle special cases
		pAlias = pAlias->m_pNextAlias;
		}
	}

void
TContact::TreeItemContact_UpdateIconOffline()
	{
	m_uFlagsContact = (m_uFlagsContact & ~FC_kmPresenceMask);
	if (m_pAccount->m_arraypContactsComposing.RemoveElementFastF(this))
		ChatLogContact_ChatStateIconUpdateComposingStopped();	// The user was composing while going offline, so remove the text "x is typing..." from the Chat Log
	TreeItemContact_UpdateIcon();
	}

void
TContact::TreeItemContact_UpdateIcon()
	{
	TreeItem_IconUpdate();
	}

void
TContact::XmppRosterSubscribed()
	{
	if ((m_uFlagsContact & FC_kfSubscribed) == 0)
		{
		m_uFlagsContact |= FC_kfSubscribed;
		m_pAccount->PGetSocket_YZ()->Socket_WriteXmlFormatted("<presence to='^j' type='subscribed'/>", this);
		}
	}

void
TContact::XmppRosterSubscriptionUpdate(PSZUC pszSubscription)
	{
	Assert(pszSubscription != NULL);
	MessageLog_AppendTextFormatSev(eSeverityNoise, "TContact::XmppRosterSubscriptionUpdate($S) to '$s'\n", &m_strJidBare, pszSubscription);
	const UINT uFlagsOld = m_uFlagsContact;
	m_uFlagsContact = (m_uFlagsContact & ~FC_kmRosterSubscriptionBoth) | FC_kfRosterItem;
	if (FCompareStrings(pszSubscription, "both"))
		{
		m_uFlagsContact |= FC_kmRosterSubscriptionBoth;
		}
	else
		{
		if (FCompareStringsBeginsWith(pszSubscription, "from"))
			m_uFlagsContact |= FC_kfRosterSubscriptionFrom;
		else if (FCompareStringsBeginsWith(pszSubscription, "to"))
			m_uFlagsContact |= FC_kfRosterSubscriptionTo;
		else
			{
			Assert(FCompareStrings(pszSubscription, "none"));
			if ((m_uFlagsContact & FC_kfSubscribeAsk) == 0)
				{
				m_uFlagsContact |= FC_kfSubscribeAsk;
				m_pAccount->PGetSocket_YZ()->Socket_WriteXmlFormatted("<iq type='set'><query xmlns='jabber:iq:roster'><item jid='^j' subscription='from' ask='subscribe'></item></query></iq>", this);
				}
			}
		if ((m_uFlagsContact & FC_kfSubscribe) == 0)
			{
			m_uFlagsContact |= FC_kfSubscribe;
			m_pAccount->PGetSocket_YZ()->Socket_WriteXmlFormatted("<presence to='^j' type='subscribe'/>", this);
			}
		}
	if (m_uFlagsContact != uFlagsOld)
		{
		MessageLog_AppendTextFormatSev(eSeverityNoise, "\t Flags changed from 0x$x to 0x$x\n", uFlagsOld, m_uFlagsContact);
		TreeItemContact_UpdateIcon();
		}

	#ifdef DEBUG
	PSZUC pszSubscriptionReturned = XmppRoster_PszGetSubscription();
	if (!FCompareStrings(pszSubscriptionReturned, pszSubscription))
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "XmppRosterSubscriptionUpdate($S) - XmppRoster_PszGetSubscription() returned '$s' however expecting '$s'\n", &m_strJidBare, pszSubscriptionReturned, pszSubscription);
	#endif
	} // XmppRosterSubscriptionUpdate()

PSZUC
TContact::XmppRoster_PszGetSubscription() const
	{
	if (m_uFlagsContact & FC_kfRosterItem)
		{
		switch (m_uFlagsContact & FC_kmRosterSubscriptionBoth)
			{
		case FC_kfRosterSubscriptionFrom:
			return (PSZUC)"from";
		case FC_kfRosterSubscriptionTo:
			return (PSZUC)"to";
		case FC_kmRosterSubscriptionBoth:
			return (PSZUC)"both";
		default:
			return (PSZUC)"none";
			}
		}
	return NULL;
	}

void
TContact::XmppPresenceUpdateIcon(const CXmlNode * pXmlNodeStanzaPresence)
	{
	if (FCompareStrings(pXmlNodeStanzaPresence->PszFindAttributeValueType_NZ(), "unavailable"))
		{
		TreeItemContact_UpdateIconOffline();
		return;
		}
	UINT uFlagsPresence = FC_kePresenceOnline;	// If there is no presence value, assume online
	PSZUC pszPresence = pXmlNodeStanzaPresence->PszuFindElementValue_ZZ("show");
	if (pszPresence != NULL && !FCompareStrings(pszPresence, "chat"))
		{
		if (FCompareStrings(pszPresence, "away"))
			uFlagsPresence = FC_kePresenceAway;
		else if (FCompareStrings(pszPresence, "xa"))
			uFlagsPresence = FC_kePresenceAwayExtended;
		else if (FCompareStrings(pszPresence, "dnd"))
			uFlagsPresence = FC_kePresenceBusy;	// Do Not Disturb
		else
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unrecognized presence value '$s'\n", pszPresence);
		}
	XmppRosterSubscribed();
	if (m_uFlagsContact & FC_kfContactNeedsInvitation)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "XmppPresenceUpdateIcon($S) - Invitation no longer needed because of presence $s\n", &m_strJidBare, pszPresence);
		ChatLogContact_RemoveInvitationMessage();
		Assert((m_uFlagsContact & FC_kfContactNeedsInvitation) == 0);
		NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(this);
		}
	m_uFlagsContact = (m_uFlagsContact & ~(FC_kmPresenceMask | FC_kfNoCambrianProtocol)) | uFlagsPresence;
	// Check if the contact uses Cambrian
	const CXmlNode * pXmlNodeXCP = pXmlNodeStanzaPresence->PFindElement(c_sza_xcp);
	if (pXmlNodeXCP == NULL)
		{
		//m_uFlagsContact |= FC_kfNoCambrianProtocol;
		}
	TreeItemContact_UpdateIcon();
	} // XmppPresenceUpdateIcon()

void
TContact::Xmpp_Ping()
	{
	CEventPing * pEvent = new CEventPing(this);
	Vault_AddEventToVault(PA_CHILD pEvent);
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->ChatLog_EventAppend(pEvent);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
/*
TContact *
CArrayPtrContacts::PFindChatContactByNameDisplay(PSZUC pszContactNameDisplay) const
	{
	Assert(pszContactNameDisplay != NULL);
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		if (pContact->m_strNameDisplay.FCompareStringsNoCase(pszContactNameDisplay))
			return pContact;
		}
	return NULL;
	}

//	Return the contact matching the comment
TContact *
CArrayPtrContacts::PFindChatContactByComment(PSZUC pszComment) const
	{
	Assert(pszComment != NULL);
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		if (pContact->m_strComment.FCompareStringsExactCase(pszComment))
			return pContact;
		}
	return NULL;
	}
*/
/*
TContact *
CArrayPtrContacts::PFindChatContactByJID(PSZUC pszContactJID) const
	{
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		if (pContact->m_strJID.FCompareStringsNoCase(pszContactJID))
			return pContact;
		}
	return NULL;
	}

//	Find a chat contact from a JID with a resource, such as "user@cambrian.org/chat"
//	This method is slower than PFindChatContactByJID() because it must strip the resource before the search.
TContact *
CArrayPtrContacts::PFindChatContactByJIDwithResource(PSZUC IN_MOD_TMP pszContactJIDwithResource) const
	{
	Assert(pszContactJIDwithResource != NULL);
	// Find the resource
	CHU * pchResource = (CHU *)pszContactJIDwithResource;
	while (TRUE)
		{
		if (*pchResource == '/')
			{
			*pchResource = '\0';	// Truncate the JID to strip out the resource
			break;
			}
		if (*pchResource == '\0')
			{
			pchResource = NULL;
			break;
			}
		pchResource++;
		} // while
	TContact * pContact;
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (TRUE)
		{
		if (ppContact == ppContactStop)
			{
			pContact = NULL;
			break;
			}
		pContact = *ppContact++;
		if (pContact->m_strJID.FCompareStringsNoCase(pszContactJIDwithResource) ||
			pContact->m_strJidFull.FCompareStringsNoCase(pszContactJIDwithResource))
			break;
		} // while
	if (pchResource != NULL)
		*pchResource = '/';	// Restore the string to its original value
	return pContact;
	} // PFindChatContactByJIDwithResource()
*/

IContactAlias::IContactAlias(TContact * pContact)
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	m_pContact = pContact;
	m_pNextAlias = pContact->m_plistAliases;
	pContact->m_plistAliases = this;				// Add the alias to its parent contact
	}

IContactAlias::~IContactAlias()
	{
	// Remove the alias from its parent contact
//	TRACE2("~IContactAlias(0x$p) - Contact=$S", this, &m_pContact->m_strNameDisplay);
	IContactAlias * pAliasPrev = NULL;
	IContactAlias * pAlias = m_pContact->m_plistAliases;
	while (pAlias != NULL)
		{
		IContactAlias * pAliasNext = pAlias->m_pNextAlias;
		if (pAlias == this)
			{
			if (pAliasPrev == NULL)
				m_pContact->m_plistAliases = pAliasNext;
			else
				pAliasPrev->m_pNextAlias = pAliasNext;
			return;
			}
		pAliasPrev = pAlias;
		pAlias = pAliasNext;
		} // while
	Assert(FALSE && "Alias not found");
	}

void
IContactAlias::XmlExchangeContactAlias(INOUT CXmlExchanger * pXmlExchanger, CHS chAttributeContactAlias)
	{
	ITreeItem::XmlExchange(INOUT pXmlExchanger);
	if (pXmlExchanger->m_fSerializing)
		pXmlExchanger->XmlExchangePointer(chAttributeContactAlias, PPX &m_pContact, d_zNA);
	}

//	IContactAlias::IXmlExchange::XmlExchange()
void
IContactAlias::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	Assert(FALSE && "Use method XmlExchangeAlias() instead");
	}

//	IContactAlias::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
IContactAlias::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return m_pContact->TreeItem_PszGetNameDisplay();	// The display name of the alias is the same as its parent
	}

void
IContactAlias::ContactAlias_IconChanged(EMenuAction eMenuIconDisplay, EMenuAction eMenuIconPresence)
	{
	Assert(eMenuIconDisplay != ezMenuActionNone);
	Assert(eMenuIconPresence <=  eMenuAction_PresenceLast);
	}

//	Return the first alias matching the contact
IContactAlias *
CArrayPtrContactAliases::PFindAliasMatchingContact(TContact * pContact) const
	{
	Assert(pContact != NULL);
	IContactAlias ** ppAliasStop;
	IContactAlias ** ppAlias = PrgpGetAliasStop(OUT &ppAliasStop);
	while (ppAlias != ppAliasStop)
		{
		IContactAlias * pAlias = *ppAlias++;
		Assert(pAlias != NULL);
		Assert(pAlias->m_pContact != NULL);
		Assert(pAlias->m_pContact->EGetRuntimeClass() == RTI(TContact));
		if (pAlias->m_pContact == pContact)
			return pAlias;
		}
	return NULL;
	}

void
CArrayPtrContactAliases::DeleteAllAliasesRelatedToContactsAboutBeingDeleted()
	{
	if (m_paArrayHdr == NULL)
		return;
	IContactAlias ** ppAliasStop;
	IContactAlias ** ppAliasStart = PrgpGetAliasStop(OUT &ppAliasStop);
	IContactAlias ** ppAliasDst = ppAliasStart;
	IContactAlias ** ppAliasSrc = ppAliasStart;
	while (ppAliasSrc != ppAliasStop)
		{
		IContactAlias * pAlias = *ppAliasSrc++;
		Assert(pAlias != NULL);
		Assert(pAlias->m_pContact != NULL);
		Assert(pAlias->m_pContact->EGetRuntimeClass() == RTI(TContact));
		if ((pAlias->m_pContact->m_uFlagsTreeItem & ITreeItem::FTI_kfTreeItemAboutBeingDeleted) == 0)
			*ppAliasDst++ = pAlias;
		else
			delete pAlias;
		}
	m_paArrayHdr->cElements = ppAliasDst - ppAliasStart;
	}


TContact *
CArrayPtrContacts::PFindContactByNameDisplay(PSZUC pszContactNameDisplay, const TContact * pContactExclude) const
	{
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		if (pContact != pContactExclude)
			{
			if (pContact->m_strNameDisplayTyped.FCompareStringsNoCase(pszContactNameDisplay))
				return pContact;
			}
		} // while
	return NULL;
	}

void
CArrayPtrContacts::ForEach_ChatLogResetNickNameAndRepopulateAllEvents()
	{
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		pContact->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEventsRepopulateAll;
		pContact->ChatLog_ResetNickname();
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TContactNew::TContactNew(TAccountXmpp * pAccount)
	{
	m_pAccount = pAccount;
	TreeItem_DisplayWithinNavigationTree(m_pAccount);
	TreeItem_SetTextColorAndIcon(d_coGray, eMenuAction_ContactAdd);
	}

//	TContactNew::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TContactNew::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return (PSZUC)"<New Contact...>";
	}

