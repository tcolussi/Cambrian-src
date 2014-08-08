///////////////////////////////////////////////////////////////////////////////////////////////////
//	IApplication.cpp

#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif

IService::IService(TProfile * pProfileParent)
	{
	Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
	m_pProfileParent = pProfileParent;
	}

//	IService::IXmlExchange::XmlExchange()
void
IService::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	pXmlExchanger->XmlExchangeWriteAttributeRtiSz(c_szaApplicationClass_, EGetRuntimeClass());
	}


IXmlExchange *
IService::S_PaAllocateService_YZ(POBJECT poProfileParent, const CXmlNode * pXmlNodeElement)
	{
	Assert(pXmlNodeElement != NULL);
	return S_PaAllocateService_YZ((TProfile *)poProfileParent, ERuntimeClassFromPsz(pXmlNodeElement->PszuFindAttributeValue_NZ(c_szaApplicationClass_)));
	}

IService *
IService::S_PaAllocateService_YZ(TProfile * pProfileParent, RTI_ENUM rtiService)
	{
	Assert(pProfileParent != NULL);
	Assert(pProfileParent->EGetRuntimeClass() == RTI(TProfile));
	switch (rtiService)
		{
	case RTI_SZ(CServiceBallotmaster):
		return new CServiceBallotmaster(pProfileParent);
	#ifdef DEBUG
	default:
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown service '$U'\n", rtiService);
	#endif
		}
	return NULL;
	}

IService *
TProfile::PGetService_NZ(RTI_ENUM rtiService) CONST_MCC
	{
	IService * pService = (IService *)m_arraypaServices.PFindRuntimeObject(rtiService);
	if (pService == NULL)
		{
		pService = IService::S_PaAllocateService_YZ(this, rtiService);	// Although this method may return NULL, it should 'never' be the case because the rtiService should be valid
		m_arraypaServices.Add(PA_CHILD pService);
		}
	return pService;
	} // PGetService_NZ()


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
    return ITreeItem::PGetRuntimeInterface(rti, m_pProfileParent);
    }

//	IApplication::IXmlExchange::XmlExchange()
void
IApplication::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
    {
	pXmlExchanger->XmlExchangeWriteAttribute(c_szaApplicationClass_, (PSZUC)PszGetClassNameApplication());	// We need to serialize the class name because the unserialize need to know what object to allocate
    }

#if 1 // The fillowing code will need to be replaced using USZU() technology
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
//    { c_szaApplicationClass_Ballotmaster, PaAllocateApplicationBallotmaster },
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
#endif


