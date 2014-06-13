///////////////////////////////////////////////////////////////////////////////////////////////////
//	Xcp.cpp
//
//	Collection of functions and methods related to the XCP (eXtensible Cambrian Protocol) as well as the event serialization/unserialization for XCP.

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "Xcp.h"

//	Essentially, there are 3 reserved attributes by the Cambrian Protocol ('i', 'o' and 'g')
//	The following attributes 'make sense' for the receiver of the XCP data.
#define d_chXCPa_tsEventID				d_chEvent_Attribute_tsEventID
#define d_szXCPa_tsEventID_t			d_szEvent_Attribute_tsEventID_t
#define d_chXCPa_tsOther				d_chEvent_Attribute_tsOther
#define d_szXCPa_tsOther_t				d_szEvent_Attribute_tsOther_t
#define d_chXCPa_pContactGroupSender	'g'		// Pointer of the contact who sent the event/message to the group.  Of course, this attribute is present only when receiving a group message

#define _tsI		d_szXCPa_tsEventID_t
#define _tsO		d_szXCPa_tsOther_t


//	The XCP protocol uses lowercase and uppercase letters:
//	A lowercase letter designates an xcp-stanza (a stanza within an xmpp-stanza) where there could be a reply.
//	An uppercase letter designates a reply to an xcp-stanza.  There is never a reply to a reply, otherwise it may cause an infinite loop.
//	The tsOther and tsEventID 'make sense' for the receiver of the xcp-stanza; it is the responsibility of the sender to swap those values accordingly.

#define d_chXCPe_GroupSelector							'g'	// Which group to assign the events.  The body of the XML element is the identifier of the group.
#define d_szXCPe_GroupSelector					d_szXCP_"g"

#define d_chXCPe_EventPrevious							'p'	// Send the timestamp of the previous event, as well as the synchronization
#define d_szXCPe_EventPrevious_tsO_tsI			d_szXCP_"p"	_tsO _tsI
#define d_chXCPe_EventsMissing							'M'	// Notification to the remote client some events are missing, and therefore should synchronize
#define d_szXCPe_EventsMissing					d_szXCP_"M"

#define d_chXCPe_EventNextRequest						'n'	// Ask the remote client to send its next event after tsEventID
#define d_szXCPe_EventNextRequest_tsI			d_szXCP_"n"	_tsI
#define d_chXCPe_EventNextReply							'N'	// Reply to the sender regarding the request of tsEventID and tsOther (the following xcp-stanzas contains serialized IEvents)
#define d_szXCPe_EventNextReply_tsO				d_szXCP_"N" _tsO
#define d_chXCPe_EventsOutstandingReply					'O'	// Any message reply 'N' must end with 'O' to indicate hoe many outstanding IEvents remain.
#define d_szXCPe_EventsOutstandingReply_i_tsO	d_szXCP_"O c='$i'" _tsO // ('R' = Remaining)
	#define d_chXCPa_EventsOutstandingReply_cEvents				'c'
#define d_chXCPe_EventConfirmation						'C'	// Confirmation an event was processed successfully (the confirmation only includes tsEventID of the sender)
#define d_szXCPe_EventConfirmation_tsI			d_szXCP_"C" _tsI
#define d_chXCPe_EventError								'E'	// The event was received, however could not be processed because its class/type is unknown. 'C' and 'E' are mutually exclusive.
#define d_szXCPe_EventError_tsI_s				d_szXCP_"E" _tsI " e='^s'"	// e=event, c=code of the error (if any), t=friendly text to display to the user (if any)

