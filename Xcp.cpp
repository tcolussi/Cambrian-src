///////////////////////////////////////////////////////////////////////////////////////////////////
//	Xcp.cpp
//
//	Collection of functions and methods related to the XCP (eXtensible Cambrian Protocol) as well as the event serialization/unserialization for XCP.

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "Xcp.h"
#include "XcpApi.h"

void
CBinXcpStanza::BinXmlAppendXcpAttributesForApiRequestError(EErrorXcpApi eErrorXcpApi, PSZUC pszxErrorData)
	{
	Assert(eErrorXcpApi != eErrorXcpApi_zSuccess);
	Assert(pszxErrorData != NULL);
	/* TBD
	if (m_ibXmlApiReply <= 0)
		return;
	Assert(m_paData != NULL);
	m_paData->cbData = m_ibXmlApiReply - 1;		// Remove the tailing '>' from BinXmlAppendXcpElementApiReplyOpen()
	Assert(m_paData->rgbData[m_paData->cbData] == '>');
	BinAppendText_VE(d_szXCPa_Api_ErrorCodeAndErrorData_i_s ">", eErrorXcpApi, pszxErrorData);
	m_ibXmlApiReply = d_zNA;
	*/
	}

//	Invoke a function by sending an API request to the contact.  This is essentially invoking a remote procedure call.
void
ITreeItemChatLogEvents::XcpApi_Invoke(PSZUC pszApiName, const CXmlNode * UNUSED_PARAMETER(pXmlNodeApiParameters), PSZUC UNUSED_PARAMETER(pszXmlApiParameters))
	{
	UNUSED_PARAMETER(pXmlNodeApiParameters);
	UNUSED_PARAMETER(pszXmlApiParameters);
	CBinXcpStanza binXcpStanza;
	binXcpStanza.BinAppendText_VE("<" d_szXop_ApiCall_s "/>", pszApiName);
	if (EGetRuntimeClass() == RTI(TContact))
		binXcpStanza.XospSendStanzaToContactAndEmpty((TContact *)this);
	}

void
CBinXcpStanza::XcpApi_CallApiWithResponseToEvent(const IEvent * pEvent, PSZUC pszApiName)
	{
	Assert(pEvent != NULL);
	if (m_pContact == NULL)
		return;	// This happens when serializing to disk
	CTaskSendReceive * pTask = m_pContact->m_listaTasksSendReceive.PAllocateTaskSend();
	pTask->m_binXmlData.BinAppendText_VE("<" d_szXop_ApiCall_RespondToEventID_s_ts_pE "/>", pszApiName, pEvent->m_tsEventID, pEvent);
	MessageLog_AppendTextFormatCo(d_coGreen, "XcpApi_CallApiWithResponseToEvent($t) - $B\n", pEvent->m_tsEventID, &pTask->m_binXmlData);
	}

void
CBinXcpStanza::XcpApi_SendDataToEvent_VE(const IEvent * pEvent, PSZAC pszFmtTemplate, ...)	// XcpApi_SendDataToContactForEvent_VE(), XcpApi_SendDataToContactForEventTask_VE()
	{
	Assert(pEvent != NULL);
	TContact * pContact = pEvent->PGetContactForReply_YZ();
	Assert(pContact != NULL);
	if (pContact == NULL)
		return;
	CTaskSendReceive * pTask = pContact->m_listaTasksSendReceive.PAllocateTaskSend();
	pTask->m_binXmlData.BinAppendText_VE("<" d_szXop_ApiDataToEventID_ts_pE ">", pEvent->m_tsOther, pEvent);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	pTask->m_binXmlData.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	pTask->m_binXmlData.BinAppendText_VE("</" d_szXop_ApiDataToEventID ">");
	XospSendStanzaToContactAndEmpty(pContact);
	}

