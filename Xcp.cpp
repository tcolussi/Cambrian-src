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
#define d_chXCPe_EventsMissing								'M'	// Notification to the remote client some events are missing, and therefore should synchronize
#define d_szXCPe_EventsMissing						d_szXCP_"M"

#define d_chXCPe_EventNextRequest							'n'	// Ask the remote client to send its next event after tsEventID
#define d_szXCPe_EventNextRequest_tsI				d_szXCP_"n"	_tsI
#define d_chXCPe_EventNextReply								'N'	// Reply to the sender regarding the request of tsEventID and tsOther (the following xcp-stanzas contains serialized IEvents)
#define d_szXCPe_EventNextReply_tsO					d_szXCP_"N" _tsO
#define d_chXCPe_EventsOutstandingReply						'O'	// Any message reply 'N' must end with 'O' to indicate hoe many outstanding IEvents remain.
#define d_szXCPe_EventsOutstandingReply_i_tsO		d_szXCP_"O c='$i'" _tsO // ('R' = Remaining)
	#define d_chXCPa_EventsOutstandingReply_cEvents				'c'
#define d_chXCPe_EventConfirmation							'C'	// Confirmation an event was processed successfully (the confirmation only includes tsEventID of the sender)
#define d_szXCPe_EventConfirmation_tsI				d_szXCP_"C" _tsI
#define d_chXCPe_EventError									'E'	// The event was received, however could not be processed because its class/type is unknown. 'C' and 'E' are mutually exclusive.
#define d_szXCPe_EventError_tsI_s					d_szXCP_"E" _tsI " e='^s'"	// e=event, c=code of the error (if any), t=friendly text to display to the user (if any)

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


