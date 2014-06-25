#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

CVaultEvents::CVaultEvents(PA_PARENT ITreeItemChatLogEvents * pTreeItemParent, const SHashSha1 * pHashFileName)
	{
	Assert(pHashFileName != NULL);
	Assert(pTreeItemParent != NULL);
	Assert(pTreeItemParent->m_paVaultEvents == NULL && "Memory leak!");
	pTreeItemParent->m_paVaultEvents = this;
	m_pParent = pTreeItemParent;
	m_pEventLastSaved = NULL;
	InitToZeroes(OUT &m_history, sizeof(m_history));
	ReadEventsFromDisk(pHashFileName);
	}

CVaultEvents::~CVaultEvents()
	{
	m_arraypaEvents.DeleteAllEvents();
	delete m_history.m_paVault;	// Recursively the vault history
	}

const char c_szE[] = "E";	// For events
void
CVaultEvents::ReadEventsFromDisk(const SHashSha1 * pHashFileName)
	{
	CWaitCursor wait;
	CXmlTree oXmlTreeEvents;
	m_sPathFileName = m_pParent->PGetConfiguration()->SGetPathOfFileName(IN pHashFileName);
	if (oXmlTreeEvents.m_binXmlFileData.BinFileReadE(m_sPathFileName) == errSuccess)
		{
		if (oXmlTreeEvents.EParseFileDataToXmlNodes_ML() == errSuccess)
			{
			if (oXmlTreeEvents.FCompareTagName(c_szE) || oXmlTreeEvents.FCompareTagName("Events"))
				{
				m_arraypaEvents.EventsUnserializeFromDisk(IN oXmlTreeEvents.m_pElementsList, m_pParent);
				// TODO: We need to check if this is the first vault in the chain.  So far, there is always only one vault, however this code will have to be revised when chaining vaults.

				/*
				TIMESTAMP tsOtherLastReceived = d_ts_zNA;
				IEvent * pEventLastSent = NULL;
				IEvent ** ppEventStop;
				IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
				while (ppEvent != ppEventStop)
					{
					IEvent * pEvent = *--ppEventStop;
					if (pEvent->Event_FIsEventTypeReceived())
						{
						if (pEventLastReceived == NULL)
							{
							pEventLastReceived = pEvent;
							m_pParent->m_tsOtherLastReceived = pEvent->m_tsOther;
							}
						else if (pEventLastSent != NULL)
							break;	// We are done
						}
					else
						{
						Assert(pEvent->Event_FIsEventTypeSent());
						if (pEventLastSent == NULL)
							{
							pEventLastSent = pEvent;
							m_pParent->m_tsEventIdLastSent = pEvent->m_tsEventID;
							}
						else if (pEventLastReceived != NULL)
							break;	// We are done
						} // if...else
					} // while
				*/
				} // if
			}
		}
	m_pParent->m_tsEventIdLastSentCached = m_arraypaEvents.TsEventIdLastEventSent();	// Update the timestamp so it is what is from the vault, rather than what was loaded from the configuration, as the Chat Log may have been deleted.
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CVaultEvents::ReadEventsFromDisk(\"{h!}.dat\") for '$s': $I events, m_tsEventIdLastSentCached=$t\n", pHashFileName, m_pParent->TreeItem_PszGetNameDisplay(), m_arraypaEvents.GetSize(), m_pParent->m_tsEventIdLastSentCached);
	if (m_pParent->EGetRuntimeClass() == RTI(TContact))
		{
		TContact * pContact = (TContact *)m_pParent;
		TIMESTAMP tsOtherLastReceived = m_arraypaEvents.TsEventOtherLastEventReceived();
		if (tsOtherLastReceived < pContact->m_tsOtherLastSynchronized || pContact->m_tsOtherLastSynchronized == d_ts_zNA)
			{
			if (pContact->m_tsOtherLastSynchronized != tsOtherLastReceived)
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Adjusting m_tsOtherLastSynchronized from $t to $t for '$s'\n", pContact->m_tsOtherLastSynchronized, tsOtherLastReceived, m_pParent->TreeItem_PszGetNameDisplay());
			pContact->m_tsOtherLastSynchronized = tsOtherLastReceived;
			}
		}
	else
		{
		Assert(m_pParent->EGetRuntimeClass() == RTI(TGroup));
		if (m_arraypaEvents.FIsEmpty())
			{
			// If there are no events, this means the Chat Log is new or was deleted.  In any regards, make sure the timestamps are initialized to zero to make sure the Chat Log is properly reconstructed.
			TGroupMember ** ppMemberStop;
			TGroupMember ** ppMember = ((TGroup *)m_pParent)->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
			while (ppMember != ppMemberStop)
				{
				TGroupMember * pMember = *ppMember++;
				Assert(pMember != NULL);
				Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
				if (pMember->m_tsOtherLastSynchronized != d_ts_zNULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Clearing m_tsOtherLastSynchronized $t for group member ^j\n", pMember->m_tsOtherLastSynchronized, pMember->m_pContact);
					pMember->m_tsOtherLastSynchronized = d_ts_zNULL;
					}
				} // while
			}
		} // if...else
	} // ReadEventsFromDisk()

