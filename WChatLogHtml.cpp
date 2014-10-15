//	WChatLogHtml.cpp

//	Experimental version of WChatLog using an HTML browser allower richer content.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WChatLogHtml.h"
#ifdef COMPILE_WITH_CHATLOG_HTML

WChatLogHtml::WChatLogHtml(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup) : QWebView(pwParent)
	{
	Assert(pContactOrGroup != NULL);
	m_pContactOrGroup = pContactOrGroup;

	setHtml(
		"<html><head><style>"
		//"div { background-color: red }"

		// Style for each division
		//".d { min-height: 1rem; line-height: 22px; padding: .25rem .1rem .1rem 3rem; }"
		".d { min-height: 1rem; line-height: 22px; padding: .25rem .1rem .1rem 40px ; }"
		".p {  }"

		// Style for an icon
		".i { float:left; position: absolute; left: 3; margin-top: 3; height: 36; width: 36; "
		"border-radius: .2rem;"
		"background-size: 100%;"
		" }"

		// Style for the sender of the message
		".s { font-weight: bold;  } "

		// Style for the timestamp
		".t { color: #BABBBF; font-size: 12px; padding-left: 5 } "

		// Style for  the message itself
		".m { display: block; }"

		".i0 { background-image: url('qrc:/ico/Avatar1') }"
		".i1 { background-image: url('qrc:/ico/Avatar2') }"

		"</style></head><body style=\""
		// Style for the body
		"font-family: Lato, sans-serif; font-size: 15px; color: #3D3C40; "
		"\"><div id='m'></div><div id='c'/></div></body></html>"
		);
	QWebPage * poPage = page();
	poPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);	// Enable the context menu item named "Inspect".  This is useful for debugging web pages.
	m_poFrame = poPage->mainFrame();
	m_poFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	m_oElementMessages = m_poFrame->findFirstElement("#m");
	m_oElementComposing = m_poFrame->findFirstElement("#c");

	CArrayPtrEvents arraypEvents;
	m_pContactOrGroup->Vault_GetEventsForChatLog(OUT &arraypEvents);
	ChatLog_EventsAppend(IN arraypEvents);
	}

void
WChatLogHtml::_BinAppendHtmlForEvents(IOUT CBin * pbinHtml, IEvent ** ppEventStart, IEvent ** ppEventStop)
	{
	while (ppEventStart < ppEventStop)
		{
		IEvent * pEvent = *ppEventStart++;
		AssertValidEvent(pEvent);
		pbinHtml->BinAppendText_VE(
			"<div class='d'>"
				"<a href='#' class='i i$i'></a>", pEvent->Event_FIsEventTypeSent()
			);
		pEvent->ChatLogAppendHtmlDivider(IOUT pbinHtml);
		pbinHtml->BinAppendText_VE("<span class='m' id='{t_}'>", pEvent->m_tsEventID);
		pEvent->AppendHtmlForChatLog(IOUT pbinHtml);
		pbinHtml->BinAppendText("</span></div>");
		}
	}

void
WChatLogHtml::_ScrollToDisplayLastEvent()
	{
	//triggerPageAction(QWebPage::MoveToEndOfDocument);
	//m_poFrame->setScrollPosition(QPoint(0, m_poFrame->scrollBarMaximum(Qt::Vertical) + 1000 ));
	m_poFrame->scrollToAnchor("c");
	}

void
WChatLogHtml::ChatLog_EventsAppend(const CArrayPtrEvents & arraypEvents)
	{
	CBin binHtml;
	binHtml.PbbAllocateMemoryAndEmpty_YZ(10000);

	IEvent ** ppEventStop;
	IEvent ** ppEventFirst = arraypEvents.PrgpGetEventsStop(OUT &ppEventStop);
	_BinAppendHtmlForEvents(IOUT &binHtml, ppEventFirst, ppEventStop);
	m_oElementMessages.appendInside(binHtml.ToQString());
	ChatLog_ChatStateTextRefresh();
	_ScrollToDisplayLastEvent();
	}

