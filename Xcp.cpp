///////////////////////////////////////////////////////////////////////////////////////////////////
//	Xcp.cpp
//
//	Collection of functions and methods related to the XCP (eXtensible Cambrian Protocol) as well as the event serialization/unserialization for XCP.

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "Xcp.h"

//	The XCP protocol uses lowercase and uppercase letters:
//	A lowercase letter designates an xcp-stanza (a stanza within an xmpp-stanza) where there could be a reply.
//	An uppercase letter designates a reply to an xcp-stanza.  There is never a reply to a reply, otherwise it may cause an infinite loop.
//	The tsOther and tsEventID 'make sense' for the receiver of the xcp-stanza; it is the responsibility of the sender to swap those values accordingly.

#define d_chXCPe_GroupSelector								'g'	// Which group to assign the events.  The body of the XML element is the identifier of the group.
#define d_szXCPe_GroupSelector						d_szXCP_"g"

#define d_chXCPe_EventPrevious								'p'	// Send the timestamp of the previous event, as well as the synchronization
#define d_szXCPe_EventPrevious_tsO_tsI				d_szXCP_"p"	_tsO _tsI
	#define d_szXCPa_EventPrevious_cEventsMissing				" c='$i'"
	#define d_chXCPa_EventPrevious_cEventsMissing				'c'
#define d_chXCPe_EventsMissing								'M'	// Notification to the remote client some events are missing, and therefore should synchronize
#define d_szXCPe_EventsMissing						d_szXCP_"M"

#define d_chXCPe_EventNextRequest							'n'	// Ask the remote client to send its next event after tsEventID
#define d_szXCPe_EventNextRequest_tsI				d_szXCP_"n"	_tsI
#define d_chXCPe_EventNextReply								'N'	// Reply to the sender regarding the request of tsEventID and tsOther (the following xcp-stanzas contains serialized IEvents)
#define d_szXCPe_EventNextReply_tsO					d_szXCP_"N" _tsO
#define d_chXCPe_EventsOutstandingReply						'O'	// Any message reply 'N' must end with 'O' to indicate hoe many outstanding IEvents remain.
#define d_szXCPe_EventsOutstandingReply_i_tsO		d_szXCP_"O c='$i'" _tsO
	#define d_chXCPa_EventsOutstandingReply_cEvents				'c'

#define d_chXCPe_EventConfirmation							'C'	// Confirmation an event was processed successfully (the confirmation only includes tsEventID of the sender)
#define d_szXCPe_EventConfirmation_tsI				d_szXCP_"C" _tsI
#define d_chXCPe_EventError									'E'	// The event was received, however could not be processed because its class/type is unknown. 'C' and 'E' are mutually exclusive.
#define d_szXCPe_EventError_tsI_s					d_szXCP_"E" _tsI " e='^s'"	// e=event, c=code of the error (if any), t=friendly text to display to the user (if any)

#define d_chXCPe_EventForwardRequest						'f'	// Ask the remote client to forward a group event sent by someone else.  This is somewhat similar to d_chXCPe_EventNextRequest, however since forwarded events are not synchronized, Cambrian cannot use d_chXCPe_EventNextRequest.
#define d_szXCPe_EventForwardRequest_tsI			d_szXCP_"f"	_tsI
#define d_chXCPe_EventForwardReply							'F'
#define d_szXCPe_EventForwardReply_tsO				d_szXCP_"F" _tsO
#define d_chXCPe_EventsForwardRemaining						'R'
#define d_szXCPe_EventsForwardRemaining_i_tsO		d_szXCP_"R c='$i'" _tsO
	#define d_chXCPa_EventsForwardRemaining_cEvents				'c'

#define d_chXCPe_EventExtraDataRequest						'x'	// Request an event to send extra data (for instance, downloading the content of a file)
#define d_szXCPe_EventExtraDataRequest_tsI			d_szXCP_"x" _tsI
#define d_chXCPe_EventExtraDataReply						'X'	// Reply from the event with the data payload
#define d_szXCPe_EventExtraDataReply				d_szXCP_"X"
#define d_szXCPe_EventExtraDataReply_tsO			d_szXCP_"X" _tsO

	#define d_chXCPa_EventExtraData_strxIdentifier				'I'		// Identifier of the extra data (this is optional, as sometimes there is only one extra data)
	#define d_chXCPa_EventExtraData_iblOffset					'O'		// Offset of the extra data (this is important, because the extra data is sent by smaller pieced of about 4 KiB)
	#define d_szXCPa_EventExtraData_iblOffset_i					" O='$i'"	// 32 bit offset
	#define d_szXCPa_EventExtraData_iblOffset_l					" O='$l'"	// 64 bit offset
	#define d_chXCPa_EventExtraData_bin85Payload				'b'			// Binary payload
	#define d_szXCPa_EventExtraData_bin85Payload_pvcb			" b='{p|}'"
	#define d_chXCPa_EventExtraData_shaData						'h'	// SHA-1 of the data (to ensure its integrity)
	#define d_szXCPa_EventExtraData_shaData_h					" h='{h|}'"

#define d_chXCPe_EventSplitDataRequest						's'
#define d_szXCPe_EventSplitDataRequest_tsI_i		d_szXCP_"s" _tsI d_szXCPa_EventExtraData_iblOffset_i
#define d_chXCPe_EventSplitDataReply						'S'
#define d_szXCPe_EventSplitDataReply_tsO_i_pvcb		d_szXCP_"S" _tsO d_szXCPa_EventExtraData_iblOffset_i d_szXCPa_EventExtraData_bin85Payload_pvcb


#define d_szXCPe_Api_bs								d_szXCP_"$b n='$s'"	// Generic formatting of an API
#define d_szXCPe_Api_b								d_szXCP_"$b"

#define d_chXCPe_ApiRequest									'a'
#define d_szXCPe_ApiRequest_s						d_szXCP_"a n='$s'"
#define d_szXCPe_ApiRequest_close					d_szXCP_"a"
	#define d_chXCPa_Api_strName								'n'		// Name of the function (API) to call
#define d_chXCPe_ApiReply									'A'
#define d_szXCPe_ApiReply_s							d_szXCP_"A n='$s'"
#define d_szXCPe_ApiReply_close						d_szXCP_"A"
	#define d_chXCPa_Api_eErrorCode								'e'	// Store the error code EErrorXcpApi
	#define d_chXCPa_Api_strxErrorData							'd'
	#define d_szXCPa_ErrorCodeAndErrorData_i_s					" e='$i' d='^s'"

//	Tasks have only one identifier (there is no tsOther) for tasks
#define d_chXCPe_TaskDownloading							't'
#define d_szXCPe_TaskDownloading_tsI				d_szXCP_"t" _tsI
#define d_chXCPe_TaskSending								'T'
#define d_szXCPe_TaskSending_tsI					d_szXCP_"T" _tsI

	#define d_chXCPa_TaskDataSizeTotal							's'			// Total size of the task's data
	#define d_szXCPa_TaskDataSizeTotal_i							" s='$i'"
	#define d_chXCPa_TaskDataOffset									'O'
	#define d_szXCPa_TaskDataOffset_i								" O='$i'"
	#define d_chXCPa_TaskDataBinary									'b'			// Chunk of data
	#define d_szXCPa_TaskDataBinary_Bii								" b='{o|}'"
//	#define d_szXCPa_TaskDataBinary_pvcb							" b='{p|}'"

#if 0
	// API call
	<_a r='identifier' n='function'/>parameters</_a>
	<_A r='identifier' e='error (if any)'>result</_A>

#endif
#define d_chXCPe_zNA									'\0'	// Not applicable

/*
XcpApi_OnRequest()
XcpApi_OnReply()
*/

//	Core method to execute an XCP API
void
CBinXcpStanza::XcpApi_Execute(const CXmlNode * pXmlNodeApiData)
	{
	Assert(pXmlNodeApiData != NULL);
	Assert(pXmlNodeApiData->m_pszuTagName != NULL);
	Assert(pXmlNodeApiData->m_pszuTagName[0] == d_chXCP_);
	const int ibXmlApiResponseBegin = (m_paData != NULL) ? m_paData->cbData : 0;	// Offset where the API data is being appended

	PSZUC pszApiName = pXmlNodeApiData->PszuFindAttributeValue_NZ(d_chXCPa_Api_strName);
	const CHS chXCPe_ApiResponse = Ch_ToOtherCase(pXmlNodeApiData->m_pszuTagName[1]);	// The response is the opposite of what received
	Assert(chXCPe_ApiResponse == d_chXCPe_ApiRequest || chXCPe_ApiResponse == d_chXCPe_ApiReply);

	BOOL fApiRequest = (chXCPe_ApiResponse != d_chXCPe_ApiRequest);
	BinAppendTextSzv_VE("<" d_szXCPe_Api_bs ">", chXCPe_ApiResponse, pszApiName);
	Assert(m_ibXmlApiReply == d_zNA);
	m_ibXmlApiReply = m_paData->cbData;

	XcpApi_ExecuteCore(fApiRequest, pszApiName, pXmlNodeApiData);

	if (m_ibXmlApiReply > 0)
		{
		if (!fApiRequest)
			{
			// We have received a reply to our request.  Unless there is real data, there is no need to reply to a reply
			if (m_paData->cbData <= m_ibXmlApiReply)
				{
				m_paData->cbData = ibXmlApiResponseBegin;
				m_ibXmlApiReply = d_zNA;
				return;
				}
			}
		BinAppendTextSzv_VE("</" d_szXCPe_Api_b ">", chXCPe_ApiResponse);	// Close the XML request
		m_ibXmlApiReply = d_zNA;
		}

	/*
	if (pXmlNodeApiData->m_pszuTagName[1] == d_chXCPe_ApiRequest)
		{
		// Respond to the request for a given API name
		BinXmlAppendXcpElementForApiRequest_ElementOpen(IN pszApiName);
		BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(IN pszApiName, IN pXmlNodeApiData);
		BinXmlAppendXcpElementForApiRequest_ElementClose();
		}
	else
		{
		Assert(pXmlNodeApiData->m_pszuTagName[1] == d_chXCPe_ApiReply);
		if (pXmlNodeApiData->m_pElementsList != NULL)
			BinXmlAppendXcpElementForApiReply(IN pszApiName, IN pXmlNodeApiData->m_pElementsList);
		}
	// Check if the response can fit in a single XMPP stanza
	if (m_paData == NULL)
		return;	// No data
	*/
	if (m_paData->cbData > 100)
		{
		// The XAPI data is too large to fit in a single XMPP stanza, therefore create a task to transmit it into smaller chunks
		CTaskSend * paTask = new CTaskSend;
		paTask->m_binData.BinAppendCBinFromOffset(IN *this, ibXmlApiResponseBegin);
		m_paData->cbData = ibXmlApiResponseBegin;	// Remove the data from the object
		m_pContact->m_listTasksSocket.InsertNodeAtHead(INOUT paTask);
		MessageLog_AppendTextFormatCo(d_coGreenDarker, "Creating Task ID '$t' of $I bytes:\n$B\n", paTask->m_tsTaskID, paTask->m_binData.CbGetData(), &paTask->m_binData);
		BinAppendTextSzv_VE("<" d_szXCPe_TaskDownloading_tsI d_szXCPa_TaskDataSizeTotal_i d_szXCPa_TaskDataBinary_Bii "/>",
			paTask->m_tsTaskID, paTask->m_binData.CbGetData(), &paTask->m_binData, 0, c_cbStanzaMaxBinary / 100);
		}
	} // XcpApi_Execute()

void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_ElementOpen(PSZUC pszApiName)
	{
	Assert(m_ibXmlApiReply == d_zNA);
	BinAppendTextSzv_VE("<" d_szXCPe_ApiReply_s ">", pszApiName);	// Open the XML tag
	m_ibXmlApiReply = m_paData->cbData;
	Assert(XcpApi_FIsXmlElementOpened());
	}

void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_ElementClose()
	{
	if (m_ibXmlApiReply <= 0)
		return;
	BinAppendText("</" d_szXCPe_ApiReply_close ">");	// Close the XML tag
	m_ibXmlApiReply = d_zNA;
	}

