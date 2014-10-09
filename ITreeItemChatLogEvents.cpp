#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "XcpApi.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Reset the nickname for the Chat Log if it was automatically generated.
void
ITreeItemChatLog::ChatLog_ResetNickname()
	{
	m_strNicknameChatLog.Empty();
	}

//	Try to return the shortest possible display name for the Chat Log.
//	Since this method is called multiple times, the nickname is cached.
PSZUC
ITreeItemChatLog::ChatLog_PszGetNickname() CONST_MCC
	{
	PSZUC pszNickName = m_strNicknameChatLog;
	if (pszNickName[0] != '\0')
		return pszNickName;
	// The nickname is empty, so attempt to find the best shortest nickname
	PSZUC pszNameDisplay = (EGetRuntimeClass() == RTI(TAccountXmpp)) ? ((TAccountXmpp *)this)->TreeItemAccount_PszGetNameDisplay() : TreeItem_PszGetNameDisplay();
	PCHUC pchAt = pszNameDisplay;
	while (TRUE)
		{
		CHS ch = *pchAt;
		if (ch == ' ' || ch == '@' || ch == ':')
			{
			BOOL fDigitsRemoved = FALSE;
			PCHUC pchTemp = pchAt;
			while (--pchTemp > pszNameDisplay)
				{
				if (Ch_FIsDigit(*pchTemp))
					fDigitsRemoved = TRUE;
				else if (fDigitsRemoved)
					{
					if (_FSetNickname(pszNameDisplay, pchTemp + 1))
						goto Done;
					break;
					}
				} // while
			if (_FSetNickname(pszNameDisplay, pchAt))
				goto Done;
			}
		else if (ch == '\0')
			break;
		pchAt++;
		} // while
	if (!_FSetNickname(pszNameDisplay, pchAt))
		{
		// If we are unable to set the full display name, it means the contact name is the same as the account, in this case, use the JID
		Assert(EGetRuntimeClass() == RTI(TContact));
		m_strNicknameChatLog = ((TContact *)this)->m_strJidBare;
		}
	Done:
	return m_strNicknameChatLog;
	} // ChatLog_PszGetNickname()

//	Return TRUE if the nickname is different than its parent account.
BOOL
ITreeItemChatLog::_FSetNickname(PSZUC pszBegin, PCHUC pchCopyUntil) CONST_MODIFIED
	{
	m_strNicknameChatLog.InitFromStringCopiedUntilPch(pszBegin, pchCopyUntil);
	if (EGetRuntimeClass() == RTI(TContact))
		{
		return !m_strNicknameChatLog.FCompareStringsNoCase(((TContact *)this)->m_pAccount->ChatLog_PszGetNickname());
		}
	return TRUE;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
ITreeItemChatLogEvents::ITreeItemChatLogEvents(TAccountXmpp * pAccount)
	{
	Assert(pAccount != NULL);
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	m_pAccount = pAccount;
	m_paVaultEvents = NULL;
	m_tsCreated = d_ts_zNULL;
	m_tsEventIdLastSentCached = d_ts_zNA;
	m_tsOtherLastReceived = d_ts_zNA;
	m_cMessagesUnread = 0;
	m_pawLayoutChatLog = NULL;
	}

ITreeItemChatLogEvents::~ITreeItemChatLogEvents()
	{
	delete m_paVaultEvents;
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutChatLog);
	}

void
ITreeItemChatLogEvents::Vault_RemoveAllReferencesToObjectsAboutBeingDeleted()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->m_arraypaEvents.ForEach_DetachFromObjectsAboutBeingDeleted();
	}

TProfile *
ITreeItemChatLogEvents::PGetProfile() const
	{
	return m_pAccount->m_pProfileParent;
	}

CChatConfiguration *
ITreeItemChatLogEvents::PGetConfiguration() const
	{
	return m_pAccount->PGetConfiguration();
	}

