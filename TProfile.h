///////////////////////////////////////////////////////////////////////////////////////////////////
//	TProfile.h

#ifndef TPROFILE_H
#define TPROFILE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
class TProfile : public ITreeItem	// (profile)
{
public:
	CChatConfiguration * m_pConfigurationParent;	// Configuration holding the profile. In the future, the GUI will be able to open multiple configurations simultaneously.
	CStr m_strNameProfile;		// Name of the profile (aka "nym")
	CStr m_strNymID;			// OT nym
	CStr m_strKeyPrivate;
	CStr m_strKeyPublic;
	CStr m_strComment;			// Description of the profile (this is useful for a user having multiple profiles)
	CArrayPtrAccountsXmpp m_arraypaAccountsXmpp;	// Accounts using the profile
	CArrayPtrApplications m_arraypaApplications;	// Applications using the profile
	CArrayPtrServices m_arraypaServices;			// Running services for the profile
	CArrayPtrBrowsers m_arraypaBrowsers;			// This is a bit of a hack, however at the moment, it will work
	CArrayPtrBrowsersTabbed m_arraypaBrowsersTabbed;
	CArrayPtrEventsRecent m_arraypEventsRecentMessagesReceived;	// Recent text message received
	CArrayPtrEventsRecent m_arraypEventsRecentBallots;			// Recent ballots sent and received
	CArrayPtrChannelNames m_arraypaChannelNamesAvailables;		// This array is necessary to know how many channels are available in the dashboard (communication panel)
	CStr m_strData;									// Data of the profile (this variable is used for the JAPI)
	CStr m_strJurisdiction;							// the current jurisdiction name, TODO: create an object TJurisdictoin
	OJapiProfile * m_paoJapiProfile;

private:
	TContact * m_paContactDummy;		// Temporary contact necessary to implement JavaScript security and perform other tasks, such as having in-memory event vaults (the vault was designed to have a contact as its parent).

public:
	TProfile(CChatConfiguration * pConfigurationParent);
	~TProfile();
	void GenerateKeys();
	void TreeItemProfile_DisplayContactsWithinNavigationTree();
	void TreeItemProfile_DisplayApplicationsWithinNavigationTree();
	void TreeItemProfile_DisplayProfileInfoWithinNavigationTree();

	void RemoveAllReferencesToObjectsAboutBeingDeleted();
	void DeleteAccount(PA_DELETING TAccountXmpp * paAccountDelete);
	void DeleteGroup(PA_DELETING TGroup * paGroupDelete);
	void DeleteContact(PA_DELETING TContact * paContactDelete);

	TAccountXmpp * PAllocateAccount();
	TAccountXmpp * PAllocateAccountAutomaticCreationUI(ISocketUI * piSocketUI);
	TAccountXmpp * PGetFirstAccountOrAllocate_NZ();
	#ifdef DEBUG_IMPORT_OLD_CONFIG_XML
	void UnserializeContactsFromOldConfigXml();
	#endif

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;			// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;					// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	void XcpApiProfile_RecommendationsSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const;
	void GetRecommendations_Contacts(IOUT CArrayPtrContacts * parraypContactsRecommended) const;
	void GetRecommendations_Groups(IOUT CArrayPtrGroups * parraypGroupsRecommended) const;
	void GetRecommendations_Channels(IOUT CArrayPtrGroups * parraypChannelsRecommended) const;
	void GetRecommendations_Links() const;	// NYI
	void InitHashTablesOfIdentifiers(IOUT CHashTableIdentifiersOfContacts * pHashTableContacts, IOUT CHashTableIdentifiersOfGroups * pHashTableGroups) const;

	void GetRecentGroups(OUT CArrayPtrGroups * parraypGroups) CONST_MCC;
	UINT GetRecentChannels(OUT CArrayPtrGroups * parraypChannels) CONST_MCC;
	void GetRecentContacts(CArrayPtrContacts * parraypContacts) CONST_MCC;
	TGroup * PFindChannelByName(PSZUC pszChannelName) CONST_MCC;
	BOOL FIsChannelNameUnused(PSZUC pszChannelName) CONST_MCC;
	TGroup * GroupChannel_PCreateAndSelectWithinNavigationTree_NZ(PSZUC pszChannelName);

	IService * PAllocateService_YZ(RTI_ENUM rtiService);
	IService * PGetService_NZ(RTI_ENUM rtiService) CONST_MCC;
	inline CServiceBallotmaster * PGetServiceBallotmaster_NZ() { return (CServiceBallotmaster *)PGetService_NZ(RTI_SZ(CServiceBallotmaster)); }

	TApplicationMayanX * PAllocateApplicationMayanX();
	//TApplicationBallotmaster * PGetApplicationBallotmaster_NZ();

	TContact * PGetContactDummy_NZ() CONST_MCC;
	POJapiProfile POJapiGet();
	OJapiCambrian * POJapiGetCambrian();
	void BallotMaster_EventBallotAddAsTemplate(IEventBallot * pEventBallot);
	void BallotMaster_OnEventNewBallotReceived(CEventBallotReceived * pEventBallotReceived);
	void BallotMaster_OnEventVoteReceived(CEventBallotSent * pEventBallotSent);

	void DisplayDialogProperties();

public:
	static IXmlExchange * S_PaAllocateProfile(PVOID pConfigurationParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	friend class CArrayPtrProfiles;
	RTI_IMPLEMENTATION(TProfile)
}; // TProfile

class CArrayPtrProfiles : public CArrayPtrTreeItems
{
public:
	inline TProfile ** PrgpGetProfiles(OUT int * pcProfiles) const { return (TProfile **)PrgpvGetElements(OUT pcProfiles); }
	inline TProfile ** PrgpGetProfilesStop(OUT TProfile *** pppProfileStop) const { return (TProfile **)PrgpvGetElementsStop(OUT (void ***)pppProfileStop); }
};

#endif // TPROFILE_H
