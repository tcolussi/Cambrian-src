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

/*
XCP API
<a n='nameApi' g='shaGroupID' i='idEvent'> ... API parameters ... </a>
<d i='idEvent'>
<t i='idTask'>

nameApi:
m = message
s = synchronize
c = composing
p = ping
v = version
*/
#define d_chApiName_NewMessage					'm'	// API call indicating a new message was written by the contact
#define d_szApiName_NewMessage					"m"
#define d_chApiName_Synchronize					's'
#define d_szApiName_Synchronize					"s"
#define d_chApiName_Composing					'c'


#define d_chAPIe_ApiCall						'a'
#define d_szAPIe_ApiCall						"a"
#define d_szAPIe_ApiCall_s						"a n='$s'"
#define d_szAPIe_ApiCallGroup_s_h				"a n='$s' g='{h|}'"
#define d_chAPIe_ApiResponse					'A'

	#define d_chAPIa_Api_strName					'n'
	#define d_chAPIa_Api_shaGroupID					'g'		// Group (if any) where the API is related

#define d_szAPIe_Api_bsA						"$b n='$s'^A"	// Generic formatting of an API response
#define d_szAPIe_Api_b							"$b"

#define d_szxmlAPIe_ApiCall_MessageDataOpen_tsO		"<a n='m'><D o='$t'>"
#define d_szxmlAPIe_ApiCall_MessageDataClose		"</D></a>"


void
CBinXcpStanza::BinXmlAppendXcpApiRequestOpen(PSZAC pszApiName)
	{
	Assert(pszApiName != NULL);
	BinAppendText_VE("<" d_szAPIe_ApiCall_s ">", pszApiName);
	}
void
CBinXcpStanza::BinXmlAppendXcpApiRequestOpenGroup(PSZAC pszApiName, const TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	BinAppendText_VE("<" d_szAPIe_ApiCallGroup_s_h ">", pszApiName, IN &pGroup->m_hashGroupIdentifier);
	}
void
CBinXcpStanza::BinXmlAppendXcpApiRequestClose()
	{
	BinAppendText("</" d_szAPIe_ApiCall ">");
	}

//	Entry point of all XCP stanzas
void
TContact::XmppXcp_ProcessStanza(const CXmlNode * pXmlNodeXmppXcp)
	{
	Assert(pXmlNodeXmppXcp != NULL);
	Assert(m_pAccount != NULL);
	Endorse(m_paTreeItemW_YZ == NULL);	// We may process stanzas (receive XMPP messages) for a contact not in the Navigation Tree
	m_tsmLastStanzaReceived = g_tsmMinutesSinceApplicationStarted;

	// Attempt to decrypt the data and verify the signature
	PSZU pszDataEncrypted = pXmlNodeXmppXcp->m_pszuTagValue;
	if (pszDataEncrypted != NULL)
		{
		CXmlTree oXmlTree;
		oXmlTree.m_binXmlFileData.BinAppendBinaryDataFromBase85Szv_ML(pXmlNodeXmppXcp->m_pszuTagValue);
		#if 1
		MessageLog_AppendTextFormatCo(d_coGrayDark, "XCP Received($S):\n", &m_strJidBare);
		MessageLog_AppendTextFormatCo(d_coBlack, "$B\n", &oXmlTree.m_binXmlFileData);
		#endif
		if (oXmlTree.EParseFileDataToXmlNodes_ML() == errSuccess)
			{
			CBinXcpStanzaTypeMessage binXcpStanzaReply;
			binXcpStanzaReply.m_pContact = this;
			binXcpStanzaReply.XcpApi_ExecuteApiList(IN &oXmlTree);
			binXcpStanzaReply.XcpSendStanza();
			}
		}
	} // XmppXcp_ProcessStanza()

