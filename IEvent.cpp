///////////////////////////////////////////////////////////////////////////////////////////////////
//	IEvent.cpp
//
//	Genetic code to process events, and code to implement specific classes of events.

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef DEBUG
	#define DEBUG_DISPLAY_TIMESTAMPS	// Always display the timestamps on the debug build
#else
	//#define DEBUG_DISPLAY_TIMESTAMPS	// Sometimes display the timestamps on the retail build
#endif

CHS
ChGetCambrianActionFromUrl(PSZUC pszUrl)
	{
	Assert(FIsSchemeCambrian(pszUrl));
	if (pszUrl[2] == d_chSchemeCambrianActionSeparator)
		return pszUrl[3];
	return d_chCambrianAction_None;
	}

//	Return TRUE if the URL is a valid hyperlink without the Cambrian scheme.
BOOL
CStr::PathUrl_FIsValidHyperlinkNonCambrian() const
	{
	if (m_paData != NULL)
		{
		const CHU * pch = m_paData->rgbData;
		if (!FIsSchemeCambrian(pch))
			{
			while (TRUE)
				{
				CHS ch = *pch++;
				if (ch == ':')
					return TRUE;
				if (!Ch_FIsAlphaNumeric(ch))
					break;
				} // while
			} // if
		}
	return FALSE;
	}

#define	_nbsp		d_szu_nbsp
#define _nbsp2		_nbsp _nbsp

#define d_chAttribute_strText					't'	// Generic text message
#define d_chAttribute_strHtml					'h'	// Generic html content
#define d_chAttribute_uFlags					'u'
#define d_chAttribute_cbFileSize				's'
#define d_chAttribute_cbDataTransferred			'a'
#define d_chAttribute_strxStanzaID				'z'
#define d_chAttribute_strxFileSID				'd'


TIMESTAMP g_tsLast;	// Last returned timestamp (this global variable is to ensure all timestamps are unique)

//	Return the current timestamp.
//	The timestamp if the number the number of milliseconds that have passed since 1970.
//	Ideally, this function should be called less than 1000 times per second, otherwise the returned timestamp will be ahead of the current time, however
//	the functionalitly will remain the same: the timestamp provides a mechanism to identify events in chronological order.
TIMESTAMP
Timestamp_GetCurrentDateTime()
	{
	const TIMESTAMP tsPrev = g_tsLast;
	g_tsLast = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	if (g_tsLast <= tsPrev)
		g_tsLast = tsPrev + 1;
	return g_tsLast;
	}

IEvent *
IEvent::S_PaAllocateEvent_YZ(EEventClass eEventClass, const TIMESTAMP * ptsEventID)
	{
	Endorse(ptsEventID == NULL);	// Assign the value of the current date and time (aka: now)
	switch (eEventClass)
		{
	case eEventClass_eMessageTextSent:
	case eEventClassLegacy_chMessageSent:
		return new CEventMessageTextSent(ptsEventID);

	case eEventClass_eMessageTextReceived:
	case eEventClassLegacy_chMessageReceived:
		return new CEventMessageTextReceived(ptsEventID);
	/*
	case eEventClass_eMessageTextReceivedToGroup:
		return new CEventMessageTextReceivedToGroup(ptsEventID);
	*/
	case eEventClass_eFileSent:
		return new CEventFileSent(ptsEventID);
	case eEventClass_eFileReceived:
		return new CEventFileReceived(ptsEventID);
/*
	case eEventClass_eFileReceivedToGroup:
		return new CEventFileReceivedToGroup(ptsEventID);
*/
	/*
	case eEventClass_eGroupMemberJoins:
		return new CEventGroupMemberJoin((TGroup *)pTreeItemParentOwner, ptsEventID);
	case eEventClass_eGroupMessageTextSent:
		return new CEventGroupMessageSent((TGroup *)pTreeItemParentOwner, ptsEventID);
	case eEventClass_eGroupMessageTextReceived:
		return new CEventGroupMessageReceived((TGroup *)pTreeItemParentOwner, ptsEventID);
	*/
	case eEventClass_eWalletTransactionSent:
		return new CEventWalletTransactionSent(ptsEventID);
	case eEventClass_eWalletTransactionReceived:
		return new CEventWalletTransactionReceived(ptsEventID);

	case eEventClass_eDownloader:
		return new CEventDownloader(ptsEventID);

	default:
		return NULL;
		} // switch
	} // S_PAllocateEvent_YZ()

//	Compare the timestamps of two events.
//	Because timestamps are stored in a 64 bit integer, we have to do the 64-bit comparison, because truncating from 64 bit to 32 bit may lead to an incorrect result even for a negative value!
int
IEvent::S_NCompareSortEventsByIDs(IEvent * pEventA, IEvent * pEventB, LPARAM lParamCompareSort)
	{
	Assert(lParamCompareSort == d_zNA);
	if (pEventA->m_tsEventID < pEventB->m_tsEventID)
		return -1;
	if (pEventA->m_tsEventID > pEventB->m_tsEventID)
		return +1;
	return 0;	// This should be very rare, as all m_tsEventID are unique
	}

//	Base constructor for all events.
IEvent::IEvent(const TIMESTAMP * ptsEventID)
	{
	m_pVaultParent_NZ = NULL;					// Although this pointer is declared 'non-zero', it is initialized to NULL because as soon ad the event is added to a vault, this poiinter will become valid
	m_pContactGroupSender_YZ = NULL;
	m_uFlagsEvent = FE_kzDefault;
	m_tsOther = d_tsOther_ezEventNeverSent;		// Initialize m_tsOther to something so we don't get unpleasant surprises
	if (ptsEventID != NULL)
		m_tsEventID = *ptsEventID;						// We are creating an event with an existing timestamp, which is typically when an event is being unserialized from disk or XCP
	else
		m_tsEventID = Timestamp_GetCurrentDateTime();	// We are creating a new event, so use the current date & time as the timestamp.
	}

IEvent::~IEvent()
	{
	}

void
IEvent::EventAddToVault(PA_PARENT CVaultEvents * pVaultParent)
	{
	Assert(m_pVaultParent_NZ == NULL);
	m_pVaultParent_NZ = pVaultParent;
	(void)pVaultParent->m_arraypaEvents.Event_FoosAddSorted(PA_CHILD this);
	}

void
IEvent::EventAddToVault(PA_PARENT TContact * pContactParent)
	{
	Assert(pContactParent != NULL);
	EventAddToVault(PA_PARENT pContactParent->Vault_PGet_NZ());
	}

//	EGetEventClassForXCP(), virtual
//
//	This virtual method is necessary for an event to serialize itself in a suitable way for the Cambrian Protocol.
//	Essentially this virtual method returns the class the remote client instanciate to unserialize the data, which is often the opposite class as the event itself.
//	For instance, a 'message sent' by the user is a 'message received' by the contact.
EEventClass
IEvent::EGetEventClassForXCP(const TContact * pContactToSerializeFor) const
	{
	Assert(pContactToSerializeFor != NULL);
	Assert(pContactToSerializeFor->EGetRuntimeClass() == RTI(TContact));
	return EGetEventClass();		// By default, return the same as the event class.  This is useful for objects which may not be serialized, however may send stanzas such as a ping or raw XML.
	}

//	XmlSerializeCore(), virtual
//
//	Core virtual method to serialize the data of the event into an XML to be saved to disk or transmitted over the Cambrian Protocol.
//	Since most events are made of simple data types, such as strings, numbers and timestamps, the entire event is often
//	serialized as multiple attributes.  Later, an event may be serialized as an XML element (TBD).
//
//	Originally this method was using a CBin object to store the XML information, however the Cambrian Protocol requires to know
//	the destination contact.  As a result, the CBinXcpStanzaType is used to store the XML info as well as providing additional information for the XCP.
//
//
//	IMPLEMENTATION NOTES
//	Most implementations of XmlSerializeCore() will use a single letter of the alphabet to designe an attribute name.
//	Using a single character makes the comparison faster to find an attribute, while reducing the storage requirement of the XML file.
void
IEvent::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	Assert(pbinXmlAttributes != NULL);
	Endorse(pbinXmlAttributes->m_pContact == NULL);	// NULL => Serialize to disk
	Assert(FALSE && "No need to call this virtual method");
	}

//	XmlUnserializeCore(), virtual
//
//	Virtual method compliment to XmlSerializeCore().  This is the method for unserializing an event from disk or or from XCP.
void
IEvent::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	Assert(pXmlNodeElement != NULL);
	Assert(FALSE && "No need to call this virtual method");
	}

void
IEvent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	Assert(pXmlNodeExtraData != NULL);
	Assert(pbinXcpStanzaReply != NULL);
	}

void
IEvent::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	Assert(pXmlNodeExtraData != NULL);
	Assert(pbinXcpStanzaReply != NULL);
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "IEvent::XcpExtraDataArrived($U): ^N", EGetEventClass(), pXmlNodeExtraData);	// The event must implement this virtual method
	}

//	ChatLogUpdateTextBlock(), virtual
void
IEvent::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	Assert(poCursorTextBlock != NULL);
	}