//	ITreeItemChatLogEvents::IRuntimeObject::PGetRuntimeInterface()
POBJECT
ITreeItemChatLogEvents::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	if (rti == RTI(ITreeItemChatLogEvents))
		return (ITreeItemChatLogEvents *)this;
	return ITreeItem::PGetRuntimeInterface(rti, piParent);
	}

//	Return the default download folder path to save files when the user clicks on the "[ Save ]" button.
//	Each contact may have his/her own folder when downloading files.
PSZUC
ITreeItemChatLogEvents::ChatLog_PszGetPathFolderDownload() const
	{
	if (!m_strPathFolderDownload.FIsEmptyString())
		return m_strPathFolderDownload;
	return m_pAccount->ChatLog_PszGetPathFolderDownload();
	}

//	ITreeItemChatLogEvents::IXmlExchange::XmlExchange()
//
//	Variables Used: ("N" + "F") + ("T" + "D" + "U")
void
ITreeItemChatLogEvents::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	if (pXmlExchanger->m_fSerializing && m_paVaultEvents != NULL)
		{
		m_paVaultEvents->WriteEventsToDiskIfModified();		// This line is important to be first because saving the events may modify some variables which may be serialized by ITreeItemChatLogEvents::XmlExchange()
		if (m_tsCreated == d_zNA)
			{
			IEvent * pEventFirst = (IEvent *)m_paVaultEvents->m_arraypaEvents.PvGetElementFirst_YZ();	// This code is a bit of legacy and should be moved when the Tree Item (contact) is created)
			if (pEventFirst != NULL)
				m_tsCreated = pEventFirst->m_tsEventID;
			else
				m_tsCreated = Timestamp_GetCurrentDateTime();
			}
		}
	pXmlExchanger->XmlExchangeTimestamp("tsCreated", INOUT_F_UNCH_S &m_tsCreated);
	pXmlExchanger->XmlExchangeTimestamp("tsOtherLastReceived", INOUT_F_UNCH_S &m_tsOtherLastReceived);
	pXmlExchanger->XmlExchangeStr("D", INOUT_F_UNCH_S &m_strPathFolderDownload);
	pXmlExchanger->XmlExchangeInt("U", INOUT_F_UNCH_S &m_cMessagesUnread);
	}

//	ITreeItemChatLogEvents::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
ITreeItemChatLogEvents::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_SynchronizeWithContact:
	case eMenuAction_SynchronizeWithGroup:
		XcpApi_Invoke_Synchronize();
		return ezMenuActionNone;
	case eMenuAction_FindText:
		ChatLog_FindText();
		return ezMenuActionNone;
	case eMenuAction_BallotSend:
		DisplayDialogBallotSend();
		return ezMenuActionNone;
	case eMenuSpecialAction_ITreeItemRenamed:
		ChatLog_ResetNickname();
		if (m_pawLayoutChatLog != NULL)
			m_pawLayoutChatLog->ChatLog_EventsRepopulateUpdateUI();
		// Fall Through //
	default:
		return ITreeItemChatLog::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	}

void
ITreeItemChatLogEvents::Vault_GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC
	{
	Assert(parraypEventsChatLog != NULL);
	parraypEventsChatLog->Copy(IN &Vault_PGet_NZ()->m_arraypaEvents);
	}

const QBrush &
ITreeItemChatLogEvents::ChatLog_OGetBrushForNewMessageReceived()
	{
	return c_brushGreenSuperPale;	// Needs to be fixed
	}

CVaultEvents *
ITreeItemChatLogEvents::Vault_PGet_NZ()
	{
	if (m_paVaultEvents == NULL)
		{
		SHashSha1 hashFileNameVault;
		Vault_GetHashFileName(OUT &hashFileNameVault);
		new CVaultEvents(PA_PARENT this, IN &hashFileNameVault);
		Assert(m_paVaultEvents != NULL);
		}
	return m_paVaultEvents;
	}