//	Core method to execute multiple XCP APIs
void
CBinXcpStanza::XcpApi_ExecuteApiList(const CXmlNode * pXmlNodeApiList)
	{
	Assert(m_pContact != NULL);
	Assert(pXmlNodeApiList != NULL);
	while (pXmlNodeApiList != NULL)
		{
		Assert(pXmlNodeApiList->m_pszuTagName != NULL);
		const CHS chXCPe_ApiResponse = Ch_ToOtherCase(pXmlNodeApiList->m_pszuTagName[0]);	// The response is always the opposite of what received, which means toggling the case of the letter
		if (chXCPe_ApiResponse == d_chAPIe_ApiCall || chXCPe_ApiResponse == d_chAPIe_ApiResponse)
			{
			// We have a normal API call
			const int ibXmlApiResponseBegin = (m_paData != NULL) ? m_paData->cbData : 0;		// Offset where the API response starts
			PSZUC pszApiName = pXmlNodeApiList->PszuFindAttributeValue_NZ(d_chAPIa_Api_strName);
			BinAppendText_VE("<" d_szAPIe_Api_bsA ">", chXCPe_ApiResponse, pszApiName, pXmlNodeApiList->PFindAttribute(d_chXCPa_Api_shaGroupID));
			Assert(m_ibXmlApiReply == d_zNA);
			m_ibXmlApiReply = m_paData->cbData;
			XcpApi_ExecuteApiName((chXCPe_ApiResponse == d_chAPIe_ApiCall), pszApiName, IN pXmlNodeApiList);	// Execute the API
			if (m_ibXmlApiReply > d_zNA)
				{
				if (m_paData->cbData > m_ibXmlApiReply)
					{
					// We have data to send for the API
					BinAppendText_VE("</" d_szAPIe_Api_b ">", chXCPe_ApiResponse);	// Close the XML request
					m_ibXmlApiReply = d_zNA;
					}
				else
					{
					// There is no real data to send for the API, therefore truncate the blob
					m_paData->cbData = ibXmlApiResponseBegin;
					m_ibXmlApiReply = d_zNA;
					}
				}
			/*
		// Check if there is any data to respond from the API
		if (m_ibXmlApiReply > d_zNA)
			{
			//if (!fResponseToApiCall)
				{
				// We have making an API call (which means it is a reply to a request).  Unless there is real data, there is no need to send a reply to a reply
				if (m_paData->cbData <= m_ibXmlApiReply)
					{
					m_paData->cbData = ibXmlApiResponseBegin;
					m_ibXmlApiReply = d_zNA;
					goto NextApi;
					}
				}
			BinAppendText_VE("</" d_szAPIe_Api_b ">", chXCPe_ApiResponse);	// Close the XML request
			m_ibXmlApiReply = d_zNA;
			}
			*/
			}
		else
			{
			const TIMESTAMP tsTaskID = pXmlNodeApiList->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID);
			const int ibData =  pXmlNodeApiList->UFindAttributeValueDecimal_ZZR(d_chAPIa_TaskDataOffset);
			if (chXCPe_ApiResponse == d_chAPIe_TaskSending)
				{
				// We are receiving task data
				CTaskSendReceive * pTaskDownload = m_pContact->m_listaTasksSendReceive.PFindTaskReceive(tsTaskID);
				if (pTaskDownload == NULL)
					{
					// The task does not exist, therefore create it so we may download the rest of the data
					pTaskDownload = new CTaskSendReceive;
					pTaskDownload->m_tsTaskID = tsTaskID;
					pTaskDownload->m_cbTotal = pXmlNodeApiList->UFindAttributeValueDecimal_ZZR(d_chAPIa_TaskDataSizeTotal);
					m_pContact->m_listaTasksSendReceive.AddTaskReceive(INOUT PA_CHILD pTaskDownload);
					MessageLog_AppendTextFormatCo(COX_MakeBold(d_coOrange), "Downloading Task ID '$t' of $I bytes\n", pTaskDownload->m_tsTaskID, pTaskDownload->m_cbTotal);
					}
				Assert(pTaskDownload->m_cbTotal > 0);
				Assert(ibData < pTaskDownload->m_cbTotal);
				if (ibData == pTaskDownload->m_binData.CbGetData())
					{
					const int cbDataNew = pTaskDownload->m_binData.BinAppendBinaryDataFromBase85SCb_ML(pXmlNodeApiList->PszuFindAttributeValue(d_chAPIa_TaskDataBinary));
					const int cbDataReceived = ibData + cbDataNew;
					Assert(cbDataReceived == pTaskDownload->m_binData.CbGetData());
					if (cbDataReceived == pTaskDownload->m_cbTotal)
						{
						// We successfully downloaded the task, therefore execute it
						MessageLog_AppendTextFormatCo(d_coGreenDarker, "Download complete ($I bytes) for Task ID '$t':\n$B\n", pTaskDownload->m_binData.CbGetData(), pTaskDownload->m_tsTaskID, &pTaskDownload->m_binData);
						CXmlTree oXmlTree;
						oXmlTree.EParseFileDataToXmlNodesModify_ML(INOUT &pTaskDownload->m_binData);
						XcpApi_ExecuteApiList(IN &oXmlTree);
						m_pContact->m_listaTasksSendReceive.DeleteTask(PA_DELETING pTaskDownload);
						}
					BinAppendText_VE("<" d_szAPIe_TaskSending_tsI d_szAPIa_TaskDataOffset_i "/>", tsTaskID, cbDataReceived);	// Send a request to download the remaining data (if any), or to indicate all the data was received and therefore delete the task
					goto NextApi;
					}
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Ignoring data from Task ID '$t' because its offset ($I) does not match the received data\n", tsTaskID, ibData);
				}
			else if (chXCPe_ApiResponse == d_chAPIe_TaskDownloading)
				{
				// We have a request to download data from the task
				CTaskSendReceive * pTaskUpload = m_pContact->m_listaTasksSendReceive.PFindTaskSend(tsTaskID);
				if (pTaskUpload != NULL)
					{
					if (ibData < pTaskUpload->m_binData.CbGetData())
						XcpSendTaskDataToContact(m_pContact, pTaskUpload, ibData); // Supply the next chunk of data
					else
						m_pContact->m_listaTasksSendReceive.DeleteTask(PA_DELETING pTaskUpload);	// The request is equal (or larger) than the data, meaning all the data was downloaded, therefore the task is no longer needed
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find TaskID $t\n", tsTaskID);
				goto NextApi;
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unknown XCP API '$s': ^N\n", pXmlNodeApiList->m_pszuTagName, pXmlNodeApiList);
				}
			} // if...else

		NextApi:
		pXmlNodeApiList = pXmlNodeApiList->m_pNextSibling;
		} // while
	} // XcpApi_ExecuteApiList()

