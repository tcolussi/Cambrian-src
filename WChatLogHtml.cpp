//	WChatLogHtml.cpp

//	Experimental version of WChatLog using an HTML browser allower richer content.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WChatLogHtml.h"
#ifdef COMPILE_WITH_CHATLOG_HTML

#define COLOR_THEME_BLACK		// Use a black background instead of a white background

WChatLogHtml::WChatLogHtml(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup) : QWebView(pwParent)
	{
	Assert(pContactOrGroup != NULL);
	m_pContactOrGroup = pContactOrGroup;
	m_tsMidnightNext = d_ts_zNA;
	m_hSenderPreviousEvent = d_zNA;	// No previous sender yet

	setHtml(
		"<html><head><style>"

		// Style for each division
		//".d { min-height: 1rem; line-height: 22px; padding: .25rem .1rem .1rem 3rem; }"
		".d { min-height: 1rem; line-height: 22px; padding: .25rem .1rem .1rem 55px ; }"

		// Style for an icon/avatar
		".i	 { float:left; position: absolute; left: 15; margin-top: 3; height: 36; width: 36; "
			"border-radius: .2rem;"
			"background-size: 100%;"
			"}"

		// Style for the sender of the message
		".s { font-weight: bold;  } "

		// Style for the day divider
		"div.dd { "
				"position: relative;"
				"text-align: center;"
				"}"
		"div.ddd { "
				"color: #555459; font-size: 16px; font-weight: 900;"			// day divider displaying the date
				"background: #FFF;"
				"padding: 1px .7rem;"
				//"padding: 0 .7rem;"
				//"margin-top: -1px;"
				"display: inline-block;"
				"position: relative;"
				"text-align: center;"
				"border-radius: 5px"
				"}"
		"hr.dd	{ "
				"position: absolute;"
				"border-top: 1px solid #DDD;"
				"width: 100%;"
				"top: .6rem;"
				"margin: 0;"
				"text-align: center;"
				"}"

		// Style for the timestamp
		".t { color: #BABBBF; font-size: 12px; padding-left: 5 } "

		// Style for a timestamp on the left (this timestamp appears below the icon/avatar when the user types multiple consecutive messages)
		".tl { color: #BABBBF; font-size: 12px;"
			"position: absolute;"
			"left:0;"
			"text-align: right;"
			"width: 50;"
			"}"

		// Style for  the message itself
		".m { display: block; }"

		// Styles for different types of hyperlinks
		"a." d_szClassForChatLog_ButtonHtml // " { color: black; background-color: silver; text-decoration: none; font-weight: bold } "
"	{"
"   border-top: 1px solid #96d1f8;"
"   background: #65a9d7;"
"   background: -webkit-gradient(linear, left top, left bottom, from(#3e779d), to(#65a9d7));"
"   background: -webkit-linear-gradient(top, #3e779d, #65a9d7);"
"   background: -moz-linear-gradient(top, #3e779d, #65a9d7);"
"   background: -ms-linear-gradient(top, #3e779d, #65a9d7);"
"   background: -o-linear-gradient(top, #3e779d, #65a9d7);"
"   padding: 5px 10px;"
"   -webkit-border-radius: 8px;"
"   -moz-border-radius: 8px;"
"   border-radius: 8px;"
"   -webkit-box-shadow: rgba(0,0,0,1) 0 1px 0;"
"   -moz-box-shadow: rgba(0,0,0,1) 0 1px 0;"
"   box-shadow: rgba(0,0,0,1) 0 1px 0;"
"   text-shadow: rgba(0,0,0,.4) 0 1px 0;"
"   color: white;"
"   font-size: 14px;"
"   text-decoration: none;"
"   vertical-align: middle;"
"   }"

"a." d_szClassForChatLog_ButtonHtml ":hover"
	"{"
	#if 0
	"border-top-color: #28597a;"
	"background: #28597a;"
	"color: #ccc;"
	#else
	"background: white;"
	"color: black;"
	#endif
	"}"
/*
.button:active {
   border-top-color: #1b435e;
   background: #1b435e;
   }
*/

		"a." d_szClassForChatLog_HyperlinkDisabled " { color: gray; text-decoration: none; } "

		#ifdef COLOR_THEME_BLACK
		"a { color: yellow; }"			// Use a different than the default blue color for hyperlinks
		"a { color: #EE82EE; }"
		#endif

		".i0 { background-image: url('qrc:/ico/Avatar1') }"
		".i1 { background-image: url('qrc:/ico/Avatar2') }"

		"</style></head>"
		"<body style=\""

			// Style for the body
			"font-family: Lato, sans-serif; font-size: 15px; color: #3D3C40; "

			#ifdef COLOR_THEME_BLACK // Black background style with a 'space' background image
			"color: white; background-color: black; background-image: url('qrc:/backgrounds/Space'); background-attachment:fixed;"
			#endif

			"\">"
			// Division for all the messages
			"<div id='m'></div>"
			// Division for the composing message
			"<div id='c' style='font-size: 13px'/></div>"

		"</body></html>");
	QWebPage * poPage = page();
	poPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	poPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);	// Enable the context menu item named "Inspect".  This is useful for debugging web pages.
	m_poFrame = poPage->mainFrame();
	m_poFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	m_oElementMessages = m_poFrame->findFirstElement("#m");
	m_oElementComposing = m_poFrame->findFirstElement("#c");

	CArrayPtrEvents arraypEvents;
	m_pContactOrGroup->Vault_GetEventsForChatLog(OUT &arraypEvents);
	ChatLog_EventsAppend(IN arraypEvents);

	//connect(this, SIGNAL(highlighted(QUrl)), this, SLOT(SL_HyperlinkMouseHovering(QUrl)));
	connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(SL_HyperlinkClicked(QUrl)));
	}