void
IEvent::HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText)
	{
	Assert(pszActionOfHyperlink != NULL);
	Assert(pstrTooltipText != NULL);
	}

void
IEvent::HyperlinkClicked(PSZUC pszActionOfHyperlink, OCursor * poCursorTextBlock)
	{
	Assert(pszActionOfHyperlink != NULL);
	Assert(poCursorTextBlock != NULL);
	}

//	This virtual method is called to get the text to display a balloon in the System Tray to notify the user when a new event arrives.
//	Return a pointer to the text to display in the System Tray.
//	Return NULL if the event does not display any text in the System Tray.
PSZUC
IEvent::PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const
	{
	Assert(pstrScratchBuffer != NULL);
	return NULL;
	}

/*
//	Always return the pointer of the contact who sent the event
TContact *
IEvent::PGetContactSender()
	{
	return m_pContact_NZ;
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "eEventClass $U, tsEventID $t has no contact\n", EGetEventClass(), m_tsEventID);
	return NULL;
	}
*/

//	Return TRUE if the event belong to a group, otherwise to a contact.
BOOL
IEvent::Event_FIsEventBelongsToGroup() const
	{
	Assert(m_pVaultParent_NZ != NULL);
	return (m_pVaultParent_NZ->m_pParent->EGetRuntimeClass() == RTI(TGroup));
	}

BOOL
IEvent::Event_FIsEventTypeSent() const
	{
	return ((EGetEventClass() & eEventClass_kfReceivedByRemoteClient) == 0);
	}

BOOL
IEvent::Event_FIsEventTypeReceived() const
	{
	return ((EGetEventClass() & eEventClass_kfReceivedByRemoteClient) != 0);
	}

void
IEvent::Event_SetCompleted(QTextEdit * pwEditChatLog)
	{
	Endorse(pwEditChatLog == NULL);		// Don't update the event in the Chat Log, typically because there is no Chat Log.  The events may be loaded in memory, however not displayed in any Chat Log.
	if (m_tsOther > d_tsOther_kmReserved)
		{
		MessageLog_AppendTextFormatSev(eSeverityNoise, "\t\t\t Event_SetCompleted() - m_tsOther $t remains unchanged for EventID $t.\n", m_tsOther, m_tsEventID);
		return;	// The task already completed, so keep the first timestamp.  This is likely to be a duplicate message.
		}
	m_tsOther = Timestamp_GetCurrentDateTime();
	m_pVaultParent_NZ->SetModified();		// This line is important so the modified event is always saved to disk (otherwise an event may be updated, however not serialized because ITreeItemChatLogEvents will never know the event was updated)
	Assert(Event_FHasCompleted());
	ChatLog_UpdateEventWithinWidget(pwEditChatLog);
	}

void
IEvent::Event_SetCompletedAndUpdateWidgetWithinChatLog()
	{
	Assert(m_pVaultParent_NZ != NULL);
	Event_SetCompleted(m_pVaultParent_NZ->m_pParent->ChatLog_PwGet_YZ());
	}

//	Return TRUE if the event completed.
//	Return FALSE if the event requires a task in order to complete.
BOOL
IEvent::Event_FHasCompleted() const
	{
	return (m_tsOther > d_tsOther_kmReserved);
	}

//	Return the text block matching the event.
//	In the rare case where there is no text block matching the event, this method will return an invalid text block,
//	however not a NULL text block which will crash the application if used by a QTextCursor.
QTextBlock
IEvent::ChatLog_GetTextBlockRelatedToDocument(QTextDocument * poDocument) const
	{
	Assert(poDocument != NULL);
	QTextBlock oTextBlock = poDocument->firstBlock();
	while (oTextBlock.isValid())
		{
		OTextBlockUserDataEvent * pUserData = (OTextBlockUserDataEvent *)oTextBlock.userData();
//		Assert(pUserData != NULL);
		if (pUserData != NULL && pUserData->m_pEvent == this)
			break;
		oTextBlock = oTextBlock.next();
		}
	if (!oTextBlock.isValid())
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Unable to find text block matching event of class $U [i=$t, o=$t]\n", EGetEventClass(), m_tsEventID, m_tsOther);
	return oTextBlock;
	}

QTextBlock
IEvent::ChatLog_GetTextBlockRelatedToWidget(QTextEdit * pwEditChatLog) const
	{
	Assert(pwEditChatLog != NULL);
	return ChatLog_GetTextBlockRelatedToDocument(pwEditChatLog->document());
	}

void
IEvent::ChatLog_UpdateEventWithinWidget(QTextEdit * pwEditChatLog)
	{
	if (pwEditChatLog != NULL)
		{
		OCursorSelectBlock oCursor(ChatLog_GetTextBlockRelatedToWidget(pwEditChatLog));
		ChatLogUpdateTextBlock(INOUT &oCursor);
		}
	}

void
IEvent::Event_UpdateWidgetWithinParentChatLog()
	{
	ChatLog_UpdateEventWithinWidget(m_pVaultParent_NZ->m_pParent->ChatLog_PwGet_YZ());
	}

const QBrush &
IEvent::ChatLog_OGetBrushForEvent() const
	{
	Assert(m_pVaultParent_NZ != NULL);
	return m_pVaultParent_NZ->m_pParent->ChatLog_OGetBrushForNewMessageReceived();
	}

//	Return the nick name of the contact related to event
PSZUC
IEvent::ChatLog_PszGetNickNameOfContact() const
	{
	Assert(m_pVaultParent_NZ != NULL);
	if (m_pContactGroupSender_YZ != NULL)
		return m_pContactGroupSender_YZ->ChatLog_PszGetNickname();
	return m_pVaultParent_NZ->m_pParent->ChatLog_PszGetNickname();	// This may return the nickname of the group (although this is somewhat an error, it will not cause any harm)
	}

ITreeItemChatLogEvents *
IEvent::PGetContactOrGroup_NZ() const
	{
	Assert(m_pVaultParent_NZ != NULL);
	return m_pVaultParent_NZ->m_pParent;
	}

TAccountXmpp *
IEvent::PGetAccount_NZ() const
	{
	Assert(m_pVaultParent_NZ != NULL);
	return m_pVaultParent_NZ->m_pParent->m_pAccount;
	}

//	Return the socket related to the event in the Chat Log.
//	This method may return NULL if the account never attempted to connect to the server.
//	This method should be used with care to make sure the context ensures the socket is valid.
CSocketXmpp *
IEvent::PGetSocket_YZ() const
	{
	return PGetAccount_NZ()->PGetSocket_YZ();
	}

void
IEvent::Socket_WriteXmlIqSet_VE_Gso(TContact * pContact, PSZAC pszFmtTemplate, ...)
	{
	Assert(pContact != NULL);
	Assert(pszFmtTemplate != NULL);
	CSocketXmpp * pSocket = pContact->Xmpp_PGetSocketOnlyIfContactIsUnableToCommunicateViaXcp();
	Report(pSocket != NULL);
	if (pSocket != NULL)
		{
		g_strScratchBufferSocket.BinInitFromTextSzv_VE("<iq type='set' to='^J' id='$t'>", pContact, m_tsEventID);
		va_list vlArgs;
		va_start(OUT vlArgs, pszFmtTemplate);
		g_strScratchBufferSocket.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
		g_strScratchBufferSocket.BinAppendBinaryData("</iq>", 5);
		pSocket->Socket_WriteBin(g_strScratchBufferSocket);
		}
	}

void
IEvent::Socket_WriteXmlIqReplyAcknowledge()
	{
	CSocketXmpp * pSocket = PGetSocket_YZ();
	if (pSocket != NULL)
		pSocket->Socket_WriteXmlIqReplyAcknowledge();
	}

/*
void
IEvent::Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...)
	{
	Assert(PGetSocket_YZ() != NULL);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	PGetSocket_YZ()->Socket_WriteXmlFormatted_VL(pszFmtTemplate, vlArgs);
	}
*/
/*
void
IEvent::Socket_WriteXmlIqError_VE_Gso(PSZAC pszErrorType, PSZUC pszErrorID, PSZAC pszFmtTemplate, ...)
	{
	CSocketXmpp * pSocket = PGetSocket_YZ();
	if (pSocket == NULL)
		{
		MessageLog_AppendTextFormatSev(eSeverityWarning, "IEvent::Socket_WriteXmlIqError_VE_Gso(type='$s', id='$', '$s') - no socket available!\n", pszErrorType, pszErrorID, pszFmtTemplate);
		return;	// Do not attempt to send an error code if the socket object was not yet create.  This is the case when attempting to cancel an event (such as declining a file offer) from a previous session where there was never any attempt to connect with that account (a rare case, however nor worth a crash).
		}
	g_strScratchBufferSocket.Empty();
	g_strScratchBufferSocket.BinAppendTextSzv_VE("<iq type='error' id='^s' from='^J' to='^J'><error type='$s'>", pszErrorID, mu_parentowner.pTreeItem->m_pAccount, mu_parentowner.pTreeItem, pszErrorType);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferSocket.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	g_strScratchBufferSocket.BinAppendBinaryData("</error></iq>", 13);
	pSocket->Socket_WriteBin(g_strScratchBufferSocket);
	}
*/

