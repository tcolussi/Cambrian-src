///////////////////////////////////////////////////////////////////////////////////////////////////
//	TProfile.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
TAccountAlias::TAccountAlias(TAccountXmpp * pAccount)
	{
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	m_pAccount = pAccount;
	//TreeItem_DisplayWithinNavigationTree(pAccount->m_pProfileParent, pAccount->m_strJID, eMenuIconXmpp);
	}

//	TAccountAlias::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TAccountAlias object to respond to the interfaces of its parent TAccountXmpp.
POBJECT
TAccountAlias::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pAccount);
	} // PGetRuntimeInterface()

const EMenuActionByte c_rgzeActionsMenuAccountAlias[] =
	{
	eMenuAction_ContactAdd,
	eMenuActionSeparator,
	eMenuAction_AccountDelete,
	eMenuAction_AccountProperties,
	ezMenuActionNone
	};

//	TAccountAlias::ITreeItem::TreeItem_MenuAppendActions()
void
TAccountAlias::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuAccountAlias);
	}

//	TAccountAlias::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TAccountAlias::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	return m_pAccount->TreeItem_EDoMenuAction(eMenuAction);
	} // TreeItem_EDoMenuAction()

///////////////////////////////////////////////////////////////////////////////////////////////////
IXmlExchange *
TProfile::S_PaAllocateProfile(PVOID pConfigurationParent)
	{
	Assert(pConfigurationParent != NULL);
	return new TProfile((CChatConfiguration *)pConfigurationParent);
	}


TProfile::TProfile(CChatConfiguration * pConfigurationParent) :
	m_arraypEventsRecentMessagesReceived(this, 100),
	m_arraypEventsRecentBallots(this, 20),
	m_strJurisdiction("Pantheon")
	{
	Assert(pConfigurationParent != NULL);
	m_pConfigurationParent = pConfigurationParent;
	m_paContactDummy = NULL;
	m_paoJapiProfile = NULL;
	}

TProfile::~TProfile()
	{
	//delete m_paContactDummy; // This is a memory leak, however this line of code crashes.  Need to investigate
	m_arraypaAccountsXmpp.DeleteAllTreeItems();
	m_arraypaApplications.DeleteAllTreeItems();
	m_arraypaBrowsers.DeleteAllTreeItems();
	m_arraypaServices.DeleteAllRuntimeObjects();
	m_arraypaBrowsersTabbed.DeleteAllRuntimeObjects();
	m_arraypaChannelNamesAvailables.DeleteAllChannels();
	//delete m_paoJapiProfile;
	}

//	Generate a pair of keys for the profile.
void
TProfile::GenerateKeys()
	{
	// At the moment, since we do not have PGP code, make both keys the SHA-1 of the name
	#if 0
	m_strNymID.BinAppendText_VE("MyNymID$l", Timestamp_GetCurrentDateTime());
	m_strKeyPrivate.BinInitFromText("PrivateKey123"); // BinInitFromCalculatingHashSha1(m_strNameProfile);
	m_strKeyPublic.BinInitFromText("PublicKey123");
	#endif
	}

void
TProfile::RemoveAllReferencesToObjectsAboutBeingDeleted()
	{
	g_arraypContactsRecentMessages.RemoveAllUnserializableTreeItems();	// Make sure there is dangling pointer to a deleted contact
	g_arraypAccounts.RemoveAllUnserializableTreeItems();
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->RemoveAllReferencesToObjectsAboutBeingDeleted();
		}
	m_arraypaServices.ForEach_DetachFromObjectsAboutBeingDeleted();
	TWallet::S_AccountsAboutBeingDeleted();
	}

void
TProfile::DeleteAccount(PA_DELETING TAccountXmpp * paAccountDelete)
	{
	Assert(paAccountDelete != NULL);
	Assert(paAccountDelete->EGetRuntimeClass() == RTI(TAccountXmpp));
	Assert(paAccountDelete->m_pProfileParent == this);
	paAccountDelete->Account_MarkForDeletion();
	RemoveAllReferencesToObjectsAboutBeingDeleted();
	m_arraypaAccountsXmpp.DeleteTreeItem(PA_DELETING paAccountDelete);
	}

