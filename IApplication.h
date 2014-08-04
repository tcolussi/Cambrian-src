#ifndef IAPPLICATION_H
#define IAPPLICATION_H
#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif

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



#endif // IAPPLICATION_H