#define d_chXCPe_EventExtraDataRequest					'x'	// Request an event to send extra data (for instance, downloading the content of a file)
#define d_szXCPe_EventExtraDataRequest_tsI		d_szXCP_"x" _tsI
#define d_chXCPe_EventExtraDataReply					'X'	// Reply from the event with the data payload
#define d_szXCPe_EventExtraDataReply			d_szXCP_"X"
#define d_szXCPe_EventExtraDataReply_tsO		d_szXCP_"X" _tsO

	#define d_chXCPa_EventExtraData_strxIdentifier				'I'		// Identifier of the extra data (this is optional, as sometimes there is only one extra data)
	#define d_chXCPa_EventExtraData_iblOffset					'O'		// Offset of the extra data (this is important, because the extra data is sent by smaller pieced of about 4 KiB)
	#define d_szXCPa_EventExtraData_iblOffset_l					" O='$l'"
	#define d_chXCPa_EventExtraData_binPayloadBase85			'b'
	#define d_szXCPa_EventExtraData_binPayloadBase85			"b"

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
	TIMESTAMP_DELTA dtsSynchronization = 1;	// Make sure
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
				dtsSynchronization = tsOther - *ptsOtherLastSynchronized;
				fEventsOutOfSync = (dtsSynchronization != 0);
				if (dtsSynchronization > 0)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t m_tsOtherLastSynchronized = $t however its value should be $t\n"
						"\t\t\t therefore requesting $s to resend its next event after $t\n", *ptsOtherLastSynchronized, tsOther, pszNameDisplay, *ptsOtherLastSynchronized);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventNextRequest_tsI "/>", *ptsOtherLastSynchronized);
					}
				else if (dtsSynchronization < 0)
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
					pEvent->XmlSerializeForXCP(INOUT &binXcpStanzaReply);
					binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventsOutstandingReply_i_tsO "/>", cEventsRemaining, pEvent->m_tsEventID);
					}
				break;
			case d_chXCPe_EventNextReply:	// Although d_chXCPe_EventNextReply may appear as useless, the variable chXCPe is set to d_chXCPe_EventNextReply which will be useful when unserializing events.
				Assert(tsEventID == d_ts_zNA);
				dtsSynchronization = tsOther - *ptsOtherLastSynchronized;
				if (dtsSynchronization == 0)
					MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t chXCPe = d_chXCPe_EventNextReply;\n");
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t chXCPe = d_chXCPe_EventNextReply; (out of sync by $T)\n", dtsSynchronization);
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
				binXcpStanzaReply.XcpAppendTimestampsToSynchronizeWithContact(this); // Need to fix this for group
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
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Invalid confirmation EventID $t\n", tsEventID);
				break;
			case d_chXCPe_EventExtraDataRequest:
				Assert(tsEventID != d_ts_zNA);
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
					pEvent = pVault->PFindEventByTimestampOther(tsOther);
					}
				if (pEvent != NULL)
					{
					pEvent->XcpExtraDataArrived(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply);
					pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);	// This will typically show a progress bar
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find i=$t, o=$t for d_chXCPe_EventExtraDataReply\n", tsEventID, tsOther);
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
			const BOOL fSynchronizingEvents = (chXCPe == d_chXCPe_EventNextReply);
			if (!fSynchronizingEvents)
				{
				// We are receiving new events from the remote client
				Assert(tsEventID == d_ts_zNULL);						// New events should not have a tsEventID
				binXcpStanzaReply.m_eStanzaType = eStanzaType_eMessage;	// Any event confirmation (or error) shall be cached as an 'XMPP message' so the remote client may know about it
				if (pwChatLog != NULL)
					pwChatLog->ChatLog_ChatStateComposerRemove(this);	// Make sure the text "<user> is typing..." will be no longer displayed
				if (pGroup == NULL)
					{
					// Search if the 'new' event is not already in the Chat Log.  This happens if the event is a resend.
					pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther);
					if (pEvent != NULL)
						{
						MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Updating existing tsEventID $t matching tsOther $t...\n", pEvent->m_tsEventID, tsOther);
						EventUnserialize:
						Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
						pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);	// Need to mark the event as updated
						pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);
						goto EventConfirmation;
						}
					// The event cannot be found in our vault (Chat Log), therefore allocate it
					}
				else
					{
					// We have received a new event for group chat
					pEvent = pVault->PFindEventReceivedByTimestampOtherMatchingContactSender(tsOther, this);
					if (pEvent != NULL)
						{
						MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Updating existing group tsEventID $t matching tsOther $t...\n", pEvent->m_tsEventID, tsOther);
						goto EventUnserialize;
						}
					}
				}
			else
				{
				// We are synchronizing by receiving previous events
				Assert(tsEventID == d_ts_zNULL || tsEventID > d_tsOther_kmReserved);	// The EventID may be zero for the case of receiving an event which was never delivered before
				//	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "tsEventID = $t for eEventClass=$i ($s)\n", tsEventID, eEventClass, pszEventName);
				if (pGroup == NULL)
					{
					// We are synchronizing an event for a one-to-one conversation
					if (tsEventID > d_tsOther_kmReserved)
						{
						// If tsEventID is present, it means the event is a re-send.
						pEvent = pVault->PFindEventByID(tsEventID);
						if (pEvent != NULL)
							{
							MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Ignoring tsEventID $t\n", tsEventID);	// We already have this event
							goto EventConfirmation;	// Just confirm we receiving it
							}
						// Although the event is a re-send, we may not have it because the Chat Log was deleted or the machine previously crashed before having time to save the event to disk
						MessageLog_AppendTextFormatSev(eSeverityNoise, "\t\t [Sync] Unable to find tsEventID $t, therefore allocating it.\n", tsEventID);
						}
					pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther);
					if (pEvent != NULL)
						{
						MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Unserializing event matching $t matching tsOther $t\n", pEvent->m_tsEventID, tsOther);
						goto EventUnserialize;
						}
					}
				else
					{
					// We are synchronizing an event for a group (this is quite complicated)
					Assert(pMember != NULL);
					pEvent = pVault->PFindEventReceivedByTimestampOtherMatchingContactSender(tsOther, this);
					if (pEvent != NULL)
						{
						/*
						PSZUC pszContactGroupSender = pXmlNodeEvent->PszuFindAttributeValue(d_chXCPa_pContactGroupSender);
						if (pszContactGroupSender != NULL)
							{
							pEvent->m_pContactGroupSender_YZ = pParent->m_pAccount->Contact_PFindByIdentifier(pszContactGroupSender);
							}
						*/
						}
					}
				/*
				if (pGroup != NULL)
					{
					// Group synchronization is a bit more complex, as tsOther has meaning only in the context of the sender
					if (USZU1_from_USZUX(eEventClass) == _g_)
						{
						pEvent = pVault->PFindEventReceivedByTimestampOtherMatchingContactSender(tsOther, this);
						if (pEvent != NULL)
							{
							MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Unserializing group event tsOther $t...\n", tsOther);
							goto EventUnserialize;
							}
						MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Allocating group event tsOther $t...\n", tsOther);
						goto EventNew;
						}
					} // if (group)
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					if (pEvent->Event_FIsEventTypeReceived())
						{
						MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Unserializing event tsEventID $t...\n", tsEventID);
						goto EventUnserialize;
						}
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Skipping the unserialization of tsEventID because it is OUR OWN event!\n", tsEventID);
					if (tsEventID > *ptsEventIdLastSent)
						{
						MessageLog_AppendTextFormatSev(eSeverityNoise, "\t\t\t Updating m_tsEventIdLastSent from $t to $t\n", *ptsEventIdLastSent, tsEventID);
						*ptsEventIdLastSent = tsEventID;
						}
					goto EventNext;	// Skip the unserialization because it is out own event
					}
				*/
				} // if...else

			// We have a new event to allocate and display into the Chat Log.
			EventNew:
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
			/*
				}
			else
				{
				// We have an old event to update
				if (pGroup == NULL)
					{
					pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther);	// Search the history to find the event to update
					if (pEvent == NULL)
						{
						if (!fSynchronizingEvents)
							MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Xcp_ProcessStanzasAndUnserializeEvents($s) - Unable to find previous tsOther $t, therefore allocating new event.\n", pszNameDisplay, tsOther);
						goto EventNew;	// If the event is not in the history, then allocate it as a new event
						}
					}
				else
					{
					// We are updating a group event.  Since the tsOther group events may are semi-sorted, it is not an error if the event is not found
					pEvent = pVault->PFindEventReceivedByTimestampOtherMatchingContactSender(tsOther, this);
					if (pEvent == NULL)
						goto EventNew;
					}
				MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Updating event tsOther = $t...\n", tsOther);
				EventUnserialize:
				Assert(pEvent != NULL);
				pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);
				if (fEventsOutOfSync)
					pEvent->Event_SetFlagOutOfSync();
				pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);
				} // if...else
			*/
			EventConfirmation:
			binXcpStanzaReply.BinAppendTextSzv_VE("<" d_szXCPe_EventConfirmation_tsI "/>", tsOther);	// Acknowledge to the remote client we received (and processed) the event
			EventSynchronize:
			if (dtsSynchronization == 0)
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
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Out of sync by $T\n", dtsSynchronization);
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

