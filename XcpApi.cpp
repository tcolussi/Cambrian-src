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
		MessageLog_AppendTextFormatCo(d_coGray, "XCP Received($S):\n", &m_strJidBare);
		MessageLog_AppendTextFormatCo(d_coBlack, "{Bm}\n", &oXmlTree.m_binXmlFileData);
		#endif
		if (oXmlTree.EParseFileDataToXmlNodes_ML() == errSuccess)
			{
			CBinXcpStanza binXcpStanzaReply;
			binXcpStanzaReply.m_pContact = this;
			binXcpStanzaReply.XcpApi_ExecuteApiList(IN &oXmlTree);
			binXcpStanzaReply.XospSendStanzaToContactAndEmpty(this);
			}
		}
	} // XmppXcp_ProcessStanza()

ITreeItemChatLogEvents *
TContact::PGetContactOrGroupDependingOnIdentifier_YZ(const CXmlNode * pXmlAttributeGroupIdentifier)
	{
	if (pXmlAttributeGroupIdentifier == NULL)
		return this;
	return (pXmlAttributeGroupIdentifier->m_pszuTagName[0] == d_chXa_GroupChannel_strName) ?
		m_pAccount->GroupChannel_PFindByNameOrAddAsAvailable_YZ(IN pXmlAttributeGroupIdentifier->m_pszuTagValue) :
		m_pAccount->Group_PFindByIdentifier_YZ(IN pXmlAttributeGroupIdentifier->m_pszuTagValue);
	}

