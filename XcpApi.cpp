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

#define d_chAPIa_shaGroupIdentifier		'g'

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

const CHU c_szaApi_Synchronize[] = "Sync";

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

void
CBinXcpStanza::XcpApi_ExecuteCore(BOOL fApiRequest, PSZUC pszApiName, const CXmlNode * pXmlNodeApiData)
	{
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Synchronize))
		{
		BinXmlAppendXcpApiSynchronize_Execute(fApiRequest, pXmlNodeApiData);
		return;
		}
	}

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

	/*
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Synchronize))
		{
		BinXmlAppendXcpApiSynchronize_OnReply(pXmlNodeApiParameters);
		return;
		}
	*/
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

	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API '$s'\n", pszApiName);
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
ITreeItemChatLogEvents::XcpApi_Invoke_Synchronize()
	{
	if (EGetRuntimeClass() == RTI(TContact))
		{
		TContact * pContact = (TContact *)this;
		if (pContact->m_cVersionXCP <= 1)
			{
			// Old legacy sync
			CBinXcpStanzaTypeInfo binXcpStanza;
			binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithContact(pContact);
			binXcpStanza.XcpSendStanzaToContact(IN pContact);
			return;
			}
		}
	CBinXcpStanzaTypeSynchronize binXcpStanza(this);
	}

#define d_chAPIe_zNULL								'\0'	// Special value to skip an API

#define d_chAPIa_Sync_EventID						d_chEvent_Attribute_tsEventID

#define d_chAPIa_Sync_tsmTimestamps					't'
#define d_szAPIa_Sync_tsmTimestamps_				" t='"

//	Synchronize operations
#define d_chAPIe_Sync_opRequestIDs					'r'
#define d_szAPIe_Sync_opRequestIDs_tsI_tsO			"r"	_tsI _tsO

#define d_chAPIe_Sync_opEventIDs					'e'
#define d_szAPIe_Sync_opEventIDs_tsO				"e"	_tsO	d_szAPIa_Sync_tsmTimestamps_
#define d_chAPIe_Sync_opEventIDsMine				'm'
#define d_szAPIe_Sync_opEventIDsMine				"m"
#define d_chAPIe_Sync_opEventIDsOthers				'o'
#define d_szAPIe_Sync_opEventIDsOthers_p			"o c='^i'"	d_szAPIa_Sync_tsmTimestamps_
#define d_chAPIa_Sync_pContact							'c'
#define d_chAPIa_Sync_strNameContact					'n'
#define d_szAPIa_Sync_strNameContact					" n=''"	// Include an empty attribute to indicate we want to fetch the name and JID of the contact
#define d_chAPIa_Sync_strJidContact						'j'

//#define d_szAPIe_Sync_opRequestContact_s			"c c='$s'"
//#define d_chAPIe_Sync_opRequestContact				'c'

#define d_chAPIe_Sync_opFetchData_EventIDs			'E'
#define d_szAPIe_Sync_opFetchData_EventIDs_tsI		"E"	_tsI	d_szAPIa_Sync_tsmTimestamps_
#define d_chAPIe_Sync_opFetchData_EventIDsMine		'M'
#define d_szAPIe_Sync_opFetchData_EventIDsMine		"M"			d_szAPIa_Sync_tsmTimestamps_
#define d_chAPIe_Sync_opFetchData_EventIDsOthers	'O'
#define d_szAPIe_Sync_opFetchData_EventIDsOthers_s	"O c='$s'"	d_szAPIa_Sync_tsmTimestamps_

#define d_chAPIe_Sync_opData						'D'
#define d_szAPIe_Sync_opData						"D"
#define d_chAPIe_Sync_opDataMine					'Y'
#define d_szAPIe_Sync_opDataMine					"Y"
#define d_chAPIe_Sync_opDataOther					'T'			// Third party data
#define d_szAPIe_Sync_opDataOther					"T"
#define d_szAPIe_Sync_opDataOther_s					"T c='$s'"
#define d_szAPIe_Sync_opDataOther_s_S_p				"T c='$s' n='^S' j='^j'"	// Include the contact identifier, name and JID