const char c_szHtmlMessageDelivered[] = "<img src=':/ico/Delivered' style='float:right'/>";
const char c_szHtmlTemplateNickname[] = "<b>{sH}</b>: ";

void
IEvent::_BinHtmlInitWithTime(OUT CBin * pbinTextHtml) const
	{
	Assert(pbinTextHtml != NULL);
	Assert(m_pVaultParent_NZ != NULL);
	ITreeItemChatLog * pTreeItemNickname;		// This pointer should be a TContact, however if there is a 'bug' where a group message has m_pContactGroupSender_YZ == NULL, then this will point to a group (which is no big deal)
	const QDateTime dtlMessage = QDateTime::fromMSecsSinceEpoch(m_tsEventID).toLocalTime();
	const QString sTime = dtlMessage.toString("hh:mm");
	const QString sDateTime = dtlMessage.toString(Qt::SystemLocaleLongDate); // DefaultLocaleLongDate);
	(void)pbinTextHtml->PvSizeAlloc(300);	// Empty the binary object and also pre-allocate 300 bytes of memory to avoid unnecessary memory re-allocations
	TIMESTAMP_DELTA dts = m_tsOther - m_tsEventID;
	const EEventClass eEventClass = EGetEventClass();
	if (eEventClass & eEventClass_kfReceivedByRemoteClient)
		{
		// The event was received (typically a message typed by someone else)
		if (dts < -10 * d_ts_cMinutes && dts > -15 * d_ts_cDays)
			pbinTextHtml->BinAppendTextSzv_VE("[$T] ", dts);
		#if 0
		pbinTextHtml->BinAppendTextSzv_VE("<span title='Message was sent {T_} before you received it'>[{T-}] </span>", dts);
		#endif
		pTreeItemNickname = (m_pContactGroupSender_YZ != NULL) ? m_pContactGroupSender_YZ : m_pVaultParent_NZ->m_pParent;	// Use the name of the group sender (if any, otherwise the name of the contact)
		}
	pbinTextHtml->BinAppendTextSzv_VE("<span title='^Q'>[^Q] </span>", &sDateTime, &sTime);
	if (m_uFlagsEvent & FE_kfEventProtocolError)
		{
		pbinTextHtml->BinAppendStringWithoutNullTerminator(" <img src=':/ico/Error' title='XCP Protocol Error: One of the contact has an old version of Cambrian and cannot process this event because its type is unknown'/> ");
		}
	if ((eEventClass & eEventClass_kfReceivedByRemoteClient) == 0)
		{
		// The message was sent by the user
		if (dts > 5 * d_ts_cMinutes)
			pbinTextHtml->BinAppendTextSzv_VE("[$T] ", dts);
		if (Event_FHasCompleted())
			{
			if ((m_uFlagsEvent & FE_kfEventDeliveryConfirmed) == 0)
				{
				m_uFlagsEvent |= FE_kfEventDeliveryConfirmed;
				pbinTextHtml->BinAppendBinaryData(c_szHtmlMessageDelivered, sizeof(c_szHtmlMessageDelivered) - 1);
				}
			}
		pTreeItemNickname = PGetAccount_NZ();	// Use the name of the user
		}
	#ifdef DEBUG_DISPLAY_TIMESTAMPS
	pbinTextHtml->BinAppendTextSzv_VE("<code>[i=<b>$t</b> o=<b>$t</b>] </code>", m_tsEventID, m_tsOther);
	#endif
	if (m_uFlagsEvent & FE_kfEventOutOfSync)
		pbinTextHtml->BinAppendStringWithoutNullTerminator(" <img src=':/ico/OutOfSync' title='Out of Sync' /> ");
	pbinTextHtml->BinAppendTextSzv_VE(c_szHtmlTemplateNickname, pTreeItemNickname->ChatLog_PszGetNickname());
	} // _BinHtmlInitWithTime()


/*
void
IEvent::_BinHtmlInitWithTimeAndNickname(OUT CBin * pbinTextHtml, PSZUC pszNickname) const
	{
	_BinHtmlInitWithTime(OUT pbinTextHtml);
	pbinTextHtml->BinAppendTextSzv_VE(c_szHtmlTemplateNickname, pszNickname);
	}

void
IEvent::_BinHtmlInitWithTimeAndNickname(OUT CBin * pbinTextHtml, ITreeItemChatLog * pTreeItemNickname) const
	{
	_BinHtmlInitWithTime(OUT pbinTextHtml);
	if (pTreeItemNickname != NULL)
		pbinTextHtml->BinAppendTextSzv_VE(c_szHtmlTemplateNickname, pTreeItemNickname->ChatLog_PszGetNickname());
	}
*/
/*
void
IEvent::_BinHtmlInitWithTimeAndNickname(OUT CBin * pbinTextHtml, PSZUC pszNickname) const
	{
	Assert(pbinTextHtml != NULL);
	const QDateTime dtlMessage = QDateTime::fromMSecsSinceEpoch(m_tsEventID).toLocalTime();
	const QString sTime = dtlMessage.toString("hh:mm");
	const QString sDateTime = dtlMessage.toString(Qt::SystemLocaleLongDate); // DefaultLocaleLongDate);
	(void)pbinTextHtml->PvSizeAlloc(300);	// Empty the binary object and also pre-allocate 300 bytes of memory to avoid unnecessary memory re-allocations
	TIMESTAMP_DELTA dts = m_tsOther - m_tsEventID;
	if (dts < 0 && dts > -15 * d_ts_cDays)
		pbinTextHtml->BinAppendTextSzv_VE("[$T] ", dts);
	pbinTextHtml->BinAppendTextSzv_VE("<span title='^Q'>[^Q] </span>", &sDateTime, &sTime);
	if (dts > 0) // d_ts_cMinutes * 5)
		pbinTextHtml->BinAppendTextSzv_VE("[$T] ", dts);
	pbinTextHtml->BinAppendTextSzv_VE(c_szHtmlTemplateNickname, pszNickname);
	}
*/
/*
void
IEvent::_BinHtmlInitWithTimeAsSender(OUT CBin * pbinTextHtml) const
	{
	_BinHtmlInitWithTimeAndNickname(OUT pbinTextHtml, mu_parentowner.pTreeItem->m_pAccount);
	}

void
IEvent::_BinHtmlInitWithTimeAsReceiver(OUT CBin * pbinTextHtml) const
	{
	_BinHtmlInitWithTimeAndNickname(OUT pbinTextHtml, mu_parentowner.pTreeItem);
	}
*/

//	Typical actions for an event hyperlink
#define d_chActionForEvent_Hyperlink			'h'		// "c:h"		// A typical hyperlink to the web
#define d_szActionForEvent_Hyperlink			"h"		// "c:h"
#define d_chActionForEvent_HyperlinkFile		'f'		// "c:f"		// An hyperlink to a local file, somewhat the equivalent of "file://c:/folder/file.txt"
#define d_szActionForEvent_HyperlinkFile		"f"		// "c:f"

#define d_chActionForEvent_ButtonSave			's'		// "c:s"
#define d_chActionForEvent_ButtonSaveAs			'a'		// "c:a"
#define d_chActionForEvent_ButtonOpen			'o'		// "c:o"
#define d_chActionForEvent_ButtonCancel			'c'		// "c:c"
#define d_chActionForEvent_ButtonDecline		'd'		// "c:d"

void
IEvent::_BinHtmlAppendHyperlinkToLocalFile(INOUT CBin * pbinTextHtml, PSZUC pszFileName, BOOL fDisabled) const
	{
	Assert(pbinTextHtml != NULL);
	Report(pszFileName != NULL && "Empty file name");
	PSZUC pszFileNameOnly = PszGetFileNameOnlyNZ(pszFileName);
	if (pszFileNameOnly != pszFileName)
		{
		// We have a full path, so it is appropriate to create an hyperlink
		PSZAC pszClass = fDisabled ? "class="d_szClassForChatLog_HyperlinkDisabled : NULL;
		pbinTextHtml->BinAppendTextSzv_VE("<a $s href='"d_szSchemeCambrian":{t_},"d_szActionForEvent_HyperlinkFile"'>^s</a>", pszClass, m_tsEventID, pszFileNameOnly);
		}
	else
		{
		// We a relative path, so display the filename in bold
		pbinTextHtml->BinAppendTextSzv_VE("<b>$s</b>", pszFileNameOnly);
		}
	}

