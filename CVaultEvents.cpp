#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_chVault_Event		'E'
#define d_szVault_Event		"E"

CVaultEventsCore::CVaultEventsCore()
	{

	}

CVaultEventsCore::~CVaultEventsCore()
	{
	m_arraypaEvents.DeleteAllEvents();
	}

void
CVaultEvents::EventsSerializeForMemory(IOUT CBinXcpStanza * pbinXmlEvents) const
	{
	pbinXmlEvents->BinAppendText("<"d_szVault_Event">");
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		if (pEvent->m_uFlagsEvent & IEvent::FE_kfEventDeleted)
			continue;	// Don't serialize deleted events
		EEventClass eEventClass = pEvent->EGetEventClass();
		pbinXmlEvents->BinAppendTextSzv_VE("<$U" _tsI, eEventClass, pEvent->m_tsEventID);
		pEvent->XmlSerializeCore(IOUT pbinXmlEvents);
		if ((eEventClass & eEventClass_kfSerializeDataAsXmlElement) == 0)
			pbinXmlEvents->BinAppendXmlForSelfClosingElement();
		else
			pbinXmlEvents->BinAppendTextSzv_VE("</$U>\n", eEventClass);
		} // while
	pbinXmlEvents->BinAppendText("</"d_szVault_Event">");
	}

void
CVaultEvents::XmlExchange(PSZAC pszTagNameVault, INOUT CXmlExchanger * pXmlExchanger)
	{
	CBinXcpStanzaTypeInfo binXcpStanza;
	if (pXmlExchanger->m_fSerializing)
		{
		EventsSerializeForMemory(INOUT &binXcpStanza);
		pXmlExchanger->XmlExchangeBin(pszTagNameVault, IN &binXcpStanza);
		}
	else
		{
		pXmlExchanger->XmlExchangeBin(pszTagNameVault, OUT &binXcpStanza);
		MessageLog_AppendTextFormatCo(d_coRed, "CVaultEvents::XmlExchange($s) : $B\n", pszTagNameVault, IN &binXcpStanza);
		CXmlTree oXmlTree;
		oXmlTree.SetFileDataCopy(binXcpStanza);
		if (oXmlTree.EParseFileDataToXmlNodes_ML() == errSuccess)
			EventsUnserialize(IN &oXmlTree);
		MessageLog_AppendTextFormatCo(d_coRed, "\t $i events read\n", m_arraypaEvents.GetSize());
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
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
	delete m_history.m_paVault;	// Recursively the vault history
	}

void
CVaultEvents::EventsUnserialize(const CXmlNode * pXmlNodeEvents)
	{
	Assert(pXmlNodeEvents != NULL);
	if (pXmlNodeEvents->FCompareTagName(d_szVault_Event) || pXmlNodeEvents->FCompareTagName("Events"))
		m_arraypaEvents.EventsUnserializeFromDisk(IN pXmlNodeEvents->m_pElementsList, m_pParent);
	}

void
CVaultEvents::EventsUnserialize(PSZUC pszXmlNodeEvents)
	{
	CXmlTree oXmlTree;
	oXmlTree.m_binXmlFileData.BinInitFromStringWithNullTerminator((PSZAC)pszXmlNodeEvents);
	if (oXmlTree.EParseFileDataToXmlNodes_ML() == errSuccess)
		EventsUnserialize(IN &oXmlTree);
	}

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
			EventsUnserialize(IN &oXmlTreeEvents);
			// TODO: We need to check if this is the first vault in the chain.  So far, there is always only one vault, however this code will have to be revised when chaining vaults.
			}
		}
	m_pParent->m_tsEventIdLastSentCached = m_arraypaEvents.TsEventIdLastEventSent();	// Update the timestamp so it is what is from the vault, rather than what was loaded from the configuration, as the Chat Log may have been deleted.
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CVaultEvents::ReadEventsFromDisk(\"{h!}.dat\") for '$s': $I events, m_tsEventIdLastSentCached=$t\n", pHashFileName, m_pParent->TreeItem_PszGetNameDisplay(), m_arraypaEvents.GetSize(), m_pParent->m_tsEventIdLastSentCached);
	if (m_pParent->EGetRuntimeClass() == RTI(TContact))
		{
		TContact * pContact = (TContact *)m_pParent;
		TIMESTAMP tsOtherLastReceived = m_arraypaEvents.TsEventOtherLastEventReceived();
		if (tsOtherLastReceived > 0)
			pContact->SetFlagContactAsInvited();	// If there is one event received, then the contact does not need an invitation
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
		binXmlEvents.BinAppendTextSzv_VE("<E v='1' c='$i'>\n", cEvents);
		m_arraypaEvents.EventsSerializeForDisk(INOUT &binXmlEvents);
		binXmlEvents.BinAppendTextSzv_VE("</E>");
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