//	Build a request to synchronize with a contact or a group
CBinXcpStanzaTypeSynchronize::CBinXcpStanzaTypeSynchronize(ITreeItemChatLogEvents * pContactOrGroup)
	{
	Assert(pContactOrGroup != NULL);
	(void)pContactOrGroup->Vault_PGet_NZ();	// Open the vault of events.  This is important to be first because opening the vault may update the timestamps
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TContact))
		{
		BinXmlAppendXcpApiRequestOpen((PSZAC)c_szaApi_Synchronize);
		BinXmlAppendXcpApiTimestamps(pContactOrGroup, IN &((TContact *)pContactOrGroup)->m_tsOtherLastSynchronized);
		XcpSendStanzaToContact((TContact *)pContactOrGroup);
		}
	else
		{
		// We have a group, therefore we have to synchronize with every group member
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TGroup));
		BinXmlAppendXcpApiRequestOpenGroup((PSZAC)c_szaApi_Synchronize, (TGroup *)pContactOrGroup);	// Since this request is always the same, make it outside of the loop
		const int cbRequestBegin = m_paData->cbData;
		TGroupMember ** ppMemberStop;
		TGroupMember ** ppMember = ((TGroup *)pContactOrGroup)->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
		while (ppMember != ppMemberStop)
			{
			TGroupMember * pMember = *ppMember++;
			Assert(pMember != NULL);
			Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
			Assert(pMember->m_pGroup == pContactOrGroup);
			Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
			BinXmlAppendXcpApiTimestamps(pContactOrGroup, IN &pMember->m_tsOtherLastSynchronized);
			XcpSendStanzaToContact(IN pMember->m_pContact);
			m_paData->cbData = cbRequestBegin;
			} // while
		} // if...else
	}

void
CBinXcpStanzaTypeSynchronize::BinXmlAppendXcpApiTimestamps(ITreeItemChatLogEvents * pContactOrGroup, const TIMESTAMP * ptsOtherLastSynchronized)
	{
	BinAppendText_VE("<"d_szAPIe_Sync_opRequestIDs_tsI_tsO"/>", *ptsOtherLastSynchronized, pContactOrGroup->m_tsEventIdLastSentCached);
	BinXmlAppendXcpApiRequestClose();
	}

//	Allocate an event and add it to the array.  The array is responsible of deleting the event.
IEvent *
CArrayPtrEvents::PAllocateEvent_YZ(const CXmlNode * pXmlNodeEvent, TIMESTAMP tsEventID, TIMESTAMP tsOther)
	{
	Assert(pXmlNodeEvent != NULL);
	Endorse(tsEventID == d_ts_zNULL);	// This is a new event, and temporary assign the value c_tsMax and later the current date & time
	Assert(tsOther != d_ts_zNULL);
	PSZUC pszEventName = pXmlNodeEvent->m_pszuTagName;
	EEventClass eEventClass = EEventClassFromPsz(pszEventName);
	MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Allocating event '$U' tsEventID $t, tsOther $t\n", eEventClass, tsEventID, tsOther);
	IEvent * pEvent = IEvent::S_PaAllocateEvent_YZ(eEventClass, IN (tsEventID != d_ts_zNULL) ? &tsEventID : &c_tsMax);
	if (pEvent != NULL)
		{
		pEvent->m_tsOther = tsOther;
		Assert(pEvent->m_tsOther != d_ts_zNULL);
		Add(PA_CHILD pEvent);
		}
	else
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Unable to allocate tsEventID $t from stanza because its class ($i) is unrecognized: '$s'\n", tsEventID, eEventClass, pszEventName);
	return pEvent;
	}


