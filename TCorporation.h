///////////////////////////////////////////////////////////////////////////////////////////////////
//	TCorporation.h
//
//	Class defining a formal business.
//	A corporation has many groups: shareholders, directors, employees, etc.

#ifndef TCORPORATION_H
#define TCORPORATION_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class TCorporation : public ITreeItemOfProfile
{
public:
	TGroup m_oGroupShareholders;
	TGroup m_oGroupDirectors;

public:
	TCorporation(TProfile * pProfile);
	~TCorporation();
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);					// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;							// From ITreeItem

public:
	static IXmlExchange * S_PaAllocateCorporation(POBJECT pProfileParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	RTI_IMPLEMENTATION(TCorporation)
}; // TCorporation

class CArrayPtrCorporations : public CArrayPtrTreeItemChatLogEvents
{
public:
	inline TCorporation ** PrgpGetCorporationsStop(OUT TCorporation *** pppCorporationStop) const { return (TCorporation **)PrgpvGetElementsStop(OUT (void ***)pppCorporationStop); }
};


#endif // TCORPORATION_H