void
CVaultEvents::WriteEventsToDiskIfModified()
	{
	Endorse(m_pEventLastSaved == NULL);	// Always write the vault to disk
	IEvent * pEventLastSaved = PGetEventLast_YZ();
	if (pEventLastSaved != m_pEventLastSaved)
		{
		int cEvents = m_arraypaEvents.GetSize();
		CBinXcpStanzaTypeInfo binXmlEvents;
		binXmlEvents.PvSizeAlloc(100 + 64 * cEvents);	// Pre-allocate about 64 bytes per event.  This estimate will reduce the number of memory re-allocations.
		binXmlEvents.BinAppendTextSzv_VE("<Events v='1' c='$i'>\n", cEvents);
		m_arraypaEvents.EventsSerializeForDisk(INOUT &binXmlEvents);
		binXmlEvents.BinAppendTextSzv_VE("</Events>");
		if (binXmlEvents.BinFileWriteE(m_sPathFileName) == errSuccess)
			m_pEventLastSaved = pEventLastSaved;
		}
	}

void
CVaultEvents::GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC
	{
	parraypEventsChatLog->Copy(IN &m_arraypaEvents);
	}

//	Return the number of remaining events after pEvent.
//	Return 0 if pEvent is at the end of the array, or not within the array.
int
CVaultEvents::UEventsRemaining(IEvent * pEvent) const
	{
	Assert(pEvent != NULL);
	return m_arraypaEvents.UFindRemainingElements(pEvent);
	}

//	Return the number of events received since (after) tsEventID
int
CVaultEvents::UCountEventsReceivedByOtherGroupMembersSinceTimestampEventID(TIMESTAMP tsEventID, TContact * pContactExclude) CONST_MCC
	{
	Assert(pContactExclude != NULL);
	int cEventsReceived = 0;

	if (tsEventID > d_tsOther_kmReserved)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		IEvent ** ppEventSince = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
		while (ppEvent != ppEventSince)
			{
			IEvent * pEvent = *--ppEventSince;
			AssertValidEvent(pEvent);
			Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
			if (pEvent->m_tsEventID == tsEventID)
				{
				ppEventSince++;	// Skip the event we just found
				break;
				}
			} // while
		// Now, count how many events have been received
		while (ppEventSince != ppEventStop)
			{
			IEvent * pEvent = *ppEventSince++;
			AssertValidEvent(pEvent);
			if (pEvent->m_pContactGroupSender_YZ != pContactExclude && pEvent->Event_FIsEventTypeReceived())
				cEventsReceived++;
			}
		} // if
	return cEventsReceived;
	} // UCountEventsReceivedByOtherGroupMembersSinceTimestampEventID()