void
CBinXcpStanza::BinXmlAppendXcpApiSynchronize_Execute(BOOL fApiRequest, const CXmlNode * pXmlNodeApiData)
	{
	Assert(pXmlNodeApiData != NULL);
	Assert(m_pContact != NULL);
	Assert(m_paData != NULL);
	Assert(m_paData->cbData > 0);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));

	TIMESTAMP * ptsOtherLastSynchronized = &m_pContact->m_tsOtherLastSynchronized;	// When the data was last synchronized with the contact
	WChatLog * pwChatLog = m_pContact->ChatLog_PwGet_YZ();
	CVaultEvents * pVault = m_pContact->Vault_PGet_NZ();	// Get the vault so we may append new events, or fetch the missing events
	TContact * pContactGroupMember = NULL;
	TGroup * pGroup = NULL;
	PSZUC pszGroupIdentifier = pXmlNodeApiData->PszuFindAttributeValue(d_chXCPa_Api_shaGroupID);
	if (pszGroupIdentifier != NULL)
		{
		pGroup = pAccount->Group_PFindByIdentifier_YZ(IN pszGroupIdentifier, INOUT this, TAccountXmpp::eFindGroupCreate);	// Find the group matching the identifier, and if the group is not there, then create it
		if (pGroup == NULL)
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The group identifier '$s' is not valid\n", pszGroupIdentifier);
			return;	// Don't attempt to synchronize with an invalid group identifier
			}
		pContactGroupMember = m_pContact;
		TGroupMember * pMember = pGroup->Member_PFindOrAddContact_NZ(m_pContact);
		Assert(pMember != NULL);
		Assert(pMember->m_pContact == m_pContact);
		ptsOtherLastSynchronized = &pMember->m_tsOtherLastSynchronized;
		pwChatLog = pGroup->ChatLog_PwGet_YZ();
		pVault = pGroup->Vault_PGet_NZ();
		} // if (group)

	PSZUC pszContactIdentifier;
	TContact * pContactOther;
	TIMESTAMP tsOther;
	SOffsets oOffsets;	// Offset where an XML request starts (this variable is used to truncate empty XML requests)
	const CXmlNode * pXmlNodeEvent;

	CArrayPtrEvents arraypaEvents;
	const CXmlNode * pXmlNodeSync = pXmlNodeApiData->m_pElementsList;
	while (pXmlNodeSync != NULL)
		{
		PSZUC pszmTimestamps = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chAPIa_Sync_tsmTimestamps);	// Since most will have a list of timestamps, we fetch them at the beginning of the loop
		CHS chAPI = pXmlNodeSync->m_pszuTagName[0];
		switch (chAPI)
			{
		case d_chAPIe_Sync_opRequestIDs:	// Return all events since tsEventID
			{
			Assert(arraypaEvents.FIsEmpty() && "Memory leak!");
			TIMESTAMP tsEventID = pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chAPIa_Sync_EventID);
			pVault->UGetEventsSinceTimestamp(IN tsEventID, OUT &arraypaEvents);	// Reuse the array for performance
			IEvent ** ppEventStop;
			IEvent ** ppEventFirst = arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
			// Scan the array for sent events
			BinAppendText_VE("<"d_szAPIe_Sync_opEventIDs_tsO, tsEventID);
			IEvent ** ppEvent = ppEventFirst;
			while (ppEvent != ppEventStop)
				{
				IEvent * pEvent = *ppEvent;
				AssertValidEvent(pEvent);
				if (pEvent->Event_FIsEventTypeSent())
					{
					BinAppendTimestampSpace(pEvent->m_tsEventID);
					*ppEvent = NULL;	// Remove the event from the array
					}
				ppEvent++;
				}
			BinAppendXmlForSelfClosingElementQuote();
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opEventIDsMine d_szAPIa_Sync_tsmTimestamps_);
			tsOther = pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
			ppEvent = ppEventFirst;
			while (ppEvent != ppEventStop)
				{
				IEvent * pEvent = *ppEvent;
				if (pEvent != NULL && pEvent->m_pContactGroupSender_YZ == pContactGroupMember)
					{
					if (pEvent->m_tsOther > tsOther)
						BinAppendTimestampSpace(pEvent->m_tsOther);
					*ppEvent = NULL;	// Remove the event from the array
					}
				ppEvent++;
				}
			BinAppendXmlForSelfClosingElementQuoteTruncateAtOffset(IN &oOffsets);
			// We are done with the events related with the contact, now check if there are any third party events (this is the case of group chat)
			TContact * pContactThirdParty = NULL;
			arraypaEvents.RemoveElementAllInstNULL();
			arraypaEvents.GroupEventsBySender();
			ppEvent = arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
			while (ppEvent != ppEventStop)
				{
				IEvent * pEvent = *ppEvent++;
				Assert(pEvent->m_pContactGroupSender_YZ != NULL);	// In a group chat, there is always an explicit sender
				if (pEvent->m_pContactGroupSender_YZ != pContactThirdParty)
					{
					if (pContactThirdParty != NULL)
						BinAppendXmlForSelfClosingElementQuote();
					pContactThirdParty = pEvent->m_pContactGroupSender_YZ;
					BinAppendText_VE("<"d_szAPIe_Sync_opEventIDsOthers_p, pContactThirdParty);
					}
				BinAppendTimestampSpace(pEvent->m_tsOther);
				//MessageLog_AppendTextFormatCo(d_coRed, "Need to include event tsOther $t from ^j\n", pEvent->m_tsOther, pEvent->m_pContactGroupSender_YZ);
				}
			if (pContactThirdParty != NULL)
				BinAppendXmlForSelfClosingElementQuote();
			arraypaEvents.RemoveAllElements();	// Flush the array so the array object may be reused by other sync operations
			}
			break;

		case d_chAPIe_Sync_opEventIDsMine:	// The user is missing its own events
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opFetchData_EventIDsMine);
			while (TRUE)
				{
				TIMESTAMP tsEventID;
				if (!Timestamp_FGetNextTimestamp(OUT_F_UNCH &tsEventID, INOUT &pszmTimestamps))
					break;
				IEvent * pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t Missing my own tsEventID $t\n", tsEventID);
					BinAppendTimestampSpace(tsEventID);
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityNoise, "\t My Event ID $t is already present\n", tsEventID);
					pEvent->Event_SetCompletedAndUpdateChatLog(pwChatLog);	// Mark the event as completed/delivered
					}
				} // while
			BinAppendXmlForSelfClosingElementQuoteTruncateAtOffset(IN &oOffsets);
			break;
		case d_chAPIe_Sync_opEventIDsOthers:	// This XML node contains a list of timestamps of events written by others (third-party group chat)
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chAPIa_Sync_pContact);
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opFetchData_EventIDsOthers_s, pszContactIdentifier);
			pContactOther = pAccount->Contact_PFindByJID(pszContactIdentifier, eFindContact_zDefault);
			if (pContactOther != NULL)
				goto BeginLoop;
			// The contact is unknown, therefore make a request to get its name and JID
			MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "Contact $s is unknown, therefore requesting its name and JID\n", pszContactIdentifier);
			BinAppendText_VE("$s'" d_szAPIa_Sync_strNameContact "/>", pszmTimestamps);	// Include all the timestamps in the request
			break;
		case d_chAPIe_Sync_opEventIDs:	// This XML node contains a list of timestamps of events written by the contact
			pContactOther = pContactGroupMember;
			Endorse(pContactOther == NULL);	// For 1:1 chat
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opFetchData_EventIDs_tsI, *ptsOtherLastSynchronized);	// Build a request to fetch the missing events
			BeginLoop:
			// Loop through all the timestamps to see if we are missing any event.  If an event is missing, then append its timestamp to the request to fetch the event data.
			tsOther = d_ts_zNA;
			while (TRUE)
				{
				if (!Timestamp_FGetNextTimestamp(OUT_F_UNCH &tsOther, INOUT &pszmTimestamps))
					break;
				IEvent * pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactOther);
				if (pEvent == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityInfoTextBlueDark, "\t Missing Event tsOther $t from ^j\n", tsOther, pContactOther);
					BinAppendTimestampSpace(tsOther);	// Add the timestamp to request the data of the missing event
					chAPI = d_chAPIe_zNULL;				// Do not update the synchronization timestamp if an event is missing
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityNoise, "\t Event ID $t (tsOther = $t) from ^j is already present\n", pEvent->m_tsEventID, tsOther, pContactOther);
					}
				} // while
			BinAppendXmlForSelfClosingElementQuoteTruncateAtOffset(IN &oOffsets);
			Endorse(tsOther == d_ts_zNA); // pszmTimestamps was empty.  This is the case when already synchronized
			break;

		case d_chAPIe_Sync_opFetchData_EventIDs:	// Request to return the data of my events (this is the typical case where a contact is missing events)
			BinAppendText_VE("<" d_szAPIe_Sync_opData _tsO ">", pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chAPIa_Sync_EventID));
			while (TRUE)
				{
				TIMESTAMP tsEventID;
				if (!Timestamp_FGetNextTimestamp(OUT_F_UNCH &tsEventID, INOUT &pszmTimestamps))
					break;
				IEvent * pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					BinXmlSerializeEventForXcpCore(pEvent, (pGroup == NULL) ? pEvent->m_tsOther: d_ts_zNA);	// Do NOT include tsOther (the confirmation timestamp) when doing group synchronization
				else
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find Event ID $t\n", tsEventID);
				} // while
			BinAppendText_VE("</" d_szAPIe_Sync_opData ">");
			break;
		case d_chAPIe_Sync_opFetchData_EventIDsMine:	// Return the data of events of the contact (this is somewhat a backup recovery when the contact lost its own Chat Log)
			BinAppendText_VE("<" d_szAPIe_Sync_opDataMine ">");
			while (TRUE)
				{
				if (!Timestamp_FGetNextTimestamp(OUT_F_UNCH &tsOther, INOUT &pszmTimestamps))
					break;
				IEvent * pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactGroupMember);
				if (pEvent != NULL)
					BinXmlSerializeEventForXcpCore(pEvent, pEvent->m_tsOther);
				else
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find Event tsOther $t\n", tsOther);
				} // while
			BinAppendText_VE("</" d_szAPIe_Sync_opDataMine ">");
			break;
		case d_chAPIe_Sync_opFetchData_EventIDsOthers:	// Return the data of events of a third party contact (this happens only in group chat where a contact is relaying/forwarding the events of another contact)
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chAPIa_Sync_pContact);
			pContactOther = pAccount->Contact_PFindByJID(pszContactIdentifier, eFindContact_zDefault);
			Report(pContactOther != NULL);
			if (pContactOther == NULL)
				break;	// Ignore the request if the contact is invalid
			BinAppendText_VE((pXmlNodeSync->PFindAttribute(d_chAPIa_Sync_strNameContact) == NULL) ?
				"<" d_szAPIe_Sync_opDataOther_s ">" :
				"<" d_szAPIe_Sync_opDataOther_s_S_p ">", pszContactIdentifier, &pContactOther->m_strNameDisplayTyped, pContactOther);	// Include the contact name and JID if requested
			while (TRUE)
				{
				pszmTimestamps = Timestamp_PchFromStringSkipWhiteSpaces(OUT &tsOther, IN pszmTimestamps);
				if (tsOther == d_ts_zNULL)
					break;
				IEvent * pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactOther);
				if (pEvent != NULL)
					{
					// Perform a special serialization when forwarding/relaying events written by a third party contact
					EEventClass eEventClass = pEvent->EGetEventClass();	// Reuse the same class
					BinAppendText_VE("<$U" _tsO, eEventClass, tsOther);	// Do NOT include tsEventID
					BinAppendXmlEventCoreDataWithClosingElement(pEvent, eEventClass);
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find Event tsOther $t\n", tsOther);
				} // while
			BinAppendText_VE("</" d_szAPIe_Sync_opDataOther ">");
			break;

		case d_chAPIe_Sync_opDataOther:	// The XML element contains events written by another third-party contact
			Assert(pGroup != NULL && "Third party events are only for group synchronization!");
			if (pGroup == NULL)
				break;
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chAPIa_Sync_pContact);
			pContactOther = pGroup->Member_PFindOrAddContact_YZ(pszContactIdentifier, IN pXmlNodeSync);
			Report(pContactOther != NULL && "The contact JID should be valid");
			if (pContactOther != NULL)
				goto AllocateEvents;
			break;
		case d_chAPIe_Sync_opData:	// The XML element contains data events written by the contact
			pContactOther = pContactGroupMember;
			Endorse(pContactOther == NULL);	// For 1:1 chat
			AllocateEvents:
			tsOther = d_ts_zNA;
			pXmlNodeEvent = pXmlNodeSync->m_pElementsList;
			Assert(pXmlNodeEvent != NULL);
			while (pXmlNodeEvent != NULL)
				{
				tsOther = pXmlNodeEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
				Assert(tsOther > d_tsOther_kmReserved);
				IEvent * pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactOther);
				if (pEvent == NULL)
					{
					pEvent = arraypaEvents.PAllocateEvent_YZ(pXmlNodeEvent, pXmlNodeEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID), tsOther);
					if (pEvent != NULL)
						{
						pEvent->m_pVaultParent_NZ = pVault;	// Assign the parent vault right away
						pEvent->m_pContactGroupSender_YZ = pContactOther;
						pEvent->XmlUnserializeCore(IN pXmlNodeEvent);
						if (pGroup != NULL)
							{
							if (pEvent->m_tsEventID != c_tsMax)
								MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Event tsOther $t from ^j has an invalid tsEventID $t\n", tsOther, pContactOther, pEvent->m_tsEventID);
							}
						}
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Event ID $t, tsOther = $t is already present\n", pEvent->m_tsEventID, tsOther);
					} // if...else
				pXmlNodeEvent = pXmlNodeEvent->m_pNextSibling;
				} // while
			Report(tsOther != d_ts_zNA && "There should be at least one XML element with an event");
			break;
		case d_chAPIe_Sync_opDataMine:	// The XML element contains my own events
			pXmlNodeEvent = pXmlNodeSync->m_pElementsList;
			while (pXmlNodeEvent != NULL)
				{
				TIMESTAMP tsEventID = pXmlNodeEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID);
				Assert(tsEventID > d_tsOther_kmReserved);
				IEvent * pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent == NULL)
					{
					pEvent = arraypaEvents.PAllocateEvent_YZ(pXmlNodeEvent, tsEventID, pXmlNodeEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther));
					if (pEvent != NULL)
						{
						pEvent->m_pVaultParent_NZ = pVault;	// Assign the parent vault right away
						pEvent->XmlUnserializeCore(IN pXmlNodeEvent);
						}
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Event ID $t is already present\n", tsEventID);
					}
				pXmlNodeEvent = pXmlNodeEvent->m_pNextSibling;
				} // while
			break;
		default:
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown sync operation: $b\n", chAPI);
			} // switch

		// Update the timestamp for the two operations
		switch (chAPI)
			{
		case d_chAPIe_Sync_opEventIDs:
		case d_chAPIe_Sync_opData:
			if ((tsOther > *ptsOtherLastSynchronized) && (*ptsOtherLastSynchronized == pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther)))
				{
				MessageLog_AppendTextFormatCo(d_coGreenDarker, "\t\t\t Updating m_tsOtherLastSynchronized from $t to $t\n", *ptsOtherLastSynchronized, tsOther);
				*ptsOtherLastSynchronized = tsOther;
				}
			} // switch

		pXmlNodeSync = pXmlNodeSync->m_pNextSibling;
		} // while

	// So far, we have allocated the events, however they have not been displayed into the Chat Log, nor added to the vault
	arraypaEvents.SortEventsByChronology();	// First, sort by chronology, to display the events according to their timestamp
	// Next, assign the current date & time to those events not having a timestamp
	IEvent ** ppEventStop;
	IEvent ** ppEvent = arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		if (pEvent->m_tsEventID == c_tsMax)
			pEvent->m_tsEventID = Timestamp_GetCurrentDateTime();	// Assign the current date & time to new events
		PSZUC pszExtra = NULL;
		switch (pEvent->EGetEventClass())
			{
		case CEventMessageTextSent::c_eEventClass:
		case CEventMessageTextReceived::c_eEventClass:
			pszExtra = ((IEventMessageText *)pEvent)->m_strMessageText;
		default:
			break;
			}
		MessageLog_AppendTextFormatCo(d_coBlue, "tsEventID $t ({tL}), tsOther $t ({tL}): $s\n", pEvent->m_tsEventID, pEvent->m_tsEventID, pEvent->m_tsOther, pEvent->m_tsOther, pszExtra);
		} // while
	pVault->m_arraypaEvents.AppendEventsSortedByIDs(PA_CHILD &arraypaEvents);
	Assert(pVault->m_arraypaEvents.FEventsSortedByIDs());

	if (pwChatLog != NULL)
		{
		//arraypaEvents.SortEventsByChronology();
		pwChatLog->ChatLog_EventsDisplay(IN arraypaEvents);
		}

	// Finally, add the events to the vault
	} // BinXmlAppendXcpApiSynchronize_Execute()