void
IEvent::XcpRequesExtraData()
	{
	CBinXcpStanza binXcpStanza;
	binXcpStanza.XcpApi_SendDataToEvent_VE(this, "<x/>");
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Method to serialize the event to be saved on disk.
void
CBinXcpStanza::BinXmlSerializeEventForDisk(const IEvent * pEvent)
	{
	Assert(FuSerializingEventToDiskOrCloning());
	EEventClass eEventClass = pEvent->EGetEventClass();
	if ((eEventClass & eEventClass_kfNeverSerializeToDisk) == 0)
		{
		BinAppendTextOffsetsInit_VE(OUT &m_oOffsets, "<$U" _tsI _tsO, pEvent->EGetEventClass(), pEvent->m_tsEventID, pEvent->m_tsOther);
		BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(IN pEvent);
		BinAppendXmlEventSerializeDataAndClose(pEvent);
		}
	} // BinXmlSerializeEventForDisk()

void
CBinXcpStanza::BinAppendXmlEventSerializeOpen(const IEvent * pEvent, TIMESTAMP tsOther)
	{
	Assert(pEvent != NULL);
	//Assert(tsOther == d_ts_zNA || tsOther == pEvent->m_tsOther);
	BinAppendTextOffsetsInit_VE(OUT &m_oOffsets, (tsOther != d_ts_zNA) ? "<$U" _tsO _tsI : "<$U" _tsO, pEvent->EGetEventClassForXCP(), pEvent->m_tsEventID, tsOther);	// Send the content of m_tsOther only if it contains a valid timestamp
	}

void
CBinXcpStanza::BinAppendXmlEventSerializeDataAndClose(const IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	//Assert(m_oOffsets.ibDataBegins > 0);
	BinAppendXmlAttributeUIntHexadecimal(d_chXCPa_IEvent_uFlagsEvent, pEvent->m_uFlagsEvent & IEvent::FE_kmSerializeMask);
	const EXml eXml = pEvent->XmlSerializeCoreE(IOUT this);
	switch (eXml)
		{
	case eXml_zAttributesOnly:
		BinAppendXmlForSelfClosingElement();
		break;
	case eXml_ElementPresent:
		BinAppendText_VE("</$U>\n", FuSerializingEventToDiskOrCloning() ? pEvent->EGetEventClass() : pEvent->EGetEventClassForXCP());
		break;
	case eXml_NoSerialize:
		m_paData->cbData = m_oOffsets.ibReset;	// Flush the data
		} // switch
	m_oOffsets.ibDataBegins = 0;	// For debugging, to make sure BinAppendXmlEventSerializeOpen() is being called
	}

//	Core method to serialize an event to be transmitted through the Cambrian Protocol.
void
CBinXcpStanza::BinXmlSerializeEventForXcpCore(const IEvent * pEvent, TIMESTAMP tsOther)
	{
	Assert(pEvent != NULL);
	Assert(tsOther == d_ts_zNULL || tsOther == pEvent->m_tsOther);
	Assert(m_pContact != NULL);
	if (m_pContact == NULL)
		return;
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	BinAppendXmlEventSerializeOpen(pEvent, tsOther);
	BinAppendXmlEventSerializeDataAndClose(pEvent);
	}



void
CArrayPtrEvents::EventsSerializeForDisk(INOUT CBinXcpStanza * pbinXmlEvents) const
	{
	Assert(pbinXmlEvents->FuSerializingEventToDisk());
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
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to allocate event $t ({tL}) of class '$s'\n", tsEventID, tsEventID, pXmlNodeEvent->m_pszuTagName);
					goto EventNext;
					}
				pEvent->m_uFlagsEvent |= pXmlNodeEvent->UFindAttributeValueHexadecimal_ZZR(d_chXCPa_IEvent_uFlagsEvent);
				pEvent->m_pVaultParent_NZ = pVault;
				AssertValidEvent(pEvent);
				Assert(pEvent->m_tsEventID == tsEventID);
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
		pVault->SetModified();	// After a sorting, the vault is 'modified', which means it must be saved again to disk with the sorted results
		}
	else
		{
		//MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Vault ^j is NOT modified\n", pVault->m_pParent);
		pVault->SetNotModified();
		}
	Assert(FEventsSortedByIDs());	// The events should always be sorted after being unserialized
	} // EventsUnserializeFromDisk()

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TContact::Xcp_ServiceDiscovery()
	{
	//Xcp_WriteStanza_VE(d_szXmlServiceDiscovery);
	}

