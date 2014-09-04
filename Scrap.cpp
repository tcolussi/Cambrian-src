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
CVaultEvents::PFindEventByTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	if (tsOther != d_ts_zNULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			Assert(pEvent->m_tsEventID != d_ts_zNULL);
			Assert(pEvent->m_tsOther != d_ts_zNULL);
			if (pEvent->m_tsOther <= tsOther)
				{
				if (pEvent->m_tsOther == tsOther)
					return pEvent;
				break;
				}
			} // while
		} // if
	return NULL;
	}

//	Return the event received matching tsOther
IEvent *
CVaultEvents::PFindEventReceivedByTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	if (tsOther != d_ts_zNULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			Assert(pEvent->m_tsEventID != d_ts_zNULL);
			if (!pEvent->Event_FIsEventTypeReceived())
				continue;
			Assert(pEvent->m_tsOther != d_ts_zNULL);
			if (pEvent->m_tsOther <= tsOther)
				{
				if (pEvent->m_tsOther == tsOther)
					return pEvent;
				break;
				}
			} // while
		} // if
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

//	Filter the events from the QLineEdit
bool
WNavigationTreeCaption::eventFilter(QObject * obj, QEvent *event)
	{
	if (event->type() == QEvent::KeyPress)
		{
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Escape)
			{
			// MessageLog_AppendTextFormatCo(d_coRed, "WNavigationTreeCaption::eventFilter() - Escape key pressed\n");

			}
		}
	return QWidget::eventFilter(obj, event);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method to serialize the data for an XCP API request.
void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(XcpApi_FIsXmlElementOpened());
	Assert(m_pContact != NULL);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	MessageLog_AppendTextFormatSev(eSeverityComment, "Processing API '$s' with the following parameters: ^N\n", pszApiName, pXmlNodeApiParameters);

	/*
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Synchronize))
		{
		BinXmlAppendXcpApiSynchronize_OnReply(pXmlNodeApiParameters);
		return;
		}
	*/
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Group_Profile_Get))
		{
		PSZUC pszProfileIdentifier = pXmlNodeApiParameters->m_pszuTagValue;
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszProfileIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileSerialize(INOUT this);
			return;
			}
		Assert(XcpApi_FIsXmlElementClosedBecauseOfError());
		return;
		}
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Contact_Recommendations_Get))
		{
		pAccount->m_pProfileParent->XcpApiProfile_RecommendationsSerialize(INOUT this); // Return all the recommendations related to the profile
		return;
		}

	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API '$s'\n", pszApiName);
	} // BinXmlAppendXcpElementForApiRequest_AppendApiParameterData()


//	Method to unserialize the data from BinXmlAppendXcpElementForApiRequest_AppendApiParameterData().
//	Although the reply typically does not need the object CBinXcpStanza, the reply may necessitate the CBinXcpStanza to make additional request.
//	For instance, when unserializing a group, the method may need to query information about new group members to fetch the contact profile.
void
CBinXcpStanza::BinXmlAppendXcpElementForApiReply(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(m_pContact != NULL);
	MessageLog_AppendTextFormatSev(eSeverityComment, "BinXmlAppendXcpElementForApiReply($s):\n^N\n", pszApiName, pXmlNodeApiParameters);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Group_Profile_Get))
		{
		// We received a profile information
		PSZUC pszGroupIdentifier = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_TGroup_shaIdentifier);
		//MessageLog_AppendTextFormatCo(d_coRed, "BinXmlAppendXcpElementForApiReply($s) - Group '$s'\n", pszApiName, pszGroupIdentifier);
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszGroupIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileUnserialize(IN pXmlNodeApiParameters, INOUT this);
			}
		return;
		}
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Contact_Recommendations_Get))
		{
		// We received new recommendations from the contact
		const_cast<CXmlNode *>(pXmlNodeApiParameters)->RemoveEmptyElements();
		m_pContact->Contact_RecommendationsUpdateFromXml(IN pXmlNodeApiParameters);
		return;
		}

	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API: $s\n^N", pszApiName, pXmlNodeApiParameters);
	}