#define d_chXCPe_zNA									'\0'	// Not applicable


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

	Assert(pVault->PFindEventNext(d_ts_zNULL, OUT &cEventsRemaining) == pVault->m_arraypaEvents.PvGetElementFirst_YZ());
	Assert(cEventsRemaining == 0 || cEventsRemaining == pVault->m_arraypaEvents.GetSize() - 1);
	BOOL fEventsOutOfSync = FALSE;
	TIMESTAMP_DELTA dtsOtherSynchronization = 1;	// By default, assume the events are NOT synchronized, therefore assign to the variable a non-zero value.
	CHS chXCPe = d_chXCPe_zNA;			// Last known XCP element
	IEvent * pEvent = NULL;
	TGroup * pGroup = NULL;				// Pointer to the selected group
	TGroupMember * pMember = NULL;		// Pointer to the contact within the selected group
	while (pXmlNodeXcpEvent != NULL)
		{
		Report(pXmlNodeXcpEvent->m_pszuTagName != NULL);
		MessageLog_AppendTextFormatCo((pXmlNodeXcpEvent->m_pszuTagName[0] == d_chXCP_ && pXmlNodeXcpEvent->m_pszuTagName[1] == d_chXCPe_EventError) ? COX_MakeBold(d_coRed) : d_coBlack, "\t Processing $s ^N", fEventsOutOfSync ? "(out of sync)" : NULL, pXmlNodeXcpEvent);
		// Get both timestamps, as most xcp-stanzas have both
		TIMESTAMP tsEventID = pXmlNodeXcpEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID);
		TIMESTAMP tsOther = pXmlNodeXcpEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
		PSZUC pszEventName = pXmlNodeXcpEvent->m_pszuTagName;
		if (pszEventName[0] == d_chXCP_)
			{
			// We have a XCP control node which contains directives how to interpret the remaining xcp-stanzas/event(s). Some xcp-stanzas (control nodes) are at the beginning of the xmpp-stanza, while others are at the end.
			chXCPe = pszEventName[1];
			switch (chXCPe)
				{
			case d_chXCPe_GroupSelector:
				// The events belong to a group, so attempt to find the group
				pszEventValue = pXmlNodeXcpEvent->m_pszuTagValue;
				pChatLogEvents = pGroup = m_pAccount->Group_PFindByIdentifier_NZ(IN pszEventValue);
				pMember = pGroup->Member_PFindOrAddContact_NZ(this);
				ptsOtherLastSynchronized = &pMember->m_tsOtherLastSynchronized;
				pVault = pChatLogEvents->Vault_PGet_NZ();
				pwChatLog = pChatLogEvents->ChatLog_PwGet_YZ();
				MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t Selecting group $s (m_tsOtherLastSynchronized=$t, m_tsEventIdLastSentCached=$t)\n", pGroup->TreeItem_PszGetNameDisplay(), *ptsOtherLastSynchronized, pGroup->m_tsEventIdLastSentCached);
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_GroupSelector ">^s</" d_szXCPe_GroupSelector ">", pszEventValue);
				cbXcpStanzaReplyEmpty = binXcpStanzaReply.CbGetData();
				break;
			case d_chXCPe_EventPrevious:	// We are receiving information regarding the previous message
				dtsOtherSynchronization = tsOther - *ptsOtherLastSynchronized;
				fEventsOutOfSync = (dtsOtherSynchronization != 0);
				if (dtsOtherSynchronization > 0)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t m_tsOtherLastSynchronized = $t however its value should be $t\n"
						"\t\t\t therefore requesting $s to resend its next event after $t\n", *ptsOtherLastSynchronized, tsOther, pszNameDisplay, *ptsOtherLastSynchronized);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventNextRequest_tsI "/>", *ptsOtherLastSynchronized);
					}
				else if (dtsOtherSynchronization < 0)
					{
					Assert(tsOther < *ptsOtherLastSynchronized);
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsOther $t < m_tsOtherLastSynchronized $t\n"
						"\t\t\t therefore $s is missing ITS OWN messages!  As a result, send the necessary info to notify $s about the situation.\n", tsOther, *ptsOtherLastSynchronized, pszNameDisplay, pszNameDisplay);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsMissing "/>");
					}
				else if (tsEventID < pChatLogEvents->m_tsEventIdLastSentCached)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t tsEventID $t < m_tsEventIdLastSentCached $t\n"
						"\t\t\t therefore $s is missing my messages!\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached, pszNameDisplay);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsMissing "/>");
					}
				else if (tsEventID > pChatLogEvents->m_tsEventIdLastSentCached)
					{
					// I have no idea what this situation means, so I am reporting it on the Message Log
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsEventID $t > m_tsEventIdLastSentCached $t\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached);
					}
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
				Assert(tsEventID == d_ts_zNA);
				dtsOtherSynchronization = tsOther - *ptsOtherLastSynchronized;
				if (dtsOtherSynchronization == 0)
					MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t chXCPe = d_chXCPe_EventNextReply;\n");
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t chXCPe = d_chXCPe_EventNextReply; (out of sync by $T)\n", dtsOtherSynchronization);
				break;
			case d_chXCPe_EventsOutstandingReply:
				Assert(tsEventID == d_ts_zNA);
				Assert(tsOther != d_ts_zNULL);	// Typically tsOther is the same as pEvent->m_tsOther (the last event we processed), however not always the case as events may have been received out-of-sync
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
//				AppendTimestampsOfLastKnownEvents:
				binXcpStanzaReply.BinXmlAppendTimestampsToSynchronizeWithContact(this); // Need to fix this for group
				break;
			case d_chXCPe_EventConfirmation:
			case d_chXCPe_EventError:
				Assert(tsEventID != d_ts_zNULL);
				Assert(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					if (chXCPe == d_chXCPe_EventError)
						pEvent->Event_SetFlagErrorProtocol();
					pEvent->Event_SetCompleted(pwChatLog);	// This will typically display a green checkmark at the right of the screen where the event is located
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unrecognized confirmation for EventID $t\n", tsEventID);
				break;
			case d_chXCPe_EventExtraDataRequest:
				Assert(tsEventID > d_tsOther_kmReserved);
				Assert(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					pEvent->XcpExtraDataRequest(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply);
					pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);	// This will typically show a progress bar
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find EventID $t for d_chXCPe_EventExtraDataRequest\n", tsEventID);
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
				CDataXmlLargeEvent * pDataXmlLargeEvent = m_pAccount->PFindOrAllocateDataXmlLargeEvent_NZ(tsEventID, IN_MOD_TMP &binXcpStanzaReply);
				int cbDataRemaining;
				int cbData = pDataXmlLargeEvent->m_binXmlData.CbGetDataAfterOffset(ibDataSource, 1 /*CBinXcpStanzaType::c_cbStanzaMaxBinary*/, OUT &cbDataRemaining);	// At the moment, send only one byte at the time (rather than c_cbStanzaMaxBinary), so we can test the code if it is working
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
			default:
				MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unknown chXCPe = $i ($b)\n", chXCPe, chXCPe);
				} // switch (chXCPe)
			}
		else
			{
			// The XML node contains an event of type EEventClass
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
			default:
				;		// Keep the compiler happy to prevent a warning the switch() statement does not handle all cases
				} // switch
			Assert(tsOther > d_tsOther_kmReserved);
			pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pMember);	// Regardless of what type of event we received, always attempt to find it in the vault
			const BOOL fSynchronizingEvents = (chXCPe == d_chXCPe_EventNextReply);
			if (!fSynchronizingEvents)
				{
				// We are receiving new events from the remote client
				Assert(tsEventID == d_ts_zNULL);						// New events should not have a tsEventID
				binXcpStanzaReply.m_eStanzaType = eStanzaType_eMessage;	// Any event confirmation (or error) shall be cached as an 'XMPP message' so the remote client may know about it
				if (pwChatLog != NULL)
					pwChatLog->ChatLog_ChatStateComposerRemove(this);	// Make sure the text "<user> is typing..." will be no longer displayed
				if (pEvent != NULL)
					{
					// The new event is already in the Chag Log.  This is the case when the event is a re-send.
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Updating existing tsEventID $t matching tsOther $t...\n", pEvent->m_tsEventID, tsOther);
					goto EventUnserialize;
					}
				}
			else
				{
				// We are synchronizing by receiving previous events
				Assert(tsEventID == d_ts_zNULL || tsEventID > d_tsOther_kmReserved);	// The EventID may be zero for the case of receiving an event which was never delivered before
				if (pEvent != NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Updating existing tsEventID $t matching tsOther $t...\n", pEvent->m_tsEventID, tsOther);
					goto EventUnserialize;
					}
				// If we are unable to find the event by tsOther, attempt to find it by its identifier
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Updating existing tsEventID $t...\n", tsEventID);
					EventUnserialize:
					Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
					if (pEvent->Event_FIsEventTypeReceived())
						{
						// Only unserialize existing 'received' events.  This is a security feature, so someone does not attempt to hijack our history log by pretending to resend our own events.
						pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);	// TODO: Need to provide a mechanism if an event has been updated
						pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);
						}
					goto EventConfirmation;
					}
				} // if...else

			// We have a new event to allocate and to display into the Chat Log.
			Assert(pEvent == NULL);
			pEvent = IEvent::S_PaAllocateEvent_YZ(eEventClass, (tsEventID == d_ts_zNULL) ? NULL : &tsEventID);
			if (pEvent == NULL)
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Unable to allocate event tsOther $t from stanza because its class ($i) is unrecognized: '$s'\n", tsOther, eEventClass, pszEventName);
				binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventError_tsI_s "/>", tsOther, pszEventName);	// Report the error to the remote client
				goto EventSynchronize;
				}
			Assert(USZU_from_USZUF(pEvent->EGetEventClass()) == (USZU)eEventClass);
			pEvent->m_tsOther = tsOther;
			pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);
			if (pGroup != NULL)
				{
				// We have received a group event, therefore assign its sender to the current contact
				if (pEvent->m_pContactGroupSender_YZ == NULL)
					pEvent->m_pContactGroupSender_YZ = this;
				else
					MessageLog_AppendTextFormatCo(d_coRed, "\t\t Group event tsOther $t already has sender ^j.\n", tsOther, pEvent->m_pContactGroupSender_YZ);
				}
			Assert(pEvent->m_pVaultParent_NZ == NULL);
			pEvent->m_pVaultParent_NZ = pVault;
			fEventsOutOfSync |= pVault->m_arraypaEvents.Event_FoosAddSorted(PA_CHILD pEvent);
			if (fEventsOutOfSync)
				pEvent->Event_SetFlagOutOfSync();
			if (pwChatLog != NULL)
				pwChatLog->ChatLog_EventDisplay(IN pEvent);

			// Update the GUI about the new event
			if (pMember != NULL)
				pMember->TreeItem_SetTextToDisplayMessagesUnread(++pMember->m_cMessagesUnread);	// The group member has unread messages as well as its group
			pChatLogEvents->TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(IN pEvent->PszGetTextOfEventForSystemTray(OUT_IGNORED &g_strScratchBufferStatusBar), this);
			TreeItem_IconUpdate();		// Update the icon of the contact, which will in turn update the icon(s) of all its aliases, including the member contact of the group.  It is important to update the icon of the contact because it is likely to be displaying the pencil icon indicating the user was composing/typing text.

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
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "tsOther $t  <  m_tsOtherLastSynchronized $t\n", tsOther, *ptsOtherLastSynchronized);
					}
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Out of sync by $T\n", dtsOtherSynchronization);
				}
			Assert(pEvent != NULL);
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
	if (binXcpStanzaReply.CbGetData() > cbXcpStanzaReplyEmpty)
		binXcpStanzaReply.XcpSendStanzaToContact(IN this);
	Assert(pVault->m_arraypaEvents.FEventsSortedByIDs());
	} // Xcp_ProcessStanzasAndUnserializeEvents()

