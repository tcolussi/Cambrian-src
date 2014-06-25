///////////////////////////////////////////////////////////////////////////////////////////////////
//	TAccount.cpp
//
//	Class to hold an account for communication purpose, either instant messaging, email or something else.
//	Each account maintains a list of contacts, groups, circles and marketplace.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

TAccountCore::TAccountCore(TProfile * pProfileParent)
	{
	Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
	m_pProfileParent = pProfileParent;
	m_uServerPort = 0;
	m_uFlagsAccountStatus = 0;
	}

//	TAccountCore::IXmlExchange::XmlExchange()
void
TAccountCore::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("ServerName", INOUT_F_UNCH_S &m_strServerName);
	pXmlExchanger->XmlExchangeUShort("ServerPort", INOUT_F_UNCH_S &m_uServerPort);
	pXmlExchanger->XmlExchangeUShort("Status", INOUT_F_UNCH_S &m_uFlagsAccountStatus);
	}

PSZUC
TAccountCore::PszGetPresenceStatus() const
	{
	EMenuAction eMenuAction_Presence = (EMenuAction)(m_uFlagsAccountStatus & FAS_kmStatusPresenceMask);
	if (eMenuAction_Presence == ezMenuActionNone)
		eMenuAction_Presence = (EMenuAction)(g_uPreferences & P_kmPresenceMask);
	if (eMenuAction_Presence == ezMenuActionNone)
		return NULL;	// If nothing is specified, assume "Online"
	return EnumerationMap_PszFindPresence(eMenuAction_Presence);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
IXmlExchange *
TAccountXmpp::S_PaAllocateAccount(PVOID pvContextAllocate)
	{
	Assert(pvContextAllocate != NULL);
	return new TAccountXmpp((TProfile *)pvContextAllocate);
	}


TAccountXmpp::TAccountXmpp(TProfile * pProfileParent) : TAccountCore(pProfileParent)
	{
	m_paSocket = NULL;
	m_pCertificateServerName = NULL;
	m_paAlias = new TAccountAlias(this);
	g_arraypAccounts.Add(this);	// Always add new account to the global array of accounts
	}

TAccountXmpp::~TAccountXmpp()
	{
	Assert(!g_arraypAccounts.FindElementF(this) && "The account should have been removed from the array before its deletion!");
	delete m_paAlias;
	if (m_pCertificateServerName != NULL)
		{
		Assert(m_pCertificateServerName->EGetRuntimeClass() == RTI(TCertificateServerName));
		(void)m_pCertificateServerName->m_arraypAccounts.RemoveElementFastAssertF(this);
		}
	m_arraypaGroups.DeleteAllTreeItems();		// Delete the groups first, as they have pointers to the contacts
	m_arraypaContacts.DeleteAllTreeItems();
	if (m_paSocket != NULL)
		m_paSocket->Socket_Destroy(PA_DELETING);
	}

CChatConfiguration *
TAccountXmpp::PGetConfiguration() const
	{
	return m_pProfileParent->m_pConfigurationParent;
	}

const SHashSha1 *
TAccountXmpp::PGetSaltOfConfiguration() const
	{
	return m_pProfileParent->m_pConfigurationParent->PGetSalt();
	}

//	Find the contact matching the JID and update the contact resource (if any).
//	If the JID is not in the contact list, the method will create a new contact to the list according to the value of eFindContact.
//	This method may return NULL if the JID is empty/invalid because a new contact cannot not be created.
//
//	IMPLEMENTATION NOTES
//	This method should use a hash table to quickly find a contact from its JID and/or use a pointer to cache the last contact found.
TContact *
TAccountXmpp::Contact_PFindByJID(PSZUC pszContactJID, EFindContact eFindContact)
	{
	Assert(pszContactJID != NULL);
	if (pszContactJID != NULL)
		{
		// Find the resource
		PSZUC pszResource = pszContactJID;
		while (TRUE)
			{
			if (*pszResource == '/')
				break;
			if (*pszResource == '\0')
				break;
			pszResource++;
			} // while
		const int cchJID = pszResource - pszContactJID;
		if (cchJID > 0)
			{
			TContact * pContact;
			TContact ** ppContactStop;
			TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
			while (ppContact != ppContactStop)
				{
				pContact = *ppContact++;
				//MessageLog_AppendTextFormatCo(d_coBlack, "Comparing $s with $S\n", pszContactJID, &pContact->m_strJidBare);
				if (pContact->m_strJidBare.FCompareStringsNoCaseCch(pszContactJID, cchJID))
					{
					// We have found our contact, so update the resource (if any)
					if (*pszResource != '\0')
						{
						if (!pContact->m_strRessource.FCompareStringsExactCase(pszResource))
							{
							if (pContact->m_strRessource.FIsEmptyString())
								MessageLog_AppendTextFormatCo(d_coBlack, "Contact $S: Assigning resource '$s'\n", &pContact->m_strJidBare, pszResource);
							else
								MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlack), "Contact $S: Updating resource from '$S' to '$s'\n", &pContact->m_strJidBare, &pContact->m_strRessource, pszResource);
							//pContact->Xcp_ServiceDiscovery();	// The resource changed, therefore query the remote client for its capabilities
							pContact->SetFlagXcpComposingSendTimestampsOfLastKnownEvents();	// Each time the resource change, re-send the timestamps of the last known events so we give an opportunity to synchronie
							pContact->m_cVersionXCP = 0;	// Also, reset the XCP version, to make sure the device connects properly
							}
						pContact->m_strRessource.InitFromStringU(pszResource);
						}
					return pContact;
					}
				} // while
			// We could not find the contact, so create a new one if the JID meets the minimal conditions
			if (eFindContact != eFindContactOnly &&
				PcheValidateJID(pszContactJID) == NULL &&			// Make sure the JID is somewhat valid to create a new contact
				!m_strJID.FCompareStringsJIDs(pszContactJID))		// Make sure the JID is not the same as the account.  It makes no sense to create a contact with the same JID as its parent account.  This situation occurs rarely when the server sends a stanza where the 'from' contains the JID of the account.
				{
				MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "Contact_PFindByJID('$s') - Creating contact for account $S\n", pszContactJID, &m_strJID);
				pContact = TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(IN pszContactJID);
				if (eFindContact == eFindContactCreateAsUnsolicited)
					pContact->SetFlagContactAsUnsolicited();
				return pContact;
				}
			} // if
		} // if
	return NULL;
	} // Contact_PFindByJID()

