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
	m_uFlagsContact = FC_kfContactNeedsInvitation | FC_kfContactRecommendationsNeverReceived | FC_kfNativeXmppOnly; // Until proven otherwise, any new contact needs an invitation and is assumed to not understand the Cambrian Protocol <xcp>
	m_plistAliases = NULL;
	m_tsOtherLastSynchronized = d_ts_zNA;
	m_tsTaskIdDownloadedLast = d_ts_zNA;
	m_paoJapiContact = NULL;
	#if 0
    m_strNymID = (PSZUC)"ContactNymID";
    m_strKeyPublic = (PSZUC)"PubKeyTest";
	#endif
	}

TContact::~TContact()
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "TContact::~TContact(0x$p) $S\n", this, &m_strJidBare);
//	NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(IN this);		// This line is necessary, so the notices are deleted before the layout, otherwise the layout will delete
	}

void
TContact::Contact_MarkForDeletion()
	{
	TreeItem_MarkForDeletion();
	if (XmppRoster_PszGetSubscription() != NULL)
		m_pAccount->Contact_RosterUnsubscribe(this);	// Remove the contact from the roster
	}

BOOL
TContact::Contact_FCanBePermenentlyDeleted() const
	{
	if (m_paoJapiContact != NULL)
		return FALSE;
	return TreeItemFlags_FuIsInvisible();
	}

//	TContact::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TContact object to respond to the interface of its parent, the TAccountXmpp
//
//	See also the identical method TGroup::PGetRuntimeInterface()
POBJECT
TContact::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItemChatLogEvents::PGetRuntimeInterface(rti, m_pAccount);
	}


//	TContact::IXmlExchange::XmlExchange()
//
//	IMPLEMENTATION NOTES
//	The Chat Log is serialized (saved) however is not read by this method.  Instead, the Chat Log
//	is read on-demand when the user clicks on the contact.
void
TContact::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ITreeItemChatLogEvents::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("JID", INOUT &m_strJidBare);
	pXmlExchanger->XmlExchangeUIntHexFlagsMasked("Flags", INOUT &m_uFlagsContact, FC_kmFlagsSerializeMask);
	pXmlExchanger->XmlExchangeTimestamp("tsSync", INOUT_F_UNCH_S &m_tsOtherLastSynchronized);
	pXmlExchanger->XmlExchangeStr("Comment", INOUT &m_strComment);
	pXmlExchanger->XmlExchangeBin("Rec", INOUT &m_binXmlRecommendations);
	pXmlExchanger->XmlExchangeStr("NymID", INOUT &m_strNymID);
	pXmlExchanger->XmlExchangeStr("KeyPublic", INOUT &m_strKeyPublic);
    pXmlExchanger->XmlExchangeStr("RoleName",&m_strRoleName);

	m_listaTasksSendReceive.XmlExchange(INOUT pXmlExchanger);

	if (!pXmlExchanger->m_fSerializing)
		{
		// We are reading the data, therefore do some adjustments
		if (m_uFlagsContact & FC_kfXospSynchronizeWhenPresenceOnline)
			m_uFlagsContact |= FC_kfXospSynchronizeOnNextXmppStanza;
		m_strJidBare.StringTruncateAtCharacter('/');	// Remove the resource from the JID. In earlier version of the chat, the serialized JID could contain the resource.  Eventually this line should go away.
		}

	#if 0
	if (m_uFlagsTreeItem != 0)
		MessageLog_AppendTextFormatCo(d_coBlack, "0x$p: TContact::XmlExchange($S) m_uFlagsTreeItem = 0x$x\n", this, &m_strJidBare, m_uFlagsTreeItem);
	//m_pAccount->DebugDumpContacts();
	#endif
	} // XmlExchange()