void
CBinXcpStanza::XcpSendTaskDataToContact(TContact * pContact, const CTaskSendReceive * pTaskUpload, int ibData)
	{
	Assert(pContact != NULL);
	Assert(pTaskUpload != NULL);
	Assert(ibData >= 0);
	#ifdef DEBUG_XCP_TASKS
	m_cbStanzaThresholdBeforeSplittingIntoTasks = c_cbStanzaThresholdBeforeSplittingIntoTasks;	// Increase the threshold to its maximum value so the task data may be transmitted as fast as possible
	int cbStanzaMaxBinary = 1 + pTaskUpload->m_binData.CbGetData() / 4;	// At the moment, send 1 byte + 25% at the time (rather than c_cbStanzaMaxBinary), so we can test the code transmitting tasks
	if (cbStanzaMaxBinary > c_cbStanzaMaxBinary)
		cbStanzaMaxBinary = c_cbStanzaMaxBinary;
	#else
		#define cbStanzaMaxBinary	c_cbStanzaMaxBinary
	#endif
	BinAppendText_VE("<" d_szAPIe_TaskDownloading_tsI d_szAPIa_TaskDataOffset_i d_szAPIa_TaskDataBinary_Bii "/>", pTaskUpload->m_tsTaskID, ibData, &pTaskUpload->m_binData, ibData, cbStanzaMaxBinary);
	Assert(m_paData->cbData < c_cbStanzaThresholdBeforeSplittingIntoTasks);
	XospSendStanzaToContactAndEmpty(m_pContact);	// Send the data immediately, as the data from the task will fill the entire XMPP stanza
	}

