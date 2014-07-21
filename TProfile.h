///////////////////////////////////////////////////////////////////////////////////////////////////
//	TProfile.h

#ifndef TPROFILE_H
#define TPROFILE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
class IApplication : public ITreeItem	// (application)
{
public:
	TProfile * m_pProfileParent;
	EMenuAction m_eMenuIcon;			// Icon to display in the Navigation Tree
//	SHashSha1 m_hashApplicationID;		// Value to identify the application (I think this is no longer needed)
public:
	IApplication(TProfile * pProfileParent, EMenuAction eMenuIcon);
	virtual PSZAC PszGetClassNameApplication() = 0;		// Return the class name of the application to be serialized to disk
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange

	void TreeItemApplication_DisplayWithinNavigationTree();
public:
	static IXmlExchange * S_PaAllocateApplication_YZ(POBJECT poProfileParent, const CXmlNode * pXmlNodeElement);	// This static method must be compatible with interface PFn_PaAllocateXmlObject2_YZ()
};


class CArrayPtrApplications : public CArrayPtrTreeItems
{
public:
	inline IApplication ** PrgpGetApplicationsStop(OUT IApplication *** pppApplicationStop) const { return (IApplication **)PrgpvGetElementsStop(OUT (void ***)pppApplicationStop); }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class TProfile : public ITreeItem	// (profile)
{
public:
	CChatConfiguration * m_pConfigurationParent;	// Configuration holding the profile. In the future, the GUI will be able to open multiple configurations simultaneously.
	CStr m_strNameProfile;		// Name of the profile (aka "nym")
	CBin m_binKeyPrivate;
	CBin m_binKeyPublic;
	CStr m_strComment;			// Description of the profile (this is useful for a user having multiple profiles)
	CArrayPtrAccountsXmpp m_arraypaAccountsXmpp;	// Accounts using the profile
	CArrayPtrApplications m_arraypaApplications;	// Applications uding the profile
	CArrayPtrBrowsers m_arraypaBrowsers;			// This is a bit of a hack, however at the moment, it will work
	CStr m_strData;									// Data of the profile (this variable is used for the JAPI)

public:
	TProfile(CChatConfiguration * pConfigurationParent);
	~TProfile();
	void GenerateKeys();
	void TreeItemProfile_DisplayContactsWithinNavigationTree();
	void TreeItemProfile_DisplayApplicationsWithinNavigationTree();
	void TreeItemProfile_DisplayProfileInfoWithinNavigationTree();

	void DeleteAccount(PA_DELETING TAccountXmpp * paAccount);

	TAccountXmpp * PAllocateAccount();
	TAccountXmpp * PAllocateAccountAutomaticCreationUI(ISocketUI * piSocketUI);
	#ifdef DEBUG_IMPORT_OLD_CONFIG_XML
	void UnserializeContactsFromOldConfigXml();
	#endif

	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;			// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;					// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	void XcpApiProfile_RecommendationsSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const;
	void GetRecommendations_Contacts(IOUT CArrayPtrContacts * parraypaContactsRecommended) const;
	void GetRecommendations_Groups(IOUT CArrayPtrGroups * parraypaGroupsRecommended) const;
	void GetRecommendations_Links() const;	// NYI
	void InitHashTablesOfIdentifiers(IOUT CHashTableIdentifiersOfContacts * pHashTableContacts, IOUT CHashTableIdentifiersOfGroups * pHashTableGroups) const;

	TApplicationMayanX * PAllocateApplicationMayanX();
	TApplicationBallotmaster * PGetApplicationBallotmaster_NZ();

	void BallotMaster_EventBallotAddAsTemplate(IEventBallot * pEventBallot);
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