//	Core method to execute multiple XCP APIs
void
CBinXcpStanza::XcpApi_ExecuteApiList(const CXmlNode * pXmlNodeApiList)
	{
	Assert(pXmlNodeApiList != NULL);
	Assert(m_pContact != NULL);
	Assert(m_paData != NULL);
	while (pXmlNodeApiList != NULL)
		{
		const CXmlNode * pXmlAttributeGroupIdentifier = pXmlNodeApiList->PFindAttribute(d_chXa_GroupIdentifier_shaBase85);	// Since the Group Identifier is used for almost every opcode, fetch it here
		if (pXmlAttributeGroupIdentifier == NULL)
			pXmlAttributeGroupIdentifier = pXmlNodeApiList->PFindAttribute(d_chXa_GroupChannel_strName);
		Assert(pXmlNodeApiList->m_pszuTagName != NULL);
		const CHS chXop = pXmlNodeApiList->m_pszuTagName[0];
		if (chXop == d_chXop_MessageNew || chXop == d_chXop_MessagesSynchronize)
			{
			SOffsets oOffsets;
			BinAppendTextOffsetsInit_VE(OUT &oOffsets, "<" d_szXop_MessagesSynchronizeGroup_A ">", pXmlAttributeGroupIdentifier);
			BinXmlAppendXcpApiMessageSynchronization(pXmlNodeApiList, chXop == d_chXop_MessageNew, pXmlAttributeGroupIdentifier);
			if (m_paData->cbData > oOffsets.ibDataBegins)
				m_uFlags |= F_kfContainsSyncData;			// This is important to prevent synchronization tasks to accumulate
			BinAppendTextOffsetsTruncateIfEmpty_VE(IN &oOffsets, "</" d_szXop_MessagesSynchronize ">");
			}
		else if (chXop == d_chXop_MessageTyping)
			{
			ITreeItemChatLogEvents * pContactOrGroup = m_pContact->PGetContactOrGroupDependingOnIdentifier_YZ(pXmlAttributeGroupIdentifier);
			//Report(pContactOrGroup != NULL);	// The group does not yet exists on the client
			if (pContactOrGroup != NULL)
				pContactOrGroup->ChatLog_ChatStateIconUpdate((pXmlNodeApiList->m_pszuTagValue == NULL) ? eChatState_zComposing : eChatState_Paused, m_pContact);
			}
		else if (chXop == d_chXop_ApiCall)
			{
			// Executing an API is easy; the difficulty is with the API response where the returned value must be redirected to the appropriate handler.
			PSZUC pszApiName = NULL;
			PSZUC pszXmlResponseName = NULL;
			PSZUC pszXmlResponseValue = NULL;
			// Find the necessary information we need to process the API
			const CXmlNode * pXmlAttribute = pXmlNodeApiList->m_pAttributesList;
			while (pXmlAttribute != NULL)
				{
				PSZUC pszTagValue = pXmlAttribute->m_pszuTagValue;
				Assert(pszTagValue != NULL);
				switch (pXmlAttribute->m_pszuTagName[0])
					{
				case d_chXa_ApiResponsePrefix:
					pszXmlResponseName = pXmlAttribute->m_pszuTagName + 1;
					pszXmlResponseValue = pszTagValue;
					break;
				case d_chXa_ApiName:
					pszApiName = pszTagValue;
					break;
					} // switch
				pXmlAttribute = pXmlAttribute->m_pNextSibling;
				} // while
			Report(pszApiName != NULL);
			if (pszApiName != NULL)
				{
				Report(pszApiName[0] != '\0');
				struct SXospApiExecute	// To be done later
					{
					SOffsets oOffsets;
					const CXmlNode * pXmlNodeApi;
					};
				if (pszXmlResponseName == NULL)
					{
					// We have a standard API call, so the value is the API name
					Assert(pszXmlResponseValue == NULL);
					pszXmlResponseName = (PSZUC)d_szXop_ApiResponse;
					pszXmlResponseValue = pszApiName;
					}
				Assert(pszXmlResponseValue != NULL);
				//Report(pszXmlResponseValue[0] != '\0');  // Temporary commented to not display an error to the user
				BinAppendTextOffsetsInit_VE(OUT &m_oOffsets, "<$s $s='^s'^A>", pszXmlResponseName, pszXmlResponseName, pszXmlResponseValue, pXmlAttributeGroupIdentifier);
				XcpApi_ExecuteApiName(pszApiName, IN pXmlNodeApiList);	// Execute the API
				BinAppendTextOffsetsTruncateIfEmpty_VE(IN &m_oOffsets, "</$s>", pszXmlResponseName);	// Close the XML request
				}
			}
		else if (chXop == d_chXop_ApiResponse)
			{
			XcpApi_ExecuteApiResponse(pXmlNodeApiList->PszuFindAttributeValue_NZ(d_chXop_ApiResponse), pXmlNodeApiList->m_pElementsList);
			}
		else if (chXop == d_chXop_ApiResponseToEventID || chXop == d_chXop_ApiResponseToEventOther)
			{
			TIMESTAMP tsEventID = pXmlNodeApiList->TsGetAttributeValueTimestamp_ML(chXop);
			ITreeItemChatLogEvents * pContactOrGroup = m_pContact->PGetContactOrGroupDependingOnIdentifier_YZ(pXmlAttributeGroupIdentifier);
			Report(pContactOrGroup != NULL);
			if (pContactOrGroup != NULL)
				{
				// Find the event matching the identifier
				CVaultEvents * pVault = pContactOrGroup->Vault_PGet_NZ();
				IEvent * pEvent;
				if (chXop == d_chXop_ApiResponseToEventID)
					pEvent = pVault->PFindEventByID(tsEventID);
				else
					pEvent = pVault->PFindEventReceivedByTimestampOther(tsEventID, (pContactOrGroup != m_pContact) ? m_pContact : NULL);
				if (pEvent != NULL)
					{
					struct SXospData
						{
						WLayoutChatLog * m_pwLayoutChatLog;			// Chat Log to update the GUI
						ITreeItemChatLogEvents * pContactOrGroup;
						CBinXcpStanza * pbinXcpStanzaReply;
						};
					Report(pXmlNodeApiList->m_pElementsList != NULL);
					if (pXmlNodeApiList->m_pElementsList != NULL)
						{
						if (pEvent->XospDataE(pXmlNodeApiList->m_pElementsList, INOUT this) == eGui_zUpdate)
							{
							// The event has been modified and must be updated in the Chat Log
							pEvent->ChatLog_UpdateEventWithinWidget(pContactOrGroup->ChatLog_PwGet_YZ());
							}
						}
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "\t\t Unable to find Event $t\n", tsEventID);
					}
				}
			}
		else if (chXop == d_chXop_ApiResponseError)
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning,
				"\t The contact ^j is unable to process the API named '$s'\n"
				"\t Please contact ^j and recommend upgrading to " d_szApplicationName " version " d_szApplicationVersion ".\n", m_pContact, pXmlNodeApiList->PszuFindAttributeValue(d_chXa_ApiName), m_pContact);
			}
		else
			{
			const TIMESTAMP tsTaskID = pXmlNodeApiList->TsGetAttributeValueTimestamp_ML(d_chXa_TaskID);
			const CXmlNode * pXmlAttributeOffset = pXmlNodeApiList->PFindAttribute(d_chXa_TaskDataOffset);
			const int ibData = (pXmlAttributeOffset != NULL) ? NStringToNumber_ZZR_ML(pXmlAttributeOffset->m_pszuTagValue) : -1;
			if (chXop == d_chXop_TaskDownloading)
				{
				// We are receiving task data
				if (tsTaskID != m_pContact->m_tsTaskIdDownloadedLast)
					{
					CTaskSendReceive * pTaskDownload = m_pContact->m_listaTasksSendReceive.PFindOrAllocateTaskDownload_NZ(tsTaskID, pXmlNodeApiList);
					Assert(pTaskDownload->m_cbTotal > 0);
					Assert(ibData < pTaskDownload->m_cbTotal);
					int cbData = pTaskDownload->m_binXmlData.CbGetData();
					if (ibData == cbData)
						{
						cbData += pTaskDownload->m_binXmlData.BinAppendBinaryDataFromBase85SCb_ML(pXmlNodeApiList->PszuFindAttributeValue(d_chXa_TaskDataBinary));
						if (cbData == ibData)
							MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "No data from XMPP stanza for Task ID $t\n", tsTaskID);
						Assert(cbData == pTaskDownload->m_binXmlData.CbGetData());
						if (cbData == pTaskDownload->m_cbTotal)
							{
							// We successfully downloaded the task, therefore execute it
							MessageLog_AppendTextFormatCo(COX_MakeBold(d_coPurple), "Task download complete ($I bytes) for Task ID $t:\n{Bm}\n", cbData, pTaskDownload->m_tsTaskID, &pTaskDownload->m_binXmlData);
							CXmlTree oXmlTree;
							oXmlTree.EParseFileDataToXmlNodesModify_ML(INOUT &pTaskDownload->m_binXmlData);
							m_pContact->m_listaTasksSendReceive.DeleteTask(PA_DELETING pTaskDownload);
							m_pContact->m_tsTaskIdDownloadedLast = tsTaskID;
							BinAppendText_VE("<" d_szXop_TaskExecuted_ts "/>", tsTaskID);	// Notify the other client the task has been executed
							XcpApi_ExecuteApiList(IN &oXmlTree);
							goto NextApi;
							}
						}
					else if (ibData >= 0)
						MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "\t\t Ignoring data from Task ID $t because its offset ($I) does not match the received data of $I bytes\n", tsTaskID, ibData, pTaskDownload->m_binXmlData.CbGetData());
					//BinAppendText_VE("<" d_szXop_TaskUploading_ts d_szXa_TaskDataOffset_i "/>", tsTaskID, cbData);	// Send a request to download the remaining data (if any), or to indicate all the data was received and therefore delete the task
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityNoise, "Ignoring TaskID $t from ^j because it already executed\n", tsTaskID, m_pContact);
					BinAppendText_VE("<" d_szXop_TaskExecuted_ts "/>", tsTaskID);	// Notify the other client the task has been executed to make sure it is removed from its list
					}
				}
			else if (chXop == d_chXop_TaskUploading)
				{
				// We have a request to download data from the task
				Assert(ibData >= 0);
				Assert(m_pContact != NULL);
				if (m_pContact == NULL)
					return;
				CListTasksSendReceive * plistaTasksSendReceive = &m_pContact->m_listaTasksSendReceive;
				CTaskSendReceive * pTaskFirst = plistaTasksSendReceive->m_plistTasks;
				CTaskSendReceive * pTaskUpload = plistaTasksSendReceive->PFindTaskSend(tsTaskID);
				if (pTaskUpload != NULL)
					{
					if (pTaskUpload != pTaskFirst)
						{
						// The request is not the first task in the list, therefore notify the client there is a task which should be downloaded
						MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "The TaskID $t requested by ^j is out of order, therefore notify ^j about Task ID $t\n", tsTaskID, m_pContact, m_pContact, pTaskFirst->m_tsTaskID);
						BinAppendText_VE("<" d_szXop_TaskDownloading_ts d_szXa_TaskDataSizeTotal_i "/>", pTaskFirst->m_tsTaskID, pTaskFirst->m_binXmlData.CbGetData());
						}
					if (ibData >= 0 && ibData < pTaskUpload->m_binXmlData.CbGetData())
						{
						// Supply the next chunk of data
						#ifdef DEBUG_XCP_TASKS
						m_cbStanzaThresholdBeforeSplittingIntoTasks = c_cbStanzaThresholdBeforeSplittingIntoTasks;	// Increase the threshold to its maximum value so the task data may be transmitted as fast as possible
						#endif
						BinAppendText_VE("<" d_szXop_TaskDownloading_ts d_szXa_TaskDataOffset_i d_szXa_TaskDataBinary_Bii "/>", pTaskUpload->m_tsTaskID, ibData, &pTaskUpload->m_binXmlData, ibData, CbGetAvailablePayloadToSendBinaryData());
						Assert(m_paData->cbData < c_cbStanzaThresholdBeforeSplittingIntoTasks + 50);
						m_uFlags |= F_kfTaskAlreadyIncluded;
						XospSendStanzaToContactAndEmpty(m_pContact);	// Send the data immediately, as the binary data from the task will fill the entire XMPP stanza
						}
					else
						plistaTasksSendReceive->DeleteTask(PA_DELETING pTaskUpload);	// The request is equal (or larger) than the data, meaning all the data was downloaded, therefore the task is no longer needed
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find TaskID $t to send to ^j\n", tsTaskID, m_pContact);
					BinAppendText_VE("<" d_szXop_TaskNotFound_ts "/>", tsTaskID);	// Notify the remote client the task cannot be found, and therefore should not attempt to download this task again
					}
				goto NextApi;
				}
			else if (chXop == d_chXop_TaskExecuted || chXop == d_chXop_TaskNotFound)
				{
				m_pContact->m_listaTasksSendReceive.DeleteTaskMatchingID(tsTaskID, chXop == d_chXop_TaskExecuted);
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unknown XOSP opcode '$s': ^N\n", pXmlNodeApiList->m_pszuTagName, pXmlNodeApiList);
				}
			} // if...else

		NextApi:
		pXmlNodeApiList = pXmlNodeApiList->m_pNextSibling;
		} // while
	} // XcpApi_ExecuteApiList()


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
TContact::XospApiContact_ContainerFetch(PSZUC pszContainerID, IOUT CBinXcpStanza * pbinXcpStanzaReply) const
	{
	int iContainer = (pszContainerID == NULL) ? 0 : NStringToNumber_ZZR_ML(pszContainerID);
	MessageLog_AppendTextFormatCo(d_coGrayDark, "\t Fetching container $i\n", iContainer);

	TProfile * pProfile = PGetProfile();
	pbinXcpStanzaReply->BinAppendText_VE("<f n='^S' k='^S'/>", &pProfile->m_strNameProfile, &pProfile->m_strKeyPublic);
	}