void
CListTasksSendReceive::SentTasksToContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	// Find the first task to send
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		if (pTask->FIsTaskSend())
			{
			CBinXcpStanzaTypeInfo binXcpStanza;
			binXcpStanza.XcpSendTaskDataToContact(pContact, pTask);
			MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "SentTasksToContact(^j) - Task ID '$t' $I bytes:\n$B\n", pContact, pTask->m_tsTaskID, pTask->m_binData.CbGetData(), &pTask->m_binData);
			return;
			}
		pTask = pTask->m_pNext;
		}
	}

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


void
CBinXcpStanza::XcpApi_ExecuteApiName(BOOL fResponseToApiCall, PSZUC pszApiName, const CXmlNode * pXmlNodeApiData)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiData != NULL);
	Assert(m_pContact != NULL);
	MessageLog_AppendTextFormatCo(d_coGrayDark, "XcpApi_ExecuteSingle() - ^N", pXmlNodeApiData);
	const CHS chApiName = pszApiName[0];
	if (chApiName != '\0' && pszApiName[1] == '\0')
		{
		// We have a one-letter API name.  This optimization is for common APIs
		switch (chApiName)
			{
		case d_chApiName_NewMessage:	// A mesage is essentially the same as a synchronizing messages with the exception the GUI is updated to remove the 'composing/typing' icon in the Chat Log
		case d_chApiName_Synchronize:
			BinXmlAppendXcpApiMessageSynchronization(chApiName, pXmlNodeApiData);
			return;
		case d_chApiName_Composing:
			m_pContact->ChatLog_ChatStateIconUpdate((pXmlNodeApiData->m_pszuTagValue == NULL) ? eChatState_zComposing : eChatState_fPaused, m_pContact);
			return;
			} // switch
		}
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unknown XCP API '$s'\n", pszApiName);
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
	binXcpStanza.XospSendStanzaToContactAndEmpty(pContact);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ITreeItemChatLogEvents::XcpApi_Invoke_Synchronize()
	{
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

#define d_chAPIe_Sync_opFetchEvents					'E'
#define d_szAPIe_Sync_opFetchEvents_tsI				"E"	_tsI	d_szAPIa_Sync_tsmTimestamps_
#define d_chAPIe_Sync_opFetchEventsMine				'M'
#define d_szAPIe_Sync_opFetchEventsMine				"M"			d_szAPIa_Sync_tsmTimestamps_
#define d_chAPIe_Sync_opFetchEventsOthers			'O'
#define d_szAPIe_Sync_opFetchEventsOthers_s			"O c='$s'"	d_szAPIa_Sync_tsmTimestamps_

#define d_chAPIe_Sync_opEventsData					'D'		// We are receiving event data from the contact
#define d_szAPIe_Sync_opEventsData					"D"
#define d_chAPIe_Sync_opEventsDataMine				'Y'		// We are receiving event data which was written by ourselves (this happens when the Chat Log is lost due to a disk crash or when installing on a new computer)
#define d_szAPIe_Sync_opEventsDataMine				"Y"
#define d_chAPIe_Sync_opEventsDataOther				'T'		// We are receiving events written by a third party (this is happens only in group chat when the third party is offline)
#define d_szAPIe_Sync_opEventsDataOther				"T"
#define d_szAPIe_Sync_opEventsDataOther_s			"T c='$s'"
#define d_szAPIe_Sync_opEventsDataOther_s_S_p		"T c='$s' n='^S' j='^j'"	// Include the contact identifier, name and JID

#define d_chAPIe_Sync_opEventsConfirmation			'C'			// Include the IDs (timestamps) of the messages received
#define d_szAPIe_Sync_opEventsConfirmation			"C"

void
CBinXcpStanza::BinXmlAppendXcpApiCall_NewMessage_Open()
	{
	BinAppendText_VE(d_szxmlAPIe_ApiCall_MessageDataOpen_tsO, m_pContact->m_tsEventIdLastSentCached);
	}

void
CBinXcpStanza::BinXmlAppendXcpApiCall_NewMessage_Close()
	{
	BinAppendText(d_szxmlAPIe_ApiCall_MessageDataClose);
	}

//	Core routine to send an event (message) to a contact.
//	Use native XMPP to send the message if the contact does not support XOSP
void
CBinXcpStanza::BinXmlAppendXcpApiCall_SendEventToContact(TContact * pContact, IEvent * pEvent, CEventUpdaterSent * pEventUpdater)
	{
	Assert(pContact != NULL);
	Assert(pEvent != NULL);
	if (pContact->Contact_FuCommunicateViaXosp())
		{
		// This is the typical case where the contact is online and ready to receive an XOSP message
		m_pContact = pContact;
		BinAppendText_VE(d_szxmlAPIe_ApiCall_MessageDataOpen_tsO, pContact->m_tsEventIdLastSentCached);
		if (pEventUpdater != NULL)
			BinXmlSerializeEventForXcpCore(IN pEventUpdater, d_ts_zNA);
		BinXmlSerializeEventForXcpCore(IN pEvent, d_ts_zNA);
		BinAppendText(d_szxmlAPIe_ApiCall_MessageDataClose);
		XospSendStanzaToContactAndEmpty(pContact);
		return;
		}
	TIMESTAMP tsEventID = pEvent->m_tsEventID;
	if (!pContact->Contact_FuIsOnline())
		{
		if (!pContact->Contact_FuCommunicateViaXmppOnly())
			{
			// The contact is offline, however capable to communicate via XOSP, therefore set a flag to synchronize next time it is online.  There is no need to add a task for this, as the synchronization will dispatch the events.
			MessageLog_AppendTextFormatSev(eSeverityComment, "Contact ^j is offline, therefore Event ID $t will be dispatched via a synchronize operation next time it is online.\n", pContact, tsEventID);
			pContact->Contact_SetFlagSynchronizeWhenPresenceOnline();
			return;
			}
		}
	// At this point we have a contact which is unable to communicate via XOSP.  Therefore send the message via native XMPP if the socket is ready.  If not, then the message will be synchronized next time the contact is online (TBD)
	CSocketXmpp * pSocket = pContact->m_pAccount->Socket_PGetOnlyIfReadyToSendMessages();
	if (pSocket == NULL)
		return;	// The socket is not ready, therefore do not send the message.
	EEventClass eEventClass = pEvent->EGetEventClass();
	switch (eEventClass)
		{
	case CEventMessageTextSent::c_eEventClass:
		// Send a regular XMPP text message
		pSocket->Socket_WriteXmlFormatted("<message to='^J' id='$t'><body>^S</body><request xmlns='urn:xmpp:receipts'/></message>", pContact, tsEventID, &((CEventMessageTextSent *)pEvent)->m_strMessageText);
		break;
	default:
		MessageLog_AppendTextFormatSev(eSeverityWarning, "Unable to send Event ID $t of class '$U' to ^j because the contact is not using SocietyPro.\n", tsEventID, eEventClass, pContact);
		} // switch
	} // BinXmlAppendXcpApiCall_SendEventToContact()


//	Build a request to synchronize with a contact or a group
CBinXcpStanzaTypeSynchronize::CBinXcpStanzaTypeSynchronize(ITreeItemChatLogEvents * pContactOrGroup)
	{
	Assert(pContactOrGroup != NULL);
	(void)pContactOrGroup->Vault_PGet_NZ();	// Open the vault of events.  This is important to be first because opening the vault may update the timestamps
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TContact))
		{
		BinXmlAppendXcpApiRequestOpen(d_szApiName_Synchronize);
		BinXmlAppendXcpApiTimestamps(pContactOrGroup, IN &((TContact *)pContactOrGroup)->m_tsOtherLastSynchronized);
		XospSendStanzaToContactAndEmpty((TContact *)pContactOrGroup);
		}
	else
		{
		// We have a group, therefore we have to synchronize with every group member
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TGroup));
		BinXmlAppendXcpApiRequestOpenGroup(d_szApiName_Synchronize, (TGroup *)pContactOrGroup);	// Since this request is always the same, make it outside of the loop
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
			XospSendStanzaToContactAndEmpty(IN pMember->m_pContact);
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
CBinXcpStanza::BinXmlAppendXcpApiMessageSynchronization(CHS chApiName, const CXmlNode *pXmlNodeApiData)
	{
	Assert(chApiName == d_chApiName_NewMessage || chApiName == d_chApiName_Synchronize);
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
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opFetchEventsMine);
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
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opFetchEventsOthers_s, pszContactIdentifier);
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
			BinAppendTextOffsets_VE(OUT &oOffsets, "<"d_szAPIe_Sync_opFetchEvents_tsI, *ptsOtherLastSynchronized);	// Build a request to fetch the missing events
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

		case d_chAPIe_Sync_opFetchEvents:	// Request to return the data of my events (this is the typical case where a contact is missing events)
			BinAppendText_VE("<" d_szAPIe_Sync_opEventsData _tsO ">", pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chAPIa_Sync_EventID));
			while (TRUE)
				{
				TIMESTAMP tsEventID;
				if (!Timestamp_FGetNextTimestamp(OUT_F_UNCH &tsEventID, INOUT &pszmTimestamps))
					break;
				IEvent * pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					BinXmlSerializeEventForXcpCore(pEvent, (pGroup == NULL) ? pEvent->m_tsOther : d_ts_zNA);	// Do NOT include tsOther (the confirmation timestamp) when doing group synchronization
				else
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find Event ID $t\n", tsEventID);
				} // while
			BinAppendText_VE("</" d_szAPIe_Sync_opEventsData ">");
			break;
		case d_chAPIe_Sync_opFetchEventsMine:	// Return the data of events of the contact (this is somewhat a backup recovery when the contact lost its own Chat Log)
			BinAppendText_VE("<" d_szAPIe_Sync_opEventsDataMine ">");
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
			BinAppendText_VE("</" d_szAPIe_Sync_opEventsDataMine ">");
			break;
		case d_chAPIe_Sync_opFetchEventsOthers:	// Return the data of events of a third party contact (this happens only in group chat where a contact is relaying/forwarding the events of another contact)
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chAPIa_Sync_pContact);
			pContactOther = pAccount->Contact_PFindByJID(pszContactIdentifier, eFindContact_zDefault);
			Report(pContactOther != NULL);
			if (pContactOther == NULL)
				break;	// Ignore the request if the contact is invalid
			BinAppendText_VE((pXmlNodeSync->PFindAttribute(d_chAPIa_Sync_strNameContact) == NULL) ?
				"<" d_szAPIe_Sync_opEventsDataOther_s ">" :
				"<" d_szAPIe_Sync_opEventsDataOther_s_S_p ">", pszContactIdentifier, &pContactOther->m_strNameDisplayTyped, pContactOther);	// Include the contact name and JID if requested
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
			BinAppendText_VE("</" d_szAPIe_Sync_opEventsDataOther ">");
			break;

		case d_chAPIe_Sync_opEventsDataOther:	// The XML element contains events written by another third-party contact
			Assert(pGroup != NULL && "Third party events are only for group synchronization!");
			if (pGroup == NULL)
				break;
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chAPIa_Sync_pContact);
			pContactOther = pGroup->Member_PFindOrAddContact_YZ(pszContactIdentifier, IN pXmlNodeSync);
			Report(pContactOther != NULL && "The contact JID should be valid");
			if (pContactOther != NULL)
				goto AllocateEvents;
			break;
		case d_chAPIe_Sync_opEventsData:	// The XML element contains data events written by the contact
			pContactOther = pContactGroupMember;
			Endorse(pContactOther == NULL);	// For 1:1 chat
			AllocateEvents:
			// Include a confirmation
			BinAppendText("<" d_szAPIe_Sync_opEventsConfirmation d_szAPIa_Sync_tsmTimestamps_);
			tsOther = d_ts_zNA;
			pXmlNodeEvent = pXmlNodeSync->m_pElementsList;
			Assert(pXmlNodeEvent != NULL);
			while (pXmlNodeEvent != NULL)
				{
				tsOther = pXmlNodeEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
				Assert(tsOther > d_tsOther_kmReserved);
				if (chAPI == d_chAPIe_Sync_opEventsData)
					BinAppendTimestampSpace(tsOther);
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
			BinAppendXmlForSelfClosingElementQuote();
			Report(tsOther != d_ts_zNA && "There should be at least one XML element with an event");
			break;
		case d_chAPIe_Sync_opEventsDataMine:	// The XML element contains my own events
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
		case d_chAPIe_Sync_opEventsConfirmation:	// We are receiving the confirmation the events have been successfully received by the contact
			while (TRUE)
				{
				TIMESTAMP tsEventID;
				if (!Timestamp_FGetNextTimestamp(OUT_F_UNCH &tsEventID, INOUT &pszmTimestamps))
					break;
				IEvent * pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					pEvent->Event_SetCompletedAndUpdateChatLog(pwChatLog);
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Confirmed Event ID $t does not exist\n", tsEventID);
				}
			break;
		default:
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown sync operation: $b\n", chAPI);
			} // switch

		// Update the timestamp for the two operations
		switch (chAPI)
			{
		case d_chAPIe_Sync_opEventIDs:
		case d_chAPIe_Sync_opEventsData:
			if ((tsOther > *ptsOtherLastSynchronized) && (*ptsOtherLastSynchronized == pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther)))
				{
				MessageLog_AppendTextFormatCo(d_coGreenDarker, "\t\t\t Updating m_tsOtherLastSynchronized from $t to $t\n", *ptsOtherLastSynchronized, tsOther);
				*ptsOtherLastSynchronized = tsOther;
				}
			} // switch

		pXmlNodeSync = pXmlNodeSync->m_pNextSibling;
		} // while

	IEvent ** ppEventStop;
	IEvent ** ppEvent = arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	if (ppEvent == ppEventStop)
		return;	// No new events were added, therefore there is nothing to do

	// So far, we have allocated the events, however they have not been displayed into the Chat Log, nor added to the vault
	MessageLog_AppendTextFormatCo(d_coBlue, "Sorting $I events by chronology...\n", arraypaEvents.GetSize());
	arraypaEvents.SortEventsByChronology();	// First, sort by chronology, to display the events according to their timestamp

	// Next, assign the current date & time to those events not having a timestamp
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
		MessageLog_AppendTextFormatCo(d_coBlue, "\t tsEventID $t ({tL}), tsOther $t ({tL}): $s\n", pEvent->m_tsEventID, pEvent->m_tsEventID, pEvent->m_tsOther, pEvent->m_tsOther, pszExtra);
		} // while

	pVault->m_arraypaEvents.AppendEventsSortedByIDs(PA_CHILD &arraypaEvents);	// Add the events to the vault
	Assert(pVault->m_arraypaEvents.FEventsSortedByIDs());

	// Display the new events into the Chat Log (if present)
	if (pwChatLog != NULL)
		{
		//arraypaEvents.SortEventsByChronology();
		if (chApiName == d_chApiName_NewMessage)
			{
			pwChatLog->ChatLog_ChatStateComposerRemove(m_pContact);	// When a new message arrives, automatically assume the user stopped typing
			m_pContact->TreeItemContact_UpdateIcon();				// Update the icon in the Navigation Tree which will remove the pencil icon
			}
		pwChatLog->ChatLog_EventsDisplay(IN arraypaEvents);
		}
	} // BinXmlAppendXcpApiMessageSynchronization()

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
	//XcpApi_Invoke(c_szaApi_Contact_Recommendations_Get, d_zNA, d_zNA);
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