void
TProfile::DeleteGroup(PA_DELETING TGroup * paGroupDelete)
	{
	Assert(paGroupDelete != NULL);
	Assert(paGroupDelete->EGetRuntimeClass() == RTI(TGroup));
	BOOL fDeletePermanently = paGroupDelete->Group_FCanBePermenentlyDeleted();
	paGroupDelete->Group_MarkForDeletion();
	RemoveAllReferencesToObjectsAboutBeingDeleted();
	paGroupDelete->TreeItemFlags_SetFlagSerializeToDisk_Yes();
	/*
	paGroupDelete->Group_UpdateFlagCannotBeDeleted();
	paGroupDelete->m_pAccount->m_arraypaGroups.DeleteTreeItem(PA_DELETING paGroupDelete);
	*/
	if (fDeletePermanently)
		paGroupDelete->m_pAccount->m_arraypaGroups.DeleteTreeItem(PA_DELETING paGroupDelete);	// Delete the group object and remove it from the Navigation Tree
	Dashboard_RefreshGroupsAndChannels();
	}

//	Method to safely delete a contact from a profile
void
TProfile::DeleteContact(PA_DELETING TContact * paContactDelete)
	{
	Assert(paContactDelete != NULL);
	Assert(paContactDelete->EGetRuntimeClass() == RTI(TContact));
	BOOL fDeletePermanently = paContactDelete->Contact_FCanBePermenentlyDeleted();
	paContactDelete->Contact_MarkForDeletion();
	RemoveAllReferencesToObjectsAboutBeingDeleted();
	paContactDelete->TreeItemFlags_SetFlagSerializeToDisk_Yes();	// Serialize the deleted contact so it does not reappear
	Assert(paContactDelete->TreeItemFlags_FuIsInvisible());
	/*
	paContactDelete->Contact_UpdateFlagCannotBeDeleted();
	paContactDelete->m_uFlagsTreeItem |= FTI_kfObjectInvisible;
	delete paContactDelete->TreeItemW_RemoveFromNavigationTree();
	//paContactDelete->m_pAccount->m_arraypaContacts.DeleteTreeItem(PA_DELETING paContactDelete);	// Delete the contact object and remove it from the Navigation Tree
	*/
	if (fDeletePermanently)
		paContactDelete->m_pAccount->m_arraypaContacts.DeleteTreeItem(PA_DELETING paContactDelete);	// Delete the contact object and remove it from the Navigation Tree
	Dashboard_RefreshContacts();
	}

//	TProfile::IRuntimeObject::PGetRuntimeInterface()
POBJECT
TProfile::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	POBJECT pObject = ITreeItem::PGetRuntimeInterface(rti, piParent);
	if (pObject != NULL)
		return pObject;
	return PGetRuntimeInterfaceOf_(rti, m_arraypaAccountsXmpp.PGetObjectUnique_YZ());	// If there is only one account, then return the interface of this account
	}

#define d_chElementName_Accounts			'A'
#define d_chElementName_Applications		'X'	// User-defined applications
#define d_chElementName_Services			'S'
#define d_chElementName_Channels			'H'

