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

const CHU c_szaApi_Group_Profile_Get[] = "Group.Profile.Get";

#define d_chAPIe_Group_					'G'
#define d_szAPIe_Group_					"G"
#define d_szAPIe_Group_h_s				"G i='{h|}' n='^S'"
	#define d_chAPIa_Group_shaIdentifier	'i'
	#define d_chAPIa_Group_strName			'n'
#define d_chAPIe_GroupMember_			'M'
#define d_szAPIe_GroupMember_			"M"
	#define d_chAPIe_GroupMember_idContact	'c'


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method to serialize the data for an XCP API request.
void
CBinXcpStanzaType::BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(XcpApi_FIsXmlElementOpened());
	Assert(m_pContact != NULL);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	MessageLog_AppendTextFormatSev(eSeverityComment, "Processing API '$s' with the following parameters: ^N\n", pszApiName, pXmlNodeApiParameters);

	if (FCompareStringsNoCase(pszApiName, c_szaApi_Group_Profile_Get))
		{
		PSZUC pszProfileIdentifier = pXmlNodeApiParameters->m_pszuTagValue;
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszProfileIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileSerialize(INOUT this);
			return;
			}
		Assert(XcpApi_FIsXmlElementClosedBecauseOfError());
		return;
		}
	/*
		pbinXcpApiParameterData->BinAppendTextSzv_VE("<Profile ID='^j' Name='^S' />", m_pAccount, &m_pAccount->m_pProfileParent->m_strNameProfile);
		return eErrorXcpApi_zSuccess;
	*/
	} // BinXmlAppendXcpElementForApiRequest_AppendApiParameterData()


//	Method to unserialize the data from BinXmlAppendXcpElementForApiRequest_AppendApiParameterData().
//	Although the reply typically does not need the CBinXcpStanzaType, it may use the CBinXcpStanzaType to make additional request.
//	For instance, when unserializing a group, the method may need to query information about new group members to fetch the contact profile.
void
CBinXcpStanzaType::BinXmlAppendXcpElementForApiReply(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(m_pContact != NULL);
	MessageLog_AppendTextFormatSev(eSeverityComment, "BinXmlAppendXcpElementForApiReply($s):\n^N\n", pszApiName, pXmlNodeApiParameters);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Group_Profile_Get))
		{
		// We received a profile information
		PSZUC pszGroupIdentifier = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_Group_shaIdentifier);
		//MessageLog_AppendTextFormatCo(d_coRed, "BinXmlAppendXcpElementForApiReply($s) - Group '$s'\n", pszApiName, pszGroupIdentifier);
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszGroupIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileUnserialize(IN pXmlNodeApiParameters, INOUT this);
			}
		return;
		}
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API: $s\n^N", pszApiName, pXmlNodeApiParameters);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
CBinXcpStanzaType::BinXmlAppendXcpApiRequest_ProfileGet(PSZUC pszGroupIdentifier)
	{
	BinXmlAppendXcpApiRequest((PSZAC)c_szaApi_Group_Profile_Get, pszGroupIdentifier);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	XcpApi_EReturnData()
//
/*
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
		TGroup * pGroup = m_pAccount->Group_PFindByIdentifier_YZ(pszProfileIdentifier, INOUT pbinXcpApiParameterData, TAccountXmpp::eFindGroupOnly);
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
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Group_Profile_Get))
		{
		// We received a profile information
		MessageLog_AppendTextFormatSev(eSeverityComment, "Updating profile ^N", pXmlNodeApiParameters);
		}
	}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TContact::XcpApiContact_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{

	}

//	Serialize a group for the XCP API.
void
TGroup::XcpApiGroup_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_Group_h_s ">", &m_hashGroupIdentifier, (m_uFlagsTreeItem & FTI_kfTreeItemNameDisplayedGenerated) ? NULL : &m_strNameDisplayTyped);
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		if (pMember->m_pContact != pbinXcpStanzaReply->m_pContact)
			{
			pbinXcpStanzaReply->BinAppendText("<" d_szAPIe_GroupMember_);
			pbinXcpStanzaReply->BinAppendXmlAttributeOfContactIdentifier(d_chAPIe_GroupMember_idContact, pMember->m_pContact);	// {ic}, {is}	^i
			pbinXcpStanzaReply->BinAppendText("/>");
			}
		}
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_GroupMember_ " c='^S'" "/>", &m_pAccount->m_strJID);	// Include self in the list
	pbinXcpStanzaReply->BinAppendText("</" d_szAPIe_Group_ ">");
	}

void
TGroup::XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanzaType * pbinXcpApiExtraRequest)
	{
	// Fill in any missing group info from the data in pXmlNodeApiParameters
	if (m_uFlagsTreeItem & FTI_kfTreeItemNameDisplayedGenerated)
		{
		PSZUC pszName = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_Group_strName);
		if (pszName[0] != '\0')
			{
			m_strNameDisplayTyped = pszName;	// Assign the group name
			m_uFlagsTreeItem &= ~FTI_kfTreeItemNameDisplayedGenerated;
			}
		}
	const CXmlNode * pXmlNodeMembers = pXmlNodeApiParameters->PFindElement(d_szAPIe_GroupMember_);
	while (pXmlNodeMembers != NULL)
		{
		Member_PFindOrAllocate_NZ(pXmlNodeMembers->PszuFindAttributeValue(d_chAPIe_GroupMember_idContact));
		//TContact * pContact = m_pAccount->Contact_PFindByIdentifierOrCreate_YZ(pXmlNodeMembers, d_chAPIe_GroupMember_idContact, INOUT pbinXcpApiExtraRequest);
		//MessageLog_AppendTextFormatCo(d_coRed, "XcpApiGroup_ProfileUnserialize() - Adding group member ^j\n", pContact);
		pXmlNodeMembers = pXmlNodeMembers->m_pNextSibling;
		}
	TreeItemChatLog_UpdateTextAndIcon();	// Need to optimize this
	} // XcpApiGroup_ProfileUnserialize()