/*
void
ITreeItemChatLogEvents::XcpStanza_AppendServiceDiscovery(IOUT CBinXcpStanza * pbinXcpStanza) const
	{
	Assert(pbinXcpStanza != NULL);
	pbinXcpStanza->BinAppendText_VE("<" d_szXCPe_EventInfo_tsO_tsI "/>");
	}
*/

CBinXcpStanza::CBinXcpStanza()
	{
	m_uFlags = F_kzSendAsXmppMessage;
	m_pContact = NULL;
	m_oOffsets.ibReset = d_zNA;
	m_oOffsets.ibDataBegins = d_zNA;
	PbbAllocateMemoryAndEmpty_YZ(300);	// Pre-allocate 300 bytes, which should be enough for a small stanza
	Assert(m_paData != NULL);
	#ifdef DEBUG_XCP_TASKS
	m_cbStanzaThresholdBeforeSplittingIntoTasks = 200; // 80;	// Use a low threshold to we use tasks often
	#endif
	}

//	Return how many bytes of binary data may be sent.
//	May return a negative value if the stanza is already too large
int
CBinXcpStanza::CbGetAvailablePayloadToSendBinaryData() const
	{
	Assert(m_paData != NULL);
	return (c_cbStanzaMaxPayload - m_paData->cbData) * 4 / 5;
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

#include "XcpApi.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core routine sending a stanza to a contact and empty the blob afterwards.
//	This routine takes care of encrypting the data and digitally signing the message.
//	If the data is too large to fit within an XMPP stanza, then will allocate a task to download it by chunks.
void
CBinXcpStanza::XospSendStanzaToContactAndEmpty(TContact * pContact) CONST_MCC
	{
	Assert(m_cbStanzaThresholdBeforeSplittingIntoTasks > 50 && m_cbStanzaThresholdBeforeSplittingIntoTasks <= c_cbStanzaThresholdBeforeSplittingIntoTasks);
	Assert(pContact != NULL);	// I think this condition is still valid
	if (pContact == NULL)
		return;	// This is not a bug, but a feature allowing an event to directly write to the socket when a contact is unable to understand XCP.
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));

	if (pContact->m_uFlagsContact & TContact::FC_kfXospSynchronizeOnNextXmppStanza)
		{
		pContact->m_uFlagsContact &= ~TContact::FC_kfXospSynchronizeOnNextXmppStanza;
		BinAppendText_VE("<" d_szXop_MessagesSynchronize "><" d_szXSop_RequestIDsLargerThanTimestamp_tsI_tsO "/></" d_szXop_MessagesSynchronize ">", pContact->m_tsOtherLastSynchronized, pContact->m_tsEventIdLastSentCached);
		}

	if ((m_uFlags & F_kfTaskAlreadyIncluded) == 0)
		{
		// Include a task (if any) if not already included
		CTaskSendReceive * pTaskPending = pContact->m_listaTasksSendReceive.m_plistTasks;
		if (pTaskPending != NULL)
			{
			MessageLog_AppendTextFormatCo(d_coPurple, "Pending Tasks for XospSendStanzaToContactAndEmpty()\n");
			pContact->m_listaTasksSendReceive.DisplayTasksToMessageLog();

			TIMESTAMP tsTaskID = pTaskPending->m_tsTaskID;
			int cbData = pTaskPending->m_binXmlData.CbGetData();
			int cbTotal = pTaskPending->m_cbTotal;
			if (cbTotal < 0)
				{
				Assert(pTaskPending->FIsTaskSend());
				Assert(CTaskSendReceive::c_cbTotal_TaskSentOnce > CTaskSendReceive::c_cbTotal_TaskSendAndRetry);
				if (cbTotal < CTaskSendReceive::c_cbTotal_TaskSendAndRetry)
					{
					// Attempt to send as much data as possible which may fit in a stanza
					const int cbAvailable = CbGetAvailablePayloadToSendBinaryData();
					if (cbAvailable > 0)
						{
						pTaskPending->m_cbTotal = CTaskSendReceive::c_cbTotal_TaskSentOnce;
						BinAppendText_VE("<" d_szXop_TaskDownloading_ts d_szXa_TaskDataSizeTotal_i d_szXa_TaskDataOffset_i d_szXa_TaskDataBinary_Bii "/>", tsTaskID, cbData, 0, &pTaskPending->m_binXmlData, 0, cbAvailable);
						if (m_paData->cbData > c_cbStanzaThresholdBeforeSplittingIntoTasks)
							MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Task ID $t contains $I bytes of data, $I bytes larger than its maximum allowed size of $I bytes\n", tsTaskID, m_paData->cbData, m_paData->cbData - c_cbStanzaThresholdBeforeSplittingIntoTasks, c_cbStanzaThresholdBeforeSplittingIntoTasks);
						goto SendStanza;
						}
					}
				if (cbTotal == CTaskSendReceive::c_cbTotal_TaskSendTryOnlyOnce)
					pContact->m_listaTasksSendReceive.DeleteTask(PA_DELETING pTaskPending);
				BinAppendText_VE("<" d_szXop_TaskDownloading_ts d_szXa_TaskDataSizeTotal_i "/>", tsTaskID, cbData);	// We already sent the task once, therefore send only the header to notify the client there is a task to send
				// The tasks to send are prioritized first, now check if there is any task to download
				while (TRUE)
					{
					pTaskPending = pTaskPending->m_pNext;
					if (pTaskPending == NULL)
						goto SendStanza;	// No task to download
					if (pTaskPending->m_cbTotal > 0)
						{
						Assert(!pTaskPending->FIsTaskSend());
						// We have found a task do download, there capture the necessary information to send the request
						tsTaskID = pTaskPending->m_tsTaskID;
						cbData = pTaskPending->m_binXmlData.CbGetData();
						break;
						}
					Assert(pTaskPending->FIsTaskSend());
					} // while
				} // if (task to upload)
			Assert(!pTaskPending->FIsTaskSend());
			BinAppendText_VE("<" d_szXop_TaskUploading_ts d_szXa_TaskDataOffset_i "/>", tsTaskID, cbData);	// Request an upload of the next outstanding/pending task to download
			} // if
		}
	if (m_paData == NULL)
		return;	// No data to send

	if (m_paData->cbData >= m_cbStanzaThresholdBeforeSplittingIntoTasks)
		{
		// The data is too big to fit into one XMPP stanza, therefore allocate a new task to send it by smaller chunks
		CTaskSendReceive * pTaskSend = pContact->m_listaTasksSendReceive.PAllocateTaskSend_YZ(m_uFlags & F_kfContainsSyncData);
		if (pTaskSend == NULL)
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "XospSendStanzaToContactAndEmpty($s) - Ignoring $I bytes of data because there is already a pending task containing synchronization data:\n{Bm}\n", pContact->ChatLog_PszGetNickname(), m_paData->cbData, this);
			return;
			}
		pTaskSend->m_binXmlData.BinAppendCBin(*this);
		BinInitFromTextSzv_VE("<" d_szXop_TaskDownloading_ts d_szXa_TaskDataSizeTotal_i d_szXa_TaskDataBinary_Bii "/>",
			pTaskSend->m_tsTaskID, pTaskSend->m_binXmlData.CbGetData(), &pTaskSend->m_binXmlData, 0, c_cbStanzaMaxBinary / 100);
		MessageLog_AppendTextFormatCo(d_coPurple, "XospSendStanzaToContactAndEmpty($s) - Creating Task ID $t of $I bytes:\n{Bm}\n", pContact->ChatLog_PszGetNickname(), pTaskSend->m_tsTaskID, pTaskSend->m_binXmlData.CbGetData(), &pTaskSend->m_binXmlData);
		}

	SendStanza:
	PSZUC pszDataStanza = m_paData->rgbData;
	const int cbDataStanza = m_paData->cbData;
	Assert(cbDataStanza >= 0);
	if (cbDataStanza <= 0)
		return;	// Nothing to do

	// TODO: Encrypt the data and sign it.
	SHashSha1 hashSignature;
	HashSha1_CalculateFromBinary(OUT &hashSignature, IN pszDataStanza, cbDataStanza);	// At the moment, use SHA-1 as the 'signature'

	MessageLog_AppendTextFormatCo(d_coBlue, "XospSendStanzaToContactAndEmpty($s) $I bytes:\n{Bm}\n", pContact->ChatLog_PszGetNickname(), cbDataStanza, this);

	PSZAC pszStanzaType = c_sza_message;
	PSZAC pszStanzaAttributesExtra = NULL;
	if (m_uFlags & F_kfSendAsXmppIQ)
		{
		pszStanzaType = c_sza_iq;
		pszStanzaAttributesExtra = " type='get'";
		}
	// Format the XML envelope for the XMPP protocol.
	g_strScratchBufferSocket.BinInitFromTextSzv_VE("<$s$s to='^J'><" d_szCambrianProtocol_xcp " " d_szCambrianProtocol_Attribute_hSignature "='{h|}'>", pszStanzaType, pszStanzaAttributesExtra, pContact, &hashSignature);
	g_strScratchBufferSocket.BinAppendStringBase85FromBinaryData(IN pszDataStanza, cbDataStanza);
	g_strScratchBufferSocket.BinAppendText_VE("</" d_szCambrianProtocol_xcp "></$s>", pszStanzaType);
	CSocketXmpp * pSocket = pContact->Xmpp_PGetSocketOnlyIfReady();
	if (pSocket != NULL)
		pSocket->Socket_WriteBin(g_strScratchBufferSocket);
	m_paData->cbData = 0;
	} // XospSendStanzaToContactAndEmpty()

