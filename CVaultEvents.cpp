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
		pbinXmlEvents->BinXmlSerializeEventForDisk(pEvent);	// Serializing for memory or disk uses the same core method
		/*
		#if 1
		pbinXmlEvents->BinAppendXmlEventSerializeOpen(pEvent, pEvent->m_tsEventID);	// TODO: need to fix the need to serialize tsEventID twice
		#else
		pbinXmlEvents->BinAppendXmlEventSerializeOpen(pEvent, d_ts_zNULL);
		#endif
		pbinXmlEvents->BinAppendXmlEventSerializeDataAndClose(pEvent);
		*/
		} // while
	pbinXmlEvents->BinAppendText("</"d_szVault_Event">");
	}

void
CVaultEvents::XmlExchange(PSZAC pszTagNameVault, INOUT CXmlExchanger * pXmlExchanger)
	{
	CBinXospStanzaForDisk binXcpStanza;
	if (pXmlExchanger->m_fSerializing)
		{
		EventsSerializeForMemory(INOUT &binXcpStanza);
		pXmlExchanger->XmlExchangeBin(pszTagNameVault, IN &binXcpStanza);
		}
	else
		{
		pXmlExchanger->XmlExchangeBin(pszTagNameVault, OUT &binXcpStanza);
		//MessageLog_AppendTextFormatCo(d_coChocolate, "CVaultEvents::XmlExchange($s) : $B\n", pszTagNameVault, IN &binXcpStanza);
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
	Assert(pTreeItemParent != NULL);
	Assert(pTreeItemParent->m_paVaultEvents == NULL && "Memory leak!");
	pTreeItemParent->m_paVaultEvents = this;
	m_pParent = pTreeItemParent;
	m_pEventLastSaved = NULL;
	InitToZeroes(OUT &m_history, sizeof(m_history));
	if (pHashFileName != d_zNA)
		ReadEventsFromDisk(pHashFileName);
	}

CVaultEvents::~CVaultEvents()
	{
	delete m_history.m_paVault;	// Recursively the vault history
	}


void
CVaultEvents::EventAdd(PA_CHILD IEvent * paEvent)
	{
	Assert(paEvent != NULL);
	Assert(paEvent->m_tsOther == d_ts_zNA);
	Assert(paEvent->m_pVaultParent_NZ == NULL);
	m_arraypaEvents.Add(PA_CHILD paEvent);
	SetModified();	// Make sure the events are saved to disk
	}

//	Core routine to add a new event to the vault and send it to the contact(s).
//	Optionally this method may also send an 'Updater' event
void
CVaultEvents::EventAddAndDispatchToContacts(PA_CHILD IEvent * paEvent, PA_CHILD CEventUpdaterSent * paEventUpdater)
	{
	Assert(paEvent != NULL);
	Assert(paEvent->m_tsOther == d_ts_zNA);
	Assert(paEvent->m_pVaultParent_NZ == NULL);
	//MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "EventAddAndDispatchToContacts() - tsEventID $t\n", paEvent->m_tsEventID);
	if (paEventUpdater != NULL)
		{
		// Always include the Updater before the updated event
		paEventUpdater->m_pVaultParent_NZ = this;
		m_arraypaEvents.Add(PA_CHILD paEventUpdater);
		}
	paEvent->m_pVaultParent_NZ = this;
	m_arraypaEvents.Add(PA_CHILD paEvent);
	SetModified();	// Make sure the events are saved to disk

	if (paEvent->EGetEventClass() & eEventClass_kfNeverSerializeToXCP)
		return;	// The event is never serialized to XOSP, therefore there is nothing else to do

	CBinXcpStanza binXcpStanza;			// All events are sent as XMPP 'messages' so they may be cached
	TGroup * pGroup = (TGroup *)m_pParent;
	TContact * pContact = (TContact *)m_pParent;
	if (pContact->EGetRuntimeClass() == RTI(TContact))
		{
		// Send the message to a contact
		MessageLog_AppendTextFormatCo(d_coGrayDark, "\t Sending message to $S\n\t\t m_tsEventIdLastSentCached $t, m_tsOtherLastSynchronized $t\n", &pContact->m_strJidBare, pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
		binXcpStanza.BinXmlAppendXcpApiCall_SendEventToContact(pContact, paEvent, paEventUpdater);
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
			binXcpStanza.BinXmlAppendXcpApiCall_SendEventToContact(pMember->m_pContact, paEvent, paEventUpdater);
			} // while
		} // if...else
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
	Assert(pHashFileName != NULL);
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
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Adjusting m_tsOtherLastSynchronized by -$T from $t ({tL}) to $t for '$s'\n", pContact->m_tsOtherLastSynchronized - tsOtherLastReceived, pContact->m_tsOtherLastSynchronized, pContact->m_tsOtherLastSynchronized, tsOtherLastReceived, m_pParent->TreeItem_PszGetNameDisplay());
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
		CBinXospStanzaForDisk binXmlEvents;
		binXmlEvents.PbbAllocateMemoryAndEmpty_YZ(100 + 64 * cEvents);	// Pre-allocate about 64 bytes per event.  This estimate will reduce the number of memory re-allocations.
		binXmlEvents.BinAppendText_VE("<E v='1' c='$i'>\n", cEvents);
		m_arraypaEvents.EventsSerializeForDisk(INOUT &binXmlEvents);
		binXmlEvents.BinAppendText_VE("</E>");
		TRACE2("CVaultEvents::WriteEventsToDiskIfModified($Q) for ^j", &m_sPathFileName, m_pParent);
		if (binXmlEvents.BinFileWriteE(m_sPathFileName) == errSuccess)
			m_pEventLastSaved = pEventLastSaved;
		}
	else
		{
		TRACE3("CVaultEvents::WriteEventsToDiskIfModified(^j) - m_pEventLastSaved=0x$p, pEventLastSaved=0x$p", m_pParent, m_pEventLastSaved, pEventLastSaved);
		if (pEventLastSaved == NULL)
			{
			Assert(m_arraypaEvents.FIsEmpty());
			}
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

//	Return all events since a timestamp
int
CVaultEvents::UGetEventsSinceTimestamp(TIMESTAMP tsEventID, OUT CArrayPtrEvents * parraypEvents) CONST_MCC
	{
	Endorse(tsEventID == d_ts_zNA);	// Return all events since the beginning
	Assert(parraypEvents != NULL);
	IEvent ** ppEventStop;
	IEvent ** ppEventFirst = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	IEvent ** ppEventCompare = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
	while (ppEventCompare != ppEventFirst)
		{
		IEvent * pEvent = *--ppEventCompare;
		AssertValidEvent(pEvent);
		Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
		if (pEvent->m_tsEventID <= tsEventID)
			{
			ppEventCompare++;
			break;
			}
		} // while
	const int cEventsSinceStimestamp = (ppEventStop - ppEventCompare);
	Assert(cEventsSinceStimestamp >= 0);
	IEvent ** ppEventDst = (IEvent **)parraypEvents->PrgpvAllocateElementsSetSize(cEventsSinceStimestamp);
	while (ppEventCompare != ppEventStop)
		{
		*ppEventDst++ = *ppEventCompare++;	// Copy the values to the array
		}
	return cEventsSinceStimestamp;
	}

/*
//	Return the event which was replaced by the new replacing event
//	Return NULL if unable to find the old event, or if the vault policies to not allow the editing an old event.
IEvent *
CArrayPtrEvents::PFindEventReplacedBy(int iEventReplacing) const
	{
	Assert(iEventReplacing >= 0);
	Assert(iEventReplacing < GetSize());
	Assert(m_paArrayHdr != NULL);
	IEvent ** ppEventFirst = (IEvent **)m_paArrayHdr->rgpvData;
	IEvent ** ppEventStop = ppEventFirst + iEventReplacing;
	IEvent * pEventReplacing = *ppEventStop;
	//Assert(pEventReplacing->m_uFlagsEvent & IEvent::FE_kfReplacing);
	if ((pEventReplacing->m_uFlagsEvent & IEvent::FE_kfReplacing) == 0)
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "PFindEventReplacedBy($i) - Event ID $t is missing flag IEvent::FE_kfReplacing\n", iEventReplacing, pEventReplacing->m_tsEventID);
		}
	const EEventClass eEventClassUpdater = pEventReplacing->Event_FIsEventTypeSent() ? eEventClass_eUpdaterSent : eEventClass_eUpdaterReceived;	// Which updater to search for
	while (ppEventFirst < ppEventStop)
		{
		// Find the updater which should be right before the replacing event
		IEvent * pEvent = *--ppEventStop;
		if (pEvent->EGetEventClass() == eEventClassUpdater)
			{
			// Search the event matching the ID
			TIMESTAMP tsEventIdOld = ((CEventUpdaterSent *)pEvent)->m_tsEventIdOld;
			while (ppEventFirst < ppEventStop)
				{
				pEvent = *--ppEventStop;
				if (pEvent->m_tsEventID == tsEventIdOld)
					{
					// Since the event has been replaced, update its flag
					pEvent->m_uFlagsEvent |= IEvent::FE_kfReplaced;
					return pEvent;
					}
				if (pEvent->m_tsEventID < tsEventIdOld)
					break;	// Since all events are sorted, then there is no need to search further
				} // while
			} // if
		} // while
	return NULL;
	}
*/

//	Return the event which was replaced by the new replacing event
IEvent *
CVaultEvents::PFindEventReplacedBy(IEvent * pEventReplacing) CONST_MCC
	{
	Assert(pEventReplacing != NULL);
	int iEventReplacing = m_arraypaEvents.FindElementI(pEventReplacing);
	if (iEventReplacing > 0)
		{
		// Find the updater which should be right before the event
		CEventUpdaterSent * pEventUpdater = (CEventUpdaterSent *)m_arraypaEvents.PvGetElementAt(iEventReplacing - 1);
		if (pEventUpdater->EGetEventClass() == eEventClass_eUpdaterSent)
			{
			return PFindEventByID(pEventUpdater->m_tsEventIdOld);
			}
		}
	return NULL;
	}

IEvent *
CVaultEvents::PFindEventReplacing(IEvent * pEventReplaced) CONST_MCC
	{
	Assert(pEventReplaced != NULL);
	Assert(pEventReplaced->m_uFlagsEvent & IEvent::FE_kfReplaced);
	IEvent * pEventReplacing = pEventReplaced;
	const EEventClass eEventClassUpdater = pEventReplaced->Event_FIsEventTypeSent() ? CEventUpdaterSent::c_eEventClass : CEventUpdaterReceived::c_eEventClass;	// Which updater to search for
	// Attempt to find the most recent updater, this is done by searching from the end
	IEvent ** ppEventStop;
	IEvent ** ppEventFirst = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	IEvent ** ppEventCompare = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
	while (ppEventCompare != ppEventFirst)
		{
		IEventUpdater * pEventUpdater = (IEventUpdater *)*--ppEventCompare;
		if (pEventUpdater->EGetEventClass() != eEventClassUpdater)
			continue;
		if (pEventUpdater->m_tsEventIdOld == pEventReplacing->m_tsEventID)
			{
			IEvent ** ppEventReplacing = ppEventCompare + 1;	// The replacing event is always after the updater
			if (ppEventReplacing < ppEventStop)
				{
				pEventReplacing = *ppEventReplacing;
				Assert(pEventReplacing->m_tsEventID == pEventUpdater->m_tsEventIdNew);
				MessageLog_AppendTextFormatSev(eSeverityNoise, "Event ID $t is replacing event $t\n", pEventReplacing->m_tsEventID, pEventReplaced->m_tsEventID);
				// Search forward for chained events
				while (ppEventCompare < ppEventStop)
					{
					pEventUpdater = (IEventUpdater *)*ppEventCompare++;
					if (pEventUpdater->EGetEventClass() != eEventClassUpdater)
						continue;
					if (pEventUpdater->m_tsEventIdOld == pEventReplacing->m_tsEventID && ppEventCompare < ppEventStop)
						{
						pEventReplacing = *ppEventCompare++;
						MessageLog_AppendTextFormatSev(eSeverityNoise, "Event ID $t is chain replacing event $t\n", pEventReplacing->m_tsEventID, pEventReplaced->m_tsEventID);
						}
					} // while
				goto Done;
				}
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Event ID $t is replacing event $t which cannot be found\n", pEventUpdater->m_tsEventIdNew, pEventReplaced->m_tsEventID);
			}
		} // while
	Done:
	return pEventReplacing;
	}