void
CBinXcpStanzaType::BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(const IEvent * pEvent)
	{
	AssertValidEvent(pEvent);
	TContact * pContact = pEvent->m_pContactGroupSender_YZ;
	if (pContact != NULL)
		{
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		pContact->BinAppendXmlAttributeContactIdentifier(INOUT this, d_chXCPa_pContactGroupSender);
		}
	}

void
CBin::BinAppendXmlForSelfClosingElement()
	{
	BinAppendBinaryData("/>\n", 3);	// Close the XML element
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Method to serialize the event to be saved on disk disk.
void
CBinXcpStanzaType::BinXmlSerializeEventForDisk(const IEvent * pEvent)
	{
	const EEventClass eEventClass = pEvent->EGetEventClass();
	if ((eEventClass & eEventClass_kfNeverSerializeToDisk) == 0)
		{
		BinAppendTextSzv_VE("<$U" _tsI _tsO, eEventClass, pEvent->m_tsEventID, pEvent->m_tsOther);
		BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
		pEvent->XmlSerializeCore(IOUT this);
		BinAppendXmlForSelfClosingElement();
		}
	}
/*
//	Core method to serialize an event to be transmitted through the Cambrian Protocol.
//
//	This method is 'core' because it is shared with the 'serializer' as well the method re-creating the cache for the 'downloader'.
//	The class CEventDownloader relies on the number of bytes serialized by this method to transmit the event.
void
CBinXcpStanzaType::BinXmlSerializeEventForXcpCore(const IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	BinAppendTextSzv_VE((pEvent->m_tsOther > d_tsOther_kmReserved) ? "<$U" _tsO _tsI : "<$U" _tsO, pEvent->EGetEventClassForXCP(m_pContact), pEvent->m_tsEventID, pEvent->m_tsOther);	// Send the content of m_tsOther only if it contains a valid timestamp
	BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
	pEvent->XmlSerializeCore(IOUT this);
	BinAppendXmlForSelfClosingElement();
	}
*/

//	Method to serialize an event to be transmitted through the Cambrian Protocol.
//	The difference between saving an event to disk and for XCP, is the eEventClass must be adjusted for the recipient as well as swapping m_tsEventID and m_tsOther,
//	because any event 'sent' becomes an event 'received' by the remote contact.  This design decision was made, so the remote client unserialize the received events the same way as unserializing events from disk.
//
//	Also, this method ensures a large event will be sent in smaller chunks, so it may be transmitted through the XMPP protocol.
void
CBinXcpStanzaType::BinXmlSerializeEventForXcp(const IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_paData	!= NULL);
	const EEventClass eEventClass = pEvent->EGetEventClassForXCP(m_pContact);
	if ((eEventClass & eEventClass_kfNeverSerializeToXCP) == 0)
		{
		const int ibDataElementStart = m_paData->cbData;
		TIMESTAMP tsEventID = pEvent->m_tsEventID;
		TIMESTAMP tsOther = pEvent->m_tsOther;
		BinAppendTextSzv_VE((tsOther > d_tsOther_kmReserved) ? "<$U" _tsO _tsI : "<$U" _tsO, eEventClass, tsEventID, tsOther);	// Send the content of m_tsOther only if it contains a valid timestamp
		BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
		pEvent->XmlSerializeCore(IOUT this);
		if (m_pContact == NULL)
			return;		// The virtual method XmlSerializeCore() may set m_pContact if the contact does not support XCP.
		Assert(m_paData != NULL);
		#ifdef DEBUG
		if (m_paData->cbData > 100)		// For debugging conider 100 characters as a 'large stanza' to force this code to be executed frequently
		#else
		if (m_paData->cbData > c_cbStanzaMaxPayload)
		#endif
			{
			// The XML data is too large, therefore use a 'downloader event' which will take care of transferring the data in smaller chunks.
			// At this point it is tempting to create a cache of the XML data, however this approach is unreliable because the XML main contain m_tsOther which may change and screw up the synchronization process.
			// It is better to let the method PFindOrAllocateDataXmlLargeEvent_NZ() take care of it.
			MessageLog_AppendTextFormatCo(d_coOrange, "BinXmlSerializeEventForXcp() - Large stanza of about $I bytes\n", m_paData->cbData);
			m_paData->cbData = ibDataElementStart;	// Truncate the binary, so send the CEventDownloader instead of the event
			CDataXmlLargeEvent * pDataXmlLargeEvent = m_pContact->m_pAccount->PFindOrAllocateDataXmlLargeEvent_NZ(tsEventID, IN_MOD_TMP this);	// Create the cache, so we can get its size
			Assert(m_paData->cbData == ibDataElementStart);
			BinAppendTextSzv_VE("<" d_szXCPe_CEventDownloader_tsO_i, pEvent->m_tsEventID, pDataXmlLargeEvent->m_binXmlData.CbGetData());	// Serialize on-the-fly the CEventDownloader
			BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);				// Make sure the downloader goes to the right group
			}
		BinAppendXmlForSelfClosingElement();
		}
	} // BinXmlSerializeEventForXcp()