CBinXcpStanzaEventCopier::CBinXcpStanzaEventCopier(ITreeItemChatLogEvents * pContactOrGroup)
	{
	TAccountXmpp * pAccount = pContactOrGroup->m_pAccount;
	m_pContact = m_paContact = new TContact(pAccount);	// We will serialize using a dummy contact
	m_pContact->m_strNameDisplayTyped = m_pContact->m_pAccount->m_pProfileParent->m_strNameProfile;	// Use the profile name as the contact so the preview looks like someone is receiving the event from the sender
	m_uFlags |= F_kfSerializeForCloning;
	}

CBinXcpStanzaEventCopier::~CBinXcpStanzaEventCopier()
	{
	// delete m_paContact;
	}

void
CBinXcpStanzaEventCopier::EventCopy(IN const IEvent * pEventSource, OUT IEvent * pEventDestination)
	{
	Assert(pEventSource != NULL);
	Assert(pEventSource->m_pVaultParent_NZ != NULL);
	BinXmlSerializeEventForDisk(pEventSource);
	/*
	BinInitFromTextSzv_VE("<$U", EGetRuntimeClass());
	BinAppendXmlEventSerializeDataAndClose(pEvent);
	*/
	MessageLog_AppendTextFormatCo(d_coOrange, "CBinXcpStanzaEventCopier::EventCopy(): $B\n", this);

	CXmlTree oXmlTree;
	(void)oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN *this);
	pEventDestination->m_pVaultParent_NZ = pEventSource->m_pVaultParent_NZ;	// Use the same vault as the source event
	pEventDestination->XmlUnserializeCore(IN &oXmlTree);
	}