// Unique file name to store the message history.  This hash is computed from the JIDs of the account and the contact, so if both are deleted and later added, the chat history is preserved.
void
TContact::Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const
	{
	Assert(pHashFileNameVault != NULL);
	// Calculate the hash from the following: the bare JID of the contact + the bare JID of the account + the salt of Config.xml
	g_strScratchBufferStatusBar.Empty();
	g_strScratchBufferStatusBar.BinAppendCBinLowercaseAscii(m_strJidBare);
	g_strScratchBufferStatusBar.BinAppendCBinLowercaseAscii(m_pAccount->m_strJID);
	g_strScratchBufferStatusBar.BinAppendBinaryData(m_pAccount->PGetSaltOfConfiguration(), CChatConfiguration::c_cbSalt);
	HashSha1_CalculateFromCBin(OUT pHashFileNameVault, IN g_strScratchBufferStatusBar);
	}

const QBrush &
TContact::ChatLog_OGetBrushForNewMessageReceived()
	{
	m_uFlagsTreeItem ^= FTI_kfChatLog_BrushColor;	// Alternate the color for every message received
	return (m_uFlagsTreeItem & FTI_kfChatLog_BrushColor) ? c_brushGreenSuperPale : c_brushGreenSuperSuperPale;
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

EMenuIcon
TContact::Contact_EGetMenuIconPresence() const
	{
	if (g_fIsConnectedToInternet)
		{
		const UINT uFlagsPresence = (m_uFlagsContact & FC_kmPresenceMaskOnline);
		if (uFlagsPresence)
			{
			if (uFlagsPresence == FC_kePresenceChat)
				return eMenuIcon_PresenceAccountOnline;
			else if (uFlagsPresence == FC_kePresenceAway)
				return eMenuIcon_PresenceAway;
			else if (uFlagsPresence == FC_kePresenceAwayExtended)
				return eMenuIcon_PresenceAwayExtended;
			else if (uFlagsPresence == FC_kePresenceBusy)
				return eMenuIcon_PresenceBusy;
			}
		}
	return eMenuIcon_PresenceAccountOffline;
	}

//	TContact::ITreeItem::TreeItem_IconUpdate()
void
TContact::TreeItem_IconUpdate()
	{
	QRGB coText = d_coTreeItem_Default;
	EMenuIcon eMenuIconPresence = eMenuIcon_PresenceAccountOffline;
	EMenuIcon eMenuIconDisplay = eMenuIcon_Contact;
	/*
	if (m_uFlagsContact & FC_kfContactNeedsInvitation)
		eMenuIconDisplay = eMenuAction_Contact;
	*/
	if (m_uFlagsContact & FC_kmRosterSubscriptionBoth)
		eMenuIconDisplay = eMenuIcon_PresenceAccountOffline;	// If there is a form of subscription, display the icon offline
	if (g_fIsConnectedToInternet)
		{
		const UINT uFlagsPresence = (m_uFlagsContact & FC_kmPresenceMaskOnline);
		if (uFlagsPresence)
			{
			if (uFlagsPresence == FC_kePresenceChat)
				eMenuIconPresence = eMenuIcon_PresenceAccountOnline;
			else if (uFlagsPresence == FC_kePresenceAway)
				eMenuIconPresence = eMenuIcon_PresenceAway;
			else if (uFlagsPresence == FC_kePresenceAwayExtended)
				eMenuIconPresence = eMenuIcon_PresenceAwayExtended;
			else if (uFlagsPresence == FC_kePresenceBusy)
				eMenuIconPresence = eMenuIcon_PresenceBusy;
			eMenuIconDisplay = eMenuIconPresence;
			}
		}
	if (m_cMessagesUnread > 0)
		{
		coText = d_coTreeItem_UnreadMessages;
		eMenuIconDisplay = eMenuIcon_MessageNew;
		}
	/*
	if (m_uFlagsContact & FC_kfContactUnsolicited)
		coText = d_coGray;
	*/

	// Update the icon for every alias
	IContactAlias * pAlias = m_plistAliases;
	while (pAlias != NULL)
		{
		pAlias->ContactAlias_IconChanged(eMenuIconDisplay, eMenuIconPresence);
		pAlias = pAlias->m_pNextAlias;
		}

	if ((m_uFlagsTreeItem & FTI_kmIconMask) == FTI_keIcon_mComposingText)
		eMenuIconDisplay = eMenuIcon_Pencil_10x10;
	TreeItemW_SetTextColorAndIcon(coText, eMenuIconDisplay);

	Dashboard_RedrawContact(this);
	} // TreeItem_IconUpdate()

const EMenuActionByte c_rgzeActionsMenuContact[] =
	{
	eMenuAction_ContactSendFile,
	eMenuActionSeparator,
	eMenuAction_ContactRename,
	eMenuAction_ContactRemove,
	eMenuAction_ContactInvite,
	eMenuAction_ContactPing,
	eMenuAction_SynchronizeWithContact,
	eMenuActionSeparator,
	eMenuAction_TreeItemRecommended,
	eMenuAction_ContactProperties,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuContactDeleted[] =
	{
	eMenuAction_ContactUndelete,
	eMenuAction_ContactRemove,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuContactView[] =
	{
	eMenuAction_Contact_SubMenuView_Recommendations,
	eMenuAction_Contact_SubMenuView_Transactions,
	ezMenuActionNone
	};

//	TContact::ITreeItem::TreeItem_MenuAppendActions()
void
TContact::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	if (!TreeItemFlags_FCanDisplayWithinNavigationTree())
		{
		pMenu->ActionsAdd(c_rgzeActionsMenuContactDeleted);
		return;
		}
	if (m_uFlagsContact & FC_kfContactUnsolicited)
		pMenu->ActionAdd(eMenuAction_ContactApprove);
	//pMenu->ActionAdd((m_uFlagsContact & mskfRosterSubscribed) ? eMenuAction_ContactUnsubscribe : eMenuAction_ContactSubscribe);
	/*
	pMenu->ActionAdd(eMenuAction_ContactSubscribe);
	pMenu->ActionAdd(eMenuAction_ContactUnsubscribe);
	*/

	WMenu * pMenuView = pMenu->PMenuAdd("View", eMenuIcon_Find);
	pMenuView->ActionsAdd(c_rgzeActionsMenuContactView);

	WMenu * pMenuGroup = pMenu->PMenuAdd("Add Peer to Group", eMenuIcon_ContactAdd);
	int eMenuActionGroup = eMenuSpecialAction_GroupFirst;
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		if (pGroup->TreeItemGroup_FCanDisplayWithinNavigationTree())
			pMenuGroup->ActionAddFromText(pGroup->TreeItem_PszGetNameDisplay(), eMenuActionGroup++, eMenuIcon_Group);
		}
	pMenuGroup->ActionAddFromText((PSZUC)"<New Group...>", eMenuAction_GroupNew, eMenuIcon_GroupAdd);

	pMenu->ActionsAdd(c_rgzeActionsMenuContact);
	pMenu->ActionSetCheck(eMenuAction_TreeItemRecommended, m_uFlagsTreeItem & FTI_kfRecommended);
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
	case eMenuAction_ContactUndelete:
		TreeItemW_RemoveFromNavigationTree();			// Remove the contact from the 'deleted items'
		TreeItemContact_DisplayWithinNavigationTreeAndClearInvisibleFlag();	// Display the contact with the other contacts
		TreeItemW_SelectWithinNavigationTreeExpanded();
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
	case eMenuAction_ContactProperties:
		DisplayDialogProperties();
		return ezMenuActionNone;
	case eMenuAction_Contact_SubMenuView_Transactions:
		TreeItemW_DisplayTransactionsBitcoin();
		return ezMenuActionNone;
	case eMenuAction_Contact_SubMenuView_Recommendations:
		XcpApi_Invoke_RecommendationsGet();
		if (!m_binXmlRecommendations.FIsEmptyBinary())
			Contact_RecommendationsDisplayWithinNavigationTree(TRUE);
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
		pAlias->TreeItemW_UpdateText();
		if (pAlias->EGetRuntimeClass() == RTI(TGroupMember))
			{
			TGroupMember * pMember = (TGroupMember *)pAlias;
			pMember->m_pGroup->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEvents_RepopulateAll;	// Refresh the Chat Log of each group where the contact participates, so the new name is updated there.
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
	EAnswer eAnswer = EMessageBoxQuestion("Are you sure you want to remove peer $S?", &m_strJidBare);
	if (eAnswer != eAnswerYes)
		return;
	Vault_WriteEventsToDiskIfModified();	// Always save the Chat Log. This is important because if the user wants to add the contact again, he/she will recover the entire Chat Log.
	m_pAccount->m_pProfileParent->DeleteContact(PA_DELETING this);
	Configuration_Save();		// Save the configuration to make sure the contact does not re-appear in case of a power failure or if the application crashes
	}

void
TContact::TreeItemContact_DisplayWithinNavigationTree()
	{
	Assert(m_pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	#if 0
	if (m_uFlagsTreeItem != 0)
		MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "0x$p: TreeItemContact_DisplayWithinNavigationTree($S) - m_uFlagsTreeItem = 0x$x\n", this, &m_strNameDisplayTyped, m_uFlagsTreeItem);
	#endif
	if (m_uFlagsTreeItem & FTI_kfObjectInvisible)
		return;	// Don't display hidden contacts
	TreeItemW_DisplayWithinNavigationTree(m_pAccount);
	TreeItemChatLog_UpdateTextAndIcon();

	if (!m_binXmlRecommendations.FIsEmptyBinary())
		Contact_RecommendationsDisplayWithinNavigationTree();
	}

void
TContact::TreeItemContact_DisplayWithinNavigationTreeAndClearInvisibleFlag()
	{
	Assert(TreeItemFlags_FuIsInvisible());
	m_uFlagsTreeItem &= ~FTI_kfObjectInvisible;	// Remove the hidden flag
	TreeItemContact_DisplayWithinNavigationTree();
	}

void
ITreeItemChatLogEvents::TreeItemChatLog_UpdateTextToDisplayMessagesUnread()
	{
	TreeItemW_SetTextToDisplayMessagesUnread(m_cMessagesUnread);
	}

void
ITreeItemChatLogEvents::TreeItemChatLog_UpdateTextAndIcon()
	{
	TreeItemChatLog_UpdateTextToDisplayMessagesUnread();
	TreeItem_IconUpdate();
	}

void
TContact::TreeItemContact_UpdateIconOffline()
	{
	m_uFlagsContact = (m_uFlagsContact & ~FC_kmPresenceMaskOnlineXosp);	// Remove any presence bit
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
				m_pAccount->PGetSocket_YZ()->Socket_WriteXmlFormatted("<iq id='$p' type='set'><query xmlns='jabber:iq:roster'><item jid='^j' subscription='from' ask='subscribe'></item></query></iq>", this, this);
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
		MessageLog_AppendTextFormatSev(eSeverityNoise, "\t m_uFlagsContact changed from 0x$x to 0x$x\n", uFlagsOld, m_uFlagsContact);
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
	UINT uFlagsPresence = FC_kePresenceChat;	// If there is no presence value, assume online
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
	//m_uFlagsContact = (m_uFlagsContact & ~(FC_kmPresenceMask | FC_kfNoCambrianProtocol)) | uFlagsPresence;
	m_uFlagsContact = (m_uFlagsContact & ~(FC_kmPresenceMaskOnlineXosp)) | uFlagsPresence;
	// Check if the contact uses Cambrian
	const CXmlNode * pXmlNodeXCP = pXmlNodeStanzaPresence->PFindElement(c_sza_xcp);
	if (pXmlNodeXCP != NULL)
		{
		// The contact is supporting XOSP
		m_uFlagsContact = (m_uFlagsContact & ~FC_kfNativeXmppOnly) | FC_kfPresenceXosp;	// Remember the XOSP is present, and remove the XMPP only flag
		if (m_uFlagsContact & FC_kfXospSynchronizeWhenPresenceOnline)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "XmppPresenceUpdateIcon($S) - FC_kfXospSynchronizeWhenPresenceOnline\n", &m_strJidBare);
			XcpApi_Invoke_Synchronize();
			}
		if (m_uFlagsContact & FC_kfContactRecommendationsNeverReceived)
			XcpApi_Invoke_RecommendationsGet();
		//m_listaTasksSendReceive.SentTasksToContact(this);	// Send any pending task
		}
	TreeItemContact_UpdateIcon();
	} // XmppPresenceUpdateIcon()


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