//	Send the event through XMPP.  If the contact can understand the Cambrian Protocol, then the event will be serialized for XCP, ortherwise will be sent throught the standard XMPP.
//
void
IEvent::Event_WriteToSocketIfNeverSent(CSocketXmpp * pSocket)
	{
	Assert(pSocket != NULL);
	Assert(pSocket->Socket_FuIsReadyToSendMessages());
	Assert(m_pVaultParent_NZ != NULL);
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
	} // Event_WriteToSocketIfNeverSent()

void
CArrayPtrEvents::EventsSerializeForDisk(INOUT CBinXcpStanzaType * pbinXmlEvents) const
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
			TIMESTAMP tsEvent = Timestamp_FromString_ML(pXmlAttributeTimestampEventID->m_pszuTagValue);
			if (tsEvent > tsEventLargest)
				{
				// We have a new event to add to the Chat Log
				tsEventLargest = tsEvent;
				EventAllocate:
				pEvent = IEvent::S_PaAllocateEvent_YZ(EEventClassFromPsz(pXmlNodeEvent->m_pszuTagName), IN &tsEvent);
				if (pEvent == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to allocate event $t of class '$s'\n", tsEvent, pXmlNodeEvent->m_pszuTagName);
					goto EventNext;
					}
				pEvent->m_pVaultParent_NZ = pVault;
				AssertValidEvent(pEvent);
				Assert(pEvent->m_tsEventID == tsEvent);
				fOutOfSync |= Event_FoosAddSorted(PA_CHILD pEvent);
				}
			else
				{
				// We are updating an existing event
				pEvent = PFindEventByID(tsEvent);
				if (pEvent == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "Info: EventsUnserializeFromDisk($s) - Out-of-sync EventID $s\n", pParent->TreeItem_PszGetNameDisplay(), pXmlAttributeTimestampEventID->m_pszuTagValue);
					goto EventAllocate;	// If the event is not in the history, then allocate it as a new event
					}
				} // if...else
			pXmlNodeEvent->UpdateAttributeValueTimestamp(d_chEvent_Attribute_tsOther, OUT_F_UNCH &pEvent->m_tsOther);	// Make sure m_tsOther is always unserialized

			PSZUC pszContactGroupSender = pXmlNodeEvent->PszuFindAttributeValue(d_chXCPa_pContactGroupSender);
			if (pszContactGroupSender != NULL)
				{
				pEvent->m_pContactGroupSender_YZ = pParent->m_pAccount->Contact_PFindByIdentifier(pszContactGroupSender);
				if (pEvent->m_pContactGroupSender_YZ == NULL)
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to find contact '$s' who sent the group EventID $t\n", pszContactGroupSender, tsEvent);
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
	#ifdef DEBUG
	if (m_paArrayHdr != NULL && m_paArrayHdr->cElements > 0)
		{
		int cEventsRemaining;
		IEvent * pEventFirst = PFindEventNextForXcp(d_ts_zNULL, OUT &cEventsRemaining);
		Assert(pEventFirst == m_paArrayHdr->rgpvData[0]);
		Assert(cEventsRemaining == m_paArrayHdr->cElements - 1);
		IEvent * pEventNext = PFindEventNextForXcp(pEventFirst->m_tsEventID, &cEventsRemaining);
		if (pEventNext != NULL)
			{
			Assert(pEventNext == m_paArrayHdr->rgpvData[1]);
			Assert(cEventsRemaining == m_paArrayHdr->cElements - 2);
			}
		else
			{
			Assert(m_paArrayHdr->cElements == 1);
			}
		IEvent * pEventLast = PGetEventLast_YZ();
		Assert(PFindEventNextForXcp(pEventLast->m_tsEventID, OUT &cEventsRemaining) == NULL);
		Assert(cEventsRemaining == 0);
		}
	#endif
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
	CBinXcpStanzaTypeInfo binXcpStanza;
	binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithContact(this);
	binXcpStanza.XcpSendStanzaToContact(IN this);
	}
