//	Code no longer used, however may be useful later


IEvent *
CVaultEvents::PFindEventLargerThanTimestamp(TIMESTAMP tsEventID) CONST_MCC
	{
	Endorse(tsEventID == d_ts_zNULL);	// Return the first event
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->m_tsEventID > tsEventID)
			return pEvent;
		} // while
	return NULL;
	}

IEvent *
CVaultEvents::PFindEventLargerThanTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	Endorse(tsOther == d_ts_zNULL);	// Return the first event
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		Assert(pEvent->m_tsOther != d_ts_zNULL);	// Not sure about this Assert()
		if (pEvent->m_tsOther > tsOther)
			return pEvent;
		} // while
	return NULL;
	}

//	Find the last group event received
IEvent *
CVaultEvents::PFindEventReceivedLastMatchingContactSender(TContact * pContactGroupSender) CONST_MCC
	{
	Assert(pContactGroupSender != NULL);
	Assert(pContactGroupSender->EGetRuntimeClass() == RTI(TContact));
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->m_pContactGroupSender_YZ == pContactGroupSender)
			{
			Assert(pEvent->m_tsOther > d_tsOther_kmReserved);
			return pEvent;
			}
		} // while
	return NULL;
	}

//	Return the first event larger than the timestamp
IEvent *
CVaultEvents::PFindEventSentLargerThanTimestamp(TIMESTAMP tsEventID) CONST_MCC
	{
	Endorse(tsEventID == d_ts_zNULL);	// Return the first event sent
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->Event_FIsEventTypeSent())
			{
			if (pEvent->m_tsEventID > tsEventID)
				return pEvent;
			}
		} // while
	return NULL;
	}


IEvent *
CArrayPtrEvents::PFindEventLastReceived() const
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		AssertValidEvent(pEvent);
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->Event_FIsEventTypeReceived())
			return pEvent;
		}
	return NULL;
	}