void
CBinXcpStanza::XcpApi_ExecuteApiName(PSZUC pszApiName, const CXmlNode * pXmlNodeApiData)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiData != NULL);
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	MessageLog_AppendTextFormatCo(d_coGrayDark, "\t XcpApi_ExecuteApiName($s) - ^N", pszApiName, pXmlNodeApiData);
	const CHS chApiName = pszApiName[0];
	if (chApiName != '\0' && pszApiName[1] == '\0')
		{
		// We have a one-letter API name.  This optimization is for common APIs
		switch (chApiName)
			{
		case d_chXv_ApiName_Version:
			BinAppendText("<" d_szXv_ApiName_Version d_szXCPa_eVersion_Version d_szXCPa_eVersion_Platform d_szXCPa_eVersion_Client "/>");
			return;
		case d_chXv_ApiName_Ping:
			BinAppendText_VE("<" d_szXv_ApiName_Ping " " d_szXv_ApiName_Ping "='$t'/>", Timestamp_GetCurrentDateTime());
			return;
		case d_chXv_ApiName_ContainerFetch:
			m_pContact->XospApiContact_ContainerFetch(pXmlNodeApiData->m_pszuTagValue, INOUT this);
			goto AlwaysReturnWhateverIsInTheBlob;
			} // switch
		}
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Contact_Recommendations_Get))
		{
		m_pContact->m_pAccount->m_pProfileParent->XcpApiProfile_RecommendationsSerialize(INOUT this); // Return all the recommendations related to the profile
		goto AlwaysReturnWhateverIsInTheBlob;
		}
	// Report the error to the user
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unknown XOSP API '$s' ^N\n", pszApiName, pXmlNodeApiData);
	m_paData->cbData = m_oOffsets.ibReset;	// Flush whatever was there for the API
	BinAppendText_VE("<" d_szXop_ApiResponseError_Name_s "/>", pszApiName);

	AlwaysReturnWhateverIsInTheBlob:
	//m_oOffsets.ibReset = m_oOffsets.ibDataBegins = m_paData->cbData;
	m_oOffsets.ibDataBegins = d_ibDataBegins_DoNotReset;
	}

