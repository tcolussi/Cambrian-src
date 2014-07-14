///////////////////////////////////////////////////////////////////////////////////////////////////
//	XcpApi.cpp
//
//	Implementation of "Personal APIs" (PAPI) available through the Cambrian Protocol.
//
//	The content of this file could be within Xcp.cpp, however since file Xcp.cpp already has 1500 lines,
//	it is easier to understand the code related to the Cambrian APIs in a dedicated file.
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
	Contact.Recommendations.Get							- Return all the recommendations from the contact (other contacts, groups, links, etc)
	Contact.Wallet.GetAddress [idContact], strCurrency, strInvoiceNumber		// Return an address for payment

	Me.Profile.Get
	Me.Wallet.GetAddress
#endif

const CHU c_szaApi_Group_Profile_Get[] = "Group.Profile.Get";

#define d_chAPIe_TGroup_					'G'
#define d_szAPIe_TGroup_					"G"
#define d_szAPIe_TGroup_h_str				"G i='{h|}' n='^S'"
	#define d_chAPIa_TGroup_shaIdentifier	'i'
	#define d_chAPIa_TGroup_strName			'n'
#define d_chAPIe_TGroupMember_				'M'
#define d_szAPIe_TGroupMember_				"M"
	#define d_chAPIa_TGroupMember_idContact	'c'	// Perhaps rename to 'i'

const CHU c_szaApi_Contact_Recommendations_Get[] = "Contact.Recommendations.Get";

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method to serialize the data for an XCP API request.
void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
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
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Contact_Recommendations_Get))
		{
		pAccount->m_pProfileParent->XcpApiProfile_RecommendationsSerialize(INOUT this); // Return all the recommendations related to the profile
		return;
		}
	} // BinXmlAppendXcpElementForApiRequest_AppendApiParameterData()


//	Method to unserialize the data from BinXmlAppendXcpElementForApiRequest_AppendApiParameterData().
//	Although the reply typically does not need the object CBinXcpStanza, the reply may necessitate the CBinXcpStanza to make additional request.
//	For instance, when unserializing a group, the method may need to query information about new group members to fetch the contact profile.
void
CBinXcpStanza::BinXmlAppendXcpElementForApiReply(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
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
		PSZUC pszGroupIdentifier = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_TGroup_shaIdentifier);
		//MessageLog_AppendTextFormatCo(d_coRed, "BinXmlAppendXcpElementForApiReply($s) - Group '$s'\n", pszApiName, pszGroupIdentifier);
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszGroupIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileUnserialize(IN pXmlNodeApiParameters, INOUT this);
			}
		return;
		}
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Contact_Recommendations_Get))
		{
		// We received new recommendations from the contact
		const_cast<CXmlNode *>(pXmlNodeApiParameters)->RemoveEmptyElements();
		m_pContact->Contact_RecommendationsUpdateFromXml(IN pXmlNodeApiParameters);
		return;
		}

	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API: $s\n^N", pszApiName, pXmlNodeApiParameters);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
CBinXcpStanza::BinXmlAppendXcpApiRequest_Group_Profile_Get(PSZUC pszGroupIdentifier)
	{
	BinXmlAppendXcpApiRequest((PSZAC)c_szaApi_Group_Profile_Get, pszGroupIdentifier);
	}

//	Ask a contact to return the profile of a group
void
TGroup::XcpApiGroup_Profile_GetFromContact(TContact * pContact)
	{
	CHU szGroupIdentifier[30];
	InitToGarbage(OUT szGroupIdentifier, sizeof(szGroupIdentifier));
	HashSha1_ToStringBase85(OUT szGroupIdentifier, IN &m_hashGroupIdentifier);
	CBinXcpStanzaTypeInfo binXcpStanza;
	binXcpStanza.BinXmlAppendXcpApiRequest_Group_Profile_Get(szGroupIdentifier);
	binXcpStanza.XcpSendStanzaToContact(pContact);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ITreeItemChatLogEvents::XcpApi_Invoke_RecommendationsGet()
	{
	XcpApi_Invoke(c_szaApi_Contact_Recommendations_Get, d_zNA, d_zNA);
	}

void
TContact::XcpApiContact_ProfileSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const
	{

	}

//	Serialize a group for the XCP API.
void
TGroup::XcpApiGroup_ProfileSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const
	{
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_TGroup_h_str ">", &m_hashGroupIdentifier, (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated) ? NULL : &m_strNameDisplayTyped);
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		if (pMember->m_pContact != pbinXcpStanzaReply->m_pContact)
			{
			pbinXcpStanzaReply->BinAppendText("<" d_szAPIe_TGroupMember_);
			pbinXcpStanzaReply->BinAppendXmlAttributeOfContactIdentifier(d_chAPIa_TGroupMember_idContact, pMember->m_pContact);	// {ic}, {is}	^i
			pbinXcpStanzaReply->BinAppendText("/>");
			}
		}
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_TGroupMember_ " c='^S'" "/>", &m_pAccount->m_strJID);	// Include self in the list
	pbinXcpStanzaReply->BinAppendText("</" d_szAPIe_TGroup_ ">");
	}

void
ITreeItem::TreeItem_SetNameDisplaySuggested(PSZUC pszNameDisplay)
	{
	m_strNameDisplayTyped = pszNameDisplay;
	m_uFlagsTreeItem = (m_uFlagsTreeItem & ~FTI_kfTreeItem_NameDisplayedGenerated) | FTI_kfTreeItem_NameDisplayedSuggested;
	}

void
TGroup::XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanza * pbinXcpApiExtraRequest)
	{
	// Fill in any missing group info from the data in pXmlNodeApiParameters
	if (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated)
		{
		PSZUC pszName = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_TGroup_strName);
		if (pszName[0] != '\0')
			TreeItem_SetNameDisplaySuggested(pszName);	// Assign the group name
		}
	const CXmlNode * pXmlNodeMembers = pXmlNodeApiParameters->PFindElement(d_chAPIe_TGroupMember_);
	while (pXmlNodeMembers != NULL)
		{
		Member_PFindOrAllocate_NZ(pXmlNodeMembers->PszuFindAttributeValue(d_chAPIa_TGroupMember_idContact));
		//TContact * pContact = m_pAccount->Contact_PFindByIdentifierOrCreate_YZ(pXmlNodeMembers, d_chAPIe_GroupMember_idContact, INOUT pbinXcpApiExtraRequest);
		//MessageLog_AppendTextFormatCo(d_coRed, "XcpApiGroup_ProfileUnserialize() - Adding group member ^j\n", pContact);
		pXmlNodeMembers = pXmlNodeMembers->m_pNextSibling;
		}
	TreeItemChatLog_UpdateTextAndIcon();	// Need to optimize this
	} // XcpApiGroup_ProfileUnserialize()