/*
EEventClass
IEvent::S_EGetEventClassFromXmlStanzaXCP(IN const CXmlNode * pXmlNodeEventsStanza, INOUT TContact * pContact, INOUT ITreeItemChatLogEvents * pChatLogEvents, IOUT CBinXcpStanzaType * pbinXmlStanzaReply)
	{
	Assert(pXmlNodeEventsStanza != NULL);
	Assert(pContact != NULL);			// Contact where the event was received
	Assert(pChatLogEvents != NULL);		// Contact (or group) where the event should be displayed
	Assert(pChatLogEvents->EGetRuntimeClass() == RTI(TContact) || pChatLogEvents->EGetRuntimeClass() == RTI(TGroup));
	Assert(pbinXmlStanzaReply != NULL);
	EEventClass eEventClass = EEventClassFromPsz(pXmlNodeEventsStanza->m_pszuTagName);
	switch (eEventClass)
		{
	case eEventClass_eMessageTextComposing:	// The contact is typing something
		pChatLogEvents->ChatLog_ChatStateIconUpdate((pXmlNodeEventsStanza->PFindAttribute(d_chXCPa_MessageTextComposing_State) == NULL) ? eChatState_zComposing : eChatState_fPaused, pContact);
		return eEventClass_eNull;
	case eEventClass_ePing:
		// The XCP ping returns the timestamp (in UTC) of the contact.  This way, it is possible to calculate the clock difference between the two devices.  Ideally the clock difference should be less than one minute.
		pbinXmlStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataReply _tsI "/>", pXmlNodeEventsStanza->TsGetAttributeValueTimestamp_ML()  m_tsEventID, iblDataSource, IN rgbBuffer, cbDataRead);
		return eEventClass_eNull;
	case eEventClass_eServiceDiscovery_Query:
		pbinXmlStanzaReply->BinAppendStringWithoutNullTerminator("<sd/>");
		return eEventClass_eNull;
	case eEventClass_eServiceDiscovery_Response:
		// We are receiving a response of what the remote client is capable
		//pContact->m_uFlagsContactSerialized |= TContact::FCS_kfServiceDiscovery;
		return eEventClass_eNull;
	default:
		return eEventClass;
		} // switch
	} // S_EGetEventClassFromXmlStanzaXCP()
*/