TContact *
TAccountXmpp::TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(PSZUC pszContactJID, PSZUC pszContactNameDisplay)
	{
	Assert(pszContactJID != NULL);
	Endorse(pszContactNameDisplay == NULL);	// Automatically generate a display name
	Assert(Contact_PFindByJID(pszContactJID) == NULL && "Contact already in the account");
	Assert(m_paTreeWidgetItem != NULL && "No Tree Item to attach to");
	TContact * pContact = new TContact(this);
	pContact->m_strRessource = pContact->m_strJidBare.AppendTextUntilCharacterPszr(pszContactJID, '/');
	pContact->m_strNameDisplayTyped = pszContactNameDisplay;
	pContact->TreeItemContact_GenerateDisplayNameFromJid();
	m_arraypaContacts.Add(PA_CHILD pContact);
	pContact->TreeItem_DisplayWithinNavigationTreeBefore(m_pTreeItemContactNew);
	pContact->TreeItemContact_UpdateIcon();
	pContact->TreeItemWidget_EnsureVisible();	// Make sure the new contact is visible in the Navigation Tree
	pContact->m_tsCreated = Timestamp_GetCurrentDateTime();
	if (m_paSocket != NULL)
		Contact_RosterSubscribe(pContact);
	Configuration_Save();		// Save the configuration after adding a new contact (just in case the application crashes)
	TWallet::S_ContactAdded(pContact);	// Notify the wallet(s) there is a new contact
	return pContact;
	}

TContact *
TAccountXmpp::TreeItemAccount_PContactAllocateNewToNavigationTreeInvited_NZ(PSZUC pszContactJID)
	{
	TContact * pContact = TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(pszContactJID);
	pContact->SetFlagContactAsInvited();
	return pContact;
	}

//	TAccountXmpp::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TAccountXmpp object to respond to the interface of other objects it is related to, such as TCertificate and TCertificateServerName
void *
TAccountXmpp::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	switch (rti)
		{
	case RTI(TProfile):
		return m_pProfileParent;
	case RTI(TCertificate):
		return Certificate_PGet_YZ();
	case RTI(TCertificateServerName):
		return Certificate_PGetServerName();
	default:
		return TAccountCore::PGetRuntimeInterface(rti);
		}
	}