void
CBinXcpStanza::XcpApi_ExecuteApiResponse(PSZUC pszApiName, const CXmlNode * pXmlNodeApiResponse)
	{
	Assert(pszApiName != NULL);
	Endorse(pXmlNodeApiResponse == NULL);	// An emtpy response value is not an error, as some APIs return no value if there is nothing (example, a contact may have no recommendations)
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	MessageLog_AppendTextFormatCo(d_coGrayDark, "\t XcpApi_ExecuteApiResponse($s): ^N\n", pszApiName, pXmlNodeApiResponse);

	const CHS chApiName = pszApiName[0];
	if (chApiName != '\0' && pszApiName[1] == '\0')
		{
		if (chApiName == d_chXv_ApiName_ContainerFetch && pXmlNodeApiResponse != NULL)
			{
			PSZUC pszKeyPublic = pXmlNodeApiResponse->PszuFindAttributeValue('k');
			if (m_pContact->m_strKeyPublic.FIsEmptyString())
				{
				MessageLog_AppendTextFormatSev(eSeverityComment, "Assigning public key to peer '$s': $s\n", m_pContact->TreeItem_PszGetNameDisplay(), pszKeyPublic);
				m_pContact->m_strKeyPublic = pszKeyPublic;	// Set the public key
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "You already have a public key for '$s', therefore ignoring received public key: $s\n", m_pContact->TreeItem_PszGetNameDisplay(), pszKeyPublic);
				}
			return;
			}
		}


	if (FCompareStringsNoCase(pszApiName, c_szaApi_Contact_Recommendations_Get))
		{
		m_pContact->Contact_RecommendationsUpdateFromXml(pXmlNodeApiResponse);
		return;
		}
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unknown SOXP API response '$s' ^N\n", pszApiName, pXmlNodeApiResponse);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
CBinXcpStanza::BinXmlAppendXcpApiRequest_Group_Profile_Get(PSZUC pszGroupIdentifier)
	{
	Assert(pszGroupIdentifier != NULL);
	//BinXmlAppendXcpApiRequest((PSZAC)c_szaApi_Group_Profile_Get, pszGroupIdentifier);
	}


//	Ask a contact to return the profile of a group
void
TGroup::XcpApiGroup_Profile_GetFromContact(TContact * pContact)
	{
	CHU szGroupIdentifier[30];
	InitToGarbage(OUT szGroupIdentifier, sizeof(szGroupIdentifier));
	HashSha1_ToStringBase85(OUT szGroupIdentifier, IN &m_hashGroupIdentifier);
	CBinXcpStanza binXcpStanza;
	binXcpStanza.BinXmlAppendXcpApiRequest_Group_Profile_Get(szGroupIdentifier);
	binXcpStanza.XospSendStanzaToContactAndEmpty(pContact);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ITreeItemChatLogEvents::XcpApi_Invoke_Synchronize()
	{
	CBinXcpStanzaTypeSynchronize binXcpStanza(this);
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
		SOffsets oOffsets;
		BinAppendTextOffsetsInit_VE(OUT &oOffsets, d_szXop_MessageNew_xmlOpen_pE_tsO, pEvent, pContact->m_tsEventIdLastSentCached);
		if (pEventUpdater != NULL)
			BinXmlSerializeEventForXcpCore(IN pEventUpdater, d_ts_zNA);
		BinXmlSerializeEventForXcpCore(IN pEvent, d_ts_zNA);
		BinAppendTextOffsetsTruncateIfEmpty_VE(IN &oOffsets, d_szXop_MessageNew_xmlClose);
		XospSendStanzaToContactAndEmpty(pContact);
		return;
		}
	TIMESTAMP tsEventID = pEvent->m_tsEventID;
	if (pContact->Contact_FQueueXospTasksUntilOnline() && !pContact->Contact_FuIsOnline())
		{
		// The contact is offline, however capable to communicate via XOSP, therefore set a flag to synchronize next time it is online.  There is no need to add a task for this, as the synchronization will dispatch the events.
		MessageLog_AppendTextFormatSev(eSeverityComment, "Contact ^j is offline, therefore Event ID $t will be dispatched via a synchronize operation next time it is online.\n", pContact, tsEventID);
		pContact->ContactFlag_SynchronizeWhenPresenceOnline_Set();
		return;
		}
	// At this point we have a contact which is unable to communicate via XOSP.  Therefore send the message via native XMPP if the socket is ready.  If not, then the message will be synchronized next time the contact is online (TBD)
	CSocketXmpp * pSocket = pContact->m_pAccount->Socket_PGetOnlyIfReadyToSendMessages();
	Assert(pSocket != NULL);
	if (pSocket == NULL)
		return;	// The socket is not ready, therefore do not send the message.
	EEventClass eEventClass = pEvent->EGetEventClass();
	switch (eEventClass)
		{
	case CEventMessageTextSent::c_eEventClass:
		// Send a regular XMPP text message
		pSocket->Socket_WriteXmlFormatted("<message to='^J' id='$t'><body>^S</body><request xmlns='urn:xmpp:receipts'/></message>", pContact, tsEventID, &((CEventMessageTextSent *)pEvent)->m_strMessageText);
		break;
	case CEventVersion::c_eEventClass:
		pSocket->Socket_WriteXmlFormatted("<iq to='^J' id='$t' type='get'><query xmlns='jabber:iq:version'/></iq>", pContact, tsEventID);
		break;
	case CEventPing::c_eEventClass:
		pSocket->Socket_WriteXmlFormatted("<iq to='^J' id='$t' type='get'><ping xmlns='urn:xmpp:ping'/></iq>", pContact, tsEventID);
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
		BinAppendText("<" d_szXop_MessagesSynchronize ">");
		BinAppendXospSynchronizeTimestampsAndClose(pContactOrGroup, IN &((TContact *)pContactOrGroup)->m_tsOtherLastSynchronized);
		XospSendStanzaToContactAndEmpty((TContact *)pContactOrGroup);
		}
	else
		{
		// We have a group, therefore we have to synchronize with every group member
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TGroup));
		TGroupMember ** ppMemberStop;
		TGroupMember ** ppMember = ((TGroup *)pContactOrGroup)->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
		while (ppMember != ppMemberStop)
			{
			TGroupMember * pMember = *ppMember++;
			Assert(pMember != NULL);
			Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
			Assert(pMember->m_pGroup == pContactOrGroup);
			Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
			BinAppendText_VE("<" d_szXop_MessagesSynchronizeGroup_p ">", IN pContactOrGroup);
			BinAppendXospSynchronizeTimestampsAndClose(pContactOrGroup, IN &pMember->m_tsOtherLastSynchronized);
			XospSendStanzaToContactAndEmpty(IN pMember->m_pContact);
			} // while
		} // if...else
	}