void
CBinXcpStanza::BinXmlAppendXcpAttributesForApiRequestError(EErrorXcpApi eErrorXcpApi, PSZUC pszxErrorData)
	{
	Assert(eErrorXcpApi != eErrorXcpApi_zSuccess);
	if (m_ibXmlApiReply <= 0)
		return;
	Assert(m_paData != NULL);
	m_paData->cbData = m_ibXmlApiReply - 1;		// Remove the tailing '>' from BinXmlAppendXcpElementApiReplyOpen()
	Assert(m_paData->rgbData[m_paData->cbData] == '>');
	BinAppendTextSzv_VE(d_szXCPa_ErrorCodeAndErrorData_i_s ">", eErrorXcpApi, pszxErrorData);
	m_ibXmlApiReply = d_zNA;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method of the Cambrian Protocol.
//
//	This method is related to a contact receiving an XCP stanza.
//	Depending on the context, the events may be assigned to the contact, or the group where the contact is member.

void
TContact::Xcp_ProcessStanzasAndUnserializeEvents(const CXmlNode * pXmlNodeXcpEvent)
	{
	Assert(pXmlNodeXcpEvent != NULL);
	PSZUC pszNameDisplay = TreeItem_PszGetNameDisplay();		// To help debugging
	ITreeItemChatLogEvents * pChatLogEvents = this;				// Which Chat Log to display the new events (by default, it is the contact receiving the stanza, however it may be the group)
	WChatLog * pwChatLog = ChatLog_PwGet_YZ();
	CVaultEvents * pVault = Vault_PGet_NZ();					// By default, use the vault of the contact.  If the message is for a group, then change the vault and the synchronization timestamps.
	TIMESTAMP * ptsOtherLastSynchronized = &m_tsOtherLastSynchronized;	// When the data was last synchronized with the remote contact
	MessageLog_AppendTextFormatCo(d_coBlack, "Xcp_ProcessStanzasAndUnserializeEvents(Account: $s, Contact: $s - $S)\n\t m_tsOtherLastSynchronized=$t, m_tsEventIdLastSentCached=$t\n",
		m_pAccount->TreeItem_PszGetNameDisplay(), pszNameDisplay, &m_strJidBare, m_tsOtherLastSynchronized, m_tsEventIdLastSentCached);
	CBinXcpStanzaTypeInfo binXcpStanzaReply;	// What stanza to send to the remote client as a response to pXmlNodeXcpEvent
	binXcpStanzaReply.m_pContact = this;
	int cbXcpStanzaReplyEmpty = 0;				// What bytes are considered 'empty' within the binXcpStanzaReply.  This variable is necessary because of group chat, where the group identifier must be store within the reply, however if nothing else was stored, then the reply is considered empty.
	int cEventsRemaining;
	PSZUC pszEventValue;

	BOOL fEventsOutOfSyncInChatLog = FALSE;			// The events in the Chat Log are out of sync and therefore display a special icon
	TIMESTAMP_DELTA dtsOtherSynchronization = 1;	// By default, assume the events are NOT synchronized, therefore assign to the variable a non-zero value.
	CHS chXCPe = d_chXCPe_zNA;			// Last known XCP element
	IEvent * pEvent = NULL;
	TGroup * pGroup = NULL;				// Pointer to the selected group
	TGroupMember * pMember = NULL;		// Pointer to the contact within the selected group
	while (pXmlNodeXcpEvent != NULL)
		{
		Report(pXmlNodeXcpEvent->m_pszuTagName != NULL);
		MessageLog_AppendTextFormatCo((pXmlNodeXcpEvent->m_pszuTagName[0] == d_chXCP_ && pXmlNodeXcpEvent->m_pszuTagName[1] == d_chXCPe_EventError) ? COX_MakeBold(d_coRed) : d_coBlack, "\t Processing $s ^N", fEventsOutOfSyncInChatLog ? "(out of sync)" : NULL, pXmlNodeXcpEvent);
		// Get both timestamps, as most xcp-stanzas have both
		TIMESTAMP tsEventID = pXmlNodeXcpEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID);
		TIMESTAMP tsOther = pXmlNodeXcpEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
		PSZUC pszEventName = pXmlNodeXcpEvent->m_pszuTagName;
		CHS chEventName0 = pszEventName[0];
		if (chEventName0 == d_chXCP_)
			{
			// We have a XCP control node which contains directives how to interpret the remaining xcp-stanzas/event(s). Some xcp-stanzas (control nodes) are at the beginning of the xmpp-stanza, while others are at the end.
			chXCPe = pszEventName[1];
			switch (chXCPe)
				{
			case d_chXCPe_GroupSelector:
				// The events belong to a group, so attempt to find the group
				Report(pGroup == NULL);
				Report(pMember == NULL);
				pszEventValue = pXmlNodeXcpEvent->m_pszuTagValue;
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_GroupSelector ">^s</" d_szXCPe_GroupSelector ">", pszEventValue);
				cbXcpStanzaReplyEmpty = binXcpStanzaReply.CbGetData();
				pGroup = m_pAccount->Group_PFindByIdentifier_YZ(IN pszEventValue, INOUT &binXcpStanzaReply, TAccountXmpp::eFindGroupCreate);	// Find the group matching the identifier, and if not there create it
				if (pGroup == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Group identifier '$s' is not valid\n", pszEventValue);
					break;
					}
				pChatLogEvents = pGroup;
				pMember = pGroup->Member_PFindOrAddContact_NZ(this);
				Assert(pMember != NULL);
				Assert(pMember->m_pContact == this);
				ptsOtherLastSynchronized = &pMember->m_tsOtherLastSynchronized;
				pVault = pChatLogEvents->Vault_PGet_NZ();
				pwChatLog = pChatLogEvents->ChatLog_PwGet_YZ();
				MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t Selecting group '$s' (m_tsOtherLastSynchronized=$t, m_tsEventIdLastSentCached=$t)\n", pGroup->TreeItem_PszGetNameDisplay(), *ptsOtherLastSynchronized, pGroup->m_tsEventIdLastSentCached);
				break;
			case d_chXCPe_EventPrevious:	// We are receiving information regarding the previous message
				dtsOtherSynchronization = tsOther - *ptsOtherLastSynchronized;
				if (dtsOtherSynchronization > 0)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t m_tsOtherLastSynchronized = $t however its value should be $t\n"
						"\t\t\t therefore requesting $s to resend its next event after $t\n", *ptsOtherLastSynchronized, tsOther, pszNameDisplay, *ptsOtherLastSynchronized);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventNextRequest_tsI "/>", *ptsOtherLastSynchronized);
					}
				else if (dtsOtherSynchronization < 0)
					{
					Assert(tsOther < *ptsOtherLastSynchronized);
					// The most common scenario for this code path is when the Chat Log has been deleted.  The timestamps from config.xml remain, however those timestamps do no longer represent the actual events.
					// Before notifying the contact it is missing its own messages, atempt to verify the integrity of our own timestamps.
					pEvent = pVault->PFindEventReceivedByTimestampOther(*ptsOtherLastSynchronized, this);
					if (pEvent != NULL)
						{
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsOther $t < m_tsOtherLastSynchronized $t\n"
							"\t\t\t therefore $s is missing ITS OWN messages!  As a result, send the necessary info to notify $s about the situation.\n", tsOther, *ptsOtherLastSynchronized, pszNameDisplay, pszNameDisplay);
						binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsMissing "/>");
						}
					else
						{
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsOther $t < m_tsOtherLastSynchronized $t\n"
							"\t\t\t however since there is no event matching $t, we adjust m_tsOtherLastSynchronized of $s from $t to $t\n", tsOther, *ptsOtherLastSynchronized, *ptsOtherLastSynchronized, pszNameDisplay, *ptsOtherLastSynchronized, tsOther);
						*ptsOtherLastSynchronized = tsOther;
						dtsOtherSynchronization = 0;	// We are now synchronized!
						}
					}
				else
					{
					Assert(dtsOtherSynchronization == 0);	// The timestamps do synchronize
					// For group chat, check if we are not missing messages from other group members
					int cEventsReceivedByOthers = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_EventPrevious_cEventsMissing);
					if (cEventsReceivedByOthers > 0)
						{
						Assert(tsOther > d_tsOther_kmReserved);
						int cEventsReceivedInChatLog = pVault->UCountEventsReceivedByOtherGroupMembersSinceTimestampOther(tsOther);
						if (cEventsReceivedInChatLog < cEventsReceivedByOthers)
							{
							MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Apparently I am missing $I events written by other group members\n"
								"\t\t\t therefore asking $s to forward other's messages since tsOther $t\n", cEventsReceivedByOthers - cEventsReceivedInChatLog, pszNameDisplay, tsOther);
							binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventForwardRequest_tsI "/>", tsOther);
							break;
							}
						}

					if (tsEventID < pChatLogEvents->m_tsEventIdLastSentCached)
						{
						MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t tsEventID $t < m_tsEventIdLastSentCached $t\n"
							"\t\t\t therefore $s is missing my messages!\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached, pszNameDisplay);
						binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsMissing "/>");
						}
					else if (tsEventID > pChatLogEvents->m_tsEventIdLastSentCached)
						{
						// I have no idea what this situation means, so I am reporting it on the Message Log
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsEventID $t > m_tsEventIdLastSentCached $t\n\t\t Apparently $s has messages from ME that I don't have.\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached, pszNameDisplay);
						}
					} // if...else
				break;
			case d_chXCPe_EventsMissing:
				Assert(tsOther == d_ts_zNA);
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Apparently I am missing events (I am out of sync)\n\t\t therefore requesting $s to resend the next event(s) after $t\n", pszNameDisplay, *ptsOtherLastSynchronized);
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventNextRequest_tsI "/>", *ptsOtherLastSynchronized);
				break;
			case d_chXCPe_EventNextRequest:
				// We are receiving a request to resend the next event following tsEventID
				Endorse(tsEventID == d_ts_zNULL);	// Get the first event
				Assert(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventNext(tsEventID, OUT &cEventsRemaining);
				Assert(pEvent == NULL || pEvent->EGetEventClass() != eEventClass_eNull);
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Resending next event following tsEventID $t (cEventsRemaining = $i)\n", tsEventID, cEventsRemaining);
				if (pEvent != NULL)
					{
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventNextReply_tsO "/>", tsEventID);
					binXcpStanzaReply.BinXmlSerializeEventForXcp(pEvent);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsOutstandingReply_i_tsO "/>", cEventsRemaining, pEvent->m_tsEventID);
					}
				break;
			case d_chXCPe_EventNextReply:	// Although d_chXCPe_EventNextReply may appear as useless, the variable chXCPe is set to d_chXCPe_EventNextReply which will be useful when unserializing events.
				Report(tsEventID == d_ts_zNA);
				dtsOtherSynchronization = tsOther - *ptsOtherLastSynchronized;
				if (dtsOtherSynchronization == 0)
					{
					MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t chXCPe = d_chXCPe_EventNextReply;\n");
					break;
					}
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t chXCPe = d_chXCPe_EventNextReply; (out of sync by $T)\n\t\t therefore ignoring remaining stanza(s): ^N\n", dtsOtherSynchronization, pXmlNodeXcpEvent->m_pNextSibling);
				goto Exit;	// If we are out of sync, it is pointless to continue the synchronization
			case d_chXCPe_EventsOutstandingReply:
				Report(tsEventID == d_ts_zNA);
				Report(tsOther != d_ts_zNULL);	// Typically tsOther is the same as pEvent->m_tsOther (the last event we processed), however not always the case as events may have been received out-of-sync
				if (pEvent == NULL)
					{
					// This is not necessary, however it is good documentation to indicate there was a previous event processed, although the event may unknown, and therefore pEvent may be NULL without being an error.
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t d_chXCPe_EventsOutstandingReply with pEvent == NULL\n");
					}
				cEventsRemaining = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_EventsOutstandingReply_cEvents);
				if (cEventsRemaining > 0)
					{
					StatusBar_SetTextFormat("Downloading messages from $s... $I remaining...", pszNameDisplay, cEventsRemaining);
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Requesting $s to resend the next $I events after $t\n", pszNameDisplay, cEventsRemaining, tsOther);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventNextRequest_tsI "/>", tsOther);
					break;
					}
				// There are no more outstanding events to download, so do cleanup and send the last known timestamps to give an opportunity to the remote client to download its missing messages (if any)
				StatusBar_ClearText();
				if (pMember != NULL)
					binXcpStanzaReply.BinXmlAppendTimestampsToSynchronizeWithGroupMember(pMember);
				else
					binXcpStanzaReply.BinXmlAppendTimestampsToSynchronizeWithContact(this);
				break;
			case d_chXCPe_EventForwardRequest:
				// We are receiving a request to forward an event written by another contact
				pEvent = pVault->PFindEventNextReceivedByOtherGroupMembers(tsEventID, this, OUT &cEventsRemaining);
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Forwarding the next event following tsEventID $t (cEventsRemaining = $i)\n", tsEventID, cEventsRemaining);
				if (pEvent != NULL)
					{
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventForwardReply_tsO "/>", tsEventID);
					binXcpStanzaReply.BinXmlSerializeEventForXcp(pEvent);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsForwardRemaining_i_tsO "/>", cEventsRemaining, pEvent->m_tsEventID);
					}
				break;
			case d_chXCPe_EventForwardReply:
				// There is nothing to do  here, except set chXCPe = d_chXCPe_EventForwardReply
				MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t chXCPe = d_chXCPe_EventForwardReply;\n");
				break;
			case d_chXCPe_EventsForwardRemaining:
				cEventsRemaining = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_EventsForwardRemaining_cEvents);
				if (cEventsRemaining > 0)
					{
					StatusBar_SetTextFormat("Downloading messages from $s... $I remaining...", pszNameDisplay, cEventsRemaining);
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Requesting $s to resend the next $I events after $t\n", pszNameDisplay, cEventsRemaining, tsOther);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventForwardRequest_tsI "/>", tsOther);
					break;
					}
				// There are no more outstanding events to download, so do cleanup and send the last known timestamps to give an opportunity to the remote client to download its missing messages (if any)
				StatusBar_ClearText();
				break;
			case d_chXCPe_EventConfirmation:
			case d_chXCPe_EventError:
				Report(tsEventID != d_ts_zNULL);
				Report(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					if (chXCPe == d_chXCPe_EventError)
						pEvent->Event_SetFlagErrorProtocol();
					pEvent->Event_SetCompletedAndUpdateChatLog(pwChatLog);	// This will typically display a green checkmark at the right of the screen where the event is located
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unrecognized confirmation for EventID $t\n", tsEventID);
				break;
			case d_chXCPe_EventExtraDataRequest:
				Report(tsEventID > d_tsOther_kmReserved);
				Report(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					pEvent->XcpExtraDataRequest(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply);
					pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);	// This will typically show a progress bar
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find EventID $t for d_chXCPe_EventExtraDataRequest (group $s)\n", tsEventID, (pGroup == NULL) ? NULL : pGroup->TreeItem_PszGetNameDisplay());
				break;
			case d_chXCPe_EventExtraDataReply:
				// A 'data reply' may occur if there is no 'request', therefore we need to check for tsEventID as well as tsOther
				if (tsEventID != d_ts_zNULL)
					{
					Assert(tsEventID > d_tsOther_kmReserved);
					Assert(tsOther == d_ts_zNA);
					pEvent = pVault->PFindEventByID(tsEventID);
					}
				else
					{
					Assert(tsOther > d_tsOther_kmReserved);
					pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pMember);
					}
				if (pEvent != NULL)
					{
					pEvent->XcpExtraDataArrived(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply);
					pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);	// This will typically show a progress bar
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find i=$t, o=$t for d_chXCPe_EventExtraDataReply\n", tsEventID, tsOther);
				break;
			case d_chXCPe_EventSplitDataRequest:
				// Send a chunk of a large data
				Assert(tsEventID > d_tsOther_kmReserved);
				Assert(tsOther == d_ts_zNA);
				{
				int ibDataSource = pXmlNodeXcpEvent->LFindAttributeXcpOffset();
				CDataXmlLargeEvent * pDataXmlLargeEvent = pVault->PFindOrAllocateDataXmlLargeEvent_NZ(tsEventID, IN_MOD_TMP &binXcpStanzaReply);
				int cbDataRemaining;
				#if 1
					int cbStanzaMaxBinary = 1 + pDataXmlLargeEvent->m_binXmlData.CbGetData() / 4;	// At the moment, send only 1 byte + 25% at the time (rather than c_cbStanzaMaxBinary), so we can test the code transmitting large events
					if (cbStanzaMaxBinary > CBinXcpStanza::c_cbStanzaMaxBinary)
						cbStanzaMaxBinary = CBinXcpStanza::c_cbStanzaMaxBinary;
				#else
					#define cbStanzaMaxBinary	CBinXcpStanza::c_cbStanzaMaxBinary
				#endif
				int cbData = pDataXmlLargeEvent->m_binXmlData.CbGetDataAfterOffset(ibDataSource, cbStanzaMaxBinary, OUT &cbDataRemaining);
				MessageLog_AppendTextFormatCo(d_coOrange, "Sending $i/$i bytes data from offset $i for tsEventID $t, cbDataRemaining = $i\n", cbData, cbStanzaMaxBinary, ibDataSource, tsEventID, cbDataRemaining);
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventSplitDataReply_tsO_i_pvcb , tsEventID, ibDataSource, pDataXmlLargeEvent->m_binXmlData.PvGetDataAtOffset(ibDataSource), cbData);
				if (cbDataRemaining <= 0)
					{
					// We reached the end of the XML data, therefore send the hash
					//pDataXmlLargeEvent->m_hashXmlData.rgbData[0] = 0;	// Corrupt the data to see how the protocol handles it
					binXcpStanzaReply.BinAppendTextSzv_VE(d_szXCPa_EventExtraData_shaData_h, IN &pDataXmlLargeEvent->m_hashXmlData);
					}
				binXcpStanzaReply.BinAppendXmlForSelfClosingElement();
				#if 0
				List_DetachNode(INOUT pDataXmlLargeEvent);
				delete pDataXmlLargeEvent;	// Destroy the cache, so we can test how stable is the code
				#endif
				}
				break;
			case d_chXCPe_EventSplitDataReply:
				// New data has arrived
				Assert(tsOther > d_tsOther_kmReserved);
				Assert(tsEventID == d_ts_zNA);
				pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pMember);
				if (pEvent != NULL && pEvent->EGetEventClass() == CEventDownloader::c_eEventClass)
					((CEventDownloader *)pEvent)->XcpDownloadedDataArrived(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply, pwChatLog);
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find CEventDownloader matching tsOther $t for d_chXCPe_EventSplitDataReply\n", tsOther);
				break;
			case d_chXCPe_ApiRequest:
			case d_chXCPe_ApiReply:
				Assert(binXcpStanzaReply.m_pContact == this);
				if (binXcpStanzaReply.m_pContact == this)
					binXcpStanzaReply.XcpApi_Execute(IN pXmlNodeXcpEvent);
				break;
			/*
				Assert(binXcpStanzaReply.m_pContact == this);
				if (binXcpStanzaReply.m_pContact == this)
					{
					PSZUC pszApiName = pXmlNodeXcpEvent->PszuFindAttributeValue_NZ(d_chXCPa_Api_strName);
					if (pXmlNodeXcpEvent->m_pElementsList != NULL)
						binXcpStanzaReply.BinXmlAppendXcpElementForApiReply(IN pszApiName, IN pXmlNodeXcpEvent->m_pElementsList);
					}
				break;
			*/
			case d_chXCPe_TaskDownloading:
				{
				// Create a task to download the remaining data
				CTaskReceive * pTask = (CTaskReceive *)m_listTasksSocket.PFindTaskByID(tsEventID, eTaskClass_CTaskReceive);
				if (pTask == NULL)
					{
					pTask = new CTaskReceive(IN &tsEventID);
					pTask->m_cbTotal = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_TaskDataSizeTotal);
					m_listTasksSocket.InsertNodeAtHead(INOUT PA_CHILD pTask);
					}
				Assert(pTask->m_cbTotal > 0);
				const int ibData =  pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_TaskDataOffset);
				Assert(ibData < pTask->m_cbTotal);
				if (ibData == pTask->m_binData.CbGetData())
					{
					const int cbDataNew = pTask->m_binData.BinAppendBinaryDataFromBase85SCb_ML(pXmlNodeXcpEvent->PszuFindAttributeValue(d_chXCPa_TaskDataBinary));
					const int cbDataReceived = ibData + cbDataNew;
					Assert(cbDataReceived == pTask->m_binData.CbGetData());
					if (cbDataReceived == pTask->m_cbTotal)
						{
						// We successfully downloaded the task, therefore execute it
						MessageLog_AppendTextFormatCo(d_coGreenDarker, "Download complete ($I bytes) for Task ID '$t':\n$B\n", pTask->m_binData.CbGetData(), pTask->m_tsTaskID, &pTask->m_binData);
						CXmlTree oXmlTree;
						oXmlTree.EParseFileDataToXmlNodesModify_ML(INOUT &pTask->m_binData);
						binXcpStanzaReply.XcpApi_Execute(IN &oXmlTree);
						m_listTasksSocket.DeleteTask(PA_DELETING pTask);
						}
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_TaskSending_tsI d_szXCPa_TaskDataOffset_i "/>", tsEventID, cbDataReceived);	// Send a request to download the remaining data (if any), or to indicate all the data was received and therefore delete the task
					break;
					}
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Ignoring data from Task ID '$t' because its offset ($I) does not match the received data\n", tsEventID, ibData);
				}
				break;
			case d_chXCPe_TaskSending:
				{
				// Send the remaining of the data for the task
				CTaskSend * pTask = (CTaskSend *)m_listTasksSocket.PFindTaskByID(tsEventID, eTaskClass_CTaskSend);
				if (pTask != NULL)
					{
					#if 1
						int cbStanzaMaxBinary = 1 + pTask->m_binData.CbGetData() / 4;	// At the moment, send only 1 byte + 25% at the time (rather than c_cbStanzaMaxBinary), so we can test the code transmitting large events
						if (cbStanzaMaxBinary > CBinXcpStanza::c_cbStanzaMaxBinary)
							cbStanzaMaxBinary = CBinXcpStanza::c_cbStanzaMaxBinary;
					#else
						#define cbStanzaMaxBinary	CBinXcpStanza::c_cbStanzaMaxBinary
					#endif
					const int ibDataRequest = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_TaskDataOffset);
					if (ibDataRequest < pTask->m_binData.CbGetData())
						binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_TaskDownloading_tsI d_szXCPa_TaskDataOffset_i d_szXCPa_TaskDataBinary_Bii "/>", tsEventID, ibDataRequest, &pTask->m_binData, ibDataRequest, cbStanzaMaxBinary);
					else
						m_listTasksSocket.DeleteTask(PA_DELETING pTask);	// The request is equal (or larger) than the data, meaning all the data was downloaded, therefore the task is no longer needed
					break;
					}
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Unable to find CTaskSend matching Task ID '$t'\n", tsEventID);
				}
				break;
			default:
				MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unknown XCP directive $s: chXCPe = $i ($b)\n", pszEventName, chXCPe, chXCPe);
				} // switch (chXCPe)
			}
		else
			{
			// The XML node contains an event of type EEventClass
			const BOOL fEventClassReceived = FEventClassReceived(chEventName0);
			EEventClass eEventClass = EEventClassFromPsz(pszEventName);
			switch (eEventClass)
				{
			case eEventClass_eMessageTextComposing:	// The contact is typing something
				pChatLogEvents->ChatLog_ChatStateIconUpdate((pXmlNodeXcpEvent->PFindAttribute(d_chXCPa_MessageTextComposing_State) == NULL) ? eChatState_zComposing : eChatState_fPaused, this);
				goto EventNext;
			case eEventClass_ePing:
				// The XCP ping returns the timestamp (in UTC) of the contact.  This way, it is possible to calculate the clock difference between the two devices.  Ideally the clock difference should be less than one minute.
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataReply _tsI d_szXCPa_PingTime_t "/>", tsOther, Timestamp_GetCurrentDateTime());
				goto EventNext;
			case eEventClass_eVersion:
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataReply _tsI d_szXCPa_eVersion_Version d_szXCPa_eVersion_Platform d_szXCPa_eVersion_Client "/>", tsOther);
				goto EventNext;
			case eEventClass_eDownloader:	// The downloader is a hybrid event, part of the Cambrian Protocol, and another part a regular event.
				/*
				if (pwChatLog == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t No Chat Log present for CEventDownloader tsOther $t, therefore initiating download without GUI...\n", tsOther);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventSplitDataRequest_tsI_i "/>", tsOther, 0);	// If there is no Chat Log present, then request the data to start a download.
					}
				*/
				break;
			default:
				;		// Keep the compiler happy to prevent a warning the switch() statement does not handle all cases
				} // switch
			Assert(tsOther > d_tsOther_kmReserved);		// Any received event should have a valid tsOther
			BOOL fSwapTimestamps = FALSE;				// Swap tsEventID and tsOther.  This happens when a group event is forwarded by another contact
			TContact * pContactGroupSender = m_pAccount->Contact_PFindByIdentifierGroupSender_YZ(pXmlNodeXcpEvent);
			Endorse(pContactGroupSender == NULL);	// The message is for a 1-to-1 conversation
			if (pContactGroupSender == NULL)
				{
				if (fEventClassReceived && pGroup != NULL)
					pContactGroupSender = this;
				}
			else if (pContactGroupSender != this)
				{
				Assert(fEventClassReceived);
				Assert(tsEventID > d_tsOther_kmReserved);
				fSwapTimestamps = TRUE;
				pEvent = pVault->PFindEventReceivedByTimestampOther(tsEventID, pContactGroupSender);	// Search the event using swapped timestamps
				Assert(pGroup != NULL);
				if (pGroup != NULL)
					(void)pGroup->Member_PFindOrAddContact_NZ(pContactGroupSender);		// Make sure the contact is part of the group
				goto EventBegin;
				}
			pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactGroupSender);	// Regardless of what type of event we received, always attempt to find it the current vault (which may be the vault of the contact or the vault of the group) and matching the contact of the group sender (if any)
			Endorse(pEvent == NULL);
			EventBegin:
			MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t pContactGroupSender = ^j, fSwapTimestamps = $s\n", pContactGroupSender, fSwapTimestamps ? "TRUE" : "FALSE");
			if (chXCPe != d_chXCPe_EventNextReply && chXCPe != d_chXCPe_EventForwardReply)
				{
				// We are receiving new events from the remote client
				Assert(tsEventID == d_ts_zNULL);						// New events should not have a tsEventID
				binXcpStanzaReply.m_eStanzaType = eStanzaType_eMessage;	// Any event confirmation (or error) shall be cached as an 'XMPP message' so the remote client may know about it
				if (pwChatLog != NULL)
					//pwChatLog->ChatLog_ChatStateComposerRemove(this);	// Make sure the text "<user> is typing..." will be no longer displayed
					pwChatLog->ChatLog_ChatStateIconUpdate(this, eChatState_fPaused);	// Make sure the text "<user> is typing..." will be no longer displayed
				if (pEvent != NULL)
					{
					// The new event is already in the Chag Log.  This is the case when the event is a re-send.
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Updating existing tsEventID $t matching tsOther $t written by ^j\n", pEvent->m_tsEventID, tsOther, pContactGroupSender);
					goto EventUnserialize;
					}
				}
			else
				{
				// We are synchronizing by receiving previous events
				Assert(tsEventID == d_ts_zNULL || tsEventID > d_tsOther_kmReserved);	// The EventID may be zero for the case of receiving an event which was never delivered before
				if (pEvent != NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Updating existing tsEventID $t matching tsOther $t written by ^j\n", pEvent->m_tsEventID, tsOther, pContactGroupSender);
					goto EventUnserialize;
					}
				// If we are unable to find the event by tsOther, attempt to find it by its identifier
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Updating existing tsEventID $t written by ^j\n", tsEventID, pContactGroupSender);
					EventUnserialize:
					Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
					if (fEventClassReceived)
						{
						// Only unserialize existing 'received' events.  This is a security feature, so someone does not attempt to hijack our history log by pretending to resend our own events.
						Assert(pEvent->Event_FIsEventTypeReceived());
						pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);	// TODO: Need to provide a mechanism if an event has been updated
						pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);
						}
					goto EventConfirmation;
					}
				} // if...else

			// We have a new event to allocate and to display into the Chat Log.
			Assert(pEvent == NULL);
			MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Allocating event '$U' tsEventID $t written by ^j\n", eEventClass, tsEventID, pContactGroupSender);
			pEvent = IEvent::S_PaAllocateEvent_YZ(eEventClass, (tsEventID == d_ts_zNULL) ? NULL : &tsEventID);
			if (pEvent == NULL)
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Unable to allocate event tsOther $t from stanza because its class ($i) is unrecognized: '$s'\n", tsOther, eEventClass, pszEventName);
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventError_tsI_s "/>", tsOther, pszEventName);	// Report the error to the remote client
				goto EventSynchronize;
				}
			Assert(pEvent->m_pVaultParent_NZ == NULL);
			pEvent->m_pVaultParent_NZ = pVault;	// Assign the parent vault right away
			#ifdef d_szEventDebug_strContactSource
			if (pGroup != NULL)
				pEvent->m_strDebugContactSource = m_strJidBare;
			#endif
			#ifdef d_szEventDebug_strVersion
			pEvent->m_strDebugVersion = (PSZUC)d_szApplicationVersion;
			#endif
			#ifdef DEBUG
			if (USZU_from_USZUF(pEvent->EGetEventClass()) != eEventClass)
				MessageLog_AppendTextFormatCo(d_coBlueDark, "\t\t\t Event tsOther $t was allocated as '$U' however its runtime class is '$U'\n", tsOther, eEventClass, pEvent->EGetEventClass());
			if (pEvent->Event_FIsEventTypeSent() && pContactGroupSender != NULL)
				MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t\t tsEventID $t, tsOther $t should NOT have a pContactGroupSender ^j\n", pEvent->m_tsEventID, tsOther, pContactGroupSender);
			#endif
			if (!fSwapTimestamps)
				pEvent->m_tsOther = tsOther;
			else
				{
				pEvent->m_tsOther = pEvent->m_tsEventID;
				pEvent->m_tsEventID = tsOther;
				}
			pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);
			Assert(pEvent->m_pContactGroupSender_YZ == NULL);
			if (fEventClassReceived)
				{
				if (!pEvent->Event_FIsEventTypeReceived())
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t\t Event tsEventID $t, tsOther $t of class '$U' should be RECEIVED\n", pEvent->m_tsEventID, pEvent->m_tsOther, eEventClass);
					}
				if (pContactGroupSender != NULL)
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t Assigning m_pContactGroupSender_YZ ^j to tsEventID $t, tsOther $t\n", pContactGroupSender, pEvent->m_tsEventID, pEvent->m_tsOther);
				pEvent->m_pContactGroupSender_YZ = pContactGroupSender;
				}
			fEventsOutOfSyncInChatLog |= pVault->m_arraypaEvents.Event_FoosAddSorted(PA_CHILD pEvent);
			if (fEventsOutOfSyncInChatLog)
				pEvent->Event_SetFlagOutOfSync();
			if (pwChatLog != NULL)
				pwChatLog->ChatLog_EventDisplay(IN pEvent);

			// Update the GUI about the new event
			pChatLogEvents->TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(IN pEvent->PszGetTextOfEventForSystemTray(OUT_IGNORED &g_strScratchBufferStatusBar), this, pMember);
			TreeItem_IconUpdate();		// Update the icon of the contact, which will in turn update the icon(s) of all its aliases, including the member contact of the group.  It is important to update the icon of the contact because it is likely to be displaying the pencil icon indicating the user was composing/typing text.

			if (chXCPe == d_chXCPe_EventForwardReply)
				goto EventNext;		// Don't attempt to synchronize a forwarded event

			EventConfirmation:
			binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventConfirmation_tsI "/>", tsOther);	// Acknowledge to the remote client we received (and processed) the event
			EventSynchronize:
			if (dtsOtherSynchronization == 0)
				{
				//Assert(tsOther > *ptsOtherLastSynchronized);
				if (tsOther > *ptsOtherLastSynchronized)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t Updating m_tsOtherLastSynchronized from $t to $t\n", *ptsOtherLastSynchronized, tsOther);
					*ptsOtherLastSynchronized = tsOther;
					}
				else if (tsOther < *ptsOtherLastSynchronized)
					{
					// I am not sure what this situation means (perhaps editing an existing event)
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "tsOther $t  <  m_tsOtherLastSynchronized $t\n", tsOther, *ptsOtherLastSynchronized);
					}
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Out of sync by $T\n", dtsOtherSynchronization);
				}
			Endorse(pEvent == NULL);	// This happens when an event cannot becreated, typically because its class is unknown
			//Assert(pEvent != NULL);
			if (pEvent != NULL && pEvent->Event_FIsEventTypeSent())
				{
				// Adjust the timestamp if the event we processed 'was' our own event.  This happens when synchronizing where the remote client re-send all the events, including our own
				if (tsEventID > pChatLogEvents->m_tsEventIdLastSentCached)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t Updating m_tsEventIdLastSentCached from $t to $t\n", pChatLogEvents->m_tsEventIdLastSentCached, tsEventID);
					pChatLogEvents->m_tsEventIdLastSentCached = tsEventID;
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t tsEventID $t  <=  m_tsEventIdLastSentCached $t\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached);
				}
			} // if...else

		EventNext:
		pXmlNodeXcpEvent = pXmlNodeXcpEvent->m_pNextSibling;	// Get the next event/xcp-stanza
		} // while
	Exit:
	if (binXcpStanzaReply.CbGetData() > cbXcpStanzaReplyEmpty)
		binXcpStanzaReply.XcpSendStanzaToContact(IN this);
	Assert(pVault->m_arraypaEvents.FEventsSortedByIDs());
	} // Xcp_ProcessStanzasAndUnserializeEvents()

