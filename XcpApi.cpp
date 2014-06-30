///////////////////////////////////////////////////////////////////////////////////////////////////
//	XcpApi.cpp
//
//	Implementation of "Personal APIs" (PAPI) available through the Cambrian Protocol.
//
//	The content of this file could be within Xcp.cpp, however since file Xcp.cpp already has 1500 lines,
//	it is easier to understand the code to have a file dedicated for the APIs.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "XcpApi.h"



#if 0
	Group.Profile.Get [idGroup]
	Group.Profile.GetPicture [idGroup]
	Contact.Profile.Get [idContact]
	Contact.Profile.GetPicture [idContact]
	Contact.Profile.GetPicturePanoramic [idContact]
	Contact.Wallet.GetAddress [idContact], strCurrency, strInvoiceNumber

	Me.Profile.Get
	Me.Wallet.GetAddress
#endif

const char c_szaApi_ProfileGet[] = "ProfileGet";

void
CBinXcpStanzaType::BinXmlAppendXcpApiRequest_ProfileGet(PSZUC pszGroupIdentifier)
	{
	BinXmlAppendXcpApiRequest(c_szaApi_ProfileGet, pszGroupIdentifier);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	XcpApi_EReturnData()
//
//	Entry point to return the data of request XCP API.
//
//	Return TRUE if the API data was stored in pbinXcpApiParameterData.
//	Return FALSE if the API is unknown.
EErrorXcpApi
TContact::XcpApi_EReturnData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanzaType * pbinXcpApiParameterData)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(pbinXcpApiParameterData != NULL);
	MessageLog_AppendTextFormatSev(eSeverityComment, "Processing API '$s'\n", pszApiName);
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_ProfileGet))
		{
		// Return the information about a profile.  The profile may be for an individual, or a group.
		PSZUC pszProfileIdentifier = pXmlNodeApiParameters->m_pszuTagValue;
		if (pszProfileIdentifier == NULL || *pszProfileIdentifier == '\0')
			{
			// Return our own profile
			//XcpApiContact_ProfileSerialize(INOUT pbinXcpStanzaReply);
			return eErrorXcpApi_zSuccess;
			}
		TGroup * pGroup = m_pAccount->Group_PFindByIdentifier(pszProfileIdentifier, NULL);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileSerialize(INOUT pbinXcpApiParameterData);
			return eErrorXcpApi_zSuccess;
			}
		pbinXcpApiParameterData->BinAppendTextSzv_VE("<Profile ID='^j' Name='^S' />", m_pAccount, &m_pAccount->m_pProfileParent->m_strNameProfile);
		return eErrorXcpApi_zSuccess;
		}
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API '$s'\n", pszApiName);
	return eErrorXcpApi_zSuccess;
	}

//	Entry point to process the data returned by XcpApi_FReturnData()
void
TContact::XcpApi_ProcessReturnedData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanzaType * pbinXcpApiExtraRequest)
	{
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_ProfileGet))
		{
		// We received a profile information
		MessageLog_AppendTextFormatSev(eSeverityComment, "Updating profile ^N", pXmlNodeApiParameters);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TContact::XcpApiContact_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{

	}

#define d_chApi_idContact			'c'
//	Serialize a group for the XCP API.
void
TGroup::XcpApiGroup_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<G ID='{h|}' Name='^S'>", &m_hashGroupIdentifier, &m_strNameDisplayTyped);
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		pbinXcpStanzaReply->BinAppendText("<M");	// {ic}, {is}
		pbinXcpStanzaReply->BinAppendXmlAttributeOfContactIdentifier(d_chApi_idContact, pMember->m_pContact);
		pbinXcpStanzaReply->BinAppendText("/>");
		}
	pbinXcpStanzaReply->BinAppendText("</G>");
	}

void
TGroup::XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanzaType * pbinXcpApiExtraRequest)
	{
	// Fill in any missing group info from the data in pXmlNodeApiParameters
	if (m_strNameDisplayTyped.FIsEmptyString())
		m_strNameDisplayTyped = pXmlNodeApiParameters->PszuFindAttributeValue("Name");
	const CXmlNode * pXmlNodeMembers = pXmlNodeApiParameters->PFindElement("M");
	while (pXmlNodeMembers != NULL)
		{
		m_pAccount->Contact_PFindByIdentifierOrCreate_YZ(pXmlNodeMembers, d_chApi_idContact, INOUT pbinXcpApiExtraRequest);
		pXmlNodeMembers = pXmlNodeMembers->m_pNextSibling;
		}

	} // XcpApiGroup_ProfileUnserialize()