void
TProfile::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	if (pXmlExchanger->m_fSerializing)
		{
		m_arraypaAccountsXmpp.ForEach_UAssignObjectIds();

		// Serialize the channels
		CBin * pbinTemp = pXmlExchanger->PGetBinTemporaryDuringSerializationInitAlreadyEncoded();	// Use the temporary buffer to serialize all the channels
		CChannelName * pChannel = NULL;
		CChannelName ** ppChannelStop;
		CChannelName ** ppChannel = m_arraypaChannelNamesAvailables.PrgpGetChannelsStop(OUT &ppChannelStop);
		while (ppChannel != ppChannelStop)
			{
			pChannel = *ppChannel++;
			pbinTemp->BinAppendText_VE("<h n='^S' t='$t'/>", &pChannel->m_strName, pChannel->m_tsFirstRecommended);
			}
		if (pChannel != NULL)
			(void)pXmlExchanger->XmlExchange_PAllocateElementFromCBinString(d_chElementName_Channels, IN_MOD_TMP *pbinTemp);
		}
	else
		{
		// Unserialize the channels
		Assert(m_arraypaChannelNamesAvailables.FIsEmpty());
		const CXmlNode * pXmlNodeChannels = pXmlExchanger->XmlExchange_PFindElement(d_chElementName_Channels);
		if (pXmlNodeChannels != NULL)
			{
			const CXmlNode * pXmlNodeChannel = pXmlNodeChannels->m_pElementsList;
			while (pXmlNodeChannel != NULL)
				{
				CChannelName * paChannel = new CChannelName;
				m_arraypaChannelNamesAvailables.Add(PA_CHILD paChannel);
				pXmlNodeChannel->UpdateAttributeValueCStr('n', OUT_F_UNCH &paChannel->m_strName);
				pXmlNodeChannel->UpdateAttributeValueTimestamp('t', OUT_F_UNCH &paChannel->m_tsFirstRecommended);
				pXmlNodeChannel = pXmlNodeChannel->m_pNextSibling;
				}
			}
		} // if...else

	ITreeItem::XmlExchange(INOUT pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("Name", INOUT_F_UNCH_S &m_strNameProfile);
	pXmlExchanger->XmlExchangeStr("Comment", INOUT_F_UNCH_S &m_strComment);
	pXmlExchanger->XmlExchangeStr("NymID", INOUT_F_UNCH_S &m_strNymID);
	pXmlExchanger->XmlExchangeStr("KeyPriv", INOUT_F_UNCH_S &m_strKeyPrivate);
	pXmlExchanger->XmlExchangeStr("KeyPub", INOUT_F_UNCH_S &m_strKeyPublic);
	pXmlExchanger->XmlExchangeObjects(d_chElementName_Accounts, INOUT &m_arraypaAccountsXmpp, TAccountXmpp::S_PaAllocateAccount, this);
	pXmlExchanger->XmlExchangeObjects2(d_chElementName_Applications, INOUT_F_UNCH_S &m_arraypaApplications, IApplication::S_PaAllocateApplication_YZ, this);
	pXmlExchanger->XmlExchangeObjects2(d_chElementName_Services, INOUT &m_arraypaServices, IService::S_PaAllocateService_YZ, this);
	pXmlExchanger->XmlExchangeObjects('B', INOUT &m_arraypaBrowsers, TBrowser::S_PaAllocateBrowser, this);
	pXmlExchanger->XmlExchangeObjects('R', INOUT &m_arraypaBrowsersTabbed, TBrowserTabs::S_PaAllocateBrowserTabbed, this);
	pXmlExchanger->XmlExchangeStr("Data", INOUT_F_UNCH_S &m_strData);
	pXmlExchanger->XmlExchangeEventPointers('E', INOUT &m_arraypEventsRecentMessagesReceived, this);
	pXmlExchanger->XmlExchangeStr("Jurisdiction", &m_strJurisdiction);

	if ( m_strJurisdiction.FIsEmptyString() )
		m_strJurisdiction = CStr("Pantheon"); /*??? Default value */

	}

//	TProfile::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TProfile::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return m_strNameProfile;
	}

const EMenuActionByte c_rgzeActionsMenuProfile[] =
	{
	eMenuAction_AccountRegister,
	eMenuAction_AccountLogin,
	eMenuActionSeparator,
	#ifdef DEBUG
	eMenuAction_ProfileShowDeletedObjects,
	eMenuActionSeparator,
	#endif
	eMenuAction_ProfileDelete,
	eMenuAction_ProfileProperties,
	ezMenuActionNone
	};

//	TProfile::ITreeItem::TreeItem_MenuAppendActions()
void
TProfile::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuProfile);
	}

class TDisplayDeletedItems : public ITreeItemOfProfileOrphaned
{
public:
	TDisplayDeletedItems(TProfile * pProfile);
	RTI_IMPLEMENTATION(TDisplayDeletedItems)
};

TDisplayDeletedItems::TDisplayDeletedItems(TProfile * pProfile) : ITreeItemOfProfileOrphaned(pProfile)
	{
	TreeItemW_DisplayWithinNavigationTreeExpand(m_pProfile, "Deleted Items", eMenuIcon_RecycleBin);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			if (pContact->TreeItemFlags_FuIsInvisible())
				{
				pContact->TreeItemW_DisplayWithinNavigationTree(this);
				pContact->TreeItemChatLog_UpdateTextAndIcon();
				}
			}

		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			if (pGroup->TreeItemFlags_FuIsInvisible())
				pGroup->TreeItemGroup_DisplayWithinNavigationTree(this);
			} // while
		} // while
	}


//	TProfile::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TProfile::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_ProfileShowDeletedObjects:
		(void)new TDisplayDeletedItems(this);	// TODO: Fix the memory leak and also provide a mechanism to have only a single instance per profile (at the moment, it is used for debugging)
		return ezMenuActionNone;
	case eMenuAction_ProfileDelete:
		if (m_arraypaAccountsXmpp.GetSize() | m_arraypaApplications.GetSize())
			{
			EMessageBoxInformation("Before deleting your " d_sza_profile " '$S', you must manually delete all its accounts.", &m_strNameProfile);
			}
		else
			{
			m_pConfigurationParent->m_arraypaProfiles.DeleteTreeItem(PA_DELETING this);
			NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(NULL);	// After deleting a profile, display all the remaining profiles
			}
		return ezMenuActionNone;
	case eMenuAction_ProfileProperties:
		DisplayDialogProperties();
		return ezMenuActionNone;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

