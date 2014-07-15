///////////////////////////////////////////////////////////////////////////////////////////////////
//	TProfile.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
IApplication::IApplication(TProfile * pProfileParent)
	{
	m_pProfileParent = pProfileParent;
	Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
	}

typedef IApplication * (* PFn_PaAllocateApplication)(TProfile * pProfileParent);
	IApplication * PaAllocateApplicationMayanX(TProfile * pProfileParent);


struct SApplicationAllocator
	{
	PSZAC pszName;		// Name of the application (this value is serialized in the configuration)
	PFn_PaAllocateApplication pfnAllocator;
	};

const SApplicationAllocator c_rgzApplicationAllocators[] =
	{
	{ "MayanX", PaAllocateApplicationMayanX },
	{ NULL, NULL }	// Must be last
	};


IXmlExchange *
IApplication::S_PaAllocateApplication_YZ(POBJECT poProfileParent, const CXmlNode * pXmlNodeElement)
	{
	Assert(pXmlNodeElement != NULL);
	TProfile * pProfileParent = (TProfile *)poProfileParent;
	Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
	// Find the type of application to allocate
	PSZUC pszName = pXmlNodeElement->PszuFindAttributeValue_NZ("NameDisplay");	// This needs to be fixed
	const SApplicationAllocator * pApplicationAllocator = c_rgzApplicationAllocators;
	while (TRUE)
		{
		if (pApplicationAllocator->pszName == NULL)
			break;
		Assert(pApplicationAllocator->pfnAllocator != NULL);
		if (FCompareStrings(pApplicationAllocator->pszName, pszName))
			return pApplicationAllocator->pfnAllocator(pProfileParent);
		pApplicationAllocator++;	// Search the next application
		} // while
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to find application of type '$s'\n", pszName);
	return NULL;
	}


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
void *
TAccountAlias::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	switch (rti)
		{
	case RTI(TAccountXmpp):
	case RTI(TCertificate):
	case RTI(TCertificateServerName):
		return m_pAccount->PGetRuntimeInterface(rti);
	default:
		return ITreeItem::PGetRuntimeInterface(rti);
		}
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
	}

TProfile::~TProfile()
	{
	m_arraypaAccountsXmpp.DeleteAllTreeItems();
	m_arraypaApplications.DeleteAllTreeItems();
	}

//	Generate a pair of keys for the profile.
void
TProfile::GenerateKeys()
	{
	// At the moment, since we do not have PGP code, make both keys the SHA-1 of the name
	m_binKeyPrivate.BinInitFromCalculatingHashSha1(m_strNameProfile);
	m_binKeyPublic = m_binKeyPrivate;
	}

/*
void
TProfile::DeleteAccount(PA_DELETING TAccountXmpp * paAccount)
	{
	Assert(paAccount != NULL);
	Assert(paAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	}
*/

//	TProfile::IRuntimeObject::PGetRuntimeInterface()
void *
TProfile::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	switch (rti)
		{
	case RTI(TAccountXmpp):	// If there is only one account, then return the interface of this account
		return m_arraypaAccountsXmpp.PvGetElementUnique_YZ();	// May return NULL
	default:
		return ITreeItem::PGetRuntimeInterface(rti);
		}
	}

#define d_chElementName_Accounts			'A'
#define d_chElementName_Applications		'X'	// User-defined applications
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

void
Profile_DisplayBallotMaster()
	{
	CStr strPathApplication = "file:///" + g_oConfiguration.SGetPathOfFileName("Apps/Ballotmaster/default.htm");
	TBrowser * pBrowser = new TBrowser(&g_oConfiguration);
	g_oConfiguration.m_arraypaBrowsers.Add(PA_CHILD pBrowser);
	pBrowser->SetIconNameAndUrl(eMenuAction_DisplayBallotMaster, "Ballotmaster", strPathApplication);
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItemW_SelectWithinNavigationTree();
	}