void
IEvent::_BinHtmlAppendHyperlinkAction(INOUT CBin * pbinTextHtml, CHS chActionOfHyperlink) const
	{
	Assert(pbinTextHtml != NULL);
	PSZAC pszButtonName;
	switch (chActionOfHyperlink)
		{
	default:
		Assert(FALSE && "Unknown Task Action");
		return;
	case d_chActionForEvent_ButtonCancel:
		pszButtonName = "Cancel";
		break;
	case d_chActionForEvent_ButtonDecline:
		pszButtonName = "Decline";
		break;
	case d_chActionForEvent_ButtonSave:
		pszButtonName = "Save";
		break;
	case d_chActionForEvent_ButtonSaveAs:
		pszButtonName = "Save As";
		break;
	case d_chActionForEvent_ButtonOpen:
		pszButtonName = "Open";
		break;
		} // switch
	pbinTextHtml->BinAppendTextSzv_VE(" " _nbsp " <a class="d_szClassForChatLog_ButtonHtml" href='"d_szSchemeCambrian":{t_},$b'>[" _nbsp2 "$s" _nbsp2 "]</a>", m_tsEventID, chActionOfHyperlink, pszButtonName);
	} // _BinHtmlAppendHyperlinkAction()

void
ITreeItemChatLogEvents::ChatLog_EventEditMessageSent(CEventMessageTextSent * pEventMessageSent)
	{
	Assert(m_pawLayoutChatLog != NULL);
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->m_pwChatInput->EditEventText(pEventMessageSent);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
IEventMessageText::IEventMessageText(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_uFlagsMessage = FM_kzMessagePlainText;
	}

//	IEventMessageText::IEvent::XmlSerializeCore()
void
IEventMessageText::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{	
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAttribute_strText, m_strMessageText);
	pbinXmlAttributes->BinAppendXmlAttributeUInt(d_chAttribute_uFlags, m_uFlagsMessage);
	pbinXmlAttributes->XmppWriteStanzaToSocketOnlyIfContactIsUnableToCommunicateViaXcp_VE("<message to='^J' id='$t'><body>^S</body><request xmlns='urn:xmpp:receipts'/></message>", pbinXmlAttributes->m_pContact, m_tsEventID, &m_strMessageText);
	}

//	IEventMessageText::IEvent::XmlUnserializeCore()
void
IEventMessageText::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAttribute_strText, OUT_F_UNCH &m_strMessageText);
	pXmlNodeElement->UpdateAttributeValueUInt(d_chAttribute_uFlags, OUT_F_UNCH &m_uFlagsMessage);
	}

//	Need to remove the parameter eEventClass of this method
void
IEventMessageText::_BinHtmlInitWithTimeAndMessage(OUT CBin * pbinTextHtml) CONST_VIRTUAL
	{
	Assert(pbinTextHtml != NULL);
	_BinHtmlInitWithTime(OUT pbinTextHtml);
	if (m_uFlagsMessage & FM_kfMessageUpdated)
		pbinTextHtml->BinAppendStringWithoutNullTerminator(" <img src=':/ico/Pencil' title='Edited' /> ");
	if ((m_uFlagsMessage & FM_kfMessageHtml) ==  0)
		pbinTextHtml->BinAppendHtmlTextWithAutomaticHyperlinks(m_strMessageText);
	else
		pbinTextHtml->BinAppendCStr(m_strMessageText);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventMessageXmlRawSent::CEventMessageXmlRawSent(const CStr & strMessage) : IEventMessageText(NULL)
	{
	m_strMessageText = strMessage;
	}

void
CEventMessageXmlRawSent::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinXmlInitStanzaWithXmlRaw(m_pVaultParent_NZ->m_pParent, m_strMessageText);
	pbinXmlAttributes->XmppWriteStanzaToSocket();
	}

//	CEventMessageXmlRawSent::IEvent::ChatLogUpdateTextBlock()
void
CEventMessageXmlRawSent::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTimeAndMessage(OUT &g_strScratchBufferStatusBar);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(0xE8CFD8));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventMessageTextSent::CEventMessageTextSent(const TIMESTAMP * ptsEventID) : IEventMessageText(ptsEventID)
	{
	}

CEventMessageTextSent::CEventMessageTextSent(const CStr & strMessageText) : IEventMessageText(NULL)
	{
	m_strMessageText = strMessageText;
	}

CEventMessageTextSent::~CEventMessageTextSent()
	{
	}

EEventClass
CEventMessageTextSent::EGetEventClassForXCP(const TContact *) const
	{
	return CEventMessageTextReceived::c_eEventClass;
	}

EEventClass
CEventFileSent::EGetEventClassForXCP(const TContact *) const
	{
	return CEventFileReceived::c_eEventClass;
	}

//	CEventMessageTextSent::IEvent::ChatLogUpdateTextBlock()
void
CEventMessageTextSent::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTimeAndMessage(OUT &g_strScratchBufferStatusBar);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(d_coWhite));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventMessageTextReceived::CEventMessageTextReceived(const TIMESTAMP * ptsEventID) : IEventMessageText(ptsEventID)
	{
	}

//	CEventMessageTextReceived::IEvent::ChatLogUpdateTextBlock()
void
CEventMessageTextReceived::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTimeAndMessage(OUT &g_strScratchBufferStatusBar);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, ChatLog_OGetBrushForEvent());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
IEventFile::IEventFile(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_cblFileSize = 0;
	m_cblDataTransferred = 0;
	m_paFile = NULL;
	}

IEventFile::~IEventFile()
	{
	delete m_paFile;
	}

CFile *
IEventFile::_PFileOpenReadOnly_NZ()
	{
	if (m_paFile == NULL)
		{
		m_paFile = new CFile(m_strFileName);
		if (m_paFile->open(QIODevice::ReadOnly))
			m_cblFileSize = m_paFile->size();
		else
			m_cblFileSize = d_IEventFile_cblFileSize_FileNotFound;
		}
	return m_paFile;
	}

CFile *
IEventFile::_PFileOpenWriteOnly_NZ()
	{
	if (m_paFile == NULL)
		{
		m_paFile = new CFile(m_strFileName);
		if (m_paFile->open(QIODevice::WriteOnly))
			{
			QFileInfo oFileInfo(*m_paFile);
			m_strFileName = oFileInfo.absoluteFilePath();	// Get the full path
			MessageLog_AppendTextFormatSev(eSeverityComment, "IEventFile::_PFileOpenWriteOnly_NZ($S) for tsEventID = $t\n", &m_strFileName, m_tsEventID);
			m_cblDataTransferred = 0;
			}
		else
			m_cblDataTransferred = d_IEventFile_cblDataTransferred_WriteError;
		}
	return m_paFile;
	}

void
IEventFile::_FileClose()
	{
	if (m_paFile != NULL)
		{
		m_paFile->close();
		m_paFile = NULL;
		}
	}

#define d_chIEventFile_Attribute_strFileName		'n'
#define d_chIEventFile_Attribute_cblFileSize		's'
#define d_chIEventFile_Attribute_cblDataTransferred	'd'

//	IEventFile::IEvent::XmlSerializeCore()
void
IEventFile::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	PSZUC pszFileNameOnly = m_strFileName.PathFile_PszGetFileNameOnly_NZ();
	const BOOL fSerializingEventToDisk = pbinXmlAttributes->FSerializingEventToDisk();
	pbinXmlAttributes->BinAppendXmlAttributeText(d_chIEventFile_Attribute_strFileName, fSerializingEventToDisk ? m_strFileName.PszuGetDataNZ() : pszFileNameOnly);	// When serializing for a contact (via XCP), only send the filename (sending the full path is a violation of privacy)
	pbinXmlAttributes->BinAppendXmlAttributeL64(d_chIEventFile_Attribute_cblFileSize, m_cblFileSize);
	if (fSerializingEventToDisk)
		pbinXmlAttributes->BinAppendXmlAttributeL64(d_chIEventFile_Attribute_cblDataTransferred, m_cblDataTransferred);	// The number of bytes transferred is serialized only to disk, never through XCP

	// Create a file offer for the contact unable to cummunicate via XCP
	pbinXmlAttributes->XmppWriteStanzaToSocketOnlyIfContactIsUnableToCommunicateViaXcp_VE(
			"<iq id='$t' type='get' to='^J'>"
			"<si id='$t' profile='^*ft' ^:si><file ^:ft name='^s' size='$l'/>"
				"<feature ^:fn><x ^:xd type='form'><field var='stream-method' type='list-single'><option><value>^*ib</value></option></field></x></feature>"
			"</si></iq>", m_tsEventID, pbinXmlAttributes->m_pContact, m_tsEventID, pszFileNameOnly, m_cblFileSize);
	}

//	IEventFile::IEvent::XmlUnserializeCore()
void
IEventFile::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueCStr(d_chIEventFile_Attribute_strFileName, OUT_F_UNCH &m_strFileName);
	pXmlNodeElement->UpdateAttributeValueL64(d_chIEventFile_Attribute_cblFileSize, OUT_F_UNCH &m_cblFileSize);
	pXmlNodeElement->UpdateAttributeValueL64(d_chIEventFile_Attribute_cblDataTransferred, OUT_F_UNCH &m_cblDataTransferred);
	}

//	IEventFile::IEvent::HyperlinkClicked()
void
IEventFile::HyperlinkClicked(PSZUC pszActionOfHyperlink, OCursor * poCursorTextBlock)
	{
	switch (pszActionOfHyperlink[0])
		{
	case d_chActionForEvent_HyperlinkFile:
	case d_chActionForEvent_ButtonOpen:
		FileShowInExplorer(m_strFileName, pszActionOfHyperlink[0] == d_chActionForEvent_ButtonOpen);
		return;
	case d_chActionForEvent_ButtonCancel:
	case d_chActionForEvent_ButtonDecline:
		_FileTransferCancelledByLocalUser(poCursorTextBlock);
		return;
		} // switch
	} // HyperlinkClicked()

