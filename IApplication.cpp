#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
IApplication::IApplication(TProfile * pProfileParent, EMenuAction eMenuIcon)
    {
    m_pProfileParent = pProfileParent;
    m_eMenuIcon = eMenuIcon;
    Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
    }

//	IApplication::IRuntimeObject::PGetRuntimeInterface()
POBJECT
IApplication::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
    {
    Report(piParent == NULL);
    /*
    switch (rti)
        {
    case RTI(TProfile):
        return m_pProfileParent;
    default:
        return ITreeItem::PGetRuntimeInterface(rti);
        }
    */
    return ITreeItem::PGetRuntimeInterface(rti, m_pProfileParent);
    }

//	IApplication::IXmlExchange::XmlExchange()
void
IApplication::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
    {
    pXmlExchanger->XmlExchangeWriteAttribute(c_szaApplicationClass_, PszGetClassNameApplication());	// We need to serialize the class name because the unserialize need to know what object to allocate
    }


typedef IApplication * (* PFn_PaAllocateApplication)(TProfile * pProfileParent);
    IApplication * PaAllocateApplicationMayanX(TProfile * pProfileParent);
    IApplication * PaAllocateApplicationBallotmaster(TProfile * pProfileParent);


struct SApplicationAllocator
    {
    PSZAC pszClass;					// Class name of the application. This name is serialized in the configuration, so when unserializing the configuration, we know which application class to allocate.
    PFn_PaAllocateApplication pfnAllocator;
    };

const SApplicationAllocator c_rgzApplicationAllocators[] =
    {
    { c_szaApplicationClass_MayanX, PaAllocateApplicationMayanX },
    { c_szaApplicationClass_Ballotmaster, PaAllocateApplicationBallotmaster },
    { NULL, NULL }	// Must be last
    };


IXmlExchange *
IApplication::S_PaAllocateApplication_YZ(POBJECT poProfileParent, const CXmlNode * pXmlNodeElement)
    {
    Assert(pXmlNodeElement != NULL);
    TProfile * pProfileParent = (TProfile *)poProfileParent;
    Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
    // Find the type of application to allocate
    PSZUC pszClass = pXmlNodeElement->PszuFindAttributeValue_NZ(c_szaApplicationClass_);
    const SApplicationAllocator * pApplicationAllocator = c_rgzApplicationAllocators;
    while (TRUE)
        {
        if (pApplicationAllocator->pszClass == NULL)
            break;
        Assert(pApplicationAllocator->pfnAllocator != NULL);
        if (FCompareStrings(pApplicationAllocator->pszClass, pszClass))
            return pApplicationAllocator->pfnAllocator(pProfileParent);
        pApplicationAllocator++;	// Search the next application
        } // while
    MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to find application of class '$s'\n", pszClass);
    return NULL;
    }