//	IContactAlias::IRuntimeObject::PGetRuntimeInterface()
POBJECT
IContactAlias::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	/*
	switch (rti)
		{
	case RTI(TContact):
		return m_pContact;
	case RTI(TProfile):
		return m_pContact->m_pAccount->m_pProfileParent;
	case RTI(TAccountXmpp):
		return m_pContact->m_pAccount;
	default:
		return ITreeItem::PGetRuntimeInterface(rti);
		} // switch
	*/
	return ITreeItem::PGetRuntimeInterface(rti, m_pContact);
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
IContactAlias::ContactAlias_IconChanged(EMenuIcon eMenuIconDisplay, EMenuIcon eMenuIconPresence)
	{
	Assert(eMenuIconDisplay != eMenuIcon_zNull);
	Assert(eMenuIconPresence <= eMenuIcon_PresenceLast);
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
		if (!pAlias->m_pContact->TreeItemFlags_FuIsDeleted())
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

//	Compare the display names of two contacts.
//
//	INTERFACE NOTES
//	This static method must have an interface compatible with PFn_NCompareSortElements().
NCompareResult
TContact::S_NCompareSortContactsByNameDisplay(TContact * pContactA, TContact * pContactB, LPARAM lParamCompareSort)
	{
	Assert(lParamCompareSort == d_zNA);
	return NCompareSortStringAsciiNoCase((PSZAC)pContactA->TreeItem_PszGetNameDisplay(), (PSZAC)pContactB->TreeItem_PszGetNameDisplay());	// At the moment sort by Ascii (it shoul be sorted by UTF-8)
	}

//	Sort the array of contacts by display name
void
CArrayPtrContacts::SortByNameDisplay()
	{
	Sort((PFn_NCompareSortElements)TContact::S_NCompareSortContactsByNameDisplay);
	}

PSZUC
CArrayPtrContacts::PszFormatDisplayNames(OUT CStr * pstrScratchBuffer) const
	{
	pstrScratchBuffer->Empty();
	TContact ** ppContactStop;
	TContact ** ppContact = PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		if (!pstrScratchBuffer->FIsEmptyBinary())
			pstrScratchBuffer->BinAppendText(", ");
		pstrScratchBuffer->BinAppendText((PSZAC)pContact->TreeItem_PszGetNameDisplay());
		}
	return pstrScratchBuffer->BinAppendNullTerminatorSz();
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
		pContact->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEvents_RepopulateAll;
		pContact->ChatLog_ResetNickname();
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TContactNew::TContactNew(TAccountXmpp * pAccount)
	{
	m_pAccount = pAccount;
	TreeItemW_DisplayWithinNavigationTree(m_pAccount);
	TreeItemW_SetTextColorAndIcon(d_coGray, eMenuIcon_ContactAdd);
	}

//	TContactNew::IRuntimeObject::PGetRuntimeInterface()
POBJECT
TContactNew::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	/*
	if (rti == RTI(TAccountXmpp))
		return m_pAccount;
	return ITreeItem::PGetRuntimeInterface(rti);
	*/
	return ITreeItem::PGetRuntimeInterface(rti, m_pAccount);
	}

//	TContactNew::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TContactNew::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return (PSZUC)"<New Peer...>";
	}