//	TAccountXmpp::IXmlExchange::XmlExchange()
void
TAccountXmpp::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	TAccountCore::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("JID", INOUT_F_UNCH_S &m_strJID);
	pXmlExchanger->XmlExchangeStr("Username", INOUT_F_UNCH_S &m_strUsername);
	pXmlExchanger->XmlExchangeStr("Password", INOUT_F_UNCH_S &m_strPassword);
	pXmlExchanger->XmlExchangeBin("CertificateFingerprint", INOUT_F_UNCH_S &m_binFingerprintCertificate);
	if (pXmlExchanger->m_fSerializing)
		m_arraypaContacts.ForEach_UAssignObjectIds();
	pXmlExchanger->XmlExchangeObjects('C', INOUT_F_UNCH_S &m_arraypaContacts, TContact::S_PaAllocateContact, this);	// Read the contacts in the old format
	pXmlExchanger->XmlExchangeObjects('G', INOUT_F_UNCH_S &m_arraypaGroups, TGroup::S_PaAllocateGroup, this);

	#ifdef DEBUG_IMPORT_OLD_CONFIG_XML
	if (!pXmlExchanger->m_fSerializing)
		pXmlExchanger->XmlExchangeObjects("Contacts", "Contact", INOUT_F_UNCH_S &m_arraypaContacts, TContact::S_PaAllocateContact, this);	// Read the contacts in the old format (this line needs to be deleted along with UnserializeContactsFromOldConfigXml())
	#endif

	m_strJIDwithResource.InitOnlyIfEmpty(IN m_strJID);	// Make sure m_strJIDwithResource is never empty
	} // XmlExchange()

//	TAccountXmpp::ITreeItem::TreeItem_FContainsMatchingText()
BOOL
TAccountXmpp::TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC
	{
	return (ITreeItem::TreeItem_FContainsMatchingText(pszTextSearchLowercase) || m_strJIDwithResource.FStringContainsSubStringNoCase(pszTextSearchLowercase));
	}

//	TAccountXmpp::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TAccountXmpp::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	PSZUC pszNameDisplay = m_strNameDisplayTyped;
	if (pszNameDisplay[0] != '\0')
		return pszNameDisplay;
	if (m_uFlagsAccountStatus & FAS_kfAccountAutoGenerated)
		return m_pProfileParent->TreeItem_PszGetNameDisplay();	// An automatic account uses the name of the parent profile
	return m_strUsername;
	}

const EMenuActionByte c_rgzeActionsMenuAccount[] =
	{
	eMenuAction_ContactAdd,
	eMenuActionSeparator,
	eMenuAction_AccountRename,
	eMenuAction_AccountDelete,
	eMenuAction_AccountProperties,
	ezMenuActionNone
	};

//	TAccountXmpp::ITreeItem::TreeItem_MenuAppendActions()
void
TAccountXmpp::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	if (!g_fIsConnectedToInternet || !Socket_FIsConnected())
		pMenu->ActionAdd(eMenuAction_AccountReconnect);
	pMenu->ActionsAdd(c_rgzeActionsMenuAccount);
	}

//	TAccountXmpp::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TAccountXmpp::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_AccountReconnect:
		if (m_paSocket != NULL)
			m_paSocket->Socket_Disconnect();	// Force a hard-reconnection
		Socket_ReconnectIfDisconnected();
		return ezMenuActionNone;
	case eMenuAction_AccountProperties:
		DisplayDialogProperties();
		return ezMenuActionNone;
	case eMenuAction_AccountDelete:
		TreeItemAccount_DeleteFromNavigationTree_MB(PA_DELETING);
		return ezMenuActionNone;
	case eMenuSpecialAction_ITreeItemRenamed:
		// When an account is renamed, we must repopulate the Chat Logs of every contact to display the new nickname
		ChatLog_ResetNickname();
		m_arraypaContacts.ForEach_ChatLogResetNickNameAndRepopulateAllEvents();
		return ezMenuActionNone;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