void
WChatLogHtml::_BinAppendHtmlForEvents(IOUT CBin * pbinHtml, IEvent ** ppEventStart, IEvent ** ppEventStop)
	{
	while (ppEventStart < ppEventStop)
		{
		IEvent * pEvent = *ppEventStart++;
		AssertValidEvent(pEvent);

		if (pEvent->m_uFlagsEvent & IEvent::FE_kfEventHidden)
			continue;
		if (pEvent->m_tsEventID >= m_tsMidnightNext)
			{
			QDateTime dtl = QDateTime::fromMSecsSinceEpoch(pEvent->m_tsEventID).toLocalTime();
			QDate date = dtl.date();	// Strip the time of the day
			m_tsMidnightNext = QDateTime(date).toMSecsSinceEpoch() + d_ts_cDays;	// I am sure there is a more elegant way to strip the time from a date, however at the moment I don't have time to investigate a better solution (and this code works)
			QString sDate = date.toString("dddd, MMMM d, yyyy");
			pbinHtml->BinAppendText_VE("<div class='dd'><hr class='dd'/><div class='ddd'>$Q</div></div>", &sDate);
			}

		pbinHtml->BinAppendText("<div class='d'>");
		m_hSenderPreviousEvent = pEvent->AppendHtmlForChatLog_HAppendHeader(IOUT pbinHtml, m_hSenderPreviousEvent);
		pbinHtml->BinAppendText_VE("<span class='m' id='{t_}'>", pEvent->m_tsEventID);
		pEvent->AppendHtmlForChatLog(IOUT pbinHtml);
		pbinHtml->BinAppendText("</span></div>");
		} // while
	} // _BinAppendHtmlForEvents()

const char c_szHtmlMessageDelivered[] = "<img src='qrc:/ico/Delivered' style='float:right'/>";