void
IEvent::XmlSerializeAttributeContactIdentifierOfGroupSender(IOUT CBin * pbinXml)
	{
	Assert(pbinXml != NULL);
	if (m_pContactGroupSender_YZ != NULL)
		{
		Assert(m_pContactGroupSender_YZ->EGetRuntimeClass() == RTI(TContact));
		m_pContactGroupSender_YZ->BinAppendXmlAttributeContactIdentifier(INOUT pbinXml, d_chXCPa_pContactGroupSender);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Method to serialize the event to be saved on disk disk.
void
IEvent::XmlSerializeForDisk(INOUT CBinXcpStanzaType * pbinXmlDisk)
	{
	const EEventClass eEventClass = EGetEventClass();
	if ((eEventClass & eEventClass_kfNeverSerializeToDisk) == 0)
		{
		pbinXmlDisk->BinAppendTextSzv_VE("<$U" _tsI _tsO, eEventClass, m_tsEventID, m_tsOther);
		XmlSerializeAttributeContactIdentifierOfGroupSender(INOUT pbinXmlDisk);
		XmlSerializeCore(INOUT pbinXmlDisk);
		pbinXmlDisk->BinAppendBinaryData("/>\n", 3);	// Close the XML element
		}
	}

//	Method to serialize the event to be transmitted through the Cambrian Protocol.
//	The difference between saving an event to disk and for XCP, is the eEventClass must be adjusted for the recipient as well as swapping m_tsEventID and m_tsOther, because any event 'sent' becomes an event 'received'.
//	This is necessary for the remote client unserialize the events the same way as unserializing them from disk.
void
IEvent::XmlSerializeForXCP(INOUT CBinXcpStanzaType * pbinXcpStanza)
	{
	Assert(pbinXcpStanza != NULL);
	Assert(pbinXcpStanza->m_pContact != NULL);
	const EEventClass eEventClass = EGetEventClassForXCP(pbinXcpStanza->m_pContact);
	if ((eEventClass & eEventClass_kfNeverSerializeToXCP) == 0)
		{
		pbinXcpStanza->BinAppendTextSzv_VE((m_tsOther > d_tsOther_kmReserved) ? "<$U" _tsO _tsI : "<$U" _tsO, eEventClass, m_tsEventID, m_tsOther);	// Send the content of m_tsOther only if it contains a valid timestamp
		XmlSerializeAttributeContactIdentifierOfGroupSender(INOUT pbinXcpStanza);
		XmlSerializeCore(INOUT pbinXcpStanza);
		pbinXcpStanza->BinAppendBinaryData("/>\n", 3);	// Close the XML element
		}
	}

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
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGrayDark), "\t Sending message to $S\n\t m_tsEventIdLastSentCached $t, m_tsOtherLastSynchronized $t\n", &pContact->m_strJidBare, pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
		binXcpStanza.m_pContact = pContact;
		binXcpStanza.XcpAppendTimestampsToSynchronizeWithContact(pContact);
		XmlSerializeForXCP(IOUT &binXcpStanza);
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
			binXcpStanza.BinInitStanzaWithGroupSelector(pGroup);	// This line could be removed out of the loop
			binXcpStanza.XcpAppendTimestampsToSynchronizeWithGroupMember(pMember);
			XmlSerializeForXCP(IOUT &binXcpStanza);
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
		pEvent->XmlSerializeForDisk(IOUT pbinXmlEvents);
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
		IEvent * pEventFirst = PFindEventNext(d_ts_zNULL, OUT &cEventsRemaining);
		Assert(pEventFirst == m_paArrayHdr->rgpvData[0]);
		Assert(cEventsRemaining == m_paArrayHdr->cElements - 1);
		IEvent * pEventNext = PFindEventNext(pEventFirst->m_tsEventID, &cEventsRemaining);
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
		Assert(PFindEventNext(pEventLast->m_tsEventID, OUT &cEventsRemaining) == NULL);
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
	binXcpStanza.XcpAppendTimestampsToSynchronizeWithContact(this);
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
		BinInitStanzaWithGroupSelector(pContactOrGroup);
		}
	}