void
CBinXcpStanza::BinXmlAppendXcpApiRequestOpen(PSZAC pszApiName)
	{
	Assert(pszApiName != NULL);
	BinAppendTextSzv_VE("<" d_szXCPe_ApiRequest_s ">", pszApiName);
	}

void
CBinXcpStanza::BinXmlAppendXcpApiRequestClose()
	{
	BinAppendText("</" d_szXCPe_ApiRequest_close ">");
	}

void
CBinXcpStanza::BinXmlAppendXcpApiRequest(PSZAC pszApiName, PSZUC pszXmlApiParameters)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "BinXmlAppendXcpApiRequest($s, $s)\n", pszApiName, pszXmlApiParameters);
	BinXmlAppendXcpApiRequestOpen(pszApiName);
	if (pszXmlApiParameters != NULL)
		BinAppendText((PSZAC)pszXmlApiParameters);
	BinXmlAppendXcpApiRequestClose();
	//BinAppendTextSzv_VE("<" d_szXCPe_ApiRequest_s ">$s</" d_szXCPe_ApiRequest_close ">", pszApiName, pszXmlApiParameters);
	}

//	Invoke a function by sending an API request to the contact.  This is essentially invoking a remote procedure call.
void
ITreeItemChatLogEvents::XcpApi_Invoke(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters, PSZUC pszXmlApiParameters)
	{
	CBinXcpStanzaTypeInfo binXcpStanza;
	binXcpStanza.BinXmlAppendXcpApiRequest((PSZAC)pszApiName, pszXmlApiParameters);
	binXcpStanza.XcpSendStanzaToContactOrGroup(IN this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Method to serialize the event to be saved on disk.
void
CBinXcpStanza::BinXmlSerializeEventForDisk(const IEvent * pEvent)
	{
	EEventClass eEventClass = pEvent->EGetEventClass();
	if (eEventClass == CEventDownloader::c_eEventClass)
		{
		// We have a 'downloader', and therefore we have to perform some processing to decide if we save the downloader, or if we save the downloaded event
		IEvent * pEventDownloaded = ((CEventDownloader *)pEvent)->m_paEvent;
		if (pEventDownloaded != NULL)
			{
			if (pEventDownloaded->m_uFlagsEvent & IEvent::FE_kfEventProtocolError)
				return;	// Don't save the downloader if there is a protocol error.  This is to prevent the downloader to attempt to download something that no longer exist
			// Substitute the class of the downloader by the class of the allocated event
			eEventClass = pEventDownloaded->EGetEventClass();	//	The downloaded event will be serialized as a native object and next time Cambrian will load, the downloader will no longer exist.
			pEvent = pEventDownloaded;
			}
		}
	if ((eEventClass & eEventClass_kfNeverSerializeToDisk) == 0)
		{
		BinAppendTextSzv_VE("<$U" _tsI _tsO, eEventClass, pEvent->m_tsEventID, pEvent->m_tsOther);
		BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
		#ifdef d_szEventDebug_strContactSource
		BinAppendXmlAttributeText(d_szEventDebug_strContactSource, pEvent->m_strDebugContactSource);
		#endif
		#ifdef d_szEventDebug_strVersion
		BinAppendXmlAttributeText(d_szEventDebug_strVersion, pEvent->m_strDebugVersion);
		#endif
		BinAppendXmlEventCoreDataWithClosingElement(pEvent, eEventClass);
		/*
		EXml eXml = pEvent->XmlSerializeCoreE(IOUT this);
		if (eXml == eXml_zAttributesOnly)
			BinAppendXmlForSelfClosingElement();
		else
			BinAppendTextSzv_VE("</$U>\n", eEventClass);
		*/
		}
	} // BinXmlSerializeEventForDisk()

void
CBinXcpStanza::BinAppendXmlEventCoreDataWithClosingElement(const IEvent * pEvent, EEventClass eEventClass)
	{
	Assert(pEvent != NULL);
	EXml eXml = pEvent->XmlSerializeCoreE(IOUT this);
	if (eXml == eXml_zAttributesOnly)
		BinAppendXmlForSelfClosingElement();
	else
		BinAppendTextSzv_VE("</$U>\n", eEventClass);
	}

//	Core method to serialize an event to be transmitted through the Cambrian Protocol.
//
//	This method is 'core' because it is shared with the 'serializer' as well the method re-creating the cache for the 'downloader'.
//	The class CEventDownloader relies on the number of bytes serialized by this method to transmit the event.
//	The parameter tsOther is either zero or the value of pEvent->m_tsOther.  The reason tsOther is a parameter is because
//	the downloader does not serialize this value, so the cached data is always the same.
void
CBinXcpStanza::BinXmlSerializeEventForXcpCore(const IEvent * pEvent, TIMESTAMP tsOther)
	{
	Assert(pEvent != NULL);
	Assert(tsOther == d_ts_zNULL || tsOther == pEvent->m_tsOther);
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	EEventClass eEventClassXcp = pEvent->EGetEventClassForXCP();
	TIMESTAMP tsEventID = pEvent->m_tsEventID;
	if (pEvent->m_pContactGroupSender_YZ != NULL)
		{
		// The event was received by a contact for a group chat, therefore we need to adjust the class
		if (!pEvent->Event_FIsEventTypeReceived())
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "BinXmlSerializeEventForXcp() - tsEventID $t, tsOther $t written by ^j class '$U' is NOT RECEIVED\n", tsEventID, tsOther, pEvent->m_pContactGroupSender_YZ, eEventClassXcp);
			}
		if (pEvent->m_pContactGroupSender_YZ != m_pContact)
			{
			eEventClassXcp = pEvent->EGetEventClass();	// Keep the same class.  Idetally the timestamps should be kept the same, however this will screw up the synchronization, so it is better to keep them as is and swap them by the receiver
			MessageLog_AppendTextFormatCo(d_coRed, "\t\t BinXmlSerializeEventForXcp() - tsEventID $t, tsOther $t written by ^j is forwarded to ^j, therefore keeping event class '$U'\n", tsEventID, tsOther, pEvent->m_pContactGroupSender_YZ, m_pContact, eEventClassXcp);
			}
		Assert(pEvent->m_tsOther > d_tsOther_kmReserved && "A received event should always have a valid tsOther");
		}
	BinAppendTextSzv_VE((tsOther > d_tsOther_kmReserved) ? "<$U" _tsO _tsI : "<$U" _tsO, eEventClassXcp, tsEventID, tsOther);	// Send the content of m_tsOther only if it contains a valid timestamp
	BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
	BinAppendXmlEventCoreDataWithClosingElement(pEvent, eEventClassXcp);
	/*
	EXml eXml = pEvent->XmlSerializeCoreE(IOUT this);
	if (eXml == eXml_zAttributesOnly)
		BinAppendXmlForSelfClosingElement();
	else
		BinAppendTextSzv_VE("</$U>\n", eEventClassXcp);
	*/
	} // BinXmlSerializeEventForXcpCore()