//	Generate the HTML for the header of a message.  This is typically the icon of the contact who wrote the message, however it may be anything
HOBJECT
IEvent::AppendHtmlForChatLog_HAppendHeader(IOUT CBin * pbinHtml, HOBJECT hSenderPreviousEvent) CONST_MCC
	{
	Assert(pbinHtml != NULL);
	Assert(m_pVaultParent_NZ != NULL);

	const QDateTime dtlMessage = QDateTime::fromMSecsSinceEpoch(m_tsEventID).toLocalTime();
	const QString sTime = dtlMessage.toString("hh:mm");
	const QString sDateTime = dtlMessage.toString(Qt::SystemLocaleLongDate); // DefaultLocaleLongDate);
//	TIMESTAMP_DELTA dts = m_tsOther - m_tsEventID;

	// Determine who is the sender of the event
	ITreeItemChatLog * pTreeItemNickname_NZ;		// This pointer should be a TContact, however if there is a 'bug' where a group message has m_pContactGroupSender_YZ == NULL, then this will point to a group (which is no big deal)
	const EEventClass eEventClass = EGetEventClass();
	const BOOL fuIsEventReceived = (eEventClass & eEventClass_kfReceivedByRemoteClient);
	if (fuIsEventReceived)
		pTreeItemNickname_NZ = (m_pContactGroupSender_YZ != NULL) ? m_pContactGroupSender_YZ : m_pVaultParent_NZ->m_pParent;	// Use the name of the group sender (if any, otherwise the name of the contact)
	else
		pTreeItemNickname_NZ = PGetAccount_NZ();	// Use the name of the user as the sender

	if (pTreeItemNickname_NZ != hSenderPreviousEvent)
		{
		// Insert the image of the sender as well as its name
		pbinHtml->BinAppendText_VE(
			//"<a href='#' class='i i$i'></a>", pEvent->Event_FIsEventTypeSent()	// This line is commented out until hyperlinks are enabled when clicking on the icon of a user
			"<div class='i i$i'></div>"
			"<span class='s'>{sH}</span>"
			 , !fuIsEventReceived, pTreeItemNickname_NZ->ChatLog_PszGetNickname());

		pbinHtml->BinAppendText_VE("<span class='t' title='^Q'>^Q</span>", &sDateTime, &sTime);
		}
	else
		{
		// Just append the timestamp
		pbinHtml->BinAppendText_VE("<span class='tl' title='^Q'>^Q", &sDateTime, &sTime);
		pbinHtml->BinAppendText("</span>");
		}

	if (m_uFlagsEvent & FE_kfEventProtocolError)
		{
		pbinHtml->BinAppendText(" <img src='qrc:/ico/Error' title='XCP Protocol Error: One of your peer has an old version of SocietyPro and cannot process this event because its type is unknown'/> ");
		}

	#if 0
		/*
		// The event was received (typically a message typed by someone else)
		if (dts < -10 * d_ts_cMinutes && dts > -15 * d_ts_cDays)
			pbinHtml->BinAppendText_VE("[$T] ", dts);
		#if 0
		pbinHtml->BinAppendText_VE("<span title='Message was sent {T_} before you received it'>[{T-}] </span>", dts);
		#endif
		*/

	if ((eEventClass & eEventClass_kfReceivedByRemoteClient) == 0)
		{
		// The message was sent by the user
		if (dts > 5 * d_ts_cMinutes)
			pbinHtml->BinAppendText_VE("[$T] ", dts);
		if (Event_FHasCompleted())
			pbinHtml->BinAppendBinaryData(c_szHtmlMessageDelivered, sizeof(c_szHtmlMessageDelivered) - 1);

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
	#endif
	return pTreeItemNickname_NZ;
	} // AppendHtmlForChatLog_HAppendHeader()

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

//	WChatLogHtml::QObject::event()
bool
WChatLogHtml::event(QEvent * pEvent)
	{
	if (pEvent->type() == QEvent::FocusIn)
		m_pContactOrGroup->TreeItem_IconUpdateOnMessagesRead();
	return QWebView::event(pEvent);
	}

#pragma GCC diagnostic ignored "-Wswitch"

//	WChatLogHtml::QTextEdit::contextMenuEvent()
void
WChatLogHtml::contextMenuEvent(QContextMenuEvent * pEventContextMenu)
	{
	QWebView::contextMenuEvent(pEventContextMenu);
	} // contextMenuEvent()

void
WChatLogHtml::SL_HyperlinkMouseHovering(const QUrl & url)
	{
	CStr strTip;
	QString sUrl = url.toString();
	CStr strUrl = sUrl;		// Convert the URL to UTF-8
	PSZUC pszUrl = strUrl;
	if (FIsSchemeCambrian(pszUrl))
		{
		const CHS chCambrianAction = ChGetCambrianActionFromUrl(pszUrl);
		if (chCambrianAction == d_chCambrianAction_None)
			{
			TIMESTAMP tsEventID;
			PSZUC pszAction = Timestamp_PchDecodeFromBase64Url(OUT &tsEventID, pszUrl + 2);
			Assert(pszAction[0] == d_chSchemeCambrianActionSeparator);
			IEvent * pEvent = m_pContactOrGroup->Vault_PFindEventByID(tsEventID);
			if (pEvent != NULL)
				pEvent->HyperlinkGetTooltipText(pszAction + 1, IOUT &strTip);
			else
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "WChatLogHtml::SL_HyperlinkMouseHovering() - Unable to find matching tsEventID $t from hyperlink $s\n", tsEventID, pszUrl);
			} // if
		sUrl = strTip.ToQString();
		}
	QToolTip::showText(QCursor::pos(), sUrl, this);
	} // SL_HyperlinkMouseHovering()

void
WChatLogHtml::SL_HyperlinkClicked(const QUrl & url)
	{
	QString sUrl = url.toString();
	CStr strUrl = sUrl;		// Convert the URL to UTF-8
	PSZUC pszUrl = strUrl;
	MessageLog_AppendTextFormatSev(eSeverityNoise, "WChatLogHtml::SL_HyperlinkClicked($s)\n", pszUrl);
	if (FIsSchemeCambrian(pszUrl))
		{
		const CHS chCambrianAction = ChGetCambrianActionFromUrl(pszUrl);
		if (chCambrianAction == d_chCambrianAction_None)
			{
			TIMESTAMP tsEventID;
			PSZUC pszAction = Timestamp_PchDecodeFromBase64Url(OUT &tsEventID, pszUrl + 2);
			Assert(pszAction[0] == d_chSchemeCambrianActionSeparator);
			IEvent * pEvent = m_pContactOrGroup->Vault_PFindEventByID(tsEventID);
			if (pEvent != NULL)
				{
				EGui eGui = pEvent->HyperlinkClickedE(pszAction + 1);
				if (eGui == eGui_zUpdate)
					ChatLog_EventUpdate(pEvent);
				return;
				}
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "WChatLogHtml::SL_HyperlinkClicked() - Unable to find matching tsEventID $t from hyperlink $s\n", tsEventID, pszUrl);
			}
		else
			{
			if (chCambrianAction == d_chCambrianAction_DisplayAllHistory)
				{
				CWaitCursor wait;
				ChatLog_EventsRepopulate();
				}
			} // if...else
		return;
		} // if
	if (url.scheme() != QLatin1String("file") && !url.isRelative())
		QDesktopServices::openUrl(url);
	} // SL_HyperlinkClicked()

///////////////////////////////////////////////////////////////////////////////////////////////////


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