void
ITreeItemChatLogEvents::Vault_AddEventToChatLogAndSendToContacts(PA_CHILD IEvent * paEvent)
	{
	AssertValidEvent(paEvent);
	Vault_PGet_NZ()->EventAddAndDispatchToContacts(PA_CHILD paEvent);
	//paEvent->EventAddToVault(PA_PARENT Vault_PGet_NZ());
	if (m_pawLayoutChatLog != NULL)
		//m_pawLayoutChatLog->ChatLog_EventAppend(IN paEvent);
		m_pawLayoutChatLog->m_pwChatLog_NZ->ChatLog_EventDisplay(IN paEvent);
	}

IEvent *
ITreeItemChatLogEvents::Vault_PFindEventByID(TIMESTAMP tsEventID) CONST_MCC
	{
	return Vault_PGet_NZ()->m_arraypaEvents.PFindEventByID(tsEventID);
	}

CEventMessageTextSent *
ITreeItemChatLogEvents::Vault_PFindEventLastMessageTextSentMatchingText(const CStr & strMessageText) const
	{
	if (m_paVaultEvents != NULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_paVaultEvents->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			CEventMessageTextSent * pEvent = (CEventMessageTextSent *)*--ppEventStop;
			if (pEvent->EGetEventClass() == CEventMessageTextSent::c_eEventClass)
				{
				if (pEvent->m_strMessageText.FCompareBinary(strMessageText))
					{
					/*
					// We have an event matching the text, however this event may have been a replacement
					if (pEvent->m_uFlagsEvent & IEvent::FE_kfReplacing)
						{
						IEvent * pEventReplacing = m_paVaultEvents->PFindEventReplacing(pEvent);
						if (pEventReplacing != NULL)
							return (CEventMessageTextSent *)pEventReplacing;
						}
					*/
					return pEvent;
					}
				break;
				}
			} // while
		}
	return NULL;
	}
/*
CEventMessageTextSent *
ITreeItemChatLogEvents::Vault_PGetEventLastMessageSentEditable_YZ() const
	{
	if (m_paVaultEvents != NULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_paVaultEvents->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			if (pEvent->EGetEventClass() == CEventMessageTextSent::c_eEventClass)
				{
				if (pEvent->Event_FIsEventRecentThanMinutes(10))
					return (CEventMessageTextSent *)pEvent;	// Only edit the last message if it was written within 10 minutes.  This is to prevent to accidentally edit an old message by pressing the up arrow.
				break;
				}
			} // while
		}
	return NULL;
	}
*/

void
ITreeItemChatLogEvents::Vault_WriteEventsToDiskIfModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->WriteEventsToDiskIfModified();
	}

QString
ITreeItemChatLogEvents::Vault_SGetPath() const
	{
	SHashSha1 hashFileNameEvents;
	Vault_GetHashFileName(OUT &hashFileNameEvents);
	return PGetConfiguration()->SGetPathOfFileName(IN &hashFileNameEvents);
	}

void
ITreeItemChatLogEvents::Vault_SendToJID(PSZAC pszJID)
	{
	Vault_WriteEventsToDiskIfModified();	// Make sure the latest events have been written to disk before sending the file
	CStr strPathVault = Vault_SGetPath();
	Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventFileSentTo(strPathVault, pszJID));
	EMessageBoxWarning("The code to send a file to $s is still under construction and does not work!!!", pszJID);
	}

/*
void
ITreeItemChatLogEvents::Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...) const
	{
	CSocketXmpp * pSocket = m_pAccount->PGetSocket_YZ();
	if (pSocket != NULL)
		{
		va_list vlArgs;
		va_start(OUT vlArgs, pszFmtTemplate);
		pSocket->Socket_WriteXmlFormatted_VL(pszFmtTemplate, vlArgs);
		}
	}
*/

//	SUPPORTED COMMANDS
//
//	/ping					Ping the contact.  If it is a group, broadcast a ping and record the timestamp of the first group member responding.
//	/sendxml	<xml>		Send XML data directly through the socket.  The user should know what he is doing, because any invalid XML may terminate the network connection.
//	/sendfile
//	/sendbtc
//	/version				Query the version of the software on the contact
//	/f						Fetch the container
//	/api		[fn] [params}	Query an API on the contact
//	/info					Query the information of the contact or group.  This is essentially a PAPI call.
//	/add [user]
//	/find [text]