/*
void
ITreeItemChatLogEvents::XcpStanza_AppendTimestampsOfLastKnownEvents(IOUT CBinXcpStanzaType * pbinXcpStanza) const
	{
	Assert(pbinXcpStanza != NULL);
	pbinXcpStanza->BinAppendTextSzv_VE("<" d_szXCPe_EventsLastKnown_tsO_tsI "/>", m_tsEventIdLastSent, m_tsOtherLastReceived);
	}
*/
/*
void
ITreeItemChatLogEvents::XcpStanza_AppendServiceDiscovery(IOUT CBinXcpStanza * pbinXcpStanza) const
	{
	Assert(pbinXcpStanza != NULL);
	pbinXcpStanza->BinAppendTextSzv_VE("<" d_szXCPe_EventInfo_tsO_tsI "/>");
	}
*/
/*
void
TContact::Xcp_WriteStanza_VE(PSZAC pszFmtTemplate, ...) CONST_MCC
	{
	CBinXcpStanzaTypeInfo binXcpStanza;
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	binXcpStanza.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	binXcpStanza.XcpSendStanzaToContact(IN this);
	}
*/
CBinXcpStanzaType::CBinXcpStanzaType(EStanzaType eStanzaType)
	{
	m_eStanzaType = eStanzaType;
	m_pContact = NULL;
	PvSizeAlloc(300);	// Pre-allocate 300 bytes, which should be enough for a small stanza
	}