void
TAccountXmpp::Socket_ReconnectIfDisconnected()
	{
	if (m_paSocket == NULL || m_paSocket->Socket_FNeedsReconnection())
		{
		Socket_Connect();
		Assert(m_paSocket != NULL);
		Assert(!m_paSocket->Socket_FNeedsReconnection());
		}
	else
		{
		// Since the socket is already connected, attempt to process pending tasks (if any)
		m_paSocket->Socket_ProcessAllPendingTasks();
		}
	Assert(m_paSocket != NULL);
	Assert(!m_paSocket->Socket_FNeedsReconnection());
	}

BOOL
TAccountXmpp::Socket_FIsConnected() const
	{
	return (m_paSocket != NULL && m_paSocket->Socket_FuIsReadyToSendMessages());
	}

CSocketXmpp *
TAccountXmpp::Socket_PGetOnlyIfReadyToSendMessages() const
	{
	if (m_paSocket != NULL && m_paSocket->Socket_FuIsReadyToSendMessages())
		return m_paSocket;
	return NULL;
	}

void
TAccountXmpp::Socket_WriteXmlPresence()
	{
	if (Socket_FIsConnected())
		m_paSocket->Socket_WriteXmlPresence();
	}

void
TAccountXmpp::OnTimerNetworkIdle()
	{
	m_arraypContactsComposing.RemoveAllContactsComposingWhoAreIdle();
	m_listaDataXmlLargeEvents.DeleteIdleNodes();
	if (Socket_FIsConnected())
		m_paSocket->Socket_WriteXmlPingToServerIfIdle();
	}

void
TAccountXmpp::Socket_Connect()
	{
	Socket_ConnectUI(NULL);
	}

//	Must call Socket_DisconnectUI() when the UI is no longer needed
void
TAccountXmpp::Socket_ConnectUI(ISocketUI * piSocketUI, BOOL fCreateAccount)
	{
	Endorse(piSocketUI == NULL);
	PSZUC pszMessageConnecting = TreeItemAccount_SetIconConnectingToServer_Gsb();
	if (piSocketUI == NULL)
		StatusBar_SetTextU(pszMessageConnecting);
	else
		piSocketUI->SocketUI_DisplayMessage(eSeverityInfoTextBlack, pszMessageConnecting);	// Display the message to the user before creating the socket (the creation of the socket is CPU intensive, and the 0.5 second delay will be noticeable by the user)
	if (m_paSocket == NULL)
		m_paSocket = new CSocketXmpp(this);
	m_paSocket->SocketUI_Init(piSocketUI, fCreateAccount);
	m_paSocket->Socket_Connect();
	}

void
TAccountXmpp::Socket_ConnectUnsecured()
	{
	Assert(m_paSocket != NULL);
	if (m_paSocket != NULL)
		m_paSocket->Socket_ConnectUnsecured();
	}

void
TAccountXmpp::Socket_DisconnectUI()
	{
	if (m_paSocket != NULL)
		m_paSocket->SocketUI_Remove();
	}

void
TAccountXmpp::ChatLog_DisplayStanza(const CXmlNode * pXmlNodeMessageStanza)
	{
	Assert(pXmlNodeMessageStanza != NULL);
	TContact * pContact = Contact_PFindByJID(IN pXmlNodeMessageStanza->PszFindAttributeValueFrom_NZ(), eFindContactCreateAsUnsolicited);	// Find the contact matching the the stanza
	if (pContact != NULL)
		pContact->ChatLogContact_DisplayStanzaToUI(pXmlNodeMessageStanza);
	else
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to deliver message from unknown contact:\n^N", pXmlNodeMessageStanza);	// This happens when the stanza is incomplete and the JID is not adequate to create a contact
	}

PSZUC
TAccountXmpp::ChatLog_PszGetPathFolderDownload()
	{
	extern CStr g_strPathFolderDefault;
	return g_strPathFolderDefault;
	}

void
TAccountXmpp::SetJIDwithResource(PSZUC pszJIDwithResource)
	{
	Report(pszJIDwithResource != NULL);
	Report(pszJIDwithResource[0] != '\0');
	m_strJIDwithResource = pszJIDwithResource;
	m_strJIDwithResource.InitOnlyIfEmpty(IN m_strJID);	// Just make sure m_strJIDwithResource is never empty, otherwise this would catastrophic.
	}