#define d_chChatLogPrefix_CommandLine		'/'

//	Return the remaining of the command, which are its parameters, or a pointer to an empty string.
//	Return NULL if the command was not recognized
PSZR
PszrCompareStringBeginCommand(PSZUC pszStringCompare, PSZAC pszCommand)
	{
	Assert(pszStringCompare != NULL);
	PSZR pszr = PszrCompareStringBeginNoCase(pszStringCompare, pszCommand);
	if (pszr != NULL)
		{
		if (!Ch_FIsWhiteSpaceOrNullTerminator(*pszr))
			return NULL;	// If there is a character following the command, it means the user typed something else, such as "/pingpong"
		while (Ch_FIsWhiteSpace(*pszr))
			pszr++;	// Skip any white spaces after the command so we return its parameters
		}
	return pszr;
	}

//	Return the next parameter of the command line.
//	If there is no more parameters, return a pointer to the null-terminator of pszParameter.
//
//	This method will insert null-terminator(s) between the parameters
//
//	Example:
//	/api profileget Etg]z/!%wBcm*[}zmFKZ#UngL
PSZRO
PszroGetParameterNext(INOUT PSZU pszParameter)
	{
	if (pszParameter != NULL)
		{
		// Skip the content of the first parameter
		while (!Ch_FIsWhiteSpaceOrNullTerminator(*pszParameter))
			pszParameter++;
		// Insert null-terminator(s) between the parameters
		while (Ch_FIsWhiteSpace(*pszParameter))
			*pszParameter++ = '\0';	// Insert null-terminators
		}
	return pszParameter;
	}