CBinXcpStanzaTypeInfo::CBinXcpStanzaTypeInfo() : CBinXcpStanzaType(eStanzaType_zInformation)
	{
	}

CBinXcpStanzaTypeInfo::CBinXcpStanzaTypeInfo(IEvent * pEvent) : CBinXcpStanzaType(eStanzaType_zInformation)
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
CBinXcpStanzaType::BinXmlInitStanzaWithGroupSelector(TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	BinInitFromTextSzv_VE("<" d_szXCPe_GroupSelector ">{h|}</" d_szXCPe_GroupSelector ">", &pGroup->m_hashGroupIdentifier);
	}

void
CBinXcpStanzaType::XmppWriteStanzaToSocket()
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
CBinXcpStanzaType::XmppWriteStanzaToSocketOnlyIfContactIsUnableToCommunicateViaXcp_VE(PSZAC pszFmtTemplate, ...)
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
		m_pContact = NULL;	// Don't send the stanza via XMPP.  Of course this NULL pointer may be confused with serialized to disk, however since the whole CBinXcpStanzaType will be discarded, who cares?
		}
	}

//	Before allocating a CDataXmlLargeEvent, check if it is not already there.
//	This is important because if we are sending a message to a group, then the same CDataXmlLargeEvent is reused, otherwise
//	there will be a copy of CDataXmlLargeEvent for each group member.  Just imagine sending a large stanza of 100 KiB to a group of 1000 people.
CDataXmlLargeEvent *
TAccountXmpp::PFindOrAllocateDataXmlLargeEvent_NZ(TIMESTAMP tsEventID, IN_MOD_TMP CBinXcpStanzaType * pbinXcpStanza)
	{
	Assert(tsEventID > d_ts_zNULL);
	Assert(pbinXcpStanza != NULL);
	Assert(pbinXcpStanza->m_pContact != NULL);
	Assert(pbinXcpStanza->m_pContact->EGetRuntimeClass() == RTI(TContact));
	IEvent * pEvent;

	// First, search the list of large stanza
	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)m_listaDataXmlLargeEvents.pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		if (pDataXmlLargeEvent->m_tsEventID == tsEventID)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "PFindOrAllocateDataXmlLargeEvent_NZ() - Found existing CDataXmlLargeEvent $t  ($I bytes)\n", tsEventID, pDataXmlLargeEvent->m_binXmlData.CbGetData());
			m_listaDataXmlLargeEvents.MoveNodeToHead(INOUT pDataXmlLargeEvent);	// Move the node at the beginning of the list so next time its access is faster (this is a form of caching, in case there are many pending CDataXmlLargeEvent)
			goto Done;
			}
		pDataXmlLargeEvent = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		} // while
	// We have been unable to find a CDataXmlLargeEvent matching tsEventID, therefore we need to allocate a new one
	pDataXmlLargeEvent = new CDataXmlLargeEvent;
	pDataXmlLargeEvent->m_tsEventID = tsEventID;

	// We need to serialize the event, and then save the blob
	pEvent = pbinXcpStanza->m_pContact->Vault_PGet_NZ()->PFindEventByID(tsEventID);
	if (pEvent != NULL)
		{
		const int ibXmlDataStart = pbinXcpStanza->CbGetData();
		//pbinXcpStanza->BinAppendTextSzv_VE("<$U" _tsO, pEvent->EGetEventClassForXCP(pbinXcpStanza->m_pContact), pEvent->m_tsEventID);	// Do not serialize m_tsOther, as the CDownloader will have it.  Same for the group identifier as well as m_pContactGroupSender_YZ
		pbinXcpStanza->BinAppendTextSzv_VE("<$U", pEvent->EGetEventClassForXCP(pbinXcpStanza->m_pContact));	// No need to serialize the timestamps, group identifier, or m_pContactGroupSender_YZ, because the CEventDownloader has this information
		pEvent->XmlSerializeCore(IOUT pbinXcpStanza);
		pbinXcpStanza->BinAppendXmlForSelfClosingElement();
		Assert(pbinXcpStanza->m_pContact != NULL);	// Sometimes XmlSerializeCore() may modify m_pContact, however it should be only for contacts not supporting XCP (which should not be the case here!)
		int cbXmlData = pbinXcpStanza->CbGetData() - ibXmlDataStart;
		pDataXmlLargeEvent->m_binXmlData.BinInitFromBinaryData(pbinXcpStanza->TruncateDataPv(ibXmlDataStart), cbXmlData);	// Make a copy of the serialized data, and restore CBinXcpStanzaType to its original state
		MessageLog_AppendTextFormatSev(eSeverityComment, "PFindOrAllocateDataXmlLargeEvent_NZ() - Initializing cache by serializing tsEventID $t (tsOther $t) -> $I bytes of data\n", tsEventID, pEvent->m_tsOther, cbXmlData);
		}
	else
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "PFindOrAllocateDataXmlLargeEvent_NZ() - Unable to find tsEventID $t, therefore initializing an empty cache entry!\n", tsEventID);
		}
	HashSha1_CalculateFromCBin(OUT &pDataXmlLargeEvent->m_hashXmlData, IN pDataXmlLargeEvent->m_binXmlData);	// Calculate the checksum for the data
	m_listaDataXmlLargeEvents.InsertNodeAtHead(PA_CHILD pDataXmlLargeEvent);	// Insert at the head, as the event is most likely to be accessed within the next minutes

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
		delete pDataXmlLargeEventNext;
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
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[$@] CListaDataXmlLargeEvents::DeleteIdleNodes() - Deleting cache entry for tsEventID $t\n", pDataXmlLargeEvent->m_tsEventID);	// This is not an error, however good to draw attention
			DetachNode(INOUT pDataXmlLargeEvent);
			delete pDataXmlLargeEvent;
			}
		pDataXmlLargeEvent = pDataXmlLargeEventNext;
		} // while
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventDownloader::CEventDownloader(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_cbDataToDownload = 0;
	m_paEvent = NULL;
	}

