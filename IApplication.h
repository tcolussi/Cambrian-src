///////////////////////////////////////////////////////////////////////////////////////////////////
//	IApplication.h

#ifndef IAPPLICATION_H
#define IAPPLICATION_H
#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif

//	A service is a module running in the background without any visible graphical component.
//	An application may use multiple services to perform its tasks.
class IService : public IXmlExchangeObjectID
{
public:
	TProfile * m_pProfileParent;
public:
	IService(TProfile * pProfileParent);
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange

public:
	static IXmlExchange * S_PaAllocateService_YZ(POBJECT poProfileParent, const CXmlNode * pXmlNodeElement);	// This static method must be compatible with interface PFn_PaAllocateXmlObject2_YZ()
	static IService * S_PaAllocateService_YZ(TProfile * pProfileParent, RTI_ENUM rtiService);
};

class CArrayPtrServices : public CArrayPtrXmlSerializableObjects
{
public:
	inline IService ** PrgpGetServicesStop(OUT IService *** pppServiceStop) const { return (IService **)PrgpvGetElementsStop(OUT (void ***)pppServiceStop); }
};


class CServiceBallotmaster : public IService
{
public:
	CVaultEvents m_oVaultBallots;	// Ballots are 'events' because they may be sent to other users, and therefore require a vault for their storage

public:
	CServiceBallotmaster(TProfile * pProfileParent);
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange

	CEventBallotPoll * PAllocateBallot(const IEventBallot * pEventBallotTemplate = NULL);
	void EventBallotAddAsTemplate(IEventBallot * pEventBallot);

	RTI_IMPLEMENTATION_SZ(CServiceBallotmaster)
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Generic interface to include a 'plugin' within Cambrian.
//	A user may add to the Navigation Tree any available application.
class IApplication : public ITreeItem	// (application)
{
public:
    TProfile * m_pProfileParent;
    EMenuAction m_eMenuIcon;			// Icon to display in the Navigation Tree
//	SHashSha1 m_hashApplicationID;		// Value to identify the application (I think this is no longer needed)
public:
    IApplication(TProfile * pProfileParent, EMenuAction eMenuIcon);
    virtual PSZAC PszGetClassNameApplication() = 0;		// Return the class name of the application to be serialized to disk

    virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;		// From IRuntimeObject
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


/*
class IApplicationHtml : public IApplication
	{
	name: "Office Kingpin"
	location: "/apps/OfficeKingpin/index.html"
	help: URL

	void go();

	};

class CApplicationHtmlOfficeKingpin : public IApplicationHtml;
class CApplicationHtmlXik : public IApplicationHtml;

class TApplicationHtml : public IApplication
{

};
*/


#endif // IAPPLICATION_H