//	Parse the text the user typed and act accordingly:
//	- The most common case is creating an event to send an instant text message to the contact or group.
//	- If the user typed a command, then execute the command.
//	Return TRUE if a 'pause' should be sent to the remote client.
EUserCommand
ITreeItemChatLogEvents::Xmpp_EParseUserCommandAndSendEvents(IN_MOD_INV CStr & strCommandLineOrMessage)
	{
	const BOOL fIsContact = (EGetRuntimeClass() == RTI(TContact));
	PSZU pszMessage = strCommandLineOrMessage.PbGetData();
	Assert(pszMessage != NULL);
	// Check if there is a command line instead of a message
	CHS chMessage0 = pszMessage[0];
	if (chMessage0 == d_chChatLogPrefix_CommandLine)
		{
		// We have a command line
		PSZUC pszCommand = pszMessage + 1;
		if (PszrCompareStringBeginCommand(pszCommand, c_sza_ping))
			{
			Xmpp_Ping();
			goto Done;
			}
		if (PszrCompareStringBeginCommand(pszCommand, "version") || PszrCompareStringBeginCommand(pszCommand, "ver"))
			{
			Xmpp_QueryVersion();
			goto Done;
			}
		if (PszrCompareStringBeginCommand(pszCommand, "sr"))
			{
			// Reset the synchronization timestamps (this is useful for debugging, so there is no need to quit the application, delete the Chat Log, and restart again)
			m_tsEventIdLastSentCached = d_ts_zNA;
			if (fIsContact)
				{
				((TContact *)this)->m_tsOtherLastSynchronized = d_ts_zNA;
				}
			else
				{
				TGroupMember ** ppMemberStop;
				TGroupMember ** ppMember = ((TGroup *)this)->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
				while (ppMember != ppMemberStop)
					{
					TGroupMember * pMember = *ppMember++;
					Assert(pMember != NULL);
					Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
					pMember->m_tsOtherLastSynchronized = d_ts_zNA;
					}
				}
			XcpApi_Invoke_Synchronize();
			goto Done;
			}
		if (PszrCompareStringBeginCommand(pszCommand, "t"))
			{
			// Dump the tasks to the Message Log
			if (fIsContact)
				{
				MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGreen), "Tasks related to ^j:\n", this);
				((TContact *)this)->m_listaTasksSendReceive.DisplayTasksToMessageLog();
				}
			goto Done;
			}
		PSZUC pszParameters = PszrCompareStringBeginCommand(pszCommand, "sync");
		if (pszParameters != NULL)
			{
			int cDays = NStringToNumber_ZZR_ML(pszParameters);
			if (cDays > 0)
				{
				//TIMESTAMP tsLastSynchornized = Timestamp_GetCurrentDateTime() - cDays * d_ts_cDays;
				m_tsEventIdLastSentCached = d_ts_zNA;
				if (fIsContact)
					{
					((TContact *)this)->m_tsOtherLastSynchronized = d_ts_zNA;
					}
				}
			XcpApi_Invoke_Synchronize();
			goto Done;
			}

		pszParameters =  PszrCompareStringBeginCommand(pszCommand, d_szXv_ApiName_ContainerFetch);
		if (pszParameters != NULL)
			{
			XcpApi_Invoke(IN (PSZUC)d_szXv_ApiName_ContainerFetch, pszParameters);
			goto Done;
			}
		pszParameters = PszrCompareStringBeginCommand(pszCommand, "sendxml");
		if (pszParameters != NULL && pszParameters[0] == '<')
			{
			Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventMessageXmlRawSent(pszParameters));
			goto Done;
			}
		if (PszrCompareStringBeginCommand(pszCommand, "sendfile") != NULL)
			{
			DisplayDialogSendFile();
			goto Done;
			}
		if (PszrCompareStringBeginCommand(pszCommand, "sendballot") != NULL)
			{
			DisplayDialogBallotSend();
			goto Done;
			}
		/*
		if (PszrCompareStringBeginCommand(pszCommand, "sendbtc") != NULL)
			{
			if (fIsContact)
				{
				((TContact *)this)->DisplayDialogDisplayDialogBallotSend();
				goto Done;
				}
			}
		*/
		if (PszrCompareStringBeginCommand(pszCommand, "rec") != NULL || PszrCompareStringBeginCommand(pszCommand, "recommendations") != NULL)
			{
			XcpApi_Invoke_RecommendationsGet();
			goto Done;
			}
		if (pszCommand[0] == d_chChatLogPrefix_CommandLine)
			{
			pszMessage = (PSZU)pszCommand;	// The message begins with "//", therefore the message is the command
			goto SendMessageText;
			}

		// The command is invalid, therefore display something to the user so he/she may learn about the syntax of the command line interface
		Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventHelp(
			g_strScratchBufferStatusBar.Format(
			"Invalid command: <b>^s</b><br/>"
			"Valid commands are:<br/>"
			"^_^_^_ <b>/ping</b> to ping a peer<br/>"
			"^_^_^_ <b>/version</b> to query the version of the peer<br/>"
			"^_^_^_ <b>/sync</b> synchronize with a peer.  /sync 14 will synchronize with the past 2 weeks<br/>"
			"^_^_^_ <b>/recommendations</b> to query the recommendations of the peer(s)<br/>"
			"^_^_^_ <b>/sendfile</b> to send a file to the peer or group<br/>"
			"^_^_^_ <b>/sendballot</b> to send a ballot to vote<br/>"
			//"^_^_^_ <b>/sendbtc</b> to send Bitcoin to the peer<br/>"
			"^_^_^_ <b>/sendxml</b> to send XML data directly through the socket (this is used for debugging)<br/>"
			//"^_^_^_ <b>/api</b> to invoke a remote API call on the peer (this is used for debugging)<br/>"
			"^_^_^_ <b>/f</b> to fetch the data of a container.  /f 2 will fetch the date of container[2]<br/>"
			"^_^_^_ <b>//</b> to send a text message starting with a <b>/</b><br/>"
			, pszMessage)));
		return eUserCommand_Error;
		} // if (command line)

	// No command line, therefore attempt to send an event
	if (PszrCompareStringBeginNoCase(pszMessage, "file://") == NULL)
		{
		SendMessageText:
		Xmpp_SendEventMessageText(pszMessage);	// Send the text message to the contact (or group)
		return eUserCommand_zMessageTextSent;
		}
	else
		{
		// We wish to send file(s) to the user or group
		Xmpp_SendEventsFileUpload(IN_MOD_INV pszMessage);
		}
	Done:
	return eUserCommand_ComposingStopped;
	} // Xmpp_EParseUserCommandAndSendEvents()