CEventDownloader::~CEventDownloader()
	{
	delete m_paEvent;
	}

//	Return the class of the event when the download is complete.
//	This is where the morphing occurs, because the downloaded event will be serialized as a native object and next time Cambrian will load, the downloader will no longer exist.
EEventClass
CEventDownloader::EGetEventClass() const
	{
	if (m_paEvent != NULL)
		return m_paEvent->EGetEventClass();
	if (m_uFlagsEvent & FE_kfEventProtocolError)
		return eEventClass_mfNeverSerialize;	// Don't save the downloader if there is a protocol error.  This is to prevent the downloader to attempt to download something that no longer exist
	return c_eEventClass;
	}

EEventClass
CEventDownloader::EGetEventClassForXCP(const TContact * pContactToSerializeFor) const
	{
	if (m_paEvent != NULL)
		return m_paEvent->EGetEventClassForXCP(pContactToSerializeFor);
	return c_eEventClass;
	}

//	CEventDownloader::IEvent::XmlSerializeCore()
void
CEventDownloader::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	if (m_paEvent != NULL)
		{
		m_paEvent->XmlSerializeCore(IOUT pbinXmlAttributes);
		return;
		}
	pbinXmlAttributes->BinAppendXmlAttributeInt(d_chXCPa_CEventDownloader_cblDataToDownload, m_cbDataToDownload);
	pbinXmlAttributes->BinAppendXmlAttributeCBin(d_chXCPa_CEventDownloader_bin85DataReceived, m_binDataDownloaded);
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
	pXmlNodeElement->UpdateAttributeValueInt(d_chXCPa_CEventDownloader_cblDataToDownload, OUT_F_UNCH &m_cbDataToDownload);
	pXmlNodeElement->UpdateAttributeValueCBin(d_chXCPa_CEventDownloader_bin85DataReceived, OUT_F_UNCH &m_binDataDownloaded);
	}