PSZUC
TAccountXmpp::Contact_RosterUpdateItem(const CXmlNode * pXmlNodeItemRoster)
	{
	Assert(pXmlNodeItemRoster != NULL);
	Assert(pXmlNodeItemRoster->FCompareTagName("item"));
	PSZUC pszJid = pXmlNodeItemRoster->PszFindAttributeValueJid_NZ();
	PSZUC pszSubscription = pXmlNodeItemRoster->PszuFindAttributeValue_NZ("subscription");	/* // The attribute "subscription" is not present for a "<iq type='set'>" */
	if (!FCompareStrings(pszSubscription, "remove"))
		{
		TContact * pContact = Contact_PFindByJID(pszJid, eFindContactCreate);
		Endorse(pContact == NULL);	// The attribute "jid" may not be valid (for example, missing the '@' character)
		if (pContact != NULL)
			pContact->XmppRosterSubscriptionUpdate(pszSubscription);
		}
	return pszJid;
	}

void
TAccountXmpp::Contacts_RosterUpdate(const CXmlNode * pXmlNodeQuery)
	{
	Assert(pXmlNodeQuery != NULL);
	Assert(pXmlNodeQuery->FCompareTagName("query"));
	MessageLog_AppendTextFormatSev(eSeverityNoise, "TAccountXmpp::Contacts_RosterUpdate()\n");
	// Loop through every <item> under the <query> element
	const CXmlNode * pXmlNodeItemRoster = pXmlNodeQuery->m_pElementsList;
	while (pXmlNodeItemRoster != NULL)
		{
		Contact_RosterUpdateItem(pXmlNodeItemRoster);
		pXmlNodeItemRoster = pXmlNodeItemRoster->m_pNextSibling;
		}
	// Make sure every contact is subscribed
	TContact * pContact;
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		pContact = *ppContact++;
		if (pContact->XmppRoster_PszGetSubscription() == NULL)
			Contact_RosterSubscribe(pContact);
			//pContact->XmppRosterSubscribed();
		}
	} // Contacts_RosterUpdate()

#ifdef DEBUG
void
TAccountXmpp::Contacts_RosterDisplayDebug(PSZAC pszName)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Roster subscription for account $S: $s\n", &m_strJID, pszName);
	TContact * pContact;
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		pContact = *ppContact++;
		PSZUC pszSubscription = pContact->XmppRoster_PszGetSubscription();
		if (pszSubscription != NULL)
			MessageLog_AppendTextFormatSev(eSeverityNoise, "\t $S: $s\n", &pContact->m_strJidBare, pszSubscription);
		}
	}
#endif

void
TAccountXmpp::Contact_RosterSubscribe(INOUT TContact * pContact)
	{
	Assert(pContact != NULL);
	if (m_paSocket == NULL)
		return;
	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coOrange), "Adding contact ^j to roster...\n", pContact);
	m_paSocket->Socket_WriteXmlFormatted("<iq type='set'><query xmlns='jabber:iq:roster'><item jid='^j'></item></query></iq>", pContact);
	//m_paSocket->Socket_WriteXmlFormatted("<presence to='^j' type='subscribe'/>", pContact);
	//m_paSocket->Socket_WriteXmlFormatted("<presence ^:jc from='^J' to='^j' type='subscribe'/>", this, pContact);
	}

void
TAccountXmpp::Contact_RosterUnsubscribe(INOUT TContact * pContact)
	{
	Assert(pContact != NULL);
	if (m_paSocket == NULL)
		return;
	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coOrange), "Unsubscribing from contact ^j...\n", pContact);
	m_paSocket->Socket_WriteXmlFormatted("<iq type='set'><query xmlns='jabber:iq:roster'><item jid='^j' subscription='remove'></item></query></iq>", pContact);
	}

void
TAccountXmpp::Contact_PresenceUpdate(const CXmlNode * pXmlNodeStanzaPresence)
	{
	Assert(pXmlNodeStanzaPresence != NULL);
	Assert(pXmlNodeStanzaPresence->FCompareTagName("presence"));
	// Find the contact to update the presence
	TContact * pContact = Contact_PFindByJID(pXmlNodeStanzaPresence->PszFindAttributeValueFrom_NZ(), eFindContactCreate);
	Endorse(pContact == NULL);	// The <presence> stanza sometimes is just a reply from the server, which means the attribute "from" is not a contact.
	if (pContact != NULL)
		pContact->XmppPresenceUpdateIcon(pXmlNodeStanzaPresence);
	}