//	IEventFile::IEvent::HyperlinkGetTooltipText()
//
//	This virtual method shows the tooltips for both CEventFileSent and CEventFileReceived.
//	Some of the tooltips are mostly for receiving a file, however it is convenient to have them here.
void
IEventFile::HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText)
	{
	switch (pszActionOfHyperlink[0])
		{
	case d_chActionForEvent_HyperlinkFile:
		pstrTooltipText->Format("Show file in Explorer:\n$S", &m_strFileName);
		return;
	case d_chActionForEvent_ButtonOpen:
		pstrTooltipText->Format("Open file: $S", &m_strFileName);
		return;
	case d_chActionForEvent_ButtonCancel:
		pstrTooltipText->Format("Cancel the transfer of file $s", m_strFileName.PathFile_PszGetFileNameOnly_NZ());
		return;
	case d_chActionForEvent_ButtonDecline:
		pstrTooltipText->Format("Decline the file offer from $s", ChatLog_PszGetNickNameOfContact());
		return;
	case d_chActionForEvent_ButtonSave:
		pstrTooltipText->Format("Save file in the folder:\n$s", m_pVaultParent_NZ->m_pParent->ChatLog_PszGetPathFolderDownload());
		return;
	case d_chActionForEvent_ButtonSaveAs:
		pstrTooltipText->Format("Save file in a designated folder");
		return;
		} // switch
	} // HyperlinkGetTooltipText()

//	Format an HTML text message for the Chat Log.
//
//	SUPPORTED FORMATS
//		@L		Insert an hyperlink to file m_strFileName
//		@C		Insert the name of the contact
//		@z		Insert the size in bytes/KiB/MiB of the file
//		@Z		Same as @Z except the size is displayed in parenthesis () if non-zero
//		@%		Display the percentage of the file transfer
//		@s		Add the button Save
//		@a		Add the button Save As
//		@o		Add the button Open
//		@c		Add the button Cancel
//		@d		Add the button Decline
//
void
IEventFile::_BinAppendHtmlForEvent(INOUT CBin * pbinTextHtml, PSZAC pszTextHtmlTemplate) const
	{
	Assert(pbinTextHtml != NULL);
	Assert(pszTextHtmlTemplate != NULL);
	while (TRUE)
		{
		UINT ch = *pszTextHtmlTemplate++;
		switch (ch)
			{
		default:
			pbinTextHtml->BinAppendByte(ch);
			break;
		case '\0':
			return;
		case '@':
			UINT chFormat = *pszTextHtmlTemplate++;
			switch (chFormat)
				{
			case 'C':
				pbinTextHtml->BinAppendXmlTextU(ChatLog_PszGetNickNameOfContact());
				break;
			case 'F':
				pbinTextHtml->BinAppendXmlTextStr(m_strFileName);
				break;
			case 'L':
				_BinHtmlAppendHyperlinkToLocalFile(INOUT pbinTextHtml, m_strFileName);
				break;
			case 'Z':
				if (m_cblFileSize <= 0)
					break;	// Don't display empty or negative values
				pbinTextHtml->BinAppendByte('(');
				// Fall Through //
			case 'z':
				pbinTextHtml->BinAppendTextBytesKiB(m_cblFileSize);
				if (chFormat == 'Z')
					pbinTextHtml->BinAppendByte(')');
				break;
			case '%':
				pbinTextHtml->BinAppendTextBytesKiBPercent(m_cblDataTransferred, m_cblFileSize);
				break;
			case d_chActionForEvent_ButtonSave:
			case d_chActionForEvent_ButtonSaveAs:
			case d_chActionForEvent_ButtonOpen:
			case d_chActionForEvent_ButtonCancel:
			case d_chActionForEvent_ButtonDecline:
				_BinHtmlAppendHyperlinkAction(INOUT pbinTextHtml, chFormat);
				break;
			default:
				Assert(FALSE && "Unknown format character");
				} // switch
			} // switch
		} // while
	Assert(FALSE && "Unreachable code!");
	} // _BinAppendHtmlForEvent()

void
IEventFile::_FileTransferCancelledByLocalUser(INOUT OCursor * poCursorTextBlock)
	{
	m_cblDataTransferred = d_IEventFile_cblDataTransferred_CancelledByLocalUser;
	m_pVaultParent_NZ->SetModified();
	ChatLogUpdateTextBlock(poCursorTextBlock);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventFileSent::CEventFileSent(const TIMESTAMP * ptsEventID) : IEventFile(ptsEventID)
	{
	}

CEventFileSent::CEventFileSent(PSZUC pszFileToSend) : IEventFile(NULL)
	{
	m_strFileName = pszFileToSend;
	(void)_PFileOpenReadOnly_NZ();	// Open the file so we can update m_cblFileSize
	}

CEventFileSent::~CEventFileSent()
	{
	}

//	CEventFileSent::IEvent::ChatLogUpdateTextBlock()
void
CEventFileSent::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CEventFileSent::ChatLogUpdateTextBlock() - tsEventID=$t\n", m_tsEventID);
	PSZAC pszTextHtmlTemplate;
	if (m_cblDataTransferred <= 0)
		{
		// So far, no data was transferred
		switch (m_cblDataTransferred)
			{
		default:
			if (m_cblFileSize >= 0)
				{
				pszTextHtmlTemplate = "Offering file to <b>@C</b>: @L @Z @c @o";
				}
			else
				{
				Assert(m_cblFileSize == d_IEventFile_cblFileSize_FileNotFound);
				pszTextHtmlTemplate = "Offering file to <b>@C</b>: @L @Z (file not found)";
				}
			break;
		case d_IEventFile_cblDataTransferred_CancelledByLocalUser:
			pszTextHtmlTemplate = "File offer revoked by you: @L @Z @o";
			break;
		case d_IEventFile_cblDataTransferred_CancelledByRemoteContact:
			pszTextHtmlTemplate = "File offer declined by <b>@C</b>: @L @Z @o";
			}
		}
	else
		{
		Assert(m_cblDataTransferred <= m_cblFileSize);
		pszTextHtmlTemplate = (m_cblDataTransferred < m_cblFileSize) ? "Sending file @L: @% sent... @o @c" : "File sent to <b>@C</b>! @L @Z @o";
		}
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	_BinAppendHtmlForEvent(INOUT &g_strScratchBufferStatusBar, pszTextHtmlTemplate);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, c_brushFileTransfer);
	} // ChatLogUpdateTextBlock()

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventFileReceived::CEventFileReceived(const TIMESTAMP * ptsEventID) : IEventFile(ptsEventID)
	{
	}
/*
CEventFileReceived::CEventFileReceived(TContact * pContactSendingFile, const CXmlNode * pXmlNodeStreamInitiation) : IEventFile(pContactSendingFile, NULL)
	{
	Assert(pXmlNodeStreamInitiation != NULL);
	CXmlNode * pXmlNodeFile = pXmlNodeStreamInitiation->PFindElement(c_sza_file);
	if (pXmlNodeFile != NULL)
		{
		// Remember the suggested file name to download (this is important because there will be no other opportunity to get the filename)
		m_strFileName = pXmlNodeFile->PszuFindAttributeValue("name");
		m_cblFileSize = pXmlNodeFile->LFindAttributeValueDecimal_ZZR(c_sza_size);
		}
	// Prepare a reply to accept the file to download.  We keep the reply into a string and wait for the user to confirm (accept) the file before sending the reply to the socket.
	m_strxStanzaID = pXmlNodeStreamInitiation->m_pParent->PszFindAttributeValueId_NZ();
	m_strxSID = pXmlNodeStreamInitiation->PszFindAttributeValueId_NZ();
	//pContactSendingFile->TreeItemContact_NewMessageArrived(IN g_strScratchBufferStatusBar.Format("File to download: $S", &m_strFileName));
	}
*/

CEventFileReceived::~CEventFileReceived()
	{
	}

/*
//	CEventFileReceived::IEvent::XmlSerializeCore()
void
CEventFileReceived::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	IEventFile::XmlSerializeCore(IOUT pbinXmlAttributes, pContactToSerializeFor);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAttribute_strxStanzaID, m_strxStanzaID);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAttribute_strxFileSID, m_strxSID);
	}

//	CEventFileReceived::IEvent::XmlUnserializeCore()
void
CEventFileReceived::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	IEventFile::XmlUnserializeCore(pXmlNodeElement);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAttribute_strxStanzaID, OUT_F_UNCH &m_strxStanzaID);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAttribute_strxFileSID, OUT_F_UNCH &m_strxSID);
	if (m_cblDataTransferred < m_cblFileSize)
		{
		m_cblDataTransferred = 0; // d_cblDataTransferred_FileOffered;;
		}
	}
*/