void
CEventDownloader::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	if (m_paEvent != NULL)
		m_paEvent->XcpExtraDataRequest(pXmlNodeExtraData, pbinXcpStanzaReply);
	}

void
CEventDownloader::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	if (m_paEvent != NULL)
		m_paEvent->XcpExtraDataArrived(pXmlNodeExtraData, pbinXcpStanzaReply);
	}

void
CEventDownloader::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
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
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventDownloader::ChatLogUpdateTextBlock() - Event tsOther $t completed with $I bytes: $B\n", m_tsOther, cbDataDownloaded, &m_binDataDownloaded);
		m_pVaultParent_NZ->SetModified();	// Make sure whatever we do next will be saved to disk

		CXmlTree oXmlTree;
		if (oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN m_binDataDownloaded) == errSuccess)
			{
			m_paEvent = IEvent::S_PaAllocateEvent_YZ(EEventClassFromPsz(oXmlTree.m_pszuTagName), IN &m_tsEventID);
			if (m_paEvent != NULL)
				{
				Assert(m_paEvent->m_pContactGroupSender_YZ == NULL);
				m_paEvent->m_pContactGroupSender_YZ = m_pContactGroupSender_YZ;
				m_paEvent->m_pVaultParent_NZ = m_pVaultParent_NZ;
				m_paEvent->m_tsOther = m_tsOther;
				m_paEvent->XmlUnserializeCore(IN &oXmlTree);
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
		m_cbDataToDownload = c_cbDataToDownload_Error1;
		}
	else
		{
		EventUpdateToGUI:
		m_paEvent->ChatLogUpdateTextBlock(INOUT poCursorTextBlock);
		} // if...else
	} // ChatLogUpdateTextBlock()

void
CEventDownloader::XcpDownloadedDataArrived(const CXmlNode * pXmlNodeData, CBinXcpStanzaType * pbinXcpStanzaReply, QTextEdit * pwEditChatLog)
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


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core routine sending a stanza to a contact.
//	This routine takes care of encrypting the data and digitally signing the message.
void
CBinXcpStanzaType::XcpSendStanzaToContact(TContact * pContact) CONST_MCC
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
CBinXcpStanzaType::XcpSendStanza() CONST_MCC
	{
	XcpSendStanzaToContact(m_pContact);
	}

void
CBinXcpStanzaType::BinXmlAppendTimestampsToSynchronizeWithContact(TContact * pContact)
	{
	BinAppendTextSzv_VE("<" d_szXCPe_EventPrevious_tsO_tsI "/>", pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
	}

void
CBinXcpStanzaType::BinXmlAppendTimestampsToSynchronizeWithGroupMember(TGroupMember * pMember)
	{
	BinAppendTextSzv_VE("<" d_szXCPe_EventPrevious_tsO_tsI "/>", pMember->m_pGroup->m_tsEventIdLastSentCached, pMember->m_tsOtherLastSynchronized);
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
		binXcpStanza.BinAppendStringWithoutNullTerminator((eChatState == eChatState_zComposing) ? d_szXCPe_MessageTextComposingStarted : d_szXCPe_MessageTextComposingPaused);
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
	binXcpStanza.BinAppendStringWithoutNullTerminator((eChatState == eChatState_zComposing) ? d_szXCPe_MessageTextComposingStarted : d_szXCPe_MessageTextComposingPaused);

	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		Assert(pMember->m_pGroup == this);
		Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
		binXcpStanza.XcpSendStanzaToContact(IN pMember->m_pContact);
		}
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
CEventFileSent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	Assert(pXmlNodeExtraData != NULL);
	BYTE rgbBuffer[CBinXcpStanzaType::c_cbStanzaMaxBinary];
	L64 iblDataSource = pXmlNodeExtraData->LFindAttributeXcpOffset();
	int cbDataRead = _PFileOpenReadOnly_NZ()->CbDataReadAtOffset(iblDataSource, sizeof(rgbBuffer), OUT rgbBuffer);
	m_cblDataTransferred = iblDataSource + cbDataRead;
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataReply_tsO d_szXCPa_EventExtraData_iblOffset_l d_szXCPa_EventExtraData_bin85Payload_pvcb "/>", m_tsEventID, iblDataSource, IN rgbBuffer, cbDataRead);
	if (cbDataRead <= 0)
		_FileClose();	// There is nothing to send, therefore assume we reached the end of the file, and consequently close the file.  Of course, if the contact wishes more data (or request to resend the last block, then the file will be re-opened)
	}

void
CEventFileReceived::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
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
		_FileClose();
	}