//	Method to serialize an event to be transmitted through the Cambrian Protocol.
//	The difference between saving an event to disk and for XCP, is the eEventClass must be adjusted for the recipient as well as swapping m_tsEventID and m_tsOther,
//	because any event 'sent' becomes an event 'received' by the remote contact.  This design decision was made, so the remote client unserialize the received events the same way as unserializing events from disk.
//
//	Also, this method ensures a large event will be sent in smaller chunks, so it may be transmitted through the XMPP protocol.
void
CBinXcpStanza::BinXmlSerializeEventForXcp(const IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	Assert(pEvent->m_pVaultParent_NZ != NULL);
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_paData	!= NULL);
	EEventClass eEventClassXcp = pEvent->EGetEventClassForXCP();
	if ((eEventClassXcp & eEventClass_kfNeverSerializeToXCP) == 0)
		{
		const int ibDataElementStart = m_paData->cbData;
		Assert(ibDataElementStart >= 0);
		/*
		TIMESTAMP tsEventID = pEvent->m_tsEventID;
		TIMESTAMP tsOther = pEvent->m_tsOther;
		if (pEvent->m_pContactGroupSender_YZ != NULL)
			{
			// The event was received by a contact for a group chat, therefore we need to adjust the class
			if (pEvent->m_pContactGroupSender_YZ != m_pContact)
				{
				eEventClassXcp = pEvent->EGetEventClass();	// Keep the same class.  Idetally the timestamps should be kept the same, however this will screw up the synchronization, so it is better to keep them as is and swap them by the receiver
				MessageLog_AppendTextFormatCo(d_coRed, "\t\t BinXmlSerializeEventForXcp() - tsEventID $t, tsOther $t written by ^j is forwarded to ^j, therefore keeping event class '$U'\n", tsEventID, tsOther, pEvent->m_pContactGroupSender_YZ, m_pContact, eEventClassXcp);
				}
			Assert(tsOther > d_tsOther_kmReserved && "A received event should always have a valid tsOther");
			}
		BinAppendTextSzv_VE((tsOther > d_tsOther_kmReserved) ? "<$U" _tsO _tsI : "<$U" _tsO, eEventClassXcp, tsEventID, tsOther);	// Send the content of m_tsOther only if it contains a valid timestamp
		BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
		pEvent->XmlSerializeCore(IOUT this);
		*/
		TIMESTAMP tsOther = pEvent->m_tsOther;
		BinXmlSerializeEventForXcpCore(pEvent, tsOther);
		if (m_pContact == NULL)
			return;		// The virtual method XmlSerializeCore() may set m_pContact to NULL if the contact does not support XCP.
		Assert(m_paData != NULL);
		#ifdef DEBUG
		if (m_paData->cbData > 100)		// For debugging conider 100 characters as a 'large stanza' to force this code to be executed frequently
		#else
		if (m_paData->cbData > 150)		// Also test this code in the release build, however with a higher threshold
		//if (m_paData->cbData > c_cbStanzaMaxPayload)
		#endif
			{
			// The XML data is too large, therefore use a 'downloader event' which will take care of transferring the data in smaller chunks.
			// At this point it is tempting to create a cache of the XML data, however this approach is unreliable because the XML main contain m_tsOther which may change and screw up the synchronization process.
			// It is better to let the method PFindOrAllocateDataXmlLargeEvent_NZ() take care of it.
			MessageLog_AppendTextFormatCo(COX_MakeBold(d_coOrange), "BinXmlSerializeEventForXcp() - Large stanza of about $I bytes\n", m_paData->cbData);
			m_paData->cbData = ibDataElementStart;	// Truncate the binary, so send the CEventDownloader instead of the event
			CDataXmlLargeEvent * pDataXmlLargeEvent = pEvent->m_pVaultParent_NZ->PFindOrAllocateDataXmlLargeEvent_NZ(pEvent->m_tsEventID, IN_MOD_TMP this);	// Create the cache, so we can get its size
			Assert(m_paData->cbData == ibDataElementStart);
			// Serialize the CEventDownloader on-the-fly
			BinAppendTextSzv_VE("<" d_szXCPe_CEventDownloader_i_tsO, pDataXmlLargeEvent->m_binXmlData.CbGetData(), pEvent->m_tsEventID);
			if (tsOther > d_tsOther_kmReserved)
				BinAppendTextSzv_VE(d_szXCPa_CEventDownloader_tsForwarded_t, tsOther);
			BinAppendXmlForSelfClosingElement();
			}
		}
	} // BinXmlSerializeEventForXcp()