/*
void
TAccountXmpp::Contacts_SetFlagAboutBeingDeleted()
	{
	m_arraypaContacts.ForEach_SetFlagTreeItemAboutBeingDeleted();
	}

//	This method is a bit slow, as it makes sure any reference to the contact is removed.
void
TAccountXmpp::Contacts_BroadcastAboutBeingDeleted()
	{
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		Assert(pContact->m_pAccount == this);
		//pContact->DetachFromContactsAboutBeingDeleted();
		}
	}
*/

//	Method to safely delete a contact from an account.
void
TAccountXmpp::Contact_DeleteSafely(PA_DELETING TContact * paContactDelete)
	{
	Assert(paContactDelete != NULL);
	Assert(paContactDelete->EGetRuntimeClass() == RTI(TContact));
	paContactDelete->m_uFlagsTreeItem |= FTI_kfTreeItemAboutBeingDeleted;
	TreeItem_SelectWithinNavigationTree();	// Select the parent deleting the contact (without this line of code, the application will crash if the contact was selected in the GUI because the contact is no longer there)
	if (m_paSocket != NULL && m_paSocket->Socket_FuIsReadyToSendMessages())
		{
		if (paContactDelete->XmppRoster_PszGetSubscription() != NULL)
			Contact_RosterUnsubscribe(paContactDelete);	// Remove the contact from the roster
		}
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		Assert(pGroup->m_pAccount == this);
		pGroup->DeleteAllAliasesRelatedToContactsAboutBeingDeleted();
		}
	m_arraypContactsComposing.RemoveAllTreeItemsAboutBeingDeleted();
	m_arraypContactsMessagesUnread.RemoveAllTreeItemsAboutBeingDeleted();
	g_arraypContactsRecentMessages.RemoveAllTreeItemsAboutBeingDeleted();	// Make sure the contact is no longer in the recent messages
	TWallet::S_DetachFromContactsAboutBeingDeleted();	// Detach the contact from the any wallet
	m_arraypaContacts.DeleteTreeItem(PA_DELETING paContactDelete);	// Delete the contact object and remove it from the Navigation Tree
	} // Contact_DeleteSafely()

void
TAccountXmpp::TreeItemAccount_DisplayWithinNavigationTree()
	{
	TreeItem_DisplayWithinNavigationTree(g_pTreeItemCommunication);
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		Assert(pContact->m_pAccount == this);
		pContact->TreeItemContact_DisplayWithinNavigationTree();
		} // while
	m_pTreeItemContactNew = new TContactNew(this);

	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		Assert(pGroup->m_pAccount == this);
		pGroup->TreeItemGroup_DisplayWithinNavigationTree();
		}

	// Display the alias as well
	m_paAlias->TreeItem_DisplayWithinNavigationTree(m_pProfileParent, m_strJID, eMenuIconXmpp);
	} // TreeItemAccount_DisplayWithinNavigationTree()

void
TAccountXmpp::TreeItemAccount_DisplayWithinNavigationTreeInit(PSZUC pszServerName, UINT uServerPort)
	{
	Assert(pszServerName != NULL);
	Assert(!m_strUsername.FIsEmptyString());
	m_uFlagsAccountStatus |= FAS_kfAccountAutoGenerated;
	m_uServerPort = uServerPort;
	m_strServerName = pszServerName;
	m_strJID.Format("$S@$S", &m_strUsername, &m_strServerName);
	m_strJIDwithResource = m_strJID;
	/*
	m_strNameDisplay = m_strJIDwithResource = m_strJID;	// Set the Display Name the same as the JID
	if (g_arraypAccounts.PFindChatAccountByNameDisplay(m_strUsername) == NULL)
		m_strNameDisplay = m_strUsername;	// Make the display name as short as possible, unless there is already a similar account displaying the username
	*/
	TreeItemAccount_DisplayWithinNavigationTree();
	TreeItemWidget_Expand();
	TreeItemWidget_EnsureVisible();	// Make sure the account is visible under the "Communicate" node
	}

