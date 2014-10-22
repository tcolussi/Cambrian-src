///////////////////////////////////////////////////////////////////////////////////////////////////
//	TCorporation.cpp
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TCorporation.h"

TCorporation::TCorporation(TProfile * pProfile) : ITreeItemOfProfile(pProfile), m_oGroupShareholders(pProfile->PGetFirstAccountOrAllocate_NZ()), m_oGroupDirectors(pProfile->PGetFirstAccountOrAllocate_NZ())
	{
	}

TCorporation::~TCorporation()
	{

	}

//	Variables Used: ("N" + "F") + ("S" + "D")
void
TCorporation::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ITreeItemOfProfile::XmlExchange(pXmlExchanger);
	#if 0 // Need to implement this
	m_oGroupShareholders.XmlExchange(pXmlExchanger);
	m_oGroupDirectors.XmlExchange(pXmlExchanger);
	#endif
	}

PSZUC
TCorporation::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return (PSZUC)"Corporation";
	}