//	Send the event through XMPP.  If the contact can understand the Cambrian Protocol, then the event will be serialized for XCP, ortherwise will be sent throught the standard XMPP.
//
void
IEvent::Event_WriteToSocket()
	{
	Assert(m_pVaultParent_NZ != NULL);
	Assert(m_tsOther == d_tsOther_ezEventNeverSent);
	if (m_tsOther != d_tsOther_ezEventNeverSent)
		return;
	m_tsOther = d_tsOther_eEventSentOnce;
	CBinXcpStanzaTypeMessage binXcpStanza;			// By default, events are sent as XMPP 'messages'
	ITreeItemChatLogEvents * pContactOrGroup = m_pVaultParent_NZ->m_pParent;
	Assert(pContactOrGroup != NULL);
	TGroup * pGroup = (TGroup *)pContactOrGroup;
	TContact * pContact = (TContact *)pContactOrGroup;
	if (pContact->EGetRuntimeClass() == RTI(TContact))
		{
		// Send the message to a contact
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGrayDark), "\t Sending message to $S\n\t\t m_tsEventIdLastSentCached $t, m_tsOtherLastSynchronized $t\n", &pContact->m_strJidBare, pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
		binXcpStanza.m_pContact = pContact;
		binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithContact(pContact);
		binXcpStanza.BinXmlSerializeEventForXcp(IN this);
		binXcpStanza.XcpSendStanza();
		}
	else
		{
		// Broadcast the message to every [active] group member
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		TGroupMember ** ppMemberStop;
		TGroupMember ** ppMember = pGroup->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
		while (ppMember != ppMemberStop)
			{
			TGroupMember * pMember = *ppMember++;
			Assert(pMember != NULL);
			Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
			binXcpStanza.m_pContact = pMember->m_pContact;
			if (binXcpStanza.m_pContact->m_cVersionXCP <= 0)
				{
				MessageLog_AppendTextFormatSev(eSeverityNoise, "Skipping group contact $S because its client does not support XCP (probably because it is offline)\n", &pMember->m_pContact->m_strJidBare);
				continue;
				}
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Sending message to group member $S\n", &pMember->m_pContact->m_strJidBare);
			binXcpStanza.BinXmlInitStanzaWithGroupSelector(pGroup);	// This line could be removed out of the loop
			binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithGroupMember(pMember);
			binXcpStanza.BinXmlSerializeEventForXcp(IN this);
			binXcpStanza.XcpSendStanza();	// Send the XCP stanza to the contact
			} // while
		} // if...else
	if ((EGetEventClass() & eEventClass_kfNeverSerializeToDisk) == 0)
		{
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGrayDark), "\t\t Updating m_tsEventIdLastSentCached from $t to $t\n", pContactOrGroup->m_tsEventIdLastSentCached, m_tsEventID);
		pContactOrGroup->m_tsEventIdLastSentCached = m_tsEventID;
		}
	m_pVaultParent_NZ->SetModified();	// This line is important so m_tsOther (as well as other timestamps) are saved to disk
	} // Event_WriteToSocket()

//	Send the event if the socket is ready
void
IEvent::Event_WriteToSocketIfReady()
	{
	if (PGetSocketOnlyIfReady() != NULL)
		Event_WriteToSocket();
	}

void
CArrayPtrEvents::EventsSerializeForDisk(INOUT CBinXcpStanza * pbinXmlEvents) const
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		pbinXmlEvents->BinXmlSerializeEventForDisk(IN pEvent);
		} // while
	} // EventsSerializeForDisk()

void
CArrayPtrEvents::EventsUnserializeFromDisk(const CXmlNode * pXmlNodeEvent, ITreeItemChatLogEvents * pParent)
	{
	Endorse(pXmlNodeEvent == NULL);
	Assert(pParent != NULL);
	CVaultEvents * pVault = pParent->Vault_PGet_NZ();
	BOOL fOutOfSync = FALSE;	// One of the event is out-of-sync
	TIMESTAMP tsEventLargest = TsEventIdLast();	// Largest EventID (useful to search back in history)
	while (pXmlNodeEvent != NULL)
		{
		Assert(pXmlNodeEvent->m_pszuTagName != NULL);
		CXmlNode * pXmlAttributeTimestampEventID = pXmlNodeEvent->PFindAttribute(d_chEvent_Attribute_tsEventID);
		Assert(pXmlAttributeTimestampEventID != NULL);
		if (pXmlAttributeTimestampEventID != NULL)
			{
			IEvent * pEvent;
			TIMESTAMP tsEventID = Timestamp_FromString_ML(pXmlAttributeTimestampEventID->m_pszuTagValue);
			if (tsEventID > tsEventLargest)
				{
				// We have a new event to add to the Chat Log
				tsEventLargest = tsEventID;
				EventAllocate:
				pEvent = IEvent::S_PaAllocateEvent_YZ(IN pXmlNodeEvent, IN &tsEventID);
				if (pEvent == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to allocate event $t of class '$s'\n", tsEventID, pXmlNodeEvent->m_pszuTagName);
					goto EventNext;
					}
				pEvent->m_pVaultParent_NZ = pVault;
				AssertValidEvent(pEvent);
				Assert(pEvent->m_tsEventID == tsEventID);
				#ifdef d_szEventDebug_strContactSource
				pEvent->m_strDebugContactSource = pXmlNodeEvent->PszuFindAttributeValue(d_szEventDebug_strContactSource);
				#endif
				#ifdef d_szEventDebug_strVersion
				pEvent->m_strDebugVersion = pXmlNodeEvent->PszuFindAttributeValue(d_szEventDebug_strVersion);
				#endif
				fOutOfSync |= Event_FoosAddSorted(PA_CHILD pEvent);
				}
			else
				{
				// We are updating an existing event
				pEvent = PFindEventByID(tsEventID);
				if (pEvent == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "Info: EventsUnserializeFromDisk($s) - Out-of-sync EventID $s\n", pParent->TreeItem_PszGetNameDisplay(), pXmlAttributeTimestampEventID->m_pszuTagValue);
					goto EventAllocate;	// If the event is not in the history, then allocate it as a new event
					}
				} // if...else
			pXmlNodeEvent->UpdateAttributeValueTimestamp(d_chEvent_Attribute_tsOther, OUT_F_UNCH &pEvent->m_tsOther);	// Make sure m_tsOther is always unserialized
			Assert(pEvent->m_pContactGroupSender_YZ  == NULL);
			if (pEvent->Event_FIsEventTypeReceived())
				pEvent->m_pContactGroupSender_YZ = pParent->m_pAccount->Contact_PFindByIdentifierGroupSender_YZ(IN pXmlNodeEvent);	// Assign the group sender to received events
			else
				{
				// This code is only for debugging
				TContact * pContactGroupSender = pParent->m_pAccount->Contact_PFindByIdentifierGroupSender_YZ(IN pXmlNodeEvent);
				if (pContactGroupSender != NULL)
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "tsEventID $t ({tL}) of class '$U' should NOT have a pContactGroupSender ^j\n", tsEventID, tsEventID, pEvent->EGetEventClass(), pContactGroupSender);
				}
			Assert(pEvent->m_pVaultParent_NZ == pVault);
			pEvent->XmlUnserializeCore(IN pXmlNodeEvent);
			} // if
		EventNext:
		pXmlNodeEvent = pXmlNodeEvent->m_pNextSibling;	// Get the next event
		} // while

	if (fOutOfSync)
		{
		// One (or more) event(s) was out-of-sync, so we take this opportunity to do some housekeeping
		Assert(FEventsSortedByIDs());						// The events should always be sorted, regardless if one (or more) of them was out-of-sync
		DeleteAllEventsReceivedHavingDuplicateTsOther();	// Take this opportunity to remove duplicate out-of-sync received events
		pParent->Vault_SetModified();	// After a sorting, the vault is 'modified', which means it must be saved again to disk with the sorted results
		}
	else
		pParent->Vault_SetNotModified();
	Assert(FEventsSortedByIDs());	// The events should always be sorted after being unserialized
	} // EventsUnserializeFromDisk()

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TContact::Xcp_ServiceDiscovery()
	{
	//Xcp_WriteStanza_VE(d_szXmlServiceDiscovery);
	}