void
WChatLogHtml::ChatLog_EventAppend(IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	CArrayPtrEvents arraypEvents;
	arraypEvents.EventAdd(pEvent);
	ChatLog_EventsAppend(arraypEvents);
	}

void
WChatLogHtml::ChatLog_EventUpdate(IEvent * pEvent)
	{
	Assert(pEvent != NULL);
//	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "TODO: Need to implement ChatLog_EventUpdate() - tsEventID = $t", pEvent->m_tsEventID);

	CBin bin;
	bin.BinAppendText_VE("#{t_}", pEvent->m_tsEventID);
	/*
	CHU szTimestamp[16];
	Timestamp_CchEncodeToBase64Url(pEvent->m_tsEventID, OUT szTimestamp);
	*/
	QWebElement oElementMessage = m_oElementMessages.findFirst(bin.ToQString());
	if (!oElementMessage.isNull())
		{
		CBin binHtml;
		pEvent->AppendHtmlForChatLog(IOUT &binHtml);
		oElementMessage.setInnerXml(binHtml.ToQString());
		//oElementMessage.setInnerXml("this is a <b>test</b>");
		}
	else
		{
		MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "TODO: Unable to find message matching id='$B'", &bin);
		}
	}

void
IEvent::Event_UpdateWithinChatLogHtml(WChatLogHtml * pwChatLog_YZ)
	{
	if (pwChatLog_YZ != NULL)
		pwChatLog_YZ->ChatLog_EventUpdate(this);
	}

void
WChatLogHtml::ChatLog_EventsRepopulate()
	{

	}

void
WChatLogHtml::ChatLog_ChatStateTextRefresh()
	{
	CBin binComposers;
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypContactsComposing.PrgpGetContactsStop(OUT &ppContactStop);
	if (ppContact == ppContactStop)
		{
		//oTextCursor.removeSelectedText();
		}
	else
		{
		//if (g_poImageComposing == NULL)
		//	g_poImageComposing = new QImage(":/ico/Pencil");
		while (TRUE)
			{
			//oTextCursor.insertImage(*g_poImageComposing);
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			binComposers.BinAppendText_VE(d_szu_nbsp " <b>^s</b> is typing...", pContact->ChatLog_PszGetNickname());
			if (ppContact == ppContactStop)
				break;
			} // while
		}
	/*
	if (m_pContactOrGroup->EGetRuntimeClass() == RTI(TContact))
		((TContact *)m_pContactOrGroup)->ChatLogContact_AppendExtraTextToChatState(INOUT oTextCursor);
	*/
	m_oElementComposing.setInnerXml(binComposers.ToQString());
	if (!binComposers.FIsEmptyBinary())
		_ScrollToDisplayLastEvent();
	}

void
WChatLogHtml::ChatLog_ChatStateTextUpdate(INOUT TContact * pContact, EChatState eChatState)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	if (eChatState == eChatState_zComposing)
		{
		if (!m_arraypContactsComposing.AddUniqueF(pContact))
			return;
		pContact->TreeItemContact_UpdateIconComposingStarted(m_pContactOrGroup);
		}
	else
		{
		if (m_arraypContactsComposing.RemoveElementI(pContact) < 0)
			return;
		pContact->TreeItemContact_UpdateIconComposingStopped(m_pContactOrGroup);
		}
	if (eChatState != eChatState_PausedNoUpdateChatLog)
		ChatLog_ChatStateTextRefresh();
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
const char c_szHtmlMessageDelivered[] = "<img src='qrc:/ico/Delivered' style='float:right'/>";
const char c_szHtmlTemplateNickname[] = "<span class='s'>{sH}</span>";