void
CBinXcpStanzaType::BinInitStanzaWithGroupSelector(TGroup * pGroup)
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

void
CBinXcpStanzaType::XcpSendStanzaToContact(TContact * pContact) const
	{
	if (pContact == NULL)
		return;	// This is not a bug, but a feature allowing an event to write to the socket directly because the contact is unable to understand XCP.
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_eStanzaType != eStanzaType_eBroadcast && "Not yet supported");
	if (pContact->m_cVersionXCP <= 0)
		{
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "Contact not supporting XCP - Therefore ignoring XcpSendStanzaToContact($s):\n$B\n", pContact->ChatLog_PszGetNickname(), this);
		return;
		}
	MessageLog_AppendTextFormatCo(d_coBlue, "XcpSendStanzaToContact($s):\n$B\n", pContact->ChatLog_PszGetNickname(), this);

	// TODO: Encrypt pbinXmlEvents and sign it.
	SHashSha1 hashSignature;
	HashSha1_CalculateFromCBin(OUT &hashSignature, IN *this);	// At the moment, use SHA-1 as the 'signature'

	PSZAC pszStanzaType = c_sza_message;
	PSZAC pszStanzaAttributesExtra = NULL;
	if (m_eStanzaType == eStanzaType_zInformation)
		{
		pszStanzaType = c_sza_iq;
		pszStanzaAttributesExtra = " type='get'";
		}
	// Format the XML envelope for the XMPP protocol.
	g_strScratchBufferSocket.BinInitFromTextSzv_VE("<$s$s to='^J'><" d_szCambrianProtocol_xcp " " d_szCambrianProtocol_Attribute_hSignature "='{h|}'>", pszStanzaType, pszStanzaAttributesExtra, pContact, &hashSignature);
	g_strScratchBufferSocket.BinAppendStringBase85FromBinaryData(IN this);
	g_strScratchBufferSocket.BinAppendTextSzv_VE("</" d_szCambrianProtocol_xcp "></$s>", pszStanzaType);
	CSocketXmpp * pSocket = pContact->Xmpp_PGetSocketOnlyIfReady();
	if (pSocket != NULL)
		pSocket->Socket_WriteBin(g_strScratchBufferSocket);
	} // XcpSendStanzaToContact()