void
TContact::Xcp_Synchronize()
	{
	#if 1
	// Old legacy sync
	CBinXcpStanzaTypeInfo binXcpStanza;
	binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithContact(this);
	binXcpStanza.XcpSendStanzaToContact(IN this);
	#else
	// New PAPI way to sync (still under development)
	CBinXcpStanzaTypeInfo binXcpStanza;
	binXcpStanza.m_pContact = this;
	binXcpStanza.BinXmlAppendXcpApiSynchronize_RequestCreate();
	binXcpStanza.XcpSendStanza();
	#endif
	}

/*
void
ITreeItemChatLogEvents::XcpStanza_AppendServiceDiscovery(IOUT CBinXcpStanza * pbinXcpStanza) const
	{
	Assert(pbinXcpStanza != NULL);
	pbinXcpStanza->BinAppendTextSzv_VE("<" d_szXCPe_EventInfo_tsO_tsI "/>");
	}
*/

CBinXcpStanza::CBinXcpStanza(EStanzaType eStanzaType)
	{
	m_eStanzaType = eStanzaType;
	m_pContact = NULL;
	m_ibXmlApiReply = d_zNA;
	PvSizeAlloc(300);	// Pre-allocate 300 bytes, which should be enough for a small stanza
	}

CBinXcpStanzaTypeInfo::CBinXcpStanzaTypeInfo() : CBinXcpStanza(eStanzaType_zInformation)
	{
	}

CBinXcpStanzaTypeInfo::CBinXcpStanzaTypeInfo(IEvent * pEvent) : CBinXcpStanza(eStanzaType_zInformation)
	{
	Assert(pEvent != NULL);
	Assert(pEvent->m_pVaultParent_NZ != NULL);
	TGroup * pContactOrGroup = (TGroup *)pEvent->m_pVaultParent_NZ->m_pParent;
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TContact))
		m_pContact = (TContact *)pContactOrGroup;
	else
		{
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TGroup));
		m_pContact = pEvent->m_pContactGroupSender_YZ;
		Report(m_pContact != NULL && "Invalid contact group sender");	// This pointer may actually be NULL if the event is the group sender
		// We have a group, therefore add the group selector to the XCP stanza
		BinXmlInitStanzaWithGroupSelector(pContactOrGroup);
		}
	}

void
CBinXcpStanza::BinXmlInitStanzaWithGroupSelector(TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	BinInitFromTextSzv_VE("<" d_szXCPe_GroupSelector ">{h|}</" d_szXCPe_GroupSelector ">", &pGroup->m_hashGroupIdentifier);
	}

void
CBinXcpStanza::XmppWriteStanzaToSocket()
	{
	if (m_pContact == NULL)
		return;
	CSocketXmpp * pSocket = m_pContact->Xmpp_PGetSocketOnlyIfReady();
	if (pSocket != NULL)
		{
		pSocket->Socket_WriteBin(*this);
		m_pContact = NULL;
		}
	}

void
CBinXcpStanza::XmppWriteStanzaToSocketOnlyIfContactIsUnableToCommunicateViaXcp_VE(PSZAC pszFmtTemplate, ...)
	{
	Assert(pszFmtTemplate != NULL);
	if (m_pContact == NULL)
		return;		// If there is no contact, it means we are serializing to disk, which means there is no XMPP communication
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));

	CSocketXmpp * pSocket = m_pContact->Xmpp_PGetSocketOnlyIfContactIsUnableToCommunicateViaXcp();
	if (pSocket != NULL)
		{
		va_list vlArgs;
		va_start(OUT vlArgs, pszFmtTemplate);
		pSocket->Socket_WriteXmlFormatted_VL(pszFmtTemplate, vlArgs);
		m_pContact = NULL;	// Don't send the stanza via XMPP.  Of course this NULL pointer may be confused with serialized to disk, however since the whole CBinXcpStanza will be discarded, who cares?
		}
	}

//	Before allocating a CDataXmlLargeEvent, check if it is not already there.
//	This is important because if we are sending a message to a group, then the same CDataXmlLargeEvent is reused, otherwise
//	there will be a copy of CDataXmlLargeEvent for each group member.  Just imagine sending a large stanza of 100 KiB to a group of 1000 people.
CDataXmlLargeEvent *
CVaultEvents::PFindOrAllocateDataXmlLargeEvent_NZ(TIMESTAMP tsEventID, IN_MOD_TMP CBinXcpStanza * pbinXcpStanza)
	{
	Assert(tsEventID > d_ts_zNULL);
	Assert(pbinXcpStanza != NULL);
	Assert(pbinXcpStanza->m_pContact != NULL);
	Assert(pbinXcpStanza->m_pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_pParent != NULL);
	Assert(m_pParent->EGetRuntimeClass() == RTI(TContact) || m_pParent->EGetRuntimeClass() == RTI(TGroup));
	IEvent * pEvent;

	// First, search the list of large stanza
	CListaDataXmlLargeEvents * plistaDataXmlLargeEvents = &m_pParent->m_pAccount->m_listaDataXmlLargeEvents;
	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)plistaDataXmlLargeEvents->pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		if (pDataXmlLargeEvent->m_tsEventID == tsEventID)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "PFindOrAllocateDataXmlLargeEvent_NZ() - Found existing CDataXmlLargeEvent $t  ($I bytes)\n", tsEventID, pDataXmlLargeEvent->m_binXmlData.CbGetData());
			plistaDataXmlLargeEvents->MoveNodeToHead(INOUT pDataXmlLargeEvent);	// Move the node at the beginning of the list so next time its access is faster (this is a form of caching, in case there are many pending CDataXmlLargeEvent)
			goto Done;
			}
		pDataXmlLargeEvent = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		} // while
	// We have been unable to find a CDataXmlLargeEvent matching tsEventID, therefore we need to allocate a new one
	pDataXmlLargeEvent = new CDataXmlLargeEvent;
	pDataXmlLargeEvent->m_tsEventID = tsEventID;

	// We need to serialize the event, and then save the blob
	pEvent = PFindEventByID(tsEventID);
	if (pEvent != NULL)
		{
		const int ibXmlDataStart = pbinXcpStanza->CbGetData();
		//Assert(ibXmlDataStart > 0);
		/*
		//pbinXcpStanza->BinAppendTextSzv_VE("<$U" _tsO, pEvent->EGetEventClassForXCP(pbinXcpStanza->m_pContact), pEvent->m_tsEventID);	// Do not serialize m_tsOther, as the CDownloader will have it.  Same for the group identifier as well as m_pContactGroupSender_YZ
		pbinXcpStanza->BinAppendTextSzv_VE("<$U", pEvent->EGetEventClassForXCP(pbinXcpStanza->m_pContact));	// No need to serialize the timestamps, group identifier, or m_pContactGroupSender_YZ, because the CEventDownloader has this information
		pEvent->XmlSerializeCore(IOUT pbinXcpStanza);
		pbinXcpStanza->BinAppendXmlForSelfClosingElement();
		*/
		pbinXcpStanza->BinXmlSerializeEventForXcpCore(pEvent, d_ts_zNULL);
		Assert(pbinXcpStanza->m_pContact != NULL);	// Sometimes XmlSerializeCore() may modify m_pContact, however it should be only for contacts not supporting XCP (which should not be the case here!)
		int cbXmlData = pbinXcpStanza->CbGetData() - ibXmlDataStart;
		pDataXmlLargeEvent->m_binXmlData.BinInitFromBinaryData(pbinXcpStanza->TruncateDataPv(ibXmlDataStart), cbXmlData);	// Make a copy of the serialized data, and restore CBinXcpStanza to its original state
		MessageLog_AppendTextFormatSev(eSeverityComment, "PFindOrAllocateDataXmlLargeEvent_NZ() - Initializing cache by serializing tsEventID $t (tsOther $t) -> $I bytes of data:\n$B\n", tsEventID, pEvent->m_tsOther, cbXmlData, &pDataXmlLargeEvent->m_binXmlData);
		}
	else
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "PFindOrAllocateDataXmlLargeEvent_NZ() - Unable to find tsEventID $t ({tL}), therefore initializing an empty cache entry!\n", tsEventID, tsEventID);
		}
	HashSha1_CalculateFromCBin(OUT &pDataXmlLargeEvent->m_hashXmlData, IN pDataXmlLargeEvent->m_binXmlData);	// Calculate the checksum for the data
	plistaDataXmlLargeEvents->InsertNodeAtHead(PA_CHILD pDataXmlLargeEvent);	// Insert at the head, as the event is most likely to be accessed within the next minutes

	Done:
	pDataXmlLargeEvent->m_tsmLastAccessed = g_tsmMinutesSinceApplicationStarted;
	MessageLog_AppendTextFormatSev(eSeverityNoise, "PFindOrAllocateDataXmlLargeEvent_NZ() returning CDataXmlLargeEvent $t of $I bytes (m_tsmLastAccessed=$I)\n", tsEventID, pDataXmlLargeEvent->m_binXmlData.CbGetData(), pDataXmlLargeEvent->m_tsmLastAccessed);
	return pDataXmlLargeEvent;
	} // PFindOrAllocateDataXmlLargeEvent_NZ()


CListaDataXmlLargeEvents::~CListaDataXmlLargeEvents()
	{
	// Destroy the list
	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		CDataXmlLargeEvent * pDataXmlLargeEventNext = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		delete pDataXmlLargeEvent;
		pDataXmlLargeEvent = pDataXmlLargeEventNext;
		} // while
	}

void
CListaDataXmlLargeEvents::DeleteIdleNodes()
	{
	// Flush any cache entry idle for more than a number of minutes
	#define d_cMinutesIdleForCDataXmlLargeEvent		0		// For debugging, use zero, however for a real world situation, 10 minutes would be a good guess

	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		CDataXmlLargeEvent * pDataXmlLargeEventNext = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		if (pDataXmlLargeEvent->m_tsmLastAccessed + d_cMinutesIdleForCDataXmlLargeEvent < g_tsmMinutesSinceApplicationStarted)
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] CListaDataXmlLargeEvents::DeleteIdleNodes() - Deleting cache entry for tsEventID $t\n", pDataXmlLargeEvent->m_tsEventID);	// This is not an error, however good to draw attention
			DetachNode(INOUT pDataXmlLargeEvent);
			delete pDataXmlLargeEvent;
			}
		pDataXmlLargeEvent = pDataXmlLargeEventNext;
		} // while
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventDownloader::CEventDownloader(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_tsForwarded = d_ts_zNA;
	m_cbDataToDownload = 0;
	m_paEvent = NULL;
	}

CEventDownloader::~CEventDownloader()
	{
	delete m_paEvent;
	}

