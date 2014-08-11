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


TProfile::TProfile(CChatConfiguration * pConfigurationParent)
	{
	Assert(pConfigurationParent != NULL);
	m_pConfigurationParent = pConfigurationParent;
	m_paContactDummy = NULL;
	}

TProfile::~TProfile()
	{
	//delete m_paContactDummy; // This is a memory leak, however this line of code crashes.  Need to investigate
	m_arraypaAccountsXmpp.DeleteAllTreeItems();
	m_arraypaApplications.DeleteAllTreeItems();
	m_arraypaBrowsers.DeleteAllTreeItems();
	m_arraypaServices.DeleteAllRuntimeObjects();
	}

//	Generate a pair of keys for the profile.
void
TProfile::GenerateKeys()
	{
	// At the moment, since we do not have PGP code, make both keys the SHA-1 of the name
	m_binKeyPrivate.BinInitFromCalculatingHashSha1(m_strNameProfile);
	m_binKeyPublic = m_binKeyPrivate;
	}

void
TProfile::RemoveAllReferencesToObjectsAboutBeingDeleted()
	{
	g_arraypContactsRecentMessages.RemoveAllTreeItemsAboutBeingDeleted();	// Make sure there is dangling pointer to a deleted contact
	g_arraypAccounts.RemoveAllTreeItemsAboutBeingDeleted();
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
	paAccountDelete->MarkForDeletion();
	RemoveAllReferencesToObjectsAboutBeingDeleted();
	m_arraypaAccountsXmpp.DeleteTreeItem(PA_DELETING this);
	}

void
TProfile::DeleteGroup(PA_DELETING TGroup * paGroupDelete)
	{
	Assert(paGroupDelete != NULL);
	Assert(paGroupDelete->EGetRuntimeClass() == RTI(TGroup));
	paGroupDelete->MarkForDeletion();
	RemoveAllReferencesToObjectsAboutBeingDeleted();
	paGroupDelete->Group_UpdateFlagCannotBeDeleted();
	paGroupDelete->m_pAccount->m_arraypaGroups.DeleteTreeItem(PA_DELETING paGroupDelete);
	}

//	Method to safely delete a contact from a profile
void
TProfile::DeleteContact(PA_DELETING TContact * paContactDelete)
	{
	Assert(paContactDelete != NULL);
	Assert(paContactDelete->EGetRuntimeClass() == RTI(TContact));
	paContactDelete->MarkForDeletion();
	RemoveAllReferencesToObjectsAboutBeingDeleted();
	paContactDelete->Contact_UpdateFlagCannotBeDeleted();
	//paContactDelete->m_pAccount->m_arraypaContacts.DeleteTreeItem(PA_DELETING paContactDelete);	// Delete the contact object and remove it from the Navigation Tree
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

void
TProfile::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(INOUT pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("Name", INOUT_F_UNCH_S &m_strNameProfile);
	pXmlExchanger->XmlExchangeStr("Comment", INOUT_F_UNCH_S &m_strComment);
	pXmlExchanger->XmlExchangeBin("KeyPrivate", INOUT_F_UNCH_S &m_binKeyPrivate);
	pXmlExchanger->XmlExchangeBin("KeyPublic", INOUT_F_UNCH_S &m_binKeyPublic);
	pXmlExchanger->XmlExchangeObjects(d_chElementName_Accounts, INOUT &m_arraypaAccountsXmpp, TAccountXmpp::S_PaAllocateAccount, this);
	pXmlExchanger->XmlExchangeObjects2(d_chElementName_Applications, INOUT_F_UNCH_S &m_arraypaApplications, IApplication::S_PaAllocateApplication_YZ, this);
	pXmlExchanger->XmlExchangeObjects2(d_chElementName_Services, INOUT &m_arraypaServices, IService::S_PaAllocateService_YZ, this);
	pXmlExchanger->XmlExchangeObjects('B', INOUT &m_arraypaBrowsers, TBrowser::S_PaAllocateBrowser, this);

	pXmlExchanger->XmlExchangeStr("Data", INOUT_F_UNCH_S &m_strData);
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
	eMenuAction_ProfileDelete,
	//eMenuAction_ProfileProperties,
	ezMenuActionNone
	};

//	TProfile::ITreeItem::TreeItem_MenuAppendActions()
void
TProfile::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuProfile);
	}

//	TProfile::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TProfile::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_ProfileDelete:
		if (m_arraypaAccountsXmpp.GetSize() | m_arraypaApplications.GetSize())
			{
			EMessageBoxInformation("Before deleting your "d_sza_profile" '$S', you must manually delete all its accounts.", &m_strNameProfile);
			}
		else
			{
			m_pConfigurationParent->m_arraypaProfiles.DeleteTreeItem(PA_DELETING this);
			NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(NULL);	// After deleting a profile, display all the remaining profiles
			}
		return ezMenuActionNone;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

void
TProfile::GetRecommendations_Contacts(IOUT CArrayPtrContacts * parraypaContactsRecommended) const
	{
	Assert(parraypaContactsRecommended != NULL);
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
			if (pContact->TreeItemFlags_FuIsRecommended())
				parraypaContactsRecommended->Add(pContact);
			} // while
		} // while
	}

void
TProfile::GetRecommendations_Groups(IOUT CArrayPtrGroups * parraypaGroupsRecommended) const
	{
	Assert(parraypaGroupsRecommended != NULL);
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
			if (pGroup->TreeItemFlags_FuIsRecommended())
				parraypaGroupsRecommended->Add(pGroup);
			} // while
		} // while
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