void
CBinXcpStanzaType::XcpSendStanza() const
	{
	XcpSendStanzaToContact(m_pContact);
	}

void
CBinXcpStanzaType::XcpAppendTimestampsToSynchronizeWithContact(TContact * pContact)
	{
	BinAppendTextSzv_VE("<" d_szXCPe_EventPrevious_tsO_tsI "/>", pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
	}

void
CBinXcpStanzaType::XcpAppendTimestampsToSynchronizeWithGroupMember(TGroupMember * pMember)
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

/*
https://tools.ietf.org/html/draft-saintandre-rfc3920bis-09
XMPP is optimized for the exchange of relatively large numbers of
relatively small stanzas.  A client or server MAY enforce a maximum
stanza size.  The maximum stanza size MUST NOT be smaller than 10000
bytes, from the opening "<" character to the closing ">" character.
If an entity receives a stanza that exceeds its maximum stanza size,
it MUST return a <not-acceptable/> stanza error or a <policy-
violation/> stream error.
*/

void
TContact::Xmpp_WriteXmlChatState(EChatState eChatState) CONST_MCC
	{
	Assert(eChatState == eChatState_zComposing || eChatState == eChatState_fPaused);
	if ((m_uFlagsContact & FC_kfNoCambrianProtocol) == 0)
		{
		CBinXcpStanzaTypeInfo binXcpStanza;
		if (m_uFlagsContact & FC_kfXcpComposingSendTimestampsOfLastKnownEvents)
			{
			m_uFlagsContact &= ~FC_kfXcpComposingSendTimestampsOfLastKnownEvents;
			binXcpStanza.XcpAppendTimestampsToSynchronizeWithContact(this);
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

/*
BOOL
CEventGroupMessageReceived::SystemTray_FDisplayEvent() const
	{
	Assert(mu_parentowner.pGroup->EGetRuntimeClass() == RTI(TGroup));
	if (m_pContactGroupSender != NULL)
		mu_parentowner.pGroup->TreeItemGroup_NewMessageArrived(IN m_strMessageText, m_pContactGroupSender);
	return TRUE;
	}
*/
void
CEventFileSent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	Assert(pXmlNodeExtraData != NULL);
	BYTE rgbBuffer[4096];	// 4 KiB is the recommended payload under XMPP
	L64 iblDataSource = pXmlNodeExtraData->LFindAttributeValueDecimal_ZZR(d_chXCPa_EventExtraData_iblOffset);
	int cbDataRead = _PFileOpenReadOnly_NZ()->CbDataReadAtOffset(iblDataSource, sizeof(rgbBuffer), OUT rgbBuffer);
	m_cblDataTransferred = iblDataSource + cbDataRead;
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataReply_tsO d_szXCPa_EventExtraData_iblOffset_l " " d_szXCPa_EventExtraData_binPayloadBase85 "='{p|}'/>", m_tsEventID, iblDataSource, IN rgbBuffer, cbDataRead);
	if (cbDataRead <= 0)
		_FileClose();	// There is nothing to send, therefore we close the file.  Of course, if the contact wishes more data (or request to resend the last block, then the file will be re-opened)
	}

void
CEventFileReceived::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	CFile * pFile = _PFileOpenWriteOnly_NZ();
	pFile->seek(pXmlNodeExtraData->LFindAttributeValueDecimal_ZZR(d_chXCPa_EventExtraData_iblOffset));
	int cbDataWritten = pFile->CbDataWriteBinaryFromBase85(pXmlNodeExtraData->PszuFindAttributeValue(d_chXCPa_EventExtraData_binPayloadBase85));
	if (cbDataWritten > 0)
		{
		// Request the next piece of data
		m_cblDataTransferred = pFile->size();
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szXCPe_EventExtraDataRequest_tsI d_szXCPa_EventExtraData_iblOffset_l "/>", m_tsOther, m_cblDataTransferred);
		}
	else
		_FileClose();
	}