void
TAccountXmpp::TreeItemAccount_DeleteFromNavigationTree_MB(PA_DELETING)
	{
	const int cContacts =  m_arraypaContacts.GetSize();
	if (cContacts > 0)
		{
		if (EMessageBoxQuestion("Are you sure you want to remove the account $S and its $i contacts?", &m_strJID, cContacts) != eAnswerYes)
			return;
		}
	m_uFlagsTreeItem |= FTI_kfTreeItemAboutBeingDeleted;
	m_arraypaContacts.ForEach_SetFlagTreeItemAboutBeingDeleted();
	m_arraypaGroups.ForEach_SetFlagTreeItemAboutBeingDeleted();
	g_arraypContactsRecentMessages.RemoveAllTreeItemsAboutBeingDeleted();	// Make sure there is dangling pointer to a deleted contact
	g_arraypAccounts.RemoveAllTreeItemsAboutBeingDeleted();
	TWallet::S_AccountsAboutBeingDeleted();
	m_pProfileParent->m_arraypaAccountsXmpp.DeleteTreeItem(PA_DELETING this);
	}

void
TAccountXmpp::TreeItemAccount_UpdateIcon()
	{
	Assert(m_paTreeWidgetItem != NULL);
	QRGB coText = d_coTreeItem_Default;
	EMenuAction eMenuAction = eMenuAction_PresenceAccountOffline;
	if (!m_arraypContactsMessagesUnread.FIsEmpty())
		{
		// We have unread messages, so display the text in Green and display the icon of unread messages
		coText = d_coTreeItem_UnreadMessages;
		eMenuAction = eMenuAction_MessageNew;
		}
	else if (m_paSocket != NULL)
		{
		// The socket is ready, so the account is online
		if (m_paSocket->Socket_FuIsReadyToSendMessages())
			{
			eMenuAction = (EMenuAction)(m_uFlagsAccountStatus & FAS_kmStatusPresenceMask);
			if (eMenuAction == ezMenuActionNone)
				{
				// Fetch the presence from the global settings
				eMenuAction = (EMenuAction)(g_uPreferences & P_kmPresenceMask);
				if (eMenuAction == ezMenuActionNone)
					eMenuAction = eMenuAction_PresenceAccountOnline;
				}
			}
		else
			{
			if (m_paSocket->Socket_FDisplayIconFailure())
				eMenuAction = eMenuIconFailure;
			else
				eMenuAction = eMenuAction_PresenceAccountConnecting;
			}
		}
	TreeItem_SetTextColor(coText);
	if (g_fIsConnectedToInternet)
		{
		if ((m_uFlagsTreeItem & FTI_kemIconMask) != 0)
			return;
		}
	else
		eMenuAction = eMenuAction_PresenceAccountDisconnected;	// Always display the disconnect icon if the device is disconnected
	TreeItem_SetIcon(eMenuAction);
	} // TreeItemAccount_UpdateIcon()

void
TAccountXmpp::TreeItemAccount_UpdateIconOfAllContacts()
	{
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		Assert(pContact->m_pAccount == this);
		pContact->TreeItemContact_UpdateIcon();
		}
	}

void
TAccountXmpp::PresenceUpdateFromGlobalSettings(EMenuAction eMenuAction_Presence)
	{
	Assert(eMenuAction_Presence <= eMenuAction_PresenceLast);
	PresenceUpdateFromContextMenu(eMenuAction_Presence);
	}

void
TAccountXmpp::PresenceUpdateFromContextMenu(EMenuAction eMenuAction_Presence)
	{
	Assert(eMenuAction_Presence <= eMenuAction_PresenceLast);
	const UINT uFlagsAccountStatus = (m_uFlagsAccountStatus & ~FAS_kmStatusPresenceMask) | eMenuAction_Presence;
	if (uFlagsAccountStatus == m_uFlagsAccountStatus)
		return;
	m_uFlagsAccountStatus = uFlagsAccountStatus;	// Update the presence
	if (m_paSocket != NULL && m_paSocket->Socket_FuIsReadyToSendMessages())
		{
		switch (eMenuAction_Presence)
			{
		case eMenuAction_PresenceAccountOnline:
		case eMenuAction_PresenceAway:
		case eMenuAction_PresenceAwayExtended:
		case eMenuAction_PresenceBusy:
			m_paSocket->Socket_WriteXmlPresence();
			break;
		default:
			m_paSocket->Socket_Disconnect();
			TContact ** ppContactStop;
			TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
			while (ppContact != ppContactStop)
				{
				TContact * pContact = *ppContact++;
				Assert(pContact != NULL);
				Assert(pContact->EGetRuntimeClass() == RTI(TContact));
				Assert(pContact->m_pAccount == this);
				pContact->TreeItemContact_UpdateIconOffline();
				}
			} // switch
		} // if
	TreeItemAccount_UpdateIcon();	// Update the icon according to the new presence
	} // PresenceUpdateFromContextMenu()