void
CBinXcpStanzaTypeSynchronize::BinAppendXospSynchronizeTimestampsAndClose(ITreeItemChatLogEvents * pContactOrGroup, const TIMESTAMP * ptsOtherLastSynchronized)
	{
	BinAppendText_VE("<" d_szXSop_RequestIDsLargerThanTimestamp_tsI_tsO "/></" d_szXop_MessagesSynchronize ">", *ptsOtherLastSynchronized, pContactOrGroup->m_tsEventIdLastSentCached);
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

//	This method handles both d_chXop_MessagesSynchronize and d_chXop_MessageNew. A new mesage is essentially the
//	same as a synchronizing messages with the exception the GUI is updated to remove the 'composing/typing' icon in the Chat Log.
void
CBinXcpStanza::BinXmlAppendXcpApiMessageSynchronization(const CXmlNode * pXmlNodeSynchronize, BOOL fNewMessage, const CXmlNode * pXmlAttributeGroupIdentifier)
	{
	Assert(pXmlNodeSynchronize != NULL);
	Assert(m_pContact != NULL);
	Assert(m_paData != NULL);
	Endorse(m_paData->cbData == 0);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));

	ITreeItemChatLogEvents * pContactOrGroup_NZ = m_pContact;
	TIMESTAMP * ptsOtherLastSynchronized = &m_pContact->m_tsOtherLastSynchronized;	// When the data was last synchronized with the contact
	WChatLog * pwChatLog = m_pContact->ChatLog_PwGet_YZ();
	CVaultEvents * pVault = m_pContact->Vault_PGet_NZ();	// Get the vault so we may append new events, or fetch the missing events
	TGroup * pGroup = NULL;
	TGroupMember * pMember = NULL;
	TContact * pContactGroupMember = NULL;
	if (pXmlAttributeGroupIdentifier != NULL)
		{
		PSZUC pszGroupIdentifier = pXmlAttributeGroupIdentifier->m_pszuTagValue;
		Assert(pszGroupIdentifier != NULL);
		pContactOrGroup_NZ = pGroup = (pXmlAttributeGroupIdentifier->m_pszuTagName[0] == d_chXa_GroupChannel_strName) ?
			pAccount->GroupChannel_PFindByNameOrCreate_YZ(IN pszGroupIdentifier, INOUT this) :
			pAccount->Group_PFindByIdentifier_YZ(IN pszGroupIdentifier, INOUT this, TAccountXmpp::eFindGroupCreate);	// Find the group matching the identifier, and if the group is not there, then create it
		if (pGroup == NULL)
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The group identifier '$s' is not valid\n", pszGroupIdentifier);
			return;	// Don't attempt to synchronize with an invalid group identifier
			}
		pContactGroupMember = m_pContact;
		pMember = pGroup->Member_PFindOrAddContact_NZ(m_pContact);
		Assert(pMember != NULL);
		Assert(pMember->m_pContact == m_pContact);
		ptsOtherLastSynchronized = &pMember->m_tsOtherLastSynchronized;
		pwChatLog = pGroup->ChatLog_PwGet_YZ();
		pVault = pGroup->Vault_PGet_NZ();
		} // if (group)

	//MessageLog_AppendTextFormatCo(d_coRed, "Sync Begin: tsOtherLastSynchronized $t, tsOtherLastReceived $t\n", *ptsOtherLastSynchronized, pContactOrGroup_NZ->m_tsOtherLastReceived);

	PSZUC pszContactIdentifier;
	TContact * pContactOther;
	TIMESTAMP tsOther;
	SOffsets oOffsets;	// Offset where an XML request starts (this variable is used to truncate empty XML requests)
	const CXmlNode * pXmlNodeEvent;

	CArrayPtrEvents arraypaEvents;
	const CXmlNode * pXmlNodeSync = pXmlNodeSynchronize->m_pElementsList;
	while (pXmlNodeSync != NULL)
		{
		PSZUC pszmTimestamps = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chXSa_tsm_);	// Since most will have a list of timestamps, we fetch them at the beginning of the loop
		CHS chXSop = pXmlNodeSync->m_pszuTagName[0];
		switch (chXSop)
			{
		case d_chXSop_RequestIDsLargerThanTimestamp:	// Return all events since tsEventID
			{
			Assert(arraypaEvents.FIsEmpty() && "Memory leak!");
			TIMESTAMP tsEventID = pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXSa_tsEventID);
			pVault->UGetEventsSinceTimestamp(IN tsEventID, OUT &arraypaEvents);	// Reuse the array for performance
			IEvent ** ppEventStop;
			IEvent ** ppEventFirst = arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
			// Scan the array for sent events
			BinAppendText_VE("<" d_szXSop_ConfirmationEventLastReceived_tsI "/><" d_szXSop_EventIDs_tsO_, pContactOrGroup_NZ->m_tsOtherLastReceived, tsEventID);
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
			tsOther = pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXSa_tsOther);
			if (tsOther > pContactOrGroup_NZ->m_tsOtherLastReceived)
				{
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t Missing events from tsOther $t until $t ({tL}) ($T)\n", pContactOrGroup_NZ->m_tsOtherLastReceived, tsOther, tsOther, tsOther - pContactOrGroup_NZ->m_tsOtherLastReceived);
				if ((m_pContact->m_uFlagsContact & TContact::FC_kfXospSynchronizeWhenPresenceOnline) == 0)
					m_pContact->m_uFlagsContact |= TContact::FC_kfXospSynchronizeWhenPresenceOnline | TContact::FC_kfXospSynchronizeOnNextXmppStanza;
				//m_pContact->m_uFlagsContact |= TContact::FC_kfXospSynchronizeOnNextXmppStanza;	// It would be better to 'inject' the XML here.  Also, there is a need to handle the group/channels
				if (pContactOrGroup_NZ->m_tsOtherLastReceived < *ptsOtherLastSynchronized)
					{
					// The last received event should be larger or equal to the last synchronized event
					MessageLog_AppendTextFormatSev(eSeverityWarning, "Adjusting m_tsOtherLastReceived from $t to $t\n", pContactOrGroup_NZ->m_tsOtherLastReceived, *ptsOtherLastSynchronized);
					pContactOrGroup_NZ->m_tsOtherLastReceived = *ptsOtherLastSynchronized;
					}
				}
			BinAppendTextOffsetsInit_VE(OUT &oOffsets, "<" d_szXSop_EventIDsMine_);
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
					BinAppendText_VE("<" d_szXSop_EventIDsOthers_p_, pContactThirdParty);
					}
				BinAppendTimestampSpace(pEvent->m_tsOther);
				//MessageLog_AppendTextFormatCo(d_coRed, "Need to include event tsOther $t from ^j\n", pEvent->m_tsOther, pEvent->m_pContactGroupSender_YZ);
				}
			if (pContactThirdParty != NULL)
				BinAppendXmlForSelfClosingElementQuote();
			arraypaEvents.Empty();	// Flush the array so the array object may be reused by other sync operations
			}
			break;
		case d_chXSop_ConfirmationEventLastReceived:	// We are receiving a confirmation of the timestamp of the last event received by the contact
			{
			TIMESTAMP tsEventID = pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID);
			if (tsEventID == pContactOrGroup_NZ->m_tsEventIdLastSentCached)
				{
				//MessageLog_AppendTextFormatSev(eSeverityNoise, "\t '$s' confirmed having received Event ID $t\n", pContactOrGroup_NZ->TreeItem_PszGetNameDisplay(), pContactOrGroup_NZ->m_tsEventIdLastSentCached);
				m_pContact->ContactFlag_SynchronizeWhenPresenceOnline_Clear();
				}
			else
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t Expecting Event ID $t ({tL}) as confirmation from '$s' instead of $t ({tL})\n",
					pContactOrGroup_NZ->m_tsEventIdLastSentCached, pContactOrGroup_NZ->m_tsEventIdLastSentCached, pContactOrGroup_NZ->TreeItem_PszGetNameDisplay(), tsEventID, tsEventID);
			}
			break;

		case d_chXSop_EventIDsMine:	// The user is missing its own events
			BinAppendTextOffsetsInit_VE(OUT &oOffsets, "<" d_szXSop_FetchEventsMine_);
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
		case d_chXSop_EventIDsOthers:	// This XML node contains a list of timestamps of events written by others (third-party group chat)
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chXSa_pContact);
			BinAppendTextOffsetsInit_VE(OUT &oOffsets, "<" d_szXSop_FetchEventsOthers_s_, pszContactIdentifier);
			pContactOther = pAccount->Contact_PFindByJID(pszContactIdentifier, eFindContact_zDefault);
			if (pContactOther != NULL)
				goto BeginLoop;
			// The contact is unknown, therefore make a request to get its name and JID
			MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "Contact $s is unknown, therefore requesting its name and JID\n", pszContactIdentifier);
			BinAppendText_VE("$s'" d_szXSa_strNameContact"/>", pszmTimestamps);	// Include all the timestamps in the request
			break;
		case d_chXSop_EventIDs:	// This XML node contains a list of timestamps of events written by the contact
			pContactOther = pContactGroupMember;
			Endorse(pContactOther == NULL);	// For 1:1 chat
			BinAppendTextOffsetsInit_VE(OUT &oOffsets, "<" d_szXSop_FetchEvents_tsI_, *ptsOtherLastSynchronized);	// Build a request to fetch the missing events
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
					MessageLog_AppendTextFormatSev(eSeverityInfoTextBlueDark, "\t Missing Event tsOther $t ({tL}) from ^j\n", tsOther, tsOther, pContactOther);
					BinAppendTimestampSpace(tsOther);	// Add the timestamp to request the data of the missing event
					chXSop = d_chXSop_zNULL;				// Do not update the synchronization timestamp if an event is missing
					}
				else
					{
					MessageLog_AppendTextFormatSev(eSeverityNoise, "\t Event ID $t (tsOther = $t) from ^j is already present\n", pEvent->m_tsEventID, tsOther, pContactOther);
					}
				} // while
			BinAppendXmlForSelfClosingElementQuoteTruncateAtOffset(IN &oOffsets);
			Endorse(tsOther == d_ts_zNA); // pszmTimestamps was empty.  This is the case when already synchronized
			break;

		case d_chXSop_FetchEvents:	// Request to return the data of my events (this is the typical case where a contact is missing events)
			BinAppendTextOffsetsInit_VE(OUT &oOffsets, "<" d_szXSop_EventsData_tsO ">", pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXSa_tsEventID));
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
			BinAppendTextOffsetsTruncateIfEmpty_VE(IN &oOffsets, "</" d_szXSop_EventsData ">");
			break;
		case d_chXSop_FetchEventsMine:	// Return the data of events of the contact (this is somewhat a backup recovery when the contact lost its own Chat Log)
			BinAppendText_VE("<" d_szXSop_EventsDataMine ">");
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
			BinAppendText_VE("</" d_szXSop_EventsDataMine ">");
			break;
		case d_chXSop_FetchEventsOthers:	// Return the data of events of a third party contact (this happens only in group chat where a contact is relaying/forwarding the events of another contact)
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chXSa_pContact);
			pContactOther = pAccount->Contact_PFindByJID(pszContactIdentifier, eFindContact_zDefault);
			Report(pContactOther != NULL);
			if (pContactOther == NULL)
				break;	// Ignore the request if the contact is invalid
			BinAppendText_VE((pXmlNodeSync->PFindAttribute(d_chXSa_strNameContact) == NULL) ?
				"<" d_szXSop_EventsDataOther_s ">" :
				"<" d_szXSop_EventsDataOther_s_S_p ">", pszContactIdentifier, &pContactOther->m_strNameDisplayTyped, pContactOther);	// Include the contact name and JID if requested
			while (TRUE)
				{
				pszmTimestamps = Timestamp_PchFromStringSkipWhiteSpaces(OUT &tsOther, IN pszmTimestamps);
				if (tsOther == d_ts_zNULL)
					break;
				IEvent * pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactOther);
				if (pEvent != NULL)
					{
					// Perform a special serialization when forwarding/relaying events written by a third party contact
					BinAppendText_VE("<$U" _tsO, pEvent->EGetEventClass(), tsOther);	// Do NOT include tsEventID
					BinAppendXmlEventSerializeDataAndClose(pEvent);
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "Unable to find Event tsOther $t\n", tsOther);
				} // while
			BinAppendText_VE("</" d_szXSop_EventsDataOther ">");
			break;

		case d_chXSop_EventsDataOther:	// The XML element contains events written by another third-party contact
			Assert(pGroup != NULL && "Third party events are only for group synchronization!");
			if (pGroup == NULL)
				break;
			pszContactIdentifier = pXmlNodeSync->PszuFindAttributeValue_NZ(d_chXSa_pContact);
			pContactOther = pGroup->Member_PFindOrAddContact_YZ(pszContactIdentifier, IN pXmlNodeSync);
			Report(pContactOther != NULL && "The contact JID should be valid");
			if (pContactOther != NULL)
				goto AllocateEvents;
			break;
		case d_chXSop_EventsData:	// The XML element contains data events written by the contact
			pContactOther = pContactGroupMember;
			Endorse(pContactOther == NULL);	// For 1:1 chat
			AllocateEvents:
			// Include a confirmation
			BinAppendText("<" d_szXSop_EventsConfirmation_);
			tsOther = d_ts_zNA;
			pXmlNodeEvent = pXmlNodeSync->m_pElementsList;
			Report(pXmlNodeEvent != NULL);
			while (pXmlNodeEvent != NULL)
				{
				tsOther = pXmlNodeEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
				Assert(tsOther > d_tsOther_kmReserved);
				if (chXSop == d_chXSop_EventsData)
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
		case d_chXSop_EventsDataMine:	// The XML element contains my own events
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
		case d_chXSop_EventsConfirmation:	// We are receiving the confirmation the events have been successfully received by the contact
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
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown sync operation: $b\n", chXSop);
			} // switch

		// Update the timestamp for the two operations
		switch (chXSop)
			{
		case d_chXSop_EventIDs:
		case d_chXSop_EventsData:
			if ((tsOther > *ptsOtherLastSynchronized) && (*ptsOtherLastSynchronized == pXmlNodeSync->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther)))
				{
				MessageLog_AppendTextFormatCo(d_coGreenDarker, "\t\t\t Updating m_tsOtherLastSynchronized from $t to $t\n", *ptsOtherLastSynchronized, tsOther);
				*ptsOtherLastSynchronized = tsOther;
				}
			} // switch

		pXmlNodeSync = pXmlNodeSync->m_pNextSibling;
		} // while

	//MessageLog_AppendTextFormatCo(d_coRed, "Sync End: tsOtherLastSynchronized $t, tsOtherLastReceived $t\n", *ptsOtherLastSynchronized, pContactOrGroup_NZ->m_tsOtherLastReceived);

	IEvent ** ppEventStop;
	IEvent ** ppEvent = arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	if (ppEvent == ppEventStop)
		return;	// No new events were added, therefore there is nothing to do

	// So far, we have allocated the events, however they have not been displayed into the Chat Log, nor added to the vault
	if (!arraypaEvents.FIsEmpty())
		MessageLog_AppendTextFormatCo(d_coBlue, "Sorting $I events by chronology...\n", arraypaEvents.GetSize());
	arraypaEvents.SortEventsByChronology();	// First, sort by chronology, to display the events according to their timestamp

	IEvent * pEvent = NULL;
	TProfile * pProfile = pAccount->m_pProfileParent;

	// Next, assign the current date & time to those events not having a timestamp
	while (ppEvent != ppEventStop)
		{
		pEvent = *ppEvent++;
		if (pEvent->m_tsEventID == c_tsMax)
			pEvent->m_tsEventID = Timestamp_GetCurrentDateTime();	// Assign the current date & time to new events
		PSZUC pszExtra = NULL;
		EEventClass eEventClass = pEvent->EGetEventClass();
		switch (eEventClass)
			{
		case CEventBallotReceived::c_eEventClass:
			pProfile->BallotMaster_OnEventNewBallotReceived((CEventBallotReceived *)pEvent);
			break;
		case CEventMessageTextReceived::c_eEventClass:
			pProfile->m_arraypEventsRecentMessagesReceived.AddEvent(pEvent);
			// Fall through //
		case CEventMessageTextSent::c_eEventClass:
			pszExtra = ((IEventMessageText *)pEvent)->m_strMessageText;
			break;
		default:
			break;
			} /// switch
		if (eEventClass & eEventClass_kfReceivedByRemoteClient)
			{
			if (pContactOrGroup_NZ->m_tsOtherLastReceived < pEvent->m_tsOther)
				pContactOrGroup_NZ->m_tsOtherLastReceived = pEvent->m_tsOther;
			else if (pGroup == NULL)
				{
				MessageLog_AppendTextFormatCo(d_coOrange, "\t pContactOrGroup_NZ->m_tsOtherLastReceived $t >= pEvent->m_tsOther $t\n", pContactOrGroup_NZ->m_tsOtherLastReceived, pEvent->m_tsOther);
				pEvent->Event_SetFlagOutOfSync();
				}
			if (pGroup == NULL)
				{
				if (pEvent->m_tsEventID < pContactOrGroup_NZ->m_tsEventIdLastSentCached)
					pEvent->Event_SetFlagOutOfSync();	// For one-to-one chat, all Event IDs are expected to be consecutive
				}
			}
		MessageLog_AppendTextFormatCo(d_coBlue, "\t tsEventID $t ({tL}), tsOther $t ({tL}): {sm}\n", pEvent->m_tsEventID, pEvent->m_tsEventID, pEvent->m_tsOther, pEvent->m_tsOther, pszExtra);
		} // while
	if (pContactOrGroup_NZ->m_tsGuiLastActivity < pContactOrGroup_NZ->m_tsOtherLastReceived)
		pContactOrGroup_NZ->m_tsGuiLastActivity = pContactOrGroup_NZ->m_tsOtherLastReceived;

	pVault->m_arraypaEvents.AppendEventsSortedByIDs(PA_CHILD IN_MOD_SORT &arraypaEvents);	// Add the events to the vault
	Assert(pVault->m_arraypaEvents.FEventsSortedByIDs());
	pVault->SetModified();	// Make sure the vault gets saved