//	CEventFileReceived::IEvent::ChatLogUpdateTextBlock()
void
CEventFileReceived::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	PSZAC pszTextHtmlTemplate;
	if (m_cblDataTransferred <= 0)
		{
		// So far, no data was transferred
		switch (m_cblDataTransferred)
			{
		default:
			pszTextHtmlTemplate = "File to download: @L @Z @s @a @d";	// Display to the user the opportunity to download the file, or decline the offer
			break;
		case d_IEventFile_cblDataTransferred_WriteError:
			// If unable to write to disk, then do not offer the 'save' option
			pszTextHtmlTemplate = "File to download: @L @Z <b>(write error)</b> @a @d";
			break;
		case d_IEventFile_cblDataTransferred_CancelledByLocalUser:
			pszTextHtmlTemplate = "You declined to download file @L @Z";
			break;
		case d_IEventFile_cblDataTransferred_CancelledByRemoteContact:
			pszTextHtmlTemplate = "File offer revoked by <b>@C</b>: @L @Z";
			} // switch
		}
	else
		{
		Assert(m_cblDataTransferred <= m_cblFileSize);
		pszTextHtmlTemplate = (m_cblDataTransferred < m_cblFileSize) ? "Downloading file @L: @% received... @c" : "Download complete! @L @Z @o";
		}
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	_BinAppendHtmlForEvent(INOUT &g_strScratchBufferStatusBar, pszTextHtmlTemplate);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, c_brushFileTransfer);
	} // ChatLogUpdateTextBlock()


// 	m_strXmlIqResultInitFileDownload = PGetSocket_YZ()->ScratchBuffer_WriteXmlIqResult_Gsb(d_szuXmlAlreadyEncoded "<si id='$S' profile='^*ft' ^:si><feature ^:fn><x ^:xd type='submit'><field var='stream-method'><value>^*ib</value></field></x></feature></si>", &m_strSessionIdentifier);
//	CEventFileReceived::IEvent::HyperlinkClicked()
void
CEventFileReceived::HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock)
	{
	CString sPath = m_strFileName;
	switch (pszActionOfHyperlink[0])
		{
	case d_chActionForEvent_ButtonCancel:
	case d_chActionForEvent_ButtonDecline:
		//Socket_WriteXmlIqError_VE_Gso(c_sza_cancel, m_strxStanzaID, "<forbidden ^:xs/><text ^:xs>Offer Declined</text>");
		break;
	case d_chActionForEvent_ButtonSaveAs:
		// Prompt the user where to save the file
		// sPath = m_pContact->ChatLog_PszGetPathFolderDownload();
		sPath = QFileDialog::getSaveFileName(g_pwMainWindow, "Save File As", IN sPath);
		if (sPath.isEmpty())
			return;
		m_strFileName = sPath;	// // We need to update the file name so the GUI can reflect the new value
		goto SaveToFile;
	case d_chActionForEvent_ButtonSave:
		if ((QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) == 0)
			{
			if (QFile::exists(sPath))
				{
				EAnswer eAnswer = EMessageBoxQuestion("The file '$Q' already exists.\n\nDo you want to overwrite it?", &sPath);
				if (eAnswer != eAnswerYes)
					return;
				}
			}
		SaveToFile:
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventFileReceived::HyperlinkClicked() - Saving event $t to file $S\n", m_tsOther, &m_strFileName);
		XcpRequesExtraData();
		return;
		} // switch
	IEventFile::HyperlinkClicked(pszActionOfHyperlink, INOUT poCursorTextBlock);
	} // HyperlinkClicked()


///////////////////////////////////////////////////////////////////////////////////////////////////
void
CEventMessageTextSent::MessageDeliveredConfirmed()
	{
	Event_SetCompleted(NULL);
	//TimestampOther_UpdateAsEventCompletedNow();
	/*
	if (mu_task.paTask == NULL)
		return;		// This happens when receiving twice a confirmation receipt for the same event/message
	Assert(mu_task.paTaskSendText->m_oTextBlockMessageLog.isValid());
	mu_task.paTaskSendText->ChatLog_UpdateEventWithinWidget();
	_TaskDestroy();
	*/
	}

void
CEventMessageTextSent::MessageResendUpdate(const CStr & strMessageUpdated, INOUT WLayoutChatLog * pwLayoutChatLogUpdate)
	{
	m_strMessageText = strMessageUpdated;
	m_uFlagsMessage |= FM_kfMessageUpdated;
	m_tsOther = d_tsOther_ezEventNeverSent;	// Pretend the message was never sent.  This will trigger a new task to resend the message
	Assert(!Event_FHasCompleted());
	//ChatLog_UpdateEventWithinWidget(pwLayoutChatLogUpdate->m_pwChatLog);
	pwLayoutChatLogUpdate->m_pwChatLog_NZ->ChatLog_EventUpdate(this);
	}