int
CVaultEvents::UCountEventsReceivedByOtherGroupMembersSinceTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	int cEventsReceived = 0;

	if (tsOther > d_tsOther_kmReserved)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		IEvent ** ppEventSince = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
		while (ppEvent != ppEventSince)
			{
			IEvent * pEvent = *--ppEventSince;
			AssertValidEvent(pEvent);
			if (pEvent->m_tsOther == tsOther)
				{
				ppEventSince++;	// Skip the event we just found
				break;
				}
			} // while
		// Now, count how many events have been received
		while (ppEventSince != ppEventStop)
			{
			IEvent * pEvent = *ppEventSince++;
			AssertValidEvent(pEvent);
			if (pEvent->Event_FIsEventTypeReceived())
				cEventsReceived++;
			}
		}
	return cEventsReceived;
	}

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
	PSZUC pszNameDisplay = TreeItem_PszGetNameDisplay();
	PCHUC pchAt = pszNameDisplay;
	while (TRUE)
		{
		CHS ch = *pchAt;
		if (ch == ' ' || ch == '@')
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
	m_cMessagesUnread = 0;
	m_pawLayoutChatLog = NULL;
	}

ITreeItemChatLogEvents::~ITreeItemChatLogEvents()
	{
	delete m_paVaultEvents;
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutChatLog);
	}

CChatConfiguration *
ITreeItemChatLogEvents::PGetConfiguration() const
	{
	return m_pAccount->PGetConfiguration();
	}

//	ITreeItemChatLogEvents::IRuntimeObject::PGetRuntimeInterface()
void *
ITreeItemChatLogEvents::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	if (rti == RTI(ITreeItemChatLogEvents))
		return (ITreeItemChatLogEvents *)this;
	return ITreeItem::PGetRuntimeInterface(rti);
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
void
ITreeItemChatLogEvents::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	if (pXmlExchanger->m_fSerializing && m_tsCreated == d_zNA && m_paVaultEvents != NULL)
		{
		IEvent * pEventFirst = (IEvent *)m_paVaultEvents->m_arraypaEvents.PvGetElementFirst_YZ();	// This code is a bit of legacy and should be moved when the Tree Item (contact) is created)
		if (pEventFirst != NULL)
			m_tsCreated = pEventFirst->m_tsEventID;
		else
			m_tsCreated = Timestamp_GetCurrentDateTime();
		}
	pXmlExchanger->XmlExchangeTimestamp("tsCreated", INOUT_F_UNCH_S &m_tsCreated);
	#if 0
	pXmlExchanger->XmlExchangeTimestamp("tsEventSyncSent", INOUT_F_UNCH_S &m_synchronization.tsEventID);
	pXmlExchanger->XmlExchangeTimestamp("tsEventSyncReceived", INOUT_F_UNCH_S &m_synchronization.tsOther);
	#endif
	pXmlExchanger->XmlExchangeStr("DownloadFolder", INOUT_F_UNCH_S &m_strPathFolderDownload);
	pXmlExchanger->XmlExchangeInt("MessagesUnread", INOUT_F_UNCH_S &m_cMessagesUnread);
	}

//	ITreeItemChatLogEvents::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
ITreeItemChatLogEvents::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_FindText:
		ChatLog_FindText();
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
ITreeItemChatLogEvents::Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD IEvent * paEvent)
	{
	AssertValidEvent(paEvent);
	paEvent->EventAddToVault(PA_PARENT Vault_PGet_NZ());
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->ChatLog_EventAppend(IN paEvent);
	}

void
ITreeItemChatLogEvents::Vault_SetNotModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->SetNotModified();
	}

void
ITreeItemChatLogEvents::Vault_SetModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->SetModified();
	}

IEvent *
ITreeItemChatLogEvents::Vault_PFindEventByID(TIMESTAMP tsEventID) CONST_MCC
	{
	return Vault_PGet_NZ()->m_arraypaEvents.PFindEventByID(tsEventID);
	}

CEventMessageTextSent *
ITreeItemChatLogEvents::Vault_PGetEventLastMessageSent_YZ() const
	{
	if (m_paVaultEvents != NULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_paVaultEvents->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			if (pEvent->EGetEventClass() == CEventMessageTextSent::c_eEventClass)
				return (CEventMessageTextSent *)pEvent;
			}
		}
	return NULL;
	}

void
ITreeItemChatLogEvents::Vault_WriteEventsToDiskIfModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->WriteEventsToDiskIfModified();
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