#if 0
IEvent *
CBinXcpStanzaEventCloner::PaEventClone(IEvent * pEventToClone)
	{
	Assert(pEventToClone != NULL);
	/*
	CVaultEvents * pVault = m_pContact->Vault_PGet_NZ();	// Get an empty vault from the dummy contact
	paEventSent->m_pVaultParent_NZ = pVault;				// We need a valid pointer because the event may need to access the vault, contact or account
	*/
	return 0;
	}
#endif

void
ITreeItemChatLogEvents::Xmpp_WriteXmlChatState(EChatState eChatState) CONST_MCC
	{
	Assert(eChatState == eChatState_zComposing || eChatState == eChatState_Paused);
	CBinXcpStanza binXcpStanza;
	if (EGetRuntimeClass() == RTI(TContact))
		{
		TContact * pContact = (TContact *)this;
		if (pContact->Contact_FuCommunicateViaXosp())
			{
			binXcpStanza.BinAppendText((eChatState == eChatState_zComposing) ? d_szXop_MessageTyping_xmlStarted : d_szXop_MessageTyping_xmPaused);
			binXcpStanza.XospSendStanzaToContactAndEmpty(IN pContact);
			}
		else if (pContact->Contact_FuIsOnline())
			{
			// The device/computer of the remote contact does not understand XCP, therefore send the chat state notification via the standard XMPP
			CSocketXmpp * pSocket = pContact->Xmpp_PGetSocketOnlyIfReady();
			Assert(pSocket != NULL);
			if (pSocket != NULL)
				pSocket->Socket_WriteXmlFormatted("<message to='^J'><$s xmlns='http://jabber.org/protocol/chatstates'/></message>", this, (eChatState == eChatState_zComposing) ? "composing" : "paused");
			}
		return;
		}
	Assert(EGetRuntimeClass() == RTI(TGroup));
	TGroup * pGroup = (TGroup *)this;
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = pGroup->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
		TContact * pContact = pMember->m_pContact;
		if (pContact->Contact_FuCommunicateViaXosp())
			{
			binXcpStanza.BinAppendText_VE((eChatState == eChatState_zComposing) ? d_szXop_MessageTyping_xmlStartedGroup_p : d_szXop_MessageTyping_xmPausedGroup_p, pGroup);
			binXcpStanza.XospSendStanzaToContactAndEmpty(IN pContact);
			}
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
	return LFindAttributeValueDecimal_ZZR('o');
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

//	CEventFileSent::IEvent::XospDataE()
EGui
CEventFileSent::XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply)
	{
	Assert(pXmlNodeData != NULL);
	BYTE rgbBuffer[CBinXcpStanza::c_cbStanzaMaxBinary];
	L64 iblDataSource = pXmlNodeData->LFindAttributeXcpOffset();
	int cbDataRead = _PFileOpenReadOnly_NZ()->CbDataReadAtOffset(iblDataSource, sizeof(rgbBuffer), OUT rgbBuffer);
	m_cblDataTransferred = iblDataSource + cbDataRead;
	MessageLog_AppendTextFormatSev(eSeverityComment, "$t: CEventFileSent::XospDataE() offset: $L + cbDataRead: $i = $L bytes completed\n", m_tsEventID, iblDataSource, cbDataRead, m_cblDataTransferred);
	pbinXospReply->BinAppendText_VE("<" d_szXop_ApiDataToEventOther_ts_pE "><x o='$l' d='{p|}'/></" d_szXop_ApiDataToEventOther ">", m_tsEventID, this, iblDataSource, IN rgbBuffer, cbDataRead);
	if (cbDataRead <= 0)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventFileSent::XospDataE() - Closing file $S\n", &m_strFileName);
		_FileClose();	// There is nothing to send, therefore assume we reached the end of the file, and consequently close the file.  Of course, if the contact wishes more data (or request to resend the last block, then the file will be re-opened)
		}
	m_pVaultParent_NZ->SetModified();
	return eGui_zUpdate;
	}

//	CEventFileReceived::IEvent::XospDataE()
EGui
CEventFileReceived::XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply)
	{
	Assert(pXmlNodeData != NULL);
	CFile * pFile = _PFileOpenWriteOnly_NZ();
	pFile->seek(pXmlNodeData->LFindAttributeXcpOffset());
	int cbDataWritten = pFile->CbDataWriteBinaryFromBase85(pXmlNodeData->PszuFindAttributeValue('d'));
	if (cbDataWritten > 0)
		{
		// Request the next piece of data
		m_cblDataTransferred = pFile->size();
		pbinXospReply->BinAppendText_VE("<" d_szXop_ApiDataToEventID_ts_pE "><x o='$l'/></" d_szXop_ApiDataToEventID ">", m_tsOther, this, m_cblDataTransferred);
		}
	else
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventFileReceived::XospDataE() - Closing file $S\n", &m_strFileName);
		_FileClose();
		}
	m_pVaultParent_NZ->SetModified();
	return eGui_zUpdate;
	}