void
CEventMessageTextReceived::MessageUpdated(PSZUC pszMessageUpdated, INOUT WChatLog * pwChatLog)
	{
	Assert(pwChatLog != NULL);
	m_strMessageText = pszMessageUpdated;
	m_uFlagsMessage |= FM_kfMessageUpdated;
//	m_tsOther = Timestamp_GetCurrentDateTime();	// Re-update the timestamp to indicate
	Assert(Event_FHasCompleted());
	//ChatLog_UpdateEventWithinWidget(pwChatLog);
	pwChatLog->ChatLog_EventUpdate(this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
#define d_cbBufferSizeDefaultTransferFiles		4096	// 4 KiB default block size to transfer files
ETaskCompletion
CTaskFileUpload::EWriteDataToSocket()
	{
	#if 0
	Assert(mu_parent.pEventFileSent != NULL);
	Assert(mu_parent.pEventFileSent->EGetEventClass() == CEventFileSent::c_eEventClass);
	if (m_uSequence == 0) // mu_parent.pEventFileSent->m_cblDataTransferred != d_cblDataTransferred_FileOffered)
		{
		const CStr * pstrFileName = &mu_parent.pEventFileSent->m_strFileName;
		if (mu_parent.pEventFileSent->m_cblFileSize != d_cblFileSize_Invalid)
			{
			m_oFile.setFileName(*pstrFileName);
			if (!m_oFile.open(QFile::ReadOnly))
				{
				//(void)EMessageBoxWarning("Unable open file '$S'", pstrFileName);
				mu_parent.pEventFileSent->m_cblFileSize = d_cblFileSize_Invalid;	// We cannot do a transfer because we are unable to open/read the file from the disk
				ChatLog_UpdateEventWithinWidget();
				return eTaskCompleted;
				}
			mu_parent.pEventFileSent->m_cblFileSize = m_oFile.size();
			}
		Socket_WriteXmlIqSet_VE_Gso(
			"<si id='$t' profile='^*ft' ^:si><file ^:ft name='^s' size='$l'/>"
				"<feature ^:fn><x ^:xd type='form'><field var='stream-method' type='list-single'><option><value>^*ib</value></option></field></x></feature>"
			"</si>", mu_parent.pEventFileSent->m_tsEventID, pstrFileName->PathFile_PszGetFileNameOnly_NZ(), mu_parent.pEventFileSent->m_cblFileSize);

		// mu_parent.pEventFileSent->m_cblDataTransferred = d_cblDataTransferred_FileOffered;
		}
	#endif
	return eTaskCompletionNeedsMoreProcessing;
	} // EWriteDataToSocket()

//	CTaskFileUpload::ITask::ProcessStanza()
void
CTaskFileUpload::ProcessStanza(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext, const CXmlNode * pXmlNodeVerb)
	{
	Assert(pXmlNodeStanza != NULL);
	Endorse(pszaVerbContext == NULL);
	Endorse(pXmlNodeVerb == NULL);
	CXmlNode * pXmlNodeStreamInitiation = pXmlNodeStanza->PFindElementSi();
	if (pXmlNodeStreamInitiation != NULL)
		{
		// The other client accepted the file, so open the connection to start sending the data
		Socket_WriteXmlIqSet_VE_Gso("<open sid='$t' block-size='$u' ^:ib/>", mu_parent.pEventFileSent->m_tsEventID, d_cbBufferSizeDefaultTransferFiles);
		return;
		}
	if (pszaVerbContext == c_sza_close || pXmlNodeStanza->PFindElementClose() != NULL)
		{
		// The client may send a stanza <close> if the number of bytes sent is smaller than the block size (indicating it has received the whole file)
		Socket_WriteXmlIqReplyAcknowledge();	// Acknowledge the <close> element
		MessageLog_AppendTextFormatCo(d_coRed, "CSocketTaskFileUpload::ProcessStanza() - close!\n");
		OnEventTransferCompleted();
		return;
		}
	#if 0
	const BOOL fTransferring = (mu_parent.pEventFileSent->m_cblDataTransferred < mu_parent.pEventFileSent->m_cblFileSize);
	if (fTransferring)
		{
		if (FCompareStrings(pXmlNodeStanza->PszFindAttributeValueType_NZ(), c_sza_result))
			{
			// We have a generic 'result' so start sending the data
			char szBuffer[d_cbBufferSizeDefaultTransferFiles];
			int cbRead = m_oFile.read(OUT szBuffer, sizeof(szBuffer));
			if (cbRead > 0)
				{
				mu_parent.pEventFileSent->m_cblDataTransferred += cbRead;
				Socket_WriteXmlIqSet_VE_Gso("<data sid='$t' seq='$u' ^:ib>{p/}</data>", mu_parent.pEventFileSent->m_tsEventID, m_uSequence++, szBuffer, cbRead);
				}
			else
				{				
				goto StreamClose;	// We are done sending the file, so send a <close> command to the remote contact
				}
			} // if
		}
	else if (m_uSequence > 0)
		{
		// If we are done transferring, close the stream
		StreamClose:
		m_uSequence = 0;
		m_oFile.close();
		Socket_WriteXmlIqSet_VE_Gso("<close sid='$t' ^:ib/>", mu_parent.pEventFileSent->m_tsEventID);
		}
	else
		{
		CXmlNode * pXmlNodeError = pXmlNodeStanza->PFindElementError();
		if (pXmlNodeError != NULL)
			{
			if (FCompareStrings(pXmlNodeError->PszFindAttributeValueType_NZ(), c_sza_cancel))
				{
				if (fTransferring)
					mu_parent.pEventFileSent->m_cblDataTransferred = d_cblDataTransferred_CancelledByRemoteContact;
				}
			} // if
		} // if...else
	#endif
	ChatLog_UpdateEventWithinWidget();	// Whatever stanza we received, always update the text block
	} // ProcessStanza()
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
	/*
ETaskCompletion
CTaskFileDownload::EWriteDataToSocket()
	{
	CStr * pstrFileName = &mu_parent.pEventFileReceived->m_strFileName;
	m_oFile.setFileName(*pstrFileName);
	if (!m_oFile.open(QFile::WriteOnly))
		{
		(void)EMessageBoxWarning("Unable to create file '$S'", pstrFileName);
		return eTaskCompleted;
		}
	// Write a reply to the socket to notify the remote client to start sending the data
	*pstrFileName = QFileInfo(m_oFile).absoluteFilePath();	// Update the filename with the full path
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CTaskFileDownload::EWriteDataToSocket() - $S\n", pstrFileName);
	Socket_WriteXmlFormatted("<iq id='$S' type='result' from='^J' to='^J'>"
		"<si id='$S' profile='^*ft' ^:si><feature ^:fn><x ^:xd type='submit'><field var='stream-method'><value>^*ib</value></field></x></feature></si></iq>",
		&mu_parent.pEventFileReceived->m_strxStanzaID, m_pAccount, mu_parent.pEvent->mu_parentowner.pTreeItem, &mu_parent.pEventFileReceived->m_strxSID);
	ChatLog_UpdateEventWithinWidget();
	return eTaskCompletionNeedsMoreProcessing;
	}

void
CTaskFileDownload::ProcessStanza(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext, const CXmlNode * pXmlNodeVerb)
	{
	Assert(pXmlNodeStanza != NULL);
	if (pszaVerbContext == c_sza_data)
		{
		Assert(pXmlNodeVerb->FCompareTagName("data"));
		// Get the data from the stanza
		//int nSequence = pXmlNodeStanza->NFindElementOrAttributeValueNumeric("seq");
		CBin bin;
		const BYTE * pbData = bin.BinAppendBinaryDataFromBase64Szv(pXmlNodeVerb->m_pszuTagValue);
		int cbData = bin.CbGetData();
		if (m_oFile.write((const char *)pbData, cbData) != cbData)
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Failure writing to file #S\n", &mu_parent.pEventFileReceived->m_strFileName);
		//mu_parent.pEventFileReceived->m_cblDataTransferred += cbData;
		}
	else if (pszaVerbContext == c_sza_close)
		{
		Assert(pXmlNodeVerb->FCompareTagName("close"));
		m_oFile.close();
		}
	else
		{
		Assert(pszaVerbContext == c_sza_open);
		Assert(pXmlNodeVerb->FCompareTagName("open"));
		}
	Socket_WriteXmlIqReplyAcknowledge();	// Reply to acknowledge we received the stanza and/or receive the next data sequence
	ChatLog_UpdateEventWithinWidget();
	} // ProcessStanza()

*/

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
IEvent *
CArrayPtrEvents::PFindEventLastSent() const
	{
	return NULL;
	}
*/

//	This is somewhat similar as CArray::Add() however guarantees the events will be sorted by m_tsEventID
//
//	Return TRUE if the event was inserted somewhere in the array because it is out-of-sync.
//	Return FALSE if the event was added at the end of the array.
BOOL
CArrayPtrEvents::Event_FoosAddSorted(IEvent * pEventNew)
	{
	Assert(pEventNew != NULL);
	Assert(pEventNew->m_tsEventID != d_ts_zNULL);
	if (m_paArrayHdr != NULL)
		{
		int cEvents = m_paArrayHdr->cElements;
		if (cEvents > 0)
			{
			// Optimize for the most common case, which is adding the new event at the end of the array
			if (pEventNew->m_tsEventID > ((IEvent *)m_paArrayHdr->rgpvData[cEvents - 1])->m_tsEventID)
				{
				if (cEvents < m_paArrayHdr->cElementsAlloc)
					{
					m_paArrayHdr->rgpvData[m_paArrayHdr->cElements++] = pEventNew;
					return FALSE;
					}
				goto EventInSync;
				}
			// Well, the new event is out of sync, so we have to find its place.
			// The comparison is done from the end of the array, as the events are assumed to be in consecutive order, and the timestamp of the new event is likely be recent.
			IEvent ** prgpEvents = (IEvent **)m_paArrayHdr->rgpvData;
			while (--cEvents >= 0)
				{
				IEvent * pEventLast = prgpEvents[cEvents];
				Assert(pEventLast->m_tsEventID != d_ts_zNULL);
				if (pEventNew->m_tsEventID > pEventLast->m_tsEventID)
					break;
				} // while
			Assert(cEvents + 1 >= 0);
			//MessageLog_AppendTextFormatCo(d_coBlue, "Event_FoosAddSorted($t) - inserting at index $i\n", pEventNew->m_tsEventID, cEvents);
			InsertElementAt(cEvents + 1, pEventNew);
			return TRUE;
			} // if
		} // if
	EventInSync:
	Add(pEventNew);
	return FALSE;
	} // Event_FoosAddSorted()


TIMESTAMP
CArrayPtrEvents::TsEventIdLastEventSent() const
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		AssertValidEvent(pEvent);
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->Event_FIsEventTypeSent())
			return pEvent->m_tsEventID;
		}
	return d_ts_zNULL;
	}

TIMESTAMP
CArrayPtrEvents::TsEventOtherLastEventReceived() const
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		AssertValidEvent(pEvent);
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->Event_FIsEventTypeReceived())
			return pEvent->m_tsOther;
		}
	return d_ts_zNULL;
	}

TIMESTAMP
CArrayPtrEvents::TsEventOtherLast() const
	{
	IEvent * pEventLast = PGetEventLast_YZ();
	if (pEventLast != NULL)
		return pEventLast->m_tsOther;
	return d_ts_zNULL;
	}

TIMESTAMP
CArrayPtrEvents::TsEventIdLast() const
	{
	IEvent * pEventLast = PGetEventLast_YZ();
	if (pEventLast != NULL)
		return pEventLast->m_tsEventID;
	return d_ts_zNULL;
	}

//	This method should be merged with CVaultEvent
CEventMessageTextReceived *
CArrayPtrEvents::PFindEventMessageReceivedByTimestamp(TIMESTAMP tsOther) const
	{
	Assert(tsOther != 0);
	// Search the array from the end, as the event to search is likely to be a recent one
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->EGetEventClass() == CEventMessageTextReceived::c_eEventClass)
			continue;
		if (pEvent->m_tsOther <= tsOther)
			{
			if (pEvent->m_tsOther == tsOther)
				return (CEventMessageTextReceived *)pEvent;
			break;
			}
		} // while
	return NULL;
	}

IEvent *
CVaultEvents::PFindEventReceivedByTimestampOther(TIMESTAMP tsOther, TGroupMember * pMember) CONST_MCC
	{
	Endorse(pMember == NULL);		// In a single chat, the pContactGroupSender alway NULL. The member variable IEvent::m_pContactGroupSender_YZ is valid only for group chat
	Assert(pMember == NULL || pMember->EGetRuntimeClass() == RTI(TGroupMember));
	return PFindEventReceivedByTimestampOther(tsOther, (pMember != NULL) ? pMember->m_pContact : NULL);
	}