//	Return the class of the downloader.
//	It is important to return the true event class because there are many instances of the code where a typecast is made based on the event class.
EEventClass
CEventDownloader::EGetEventClass() const
	{
	return c_eEventClass;
	}

//	Return the event class to serialize the event via XCP.
//	In this virtual method, it is appropriate to substitute the class by the downloaded event if available because no typecast is made based on this method.
EEventClass
CEventDownloader::EGetEventClassForXCP() const
	{
	if (m_paEvent != NULL)
		return m_paEvent->EGetEventClassForXCP();
	return c_eEventClass;	// This value does not serialize via XCP
	}

//	CEventDownloader::IEvent::XmlSerializeCoreE()
EXml
CEventDownloader::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	if (m_paEvent != NULL)
		{
		return m_paEvent->XmlSerializeCoreE(IOUT pbinXmlAttributes);
		}
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chXCPa_CEventDownloader_tsForwarded, m_tsForwarded);
	pbinXmlAttributes->BinAppendXmlAttributeInt(d_chXCPa_CEventDownloader_cblDataToDownload, m_cbDataToDownload);
	pbinXmlAttributes->BinAppendXmlAttributeCBin(d_chXCPa_CEventDownloader_bin85DataReceived, m_binDataDownloaded);
	return eXml_zAttributesOnly;
	}

//	CEventDownloader::IEvent::XmlUnserializeCore()
void
CEventDownloader::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	if (m_paEvent != NULL)
		{
		m_paEvent->XmlUnserializeCore(IN pXmlNodeElement);	// This happens when synchronizing
		return;
		}
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chXCPa_CEventDownloader_tsForwarded, OUT_F_UNCH &m_tsForwarded);
	pXmlNodeElement->UpdateAttributeValueInt(d_chXCPa_CEventDownloader_cblDataToDownload, OUT_F_UNCH &m_cbDataToDownload);
	pXmlNodeElement->UpdateAttributeValueCBin(d_chXCPa_CEventDownloader_bin85DataReceived, OUT_F_UNCH &m_binDataDownloaded);
	}

void
CEventDownloader::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanza * pbinXcpStanzaReply)
	{
	if (m_paEvent != NULL)
		m_paEvent->XcpExtraDataRequest(pXmlNodeExtraData, pbinXcpStanzaReply);
	}

void
CEventDownloader::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanza * pbinXcpStanzaReply)
	{
	if (m_paEvent != NULL)
		m_paEvent->XcpExtraDataArrived(pXmlNodeExtraData, pbinXcpStanzaReply);
	}

void
CEventDownloader::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	Assert(m_tsEventID > d_tsOther_kmReserved);
	if (m_paEvent == NULL)
		{
		// Show progress of the download
		int cbDataDownloaded = m_binDataDownloaded.CbGetData();
		if (cbDataDownloaded != m_cbDataToDownload)
			{
			_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
			g_strScratchBufferStatusBar.BinAppendTextSzv_VE("receiving large data... {kT}", cbDataDownloaded, m_cbDataToDownload);
			//g_strScratchBufferStatusBar.BinAppendTextBytesKiBPercentProgress(cbDataDownloaded, m_cbDataToDownload);
			poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(d_coSilver));

			// Request the next block of data
			MessageLog_AppendTextFormatSev(eSeverityComment, "CEventDownloader::ChatLogUpdateTextBlock() - Event tsOther $t is requesting data at offset $I (until $I)\n", m_tsOther, cbDataDownloaded, m_cbDataToDownload);
			CBinXcpStanzaTypeInfo binXcpStanza(this);
			binXcpStanza.BinAppendTextSzv_VE("<" d_szXCPe_EventSplitDataRequest_tsI_i "/>", m_tsOther, cbDataDownloaded);
			binXcpStanza.XcpSendStanza();
			return;
			}
		// We are done downloading, therefore allocate the event by unserializing the downloaded XML data
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventDownloader::ChatLogUpdateTextBlock() - Event tsOther $t completed with $I bytes:\n$B\n", m_tsOther, cbDataDownloaded, &m_binDataDownloaded);
		m_pVaultParent_NZ->SetModified();	// Make sure whatever we do next will be saved to disk

		CXmlTree oXmlTree;
		if (oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN m_binDataDownloaded) == errSuccess)
			{
			m_paEvent = IEvent::S_PaAllocateEvent_YZ(IN &oXmlTree, IN &m_tsEventID);
			if (m_paEvent != NULL)
				{
				// We have successfully re-created a blank event class matching the blueprint of the downloader.  Now, we need to initialize the event variables.
				m_paEvent->m_pVaultParent_NZ = m_pVaultParent_NZ;		// The new event uses the same vault as the downloader
				Assert(m_paEvent->m_pContactGroupSender_YZ == NULL);	// New allocated events do not have any contact group sender yet
				m_paEvent->m_pContactGroupSender_YZ = PGetAccount_NZ()->Contact_PFindByIdentifierGroupSender_YZ(IN &oXmlTree);	// The contact who created the event may be different than the contact who transmitted the event, as the [large] event may be have been forwarded
				if (m_paEvent->m_pContactGroupSender_YZ == NULL)
					m_paEvent->m_pContactGroupSender_YZ = m_pContactGroupSender_YZ;	// If the contact sender was not present in the downloaded XML, use the contact of the downloader (if any)
				Assert(m_paEvent->m_tsOther == d_ts_zNULL);
				//oXmlTree.UpdateAttributeValueTimestamp(d_chEvent_Attribute_tsOther, OUT_F_UNCH &m_paEvent->m_tsOther);	// Make sure m_tsOther is always unserialized.  This line is somewhat similar as EventsUnserializeFromDisk()
				//Assert(oXmlTree.TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther) == d_ts_zNA);
				/*
				m_paEvent->m_tsOther = m_tsOther;	// Since tsOther is never serialized in the cache, just initialize its value from the downloader
				Assert(m_paEvent->m_tsOther > d_tsOther_kmReserved);
				*/
				MessageLog_AppendTextFormatCo(d_coOrange, "CEventDownloader::m_tsOther $t, m_tsForwarded $t\n", m_tsOther, m_tsForwarded);
				m_paEvent->m_tsOther = m_tsForwarded;
				if (m_paEvent->m_tsOther == d_ts_zNULL)
					m_paEvent->m_tsOther = m_tsOther;
				Assert(m_paEvent->m_tsOther > d_tsOther_kmReserved);
				m_paEvent->XmlUnserializeCore(IN &oXmlTree);	// Finally, unserialize the data specific to the event
				goto EventUpdateToGUI;
				}
			else
				{
				if (m_uFlagsEvent & FE_kfEventProtocolWarning)
					m_uFlagsEvent |= FE_kfEventProtocolError;	// Upgrade the 'warning' to an 'error'
				m_uFlagsEvent |= FE_kfEventProtocolWarning;
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[$s] Unable to allocate event '$s' $t (tsOther = $t) from XML data of CEventDownloader (m_cbDataToDownload = $I): ^N",
					(m_uFlagsEvent & FE_kfEventProtocolError) ? "Error" : "Warnign", oXmlTree.m_pszuTagName, m_tsEventID, m_tsOther, m_cbDataToDownload, &oXmlTree);
				goto EventDisplayError;
				}
			}
		// The data is not good (probably corrupted), so display a notification and flush the data
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader::ChatLogUpdateTextBlock() - Data corrupted for event tsOther $t, therefore flushing data.\n", m_tsOther);
		EventDisplayError:
		_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("<span style='color:red'>Error allocating event $s of {kK}", oXmlTree.m_pszuTagName, cbDataDownloaded);
		poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(d_coSilver));
		m_binDataDownloaded.Empty();
		m_cbDataToDownload = c_cbDataToDownload_Error;
		}
	else
		{
		EventUpdateToGUI:
		m_paEvent->ChatLogUpdateTextBlock(INOUT poCursorTextBlock);
		} // if...else
	} // ChatLogUpdateTextBlock()

void
CEventDownloader::XcpDownloadedDataArrived(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXcpStanzaReply, QTextEdit * pwEditChatLog)
	{
	const int ibDataNew = pXmlNodeData->LFindAttributeXcpOffset();
	const int ibDataRequested = m_binDataDownloaded.CbGetData();
	if (ibDataNew == ibDataRequested)
		{
		m_binDataDownloaded.BinAppendBinaryDataFromBase85Szv_ML(pXmlNodeData->PszuFindAttributeValue(d_chXCPa_EventExtraData_bin85Payload));	// Append new binary data
		int cbDataNew = m_binDataDownloaded.CbGetData();
		SHashSha1 hashDataChecksum;
		if (pXmlNodeData->UpdateAttributeValueHashSha1(d_chXCPa_EventExtraData_shaData, OUT_F_UNCH &hashDataChecksum))
			{
			// If the hash is present, it means we have received everyting.  All we need is to compare if the value is good
			if (m_binDataDownloaded.FCompareFingerprint(IN hashDataChecksum))
				{
				// We have the correct fingerprint, meaning the data is valid
				if (cbDataNew != m_cbDataToDownload)
					{
					if ((m_uFlagsEvent & FE_kfEventProtocolWarning) == 0)
						{
						m_uFlagsEvent |= FE_kfEventProtocolWarning;
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[Warning] CEventDownloader tsEventID $t: Adjusting m_cbDataToDownload from $I to $I.\n", m_tsEventID, m_cbDataToDownload, cbDataNew);
						m_cbDataToDownload = cbDataNew;
						}
					}
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader tsEventID $t: Fingerprint mismatch for $I bytes of data\n", m_tsEventID, cbDataNew);
				if ((m_uFlagsEvent & FE_kfEventProtocolWarning) == 0)
					{
					m_uFlagsEvent |= FE_kfEventProtocolWarning;
					m_binDataDownloaded.Empty();	// Flush what we downloaded, and try again
					cbDataNew = 0;
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t [Warning]: CEventDownloader tsEventID $t: Retrying to download again...\n", m_tsEventID);
					}
				} // if...else
			goto UpdateGUI;	// Always update the GUI when the hash is present
			} // if
		if (cbDataNew > ibDataRequested)
			{
			// The content m_binDataDownloaded grew (which means received new data), therefore update the GUI if available, otherwise request another chunk
			UpdateGUI:
			if (pwEditChatLog != NULL)
				ChatLog_UpdateEventWithinWidget(pwEditChatLog);	// Update the UI (which will requests another chunk)
			else
				pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventSplitDataRequest_tsI_i "/>", m_tsOther, cbDataNew);
			m_pVaultParent_NZ->SetModified();	// Make sure the new data gets saved to disk
			}
		else
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader tsEventID $t: Ignoring empty stanza (cbDataDownloaded=$I, m_cbDataToDownload=$I)\n", m_tsEventID, ibDataRequested, m_cbDataToDownload);
		}
	else
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader tsEventID $t: Ignoring data because its offset $I does not match the requested at $I\n", m_tsEventID, ibDataNew, ibDataRequested);
	} // XcpDownloadedDataArrived()

void
CEventDownloader::HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText)
	{
	if (m_paEvent != NULL)
		m_paEvent->HyperlinkGetTooltipText(pszActionOfHyperlink, IOUT pstrTooltipText);
	}

void
CEventDownloader::HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock)
	{
	if (m_paEvent != NULL)
		m_paEvent->HyperlinkClicked(pszActionOfHyperlink, INOUT poCursorTextBlock);
	}

CEventDownloader *
CVaultEvents::PFindEventDownloaderMatchingEvent(const IEvent * pEvent) const
	{
	Assert(pEvent != NULL);
	Assert(pEvent->EGetEventClass() != CEventDownloader::c_eEventClass);
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		CEventDownloader * pEventDownloader = (CEventDownloader *)*--ppEventStop;
		if (pEventDownloader->EGetEventClass() == CEventDownloader::c_eEventClass)
			{
			if (pEventDownloader->FIsDownloaderMatchingEvent(pEvent))
				return pEventDownloader;
			}
		}
	return NULL;
	}

void
CBinXcpStanza::XcpSendStanzaToContactOrGroup(const ITreeItemChatLogEvents * pContactOrGroup) CONST_MCC
	{
	Assert(pContactOrGroup != NULL);
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TGroup))
		{
		TGroupMember ** ppMemberStop;
		TGroupMember ** ppMember = ((TGroup *)pContactOrGroup)->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
		while (ppMember != ppMemberStop)
			{
			TGroupMember * pMember = *ppMember++;
			Assert(pMember != NULL);
			Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
			Assert(pMember->m_pGroup == pContactOrGroup);
			Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
			XcpSendStanzaToContact(IN pMember->m_pContact);
			}
		return;
		}
	Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TContact));
	XcpSendStanzaToContact((TContact *)pContactOrGroup);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core routine sending a stanza to a contact.