void
TProfile::GetRecommendations_Contacts(IOUT CArrayPtrContacts * parraypContactsRecommended) const
	{
	Assert(parraypContactsRecommended != NULL);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			if (pContact->TreeItemFlags_FuIsDeletedOrTemporary())
				continue;
			if (pContact->TreeItemFlags_FuIsRecommended())
				{
				Assert(pContact->TreeItemFlags_FCanDisplayWithinNavigationTree());	// A recommended contact should not have been deleted
				parraypContactsRecommended->Add(pContact);
				}
			} // while
		} // while
	}

void
TProfile::GetRecommendations_Groups(IOUT CArrayPtrGroups * parraypGroupsRecommended) const
	{
	Assert(parraypGroupsRecommended != NULL);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			Assert(pGroup->m_pAccount == pAccount);
			if (pGroup->TreeItemFlags_FuIsRecommended() && !pGroup->Group_FuIsChannel())
				{
				Assert(pGroup->TreeItemFlags_FCanDisplayWithinNavigationTree());	// A recommended group should not have been deleted
				parraypGroupsRecommended->Add(pGroup);
				}
			} // while
		} // while
	}

void
TProfile::GetRecommendations_Channels(IOUT CArrayPtrGroups * parraypChannelsRecommended) const
	{
	Assert(parraypChannelsRecommended != NULL);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			Assert(pGroup->m_pAccount == pAccount);
			if (pGroup->TreeItemFlags_FCanDisplayWithinNavigationTree() && pGroup->Group_FuIsChannel())
				parraypChannelsRecommended->Add(pGroup);
			} // while
		} // while
	}

UINT
TProfile::GetRecentGroups(OUT CArrayPtrGroups * parraypGroups) CONST_MCC
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->m_arraypaGroups.SortByLastActivity();
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			if (pGroup->TreeItemFlags_FCanDisplayWithinNavigationTree() && pGroup->UGetGroupType() == eGroupType_kzOpen)
				parraypGroups->Add(pGroup);
			}
		}
	parraypGroups->SortByLastActivity();
	return parraypGroups->GetSize();
	}


UINT
TProfile::GetRecentCorporations(OUT CArrayPtrGroups * parraypCorporations) CONST_MCC
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->m_arraypaGroups.SortByLastActivity();
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			if (pGroup->TreeItemFlags_FCanDisplayWithinNavigationTree() && pGroup->UGetGroupType() == eGroupType_keCorporation)
				parraypCorporations->Add(pGroup);
			}
		}
	parraypCorporations->SortByLastActivity();
	return parraypCorporations->GetSize();
	}

UINT
TProfile::GetRecentChannels(OUT CArrayPtrGroups * parraypChannels) CONST_MCC
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->m_arraypaGroups.SortByLastActivity();
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			if (pGroup->TreeItemFlags_FCanDisplayWithinNavigationTree() && pGroup->Group_FuIsChannel())
				parraypChannels->Add(pGroup);
			}
		}
	parraypChannels->SortByLastActivity();
	return m_arraypaChannelNamesAvailables.GetSize();
	}

UINT
TProfile::GetRecentContacts(CArrayPtrContacts * parraypContacts) CONST_MCC
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->m_arraypaContacts.SortByLastActivity();
		parraypContacts->Append(IN &pAccount->m_arraypaContacts);
		}
	parraypContacts->RemoveAllTreeItemsMatchingFlag(ITreeItem::FTI_kfObjectInvisible);
	parraypContacts->SortByLastActivity();
	return parraypContacts->GetSize();
	}

TContact *
TProfile::PGetContactDummy_NZ() CONST_MCC
	{
	if (m_paContactDummy == NULL)
		{
		TAccountXmpp * pAccount = (TAccountXmpp *)m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
		Assert(pAccount != NULL);	// The code should not call PGetContactDummy_NZ() without having an account.
		if (pAccount == NULL)
			pAccount = new TAccountXmpp(this);	// This will cause a memory leak.  At the moment, I just want the code to work as a proof of concept
		m_paContactDummy = new TContact(pAccount);
		}
	return m_paContactDummy;
}