void
TAccountXmpp::PresenceUnsubscribe()
	{
	Assert(FALSE && "NYI");
	}

void
TAccountXmpp::IconUpdate_NetworkOnlineStateChanged()
	{
	TreeItemAccount_UpdateIcon();
	TreeItemAccount_UpdateIconOfAllContacts();
	}

void
TAccountXmpp::IconUpdate_MessageArrivedFromContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (m_arraypContactsMessagesUnread.AddUniqueF(pContact))
		{
		// A new contact containing an unread message was added, so update the icon
		m_uFlagsAccountStatus |= FAS_kfMessageUnread;
		TreeItemAccount_UpdateIcon();
		}
	}

void
TAccountXmpp::IconUpdate_MessagesReadForContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (m_arraypContactsMessagesUnread.RemoveElementFastF(pContact))
		{
		// We have removed the contact, so check if the list is empty to update the icon
		if (m_arraypContactsMessagesUnread.FIsEmpty())
			{
			m_uFlagsAccountStatus &= ~FAS_kfMessageUnread;
			TreeItemAccount_UpdateIcon();	// No more unread messages, so set the normal icon
			}
		}
	MainWindow_SystemTrayMessageReadForContact(pContact);
	}

PSZUC
TAccountXmpp::TreeItemAccount_SetIconConnectingToServer_Gsb()
	{
	PSZUC pszMessage = g_strScratchBufferStatusBar.Format("Connecting to server $S...", &m_strServerName);
	TreeItem_SetIconWithToolTip(eMenuAction_PresenceAccountConnecting, g_strScratchBufferStatusBar);
	return pszMessage;
	}

void
TAccountXmpp::TreeItemAccount_SetIconOnline()
	{
	m_uFlagsTreeItem &= ~FTI_kemIconMask;	// Clear any previous icon error
	TreeItemAccount_UpdateIcon();	// Update the icon (this is likely to be eMenuAction_PresenceAccountOnline, however it may be the "away", "busy" and so on)
	}

/*
void
TAccountXmpp::TreeItemAccount_SetIconOffline()
	{
	// Set the offline icon only if there is no error
	if ((m_uFlagsTreeItem & FTI_kemIconMask) == 0)
		TreeItem_SetIcon(eMenuAction_PresenceAccountOffline);
	}
*/

void
TAccountXmpp::TreeItemAccount_SetIconDisconnected()
	{
	// Set the disconnected icon only if there is no error
	if ((m_uFlagsTreeItem & FTI_kemIconMask) == 0)
		TreeItem_SetIcon(eMenuAction_PresenceAccountDisconnected);
	TreeItemAccount_UpdateIconOfAllContacts();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TAccountXmpp *
CArrayPtrAccountsXmpp::PFindAccountByJID(PSZUC pszuAccountJID) const
	{
	Assert(pszuAccountJID != NULL);
	if (pszuAccountJID[0] != '\0')
		{
		TAccountXmpp ** ppAccountStop;
		TAccountXmpp ** ppAccount = PrgpGetAccountsStop(OUT &ppAccountStop);
		while (ppAccount != ppAccountStop)
			{
			TAccountXmpp * pAccount = *ppAccount++;
			if (pAccount->m_strJID.FCompareStringsJIDs(pszuAccountJID))
				return pAccount;
			}
		}
	return NULL;
	}

/*
TAccountXmpp *
CArrayPtrAccountsXmpp::PFindChatAccountByNameDisplay(PSZUC pszAccountNameDisplay) const
	{
	Assert(pszAccountNameDisplay != NULL);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		if (pAccount->m_strNameDisplay.FCompareStringsNoCase(pszAccountNameDisplay))
			return pAccount;
		}
	return NULL;
	}
*/