//	Find the event received matching the timestamp and the group member.
//	If there is no group member, then the event is for a one-to-one chat, rather than group chat.
IEvent *
CVaultEvents::PFindEventReceivedByTimestampOther(TIMESTAMP tsOther, TContact * pContactGroupSender) CONST_MCC
	{
	Assert(pContactGroupSender == NULL || pContactGroupSender->EGetRuntimeClass() == RTI(TContact));
	if (tsOther > d_tsOther_kmReserved)
		{
		const TIMESTAMP tsOtherStop = tsOther - (25 * d_ts_cHours);	// In a chat, the m_tsOther are semi-sorted for message received, as they represent the timestamps from the remote computers.  Therefore any timestamp older than one day (25 hours) is considered out of range, and there is no need to search the entire list of event.
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
			Endorse(pEvent->m_tsOther <= d_tsOther_kmReserved);		// tsOther may be anything
			if (pEvent->EGetEventClass() & eEventClass_kfReceivedByRemoteClient)
				{
				Assert(pEvent->Event_FIsEventTypeReceived());
				Assert(pEvent->m_tsOther > d_tsOther_kmReserved);
				if (pEvent->m_tsOther == tsOther && pEvent->m_pContactGroupSender_YZ == pContactGroupSender)
					return pEvent;
				if (pEvent->m_tsOther < tsOtherStop)
					{
					MessageLog_AppendTextFormatSev(eSeverityNoise, "PFindEventByTimestampOther_($t, 0x$p) - Stopping search at pEvent->m_tsOther $t\n", tsOther, pContactGroupSender, pEvent->m_tsOther);
					break;	// We went far enough in the list to conclude the event does not match the timestamp
					}
				} // if
			} // while
		} // if
	return NULL;
	} // PFindEventReceivedByTimestampOther()

//	Find the next event after tsEventID, and return how many events are remaining.
//	Since this method is for XCP, the method will skip returning non-XCP events and only return events which may be transmitted throught XCP.
//	For instance, a 'ping' event will not be returned by this method.
//
IEvent *
CArrayPtrEvents::PFindEventNextForXcp(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) const
	{
	Endorse(tsEventID == d_ts_zNULL);	// Return the first event in the array
	Assert(pcEventsRemaining != NULL);
	if (m_paArrayHdr != NULL)
		{
		int cEvents = m_paArrayHdr->cElements;
		if (cEvents > 0)
			{
			IEvent ** ppEventFirst = (IEvent **)m_paArrayHdr->rgpvData;
			IEvent ** ppEventStop = ppEventFirst + cEvents;
			IEvent ** ppEventCompare = ppEventStop;
			// Search from the end until we find an event smaller or equal than tsEventID.  This means the next event (if any) in the array is the one to return;
			while (--ppEventCompare >= ppEventFirst)
				{
				IEvent * pEvent = *ppEventCompare;
				AssertValidEvent(pEvent);
				Assert(pEvent->m_tsEventID != d_ts_zNULL);
				if (pEvent->m_tsEventID <= tsEventID)
					break;
				} // while

			// Loop until we have a valid event for XCP
			while (++ppEventCompare < ppEventStop)
				{
				IEvent * pEvent = *ppEventCompare;	// Get the next event
				if ((pEvent->EGetEventClass() & eEventClass_kfNeverSerializeToXCP) == 0)
					{
					*pcEventsRemaining = (ppEventStop - ppEventCompare) - 1;
					Assert(*pcEventsRemaining >= 0);
					return pEvent;
					}
				} // while
			} // if
		} // if
	*pcEventsRemaining = 0;
	return NULL;
	} // PFindEventNextForXcp()

IEvent *
CVaultEvents::PFindEventNext(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) CONST_MCC
	{
	return m_arraypaEvents.PFindEventNextForXcp(tsEventID, OUT pcEventsRemaining);
	}

IEvent *
CArrayPtrEvents::PFindEventByID(TIMESTAMP tsEventID) const
	{
	if (tsEventID > d_tsOther_kmReserved)
		{
		// Search the array from the end, as the event to search is likely to be a recent one
		IEvent ** ppEventStop;
		IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			AssertValidEvent(pEvent);
			Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
			if (pEvent->m_tsEventID == tsEventID)
				return pEvent;
			}
		}
	return NULL;
	}

void
CArrayPtrEvents::SortEventsByIDs()
	{
	Sort((PFn_NCompareSortElements)IEvent::S_NCompareSortEventsByIDs);
	}

BOOL
CArrayPtrEvents::FEventsSortedByIDs() const
	{
	return FIsSortedAscending((PFn_NCompareSortElements)IEvent::S_NCompareSortEventsByIDs);
	}

void
CArrayPtrEvents::DeleteAllEvents()
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		delete *ppEvent++;
	RemoveAllElements();
	}

void
CArrayPtrEvents::DeleteAllEventsReceivedHavingDuplicateTsOther()
	{
	if (m_paArrayHdr == NULL)
		return;
	TIMESTAMP tsOtherPrevious = d_ts_zNA;
	IEvent ** ppEventStop;
	IEvent ** ppEventStart = PrgpGetEventsStop(OUT &ppEventStop);
	IEvent ** ppEventDst = ppEventStart;
	IEvent ** ppEventSrc = ppEventStart;
	while (ppEventSrc != ppEventStop)
		{
		IEvent * pEvent = *ppEventSrc++;
		AssertValidEvent(pEvent);
		if (pEvent->Event_FIsEventTypeReceived())
			{
			Assert(pEvent->m_tsOther != d_ts_zNULL);
			if (pEvent->m_tsOther == tsOtherPrevious)
				{
				MessageLog_AppendTextFormatSev(eSeverityNoise, "DeleteAllEventsReceivedHavingDuplicateTsOther() - Deleting event EventID=$t because it is identical to its previous event tsOther=$t\n", pEvent->m_tsEventID, tsOtherPrevious);
				delete pEvent;
				continue;
				}
			tsOtherPrevious = pEvent->m_tsOther;
			}
		*ppEventDst++ = pEvent;	// Keep the event
		}
	m_paArrayHdr->cElements = ppEventDst - ppEventStart;
	} // DeleteAllEventsReceivedHavingDuplicateTsOther

#if 0
CTaskFileDownload *
CArrayPtrTasksDownloading::PFindTaskMatchingSessionIdentifier(PSZUC pszSessionIdentifier) const
	{
	Assert(pszSessionIdentifier != NULL);
	if (pszSessionIdentifier[0] != '\0')
		{
		// Search the array from the end, as the event to search is likely to be a recent one
		CEventFileReceived ** ppEventStop;
		CEventFileReceived ** ppEvent = (CEventFileReceived **)PrgpvGetElementsStop(OUT (void ***)&ppEventStop);
		while (ppEvent != ppEventStop)
			{
			CEventFileReceived * pEvent = *--ppEventStop;
			Assert(pEvent != NULL);
			/*
			Assert(pEvent->EGetEventClass() == CEventFileReceived::c_eEventClass);
			if (pEvent->m_strxSID.FCompareStringsExactCase(pszSessionIdentifier))
				return pEvent->mu_task.paTaskFileDownload;
			*/
			}
		}
	return NULL;
	}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventPing::CEventPing()
	{
	m_tsContact = d_ts_zNULL;
	}

//	CEventPing::IEvent::XmlSerializeCore()
//	Send a ping request, both in XMPP and XCP.  Since the ping contains no 'data', the XCP ping is automaticaly handled by the core XCP routines handling the protocol.
void
CEventPing::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	pbinXmlAttributes->XmppWriteStanzaToSocketOnlyIfContactIsUnableToCommunicateViaXcp_VE("<iq id='$t' type='get' to='^J'><ping xmlns='urn:xmpp:ping'/></iq>", m_tsEventID, pbinXmlAttributes->m_pContact);
	}

void
CEventPing::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType *)
	{
	if (m_tsContact == d_ts_zNULL)
		m_tsContact = pXmlNodeExtraData->TsGetAttributeValueTimestamp_ML(d_chXCPa_PingTime);
	Event_SetCompleted(NULL);
	}

//	CEventPing::IEvent::ChatLogUpdateTextBlock()
void
CEventPing::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	g_strScratchBufferStatusBar.BinAppendTextSzv_VE("Pinging <b>^s</b>...", ChatLog_PszGetNickNameOfContact());
	const TIMESTAMP_DELTA dtsResponse = m_tsOther - m_tsEventID;
	if (dtsResponse > 0)
		{
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("  response time: <b>$L</b> ms", dtsResponse);
		const TIMESTAMP_DELTA dtsClockDifference = m_tsContact - (m_tsEventID + m_tsOther) / 2;
		if (dtsClockDifference > -1000 * d_ts_cDays && dtsClockDifference < 1000 * d_ts_cDays)
			g_strScratchBufferStatusBar.BinAppendTextSzv_VE(" (clock difference: $T)", dtsClockDifference);	// Display the clock difference only if it is less than 1000 days (about 3 years).  Typically the clock difference should be a few seconds for systems connected to the UTC server, otherwise it may be a few at most a few minutes
		}
	if (!m_strError.FIsEmptyString())
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("  <b>(error: ^S)</b>", &m_strError);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(0xE8CFD8));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG_WANT_ASSERT
//	Make sure the event points to valid data in memory.
//	This is done by calling a virtual method.  If the object is not valid, then its vtable will point to garbage data and the application will crash.
void
AssertValidEvent(const IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	EEventClass eEventClass = pEvent->EGetEventClass();
	Assert(eEventClass != eEventClass_eNull);
	}
#endif