/*
void
CBinXcpStanza::BinXmlAppendXcpApiSynchronize_OnRequest(const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(m_pContact != NULL);
	BinXmlAppendXcpElementForApiRequest_ElementOpen(c_szaApi_Synchronize);
	CVaultEvents * pVault = m_pContact->Vault_PGet_NZ();
	CXmlNode * pXmlNodeSent = pXmlNodeApiParameters->PFindElement('s');
	if (pXmlNodeSent != NULL)
		{
		TIMESTAMP tsEventID = pXmlNodeSent->TsGetAttributeValueTimestamp_ML('i');
		// Return all the timestamps of sent events since tsEventID
		BinAppendText_VE("<s o='$t'/>", tsEventID);
		}
	BinXmlAppendXcpElementForApiRequest_ElementClose();
	}

void
CBinXcpStanza::BinXmlAppendXcpApiSynchronize_OnReply(const CXmlNode * pXmlNodeApiParameters)
	{

	}
*/

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
	pbinXcpStanzaReply->BinAppendText_VE("<" d_szAPIe_TGroup_h_str ">", &m_hashGroupIdentifier, (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated) ? NULL : &m_strNameDisplayTyped);
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
	pbinXcpStanzaReply->BinAppendText_VE("<" d_szAPIe_TGroupMember_ " c='^S'" "/>", &m_pAccount->m_strJID);	// Include self in the list
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