//	This routine takes care of encrypting the data and digitally signing the message.
void
CBinXcpStanza::XcpSendStanzaToContact(TContact * pContact) CONST_MCC
	{
	Assert(m_eStanzaType != eStanzaType_eBroadcast && "Not yet supported");
	Assert(m_paData != NULL && m_paData->cbData > 0);	// There should be some data into the stanza to send
	if (pContact == NULL)
		return;	// This is not a bug, but a feature allowing an event to directly write to the socket when a contact is unable to understand XCP.
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	if (pContact->m_cVersionXCP <= 0)
		{
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "Contact not supporting XCP - Therefore ignoring XcpSendStanzaToContact($s):\n$B\n", pContact->ChatLog_PszGetNickname(), this);
		return;
		}
	MessageLog_AppendTextFormatCo(d_coBlue, "XcpSendStanzaToContact($s):\n$B\n", pContact->ChatLog_PszGetNickname(), this);
	PSZUC pszDataStanza = m_paData->rgbData;
	int cbDataStanza = m_paData->cbData;

	// TODO: Encrypt the data and sign it.
	SHashSha1 hashSignature;
	HashSha1_CalculateFromBinary(OUT &hashSignature, IN pszDataStanza, cbDataStanza);	// At the moment, use SHA-1 as the 'signature'

	PSZAC pszStanzaType = c_sza_message;
	PSZAC pszStanzaAttributesExtra = NULL;
	if (m_eStanzaType == eStanzaType_zInformation)
		{
		pszStanzaType = c_sza_iq;
		pszStanzaAttributesExtra = " type='get'";
		}
	// Format the XML envelope for the XMPP protocol.
	g_strScratchBufferSocket.BinInitFromTextSzv_VE("<$s$s to='^J'><" d_szCambrianProtocol_xcp " " d_szCambrianProtocol_Attribute_hSignature "='{h|}'>", pszStanzaType, pszStanzaAttributesExtra, pContact, &hashSignature);
	g_strScratchBufferSocket.BinAppendStringBase85FromBinaryData(IN pszDataStanza, cbDataStanza);
	g_strScratchBufferSocket.BinAppendTextSzv_VE("</" d_szCambrianProtocol_xcp "></$s>", pszStanzaType);
	CSocketXmpp * pSocket = pContact->Xmpp_PGetSocketOnlyIfReady();
	if (pSocket != NULL)
		pSocket->Socket_WriteBin(g_strScratchBufferSocket);
	} // XcpSendStanzaToContact()

void
CBinXcpStanza::XcpSendStanza() CONST_MCC
	{
	XcpSendStanzaToContact(m_pContact);
	}

void
CBinXcpStanza::BinXmlAppendTimestampsToSynchronizeWithContact(TContact * pContact)
	{
	BinAppendTextSzv_VE("<" d_szXCPe_EventPrevious_tsO_tsI "/>", pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
	}

void
CBinXcpStanza::BinXmlAppendTimestampsToSynchronizeWithGroupMember(TGroupMember * pMember)
	{
	Assert(pMember != NULL);
	Assert(pMember->m_pContact == m_pContact || m_pContact == NULL);

	// When sending the timestamps for a group, also include the number of messages received since the last timestamp
	TGroup * pGroup = pMember->m_pGroup;
	BinAppendTextSzv_VE("<" d_szXCPe_EventPrevious_tsO_tsI, pGroup->m_tsEventIdLastSentCached, pMember->m_tsOtherLastSynchronized);
	// Since we are sending the timestamps to synchronize with a group, include the number of received messages since the last synchronization.
	// This way, the contact will be able to know if it is missing group messages
	CVaultEvents * pVault = pGroup->Vault_PGet_NZ();
	IEvent * pEventLast = pVault->PGetEventLast_YZ();
	if (pEventLast != NULL && pEventLast->Event_FIsEventTypeSent())
		{
		// Send the counter only if the last event was sent. This will avoid every group member to send the count of missing messages and having redundant synchronization
		int cEventsReceived = pVault->UCountEventsReceivedByOtherGroupMembersSinceTimestampEventID(pGroup->m_tsEventIdLastSentCached, pMember->m_pContact);
		if (cEventsReceived > 0)
			BinAppendTextSzv_VE(d_szXCPa_EventPrevious_cEventsMissing, cEventsReceived);
		}
	BinAppendXmlForSelfClosingElement();
	}

void
IEvent::XcpRequesExtraData()
	{
	CBinXcpStanzaTypeInfo binXcpStanza(this);
	binXcpStanza.BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataRequest_tsI "/>", m_tsOther);
	binXcpStanza.XcpSendStanza();
	}

void
TContact::Xmpp_WriteXmlChatState(EChatState eChatState) CONST_MCC
	{
	Assert(eChatState == eChatState_zComposing || eChatState == eChatState_fPaused);
	if (m_cVersionXCP > 0)
		{
		CBinXcpStanzaTypeInfo binXcpStanza;
		if (m_uFlagsContact & FC_kfXcpComposingSendTimestampsOfLastKnownEvents)
			{
			m_uFlagsContact &= ~FC_kfXcpComposingSendTimestampsOfLastKnownEvents;
			binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithContact(this);
			}
		binXcpStanza.BinAppendText((eChatState == eChatState_zComposing) ? d_szXCPe_MessageTextComposingStarted : d_szXCPe_MessageTextComposingPaused);
		binXcpStanza.XcpSendStanzaToContact(IN this);
		}
	else
		{
		// The device/computer of the remote contact does not understand XCP, therefore send the chat state notification via the standard XMPP
		CSocketXmpp * pSocket = Xmpp_PGetSocketOnlyIfReady();
		if (pSocket != NULL)
			pSocket->Socket_WriteXmlFormatted("<message to='^J'><$s xmlns='http://jabber.org/protocol/chatstates'/></message>", this, (eChatState == eChatState_zComposing) ? "composing" : "paused");
		}
	}

void
TGroup::Members_BroadcastChatState(EChatState eChatState) const
	{
	CBinXcpStanzaTypeInfo binXcpStanza;
	binXcpStanza.BinInitFromTextSzv_VE("<" d_szXCPe_GroupSelector ">{h|}</" d_szXCPe_GroupSelector ">", &m_hashGroupIdentifier);
	binXcpStanza.BinAppendText((eChatState == eChatState_zComposing) ? d_szXCPe_MessageTextComposingStarted : d_szXCPe_MessageTextComposingPaused);
	binXcpStanza.XcpSendStanzaToContactOrGroup(this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
PSZUC
CEventMessageTextReceived::PszGetTextOfEventForSystemTray(OUT_IGNORE CStr *) const
	{
	return m_strMessageText;
	}

PSZUC
CEventFileReceived::PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const
	{
	return pstrScratchBuffer->Format("File to download: $S", &m_strFileName);
	}

L64
CXmlNode::LFindAttributeXcpOffset() const
	{
	return LFindAttributeValueDecimal_ZZR(d_chXCPa_EventExtraData_iblOffset);
	}

//	Code specific for XMPP clients unable to communicate via XCP
void
CEventFileSent::XmppProcessStanzaFromContact(const CXmlNode * pXmlNodeStanza, TContact * pContact)
	{
	Assert(pXmlNodeStanza != NULL);
	Assert(pContact != NULL);
	CXmlNode * pXmlNodeStreamInitiation = pXmlNodeStanza->PFindElementSi();
	if (pXmlNodeStreamInitiation != NULL)
		{
		// The contact accepted the file offer, therefore open a connection to start sending the data
		(void)_PFileOpenReadOnly_NZ();	// Open the file
		Assert(m_paFile != NULL);
		Socket_WriteXmlIqSet_VE_Gso(pContact, "<open sid='$t' block-size='$u' ^:ib/>", m_tsEventID, c_cbBufferSizeMaxXmppBase64);
		return;
		}
	#if 1
	if (m_paFile == NULL)
		return;		// The file is not opened, therefore there is nothing to do
	#endif
	if (FCompareStrings(pXmlNodeStanza->PszFindAttributeValueType_NZ(), c_sza_result) && pXmlNodeStanza->PFindElementErrorStanza() == NULL)
		{
		// We have a generic 'result' without any 'error', therefore send the data
		BYTE rgbBuffer[c_cbBufferSizeMaxXmppBase64];

		int ibDataSource = m_paFile->pos();	// Get the position of the file (this is used to calculate the sequence number)
		int cbDataRead = m_paFile->read(OUT (char *)rgbBuffer, sizeof(rgbBuffer));
		if (cbDataRead > 0)
			{
			Socket_WriteXmlIqSet_VE_Gso(pContact, "<data sid='$t' seq='$u' ^:ib>{p/}</data>", m_tsEventID, ibDataSource / sizeof(rgbBuffer), IN rgbBuffer, cbDataRead);
			m_cblDataTransferred = ibDataSource + cbDataRead;
			}
		else
			{
			// We are done sending the file, so send a <close> command to the remote contact
			Socket_WriteXmlIqSet_VE_Gso(pContact, "<close sid='$t' ^:ib/>", m_tsEventID);
			_FileClose();
			}
		}
	Event_UpdateWidgetWithinParentChatLog();
	} // XmppProcessStanzaFromContact()

void
CEventFileSent::XmppProcessStanzaVerb(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext, const CXmlNode * pXmlNodeVerb)
	{
	Assert(pXmlNodeStanza != NULL);
	if (pszaVerbContext == c_sza_data)
		{
		Assert(pXmlNodeVerb->FCompareTagName("data"));
		// Get the data from the stanza
		//int nSequence = pXmlNodeStanza->NFindElementOrAttributeValueNumeric("seq");
		int cbDataWritten = _PFileOpenWriteOnly_NZ()->CbDataWriteBinaryFromBase85(pXmlNodeVerb->m_pszuTagValue);
		m_cblDataTransferred += cbDataWritten;
		if (cbDataWritten <= 0)
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Failure writing to file #S\n", &m_strFileName);
		}
	else if (pszaVerbContext == c_sza_close)
		{
		Assert(pXmlNodeVerb->FCompareTagName("close"));
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventFileSent::XmppProcessStanzaVerb() - Closing file $S\n", &m_strFileName);
		_FileClose();
		}
	else
		{
		Assert(pszaVerbContext == c_sza_open);
		Assert(pXmlNodeVerb->FCompareTagName("open"));
		}
	Socket_WriteXmlIqReplyAcknowledge();	// Reply to acknowledge we received the stanza and/or receive the next data sequence
	Event_UpdateWidgetWithinParentChatLog();
	} // XmppProcessStanzaVerb()

void
CEventFileSent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, CBinXcpStanza * pbinXcpStanzaReply)
	{
	Assert(pXmlNodeExtraData != NULL);
	BYTE rgbBuffer[CBinXcpStanza::c_cbStanzaMaxBinary];
	L64 iblDataSource = pXmlNodeExtraData->LFindAttributeXcpOffset();
	int cbDataRead = _PFileOpenReadOnly_NZ()->CbDataReadAtOffset(iblDataSource, sizeof(rgbBuffer), OUT rgbBuffer);
	m_cblDataTransferred = iblDataSource + cbDataRead;
	//MessageLog_AppendTextFormatSev(eSeverityComment, "$t: CEventFileSent::XcpExtraDataRequest() offset: $L + cbDataRead: $i = $L bytes completed\n", m_tsEventID, iblDataSource, cbDataRead, m_cblDataTransferred);
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataReply_tsO d_szXCPa_EventExtraData_iblOffset_l d_szXCPa_EventExtraData_bin85Payload_pvcb "/>", m_tsEventID, iblDataSource, IN rgbBuffer, cbDataRead);
	if (cbDataRead <= 0)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventFileSent::XcpExtraDataRequest() - Closing file $S\n", &m_strFileName);
		_FileClose();	// There is nothing to send, therefore assume we reached the end of the file, and consequently close the file.  Of course, if the contact wishes more data (or request to resend the last block, then the file will be re-opened)
		}
	}

void
CEventFileReceived::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanza * pbinXcpStanzaReply)
	{
	CFile * pFile = _PFileOpenWriteOnly_NZ();
	pFile->seek(pXmlNodeExtraData->LFindAttributeXcpOffset());
	int cbDataWritten = pFile->CbDataWriteBinaryFromBase85(pXmlNodeExtraData->PszuFindAttributeValue(d_chXCPa_EventExtraData_bin85Payload));
	if (cbDataWritten > 0)
		{
		// Request the next piece of data
		m_cblDataTransferred = pFile->size();
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataRequest_tsI d_szXCPa_EventExtraData_iblOffset_l "/>", m_tsOther, m_cblDataTransferred);
		}
	else
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventFileReceived::XcpExtraDataArrived() - Closing file $S\n", &m_strFileName);
		_FileClose();
		}
	}