void
IEvent::ChatLogAppendHtmlDivider(IOUT CBin * pbinHtml) CONST_MCC
	{
	Assert(m_pVaultParent_NZ != NULL);
	ITreeItemChatLog * pTreeItemNickname = NULL;		// This pointer should be a TContact, however if there is a 'bug' where a group message has m_pContactGroupSender_YZ == NULL, then this will point to a group (which is no big deal)
	const QDateTime dtlMessage = QDateTime::fromMSecsSinceEpoch(m_tsEventID).toLocalTime();
	const QString sTime = dtlMessage.toString("hh:mm");
	const QString sDateTime = dtlMessage.toString(Qt::SystemLocaleLongDate); // DefaultLocaleLongDate);
	TIMESTAMP_DELTA dts = m_tsOther - m_tsEventID;
	const EEventClass eEventClass = EGetEventClass();
	if (eEventClass & eEventClass_kfReceivedByRemoteClient)
		{
		// The event was received (typically a message typed by someone else)
		if (dts < -10 * d_ts_cMinutes && dts > -15 * d_ts_cDays)
			pbinHtml->BinAppendText_VE("[$T] ", dts);
		#if 0
		pbinHtml->BinAppendText_VE("<span title='Message was sent {T_} before you received it'>[{T-}] </span>", dts);
		#endif
		pTreeItemNickname = (m_pContactGroupSender_YZ != NULL) ? m_pContactGroupSender_YZ : m_pVaultParent_NZ->m_pParent;	// Use the name of the group sender (if any, otherwise the name of the contact)
		}

	if (m_uFlagsEvent & FE_kfEventProtocolError)
		{
		pbinHtml->BinAppendText(" <img src='qrc:/ico/Error' title='XCP Protocol Error: One of the peers has an old version of SocietyPro and cannot process this event because its type is unknown'/> ");
		}
	if ((eEventClass & eEventClass_kfReceivedByRemoteClient) == 0)
		{
		// The message was sent by the user
		if (dts > 5 * d_ts_cMinutes)
			pbinHtml->BinAppendText_VE("[$T] ", dts);
		if (Event_FHasCompleted())
			pbinHtml->BinAppendBinaryData(c_szHtmlMessageDelivered, sizeof(c_szHtmlMessageDelivered) - 1);
		pTreeItemNickname = PGetAccount_NZ();	// Use the name of the user
		}
	#ifdef DEBUG_DISPLAY_TIMESTAMPS
	pbinHtml->BinAppendText_VE("<code>[i=<b>$t</b> o=<b>$t</b>] </code>", m_tsEventID, m_tsOther);
	#endif
	if (m_uFlagsEvent & FE_kfEventOutOfSync)
		pbinHtml->BinAppendText(" <img src='qrc:/ico/OutOfSync' title='Out of Sync' /> ");
	pbinHtml->BinAppendText_VE(c_szHtmlTemplateNickname, pTreeItemNickname->ChatLog_PszGetNickname());
	pbinHtml->BinAppendText_VE("<span class='t' title='^Q'>^Q</span>", &sDateTime, &sTime);

	if (m_uFlagsEvent & (FE_kfReplaced | FE_kfReplacing))
		pbinHtml->BinAppendText(" <img src=':/ico/Pencil' title='Edited' /> ");	// Any event replaced or replacing another one displays the 'edited' icon
	}

void
IEvent::AppendHtmlForChatLog(IOUT CBin * pbinHtml) CONST_MCC
	{
	Assert(pbinHtml != NULL);
	}

void
IEventMessageText::AppendHtmlForChatLog(IOUT CBin * pbinHtml) CONST_MCC
	{
	pbinHtml->BinAppendHtmlTextWithAutomaticHyperlinks(m_strMessageText);
	}

#endif

CEventHelp::CEventHelp(PSZUC pszHtmlHelp)
	{
	m_strHtmlHelp = pszHtmlHelp;
	}

#ifdef COMPILE_WITH_CHATLOG_HTML
void
CEventHelp::AppendHtmlForChatLog(IOUT CBin * pbinHtml) CONST_MCC
	{
	pbinHtml->BinAppendCStr(m_strHtmlHelp);
	}
#else
void
CEventHelp::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	poCursorTextBlock->InsertHtmlBin(m_strHtmlHelp, c_brushSilver);
	}
#endif