//	Send a text message to a contact or a group.
void
ITreeItemChatLogEvents::Xmpp_SendEventMessageText(PSZUC pszMessage)
	{
	Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventMessageTextSent(pszMessage));
	}


//	Upload a file to the contact.
//	The file name is expected to be a local path.
void
ITreeItemChatLogEvents::Xmpp_SendEventFileUpload(PSZUC pszFileUpload)
	{
	Assert(m_pawLayoutChatLog != NULL);
	if (pszFileUpload == NULL || pszFileUpload[0] == '\0')
		return;
	Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventFileSent(pszFileUpload));
	}

//	Upload multiple files.
//	This method expect files to be in the 'URL' with the prefix "file://"
void
ITreeItemChatLogEvents::Xmpp_SendEventsFileUpload(IN_MOD_INV PSZU pszmFilesUpload)
	{
	Assert(pszmFilesUpload != NULL);
	PSZUC pszFileUpload = pszmFilesUpload;
	while (TRUE)
		{
		UINT ch = *pszmFilesUpload++;
		if (ch == '\n' || ch == '\0')
			{
			pszmFilesUpload[-1] = '\0';	// Insert a null-terminator
			CStr strFile = QUrl::fromUserInput(CString(pszFileUpload)).toLocalFile();	// This code is grossly inefficient, however necessary because Qt uses triple slashes (///) after the schema, such as: "file:///c:/folder/file.txt".  I could manually skip the extra slash, however it may break under other platrorms.
			MessageLog_AppendTextFormatCo(d_coBlack, "XmppUploadFiles() - $S\n", &strFile);
			Xmpp_SendEventFileUpload(strFile);
			if (ch == '\0')
				return;	// We are done
			pszFileUpload = pszmFilesUpload;
			}
		}
	}

void
ITreeItemChatLogEvents::DisplayDialogSendFile()
	{
	CStr strCaption;
	strCaption.Format("Send file to $s", TreeItem_PszGetNameDisplay());
	strCaption = QFileDialog::getOpenFileName(g_pwMainWindow, IN strCaption);
	Xmpp_SendEventFileUpload(strCaption);	// An empty filename will be ignored by Xmpp_SendEventFileUpload()
	}


CSocketXmpp *
ITreeItemChatLogEvents::Xmpp_PGetSocketOnlyIfReady() const
	{
	return m_pAccount->Socket_PGetOnlyIfReadyToSendMessages();
	}

CSocketXmpp *
TContact::Xmpp_PGetSocketOnlyIfContactIsUnableToCommunicateViaXcp() const
	{
	if (!Contact_FuCommunicateViaXosp())
		return m_pAccount->Socket_PGetOnlyIfReadyToSendMessages();
	return NULL;
	}

void
ITreeItemChatLogEvents::Xmpp_Ping()
	{
	Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventPing);	// Pinging a group makes little sense, however there is no harm.  The first group member responding to the ping will set the timestamp.
	}

void
ITreeItemChatLogEvents::Xmpp_QueryVersion()
	{
	Vault_AddEventToChatLogAndSendToContacts(PA_CHILD new CEventVersion);
	}


//	The sorting is reversed, where the most recent appear first
NCompareResult
ITreeItemChatLogEvents::S_NCompareSortByTimestampEventLastReceived(ITreeItemChatLogEvents * pChatLogA, ITreeItemChatLogEvents * pChatLogB, LPARAM)
	{
	return NCompareSortTimestamps(pChatLogB->m_tsOtherLastReceived, pChatLogA->m_tsOtherLastReceived);
	}

void
CArrayPtrTreeItemChatLogEvents::SortByEventLastReceived()
	{
	Sort((PFn_NCompareSortElements)ITreeItemChatLogEvents::S_NCompareSortByTimestampEventLastReceived);
	}