//	pVault->WriteEventsToDiskIfModified();	// This line is for debugging

	// Display the new events into the Chat Log (if present)
	if (fNewMessage)
		pContactOrGroup_NZ->ChatLog_ChatStateIconUpdate(eChatState_PausedNoUpdateChatLog, m_pContact);
	if (pwChatLog != NULL)
		pwChatLog->ChatLog_EventsDisplay(IN arraypaEvents);

	// Update the GUI about the new event
	Assert(pEvent != NULL);
	if (pEvent != NULL)
		{
		CStr str;
		PSZUC pszMessage = pEvent->PszGetTextOfEventForSystemTray(OUT_IGNORED &str);
		pContactOrGroup_NZ->TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(IN pszMessage, m_pContact, pMember);
		}
	Dashboard_NewEventsFromContactOrGroup(pContactOrGroup_NZ);
	} // BinXmlAppendXcpApiMessageSynchronization()

///////////////////////////////////////////////////////////////////////////////////////////////////

void
ITreeItemChatLogEvents::XcpApi_Invoke_RecommendationsGet()
	{
	XcpApi_Invoke(c_szaApi_Contact_Recommendations_Get);
	}

void
TContact::XcpApiContact_ProfileSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const
	{
	Assert(pbinXcpStanzaReply != NULL);

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

CString
ITreeItem::TreeItem_SGetNameDisplay() CONST_MCC
	{
	return TreeItem_PszGetNameDisplay();	// Cast a psz to a CString
	}

void
TGroup::XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanza * UNUSED_PARAMETER(pbinXcpApiExtraRequest))
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

