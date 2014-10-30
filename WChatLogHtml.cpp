//	WChatLogHtml.cpp

//	Experimental version of WChatLog using an HTML browser allower richer content.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef COMPILE_WITH_CHATLOG_HTML
//#define COMPILE_WITH_CSS_FROM_RESOURCE
//#define INPUT_TEXT_WITH_HTML_FORM			// Use an HTML form to input the text rather than using a widget

#define d_cEventsMaxDefault		500	// By default, display the first 500 events
#define d_nEventsMultiplyBy		3	// Triple the number of events (this gives the illusion of doubling the number of events to fetch)

#define COLOR_THEME_BLACK				// Use a black background instead of a white background
//#define BIG_SPACING_BETWEEN_MESSAGES		// Use big spacing between lines


OJapiChatLog::OJapiChatLog(WChatLogHtml * pwChatLog)
	{
	m_pwChatLog = pwChatLog;
	}

//	pin(), slot
//	Pin the current contact or group to the toolbar
void
OJapiChatLog::pin()
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "OJapiChatLog::pin()\n");
    #ifdef COMPILE_WITH_TOOLBAR
    Toolbar_TabAddAndSelect(m_pwChatLog->m_pContactOrGroup);
    #endif
    }

//	typingStarted(), slot
//	Call this method to indicate the use started typing something.
//	This will send a notification to the contact or group, so the UI may display the pen/composing icon
void
OJapiChatLog::typingStarted()
	{

	}

//	sendMessage(), slot
//	Dispatch the message to the contact or group.
void
OJapiChatLog::sendMessage(const QString & sMessage)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "OJapiChatLog::sendMessage($Q)\n", &sMessage);
	CStr strText = sMessage;
	if (!strText.FIsEmptyString())
		{
		// This code was copied from WChatInput::event()
		ITreeItemChatLogEvents * pContactOrGroup = m_pwChatLog->m_pContactOrGroup;
		EUserCommand eUserCommand = pContactOrGroup->Xmpp_EParseUserCommandAndSendEvents(IN_MOD_INV strText);
		WLayoutChatLog * pwLayoutChatLog = pContactOrGroup->ChatLog_PwGetLayout_YZ();
		Report(pwLayoutChatLog != NULL);	// This pointer should be valid
		if (pwLayoutChatLog != NULL)
			pwLayoutChatLog->m_pwChatInput->ChatStateComposingCancelTimer(eUserCommand);
		}
	}


WChatLogHtml::WChatLogHtml(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup) : QWebView(pwParent), m_oJapi(this)
	{
	Assert(pContactOrGroup != NULL);
	m_pContactOrGroup = pContactOrGroup;
	m_tsMidnightNext = d_ts_zNA;
	m_hSenderPreviousEvent = d_zNA;	// No previous sender yet
	m_cEventsMax = d_cEventsMaxDefault;

	QWebPage * poPage = page();
	poPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	poPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);	// Enable the context menu item named "Inspect".  This is useful for debugging web pages.
	m_poFrame = poPage->mainFrame();
	m_poFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

//	connect(m_poFrame, SIGNAL(initialLayoutCompleted()),this, SLOT(SL_ScrollToDisplayLastEvent()));
	connect(m_poFrame, SIGNAL(contentsSizeChanged(QSize)),this, SLOT(SL_SizeChanged(QSize)));

	CBin binCSS;	// External CSS
	#ifdef COMPILE_WITH_CSS_FROM_RESOURCE
	QString sFileNameCSS = QCoreApplication::applicationDirPath() + "/ChatLog.css";
	binCSS.BinFileReadE(sFileNameCSS);
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Opening file '$Q':\n'$B'\n", &sFileNameCSS, &binCSS);
	if (binCSS.FIsEmptyBinary())
		binCSS.BinFileReadE(":/css/ChatLog.css");
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CSS used:\n'$B'\n", &binCSS);
	#endif
	CBin binFooter;
	QString sFileNameFooter = QCoreApplication::applicationDirPath() + "/Footer.htm";
	binFooter.BinFileReadE(sFileNameFooter);
	#ifdef INPUT_TEXT_WITH_HTML_FORM
	if (binFooter.FIsEmptyBinary())
		binFooter.BinAppendText(
		// Create a footer at the bottom
		"<div id='footer'>"

			// Use an HTML form for the user to type the message
			"<div id='fd'>"
			"<form id='-f-' onSubmit='sendMessage(document.getElementById(\"-i-\"));'>"
				//"<textarea id='-i-' class='' spellcheck='true' style='overflow-y: hidden; height: 38px;'></textarea>"
				"<input id='-i-' spellcheck='true'></input>"
			"</form>"
			"</div>"
		"</div>"
		);
	#endif

	CBin binHtml;
	binHtml.PbbAllocateMemoryAndEmpty_YZ(8*1024);	// Pre-allocate 8 KiB
	binHtml.BinAppendText_VE(
		"<html><head><style>"

		#ifndef COMPILE_WITH_CSS_FROM_RESOURCE
		// Style for each division
		//".d { min-height: 1rem; line-height: 22px; padding: .25rem .1rem .1rem 3rem; }"
		#ifdef BIG_SPACING_BETWEEN_MESSAGES
		"div.d { min-height: 1rem; line-height: 22px; padding: .25rem .1rem .1rem 55px ; }"
		#else
		"div.d { min-height: 1rem; padding: 1 1 1 55; }"
		#endif

		"div.a { margin-top: 6; }"	// Spacing above a sender
		"div.b { height: 4; }"		// Spacing below the sender

		// Style for an icon/avatar
		".i	 { float:left; position: absolute; left: 15; margin-top: 3; height: 36; width: 36; "
			"border-radius: .2rem;"
			"background-size: 100%;"
			"}"

		// Style for the sender of the message
		".s { font-weight: bold; } "

		// Style for the day divider
		"div.dd { "
				"position: relative;"
				"text-align: center;"
				"margin-top: 10;"
				//"height: 50;"
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
			"display: none;"
			"position: absolute;"
			"left: 0;"
			#ifndef BIG_SPACING_BETWEEN_MESSAGES
			"padding-top: 2;"
			#endif
			"text-align: right;"
			"width: 50;"
			"}"
		".d:hover .tl { display: block; }"	// Display the timestamp on the left only if he mouse is over the division

		// Style for  the message itself
		".m {"
			"display: block;"
			//"word-break: break-all;"	// Prevent any horizontal scrolling of messages
			"word-break: break-word;"
			"}"

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

		"#body {"
			"font-family: Lato, sans-serif; font-size: 15px; color: #3D3C40; "
			#ifdef COLOR_THEME_BLACK // Black background style with a 'space' background image
			"color: white; background-color: black; background-image: url('qrc:/backgrounds/Space'); background-attachment: fixed;"
			#endif
			 "}"
		"#footer { position: fixed; bottom: 0; height=100; }"
		"#fd { position: absolute; bottom: 0; left: 52; right: 4px; }"
		"#-f- { height: 41px; }"
		"#-i- { overflow-y: hidden; height: 38px; width: 500; }"
		#endif // COMPILE_WITH_CSS_FROM_RESOURCE

		"\n$B\n"		// Include the external CSS
		"</style>"
		"<script>"
		"function sendMessage(o)"
			"{"
			"SocietyPro.sendMessage(o.value);"	// Request SocietyPro to send the message
			"o.value = '';"	// Clear the value once the message has been sent
			"}"
		"</script>"
		"</head>"
		"<body id='body'>" // Close the <body>

			// Draw the pin at the top right of the page
			"<img src='qrc:/ico/Pin' style='position: fixed; top: 7; right: 8; z-index:1' title='Pin to tab' onClick='SocietyPro.pin();' />"

			// Division for all the messages
			"<div id='-m-'"
			//" style='overflow: scroll;'"
			#ifdef INPUT_TEXT_WITH_HTML_FORM
			//" style='height: 500'"
			#endif
			">", &binCSS);

	//	Append the events
	CArrayPtrEvents arraypEvents;
	m_pContactOrGroup->Vault_GetEventsForChatLog(OUT &arraypEvents);
	IEvent ** ppEventStop;
	IEvent ** ppEventFirst = arraypEvents.PrgpGetEventsStop(OUT &ppEventStop);
	_BinAppendHtmlForEvents(IOUT &binHtml, ppEventFirst, ppEventStop);

	binHtml.BinAppendText_VE(
			"</div>"

			// Division for the composing message
			"<div id='-c-' style='font-size: 13px'/></div>"

			"$B"
			/*
			#ifdef INPUT_TEXT_WITH_HTML_FORM
			// Create a footer at the bottom
			"<div id='footer'>"

				// Use an HTML form for the user to type the message
				"<div id='fd'>"
				"<form id='-f-' onSubmit='sendMessage(document.getElementById(\"-i-\"));'>"
					//"<textarea id='-i-' class='' spellcheck='true' style='overflow-y: hidden; height: 38px;'></textarea>"
					"<input id='-i-' spellcheck='true'></input>"
				"</form>"
				"</div>"
			"</div>"
			#endif
			*/

		"</body></html>", &binFooter);

	setContent(binHtml.ToQByteArrayShared(), "text/html; charset=utf-8");	// Should work, but produces artifacts for special HTML characters
//	_ScrollToDisplayLastEvent();		// This line is necessary in case the setContent() is synchronous

	m_oElementMessages = m_poFrame->findFirstElement("#-m-");
	m_oElementComposing = m_poFrame->findFirstElement("#-c-");

	//m_oElementMessages.setPlainText(c_sEmpty);
	/*
	m_oElementComposing.setPlainText(c_sEmpty);
	QWebElement oElementBody = m_poFrame->findFirstElement("body");
	oElementBody.setPlainText(c_sEmpty);
	*/

	//connect(this, SIGNAL(highlighted(QUrl)), this, SLOT(SL_HyperlinkMouseHovering(QUrl)));
	connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(SL_HyperlinkClicked(QUrl)));

	SL_InitJavaScript();
	connect(m_poFrame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(SL_InitJavaScript()));
	}

WChatLogHtml::~WChatLogHtml()
	{
	//m_oElementMessages.setPlainText(c_sEmpty);	// This reduces 99% of the memory leaks from QWebView, however too late.  I need to find a way to clear the HTML before its destructor.
	}

void
WChatLogHtml::_BinAppendHtmlForEvents(IOUT CBin * pbinHtml, IEvent ** ppEventStart, IEvent ** ppEventStop)
	{
	Assert(pbinHtml != NULL);
	Assert(ppEventStart <= ppEventStop);
	Assert(m_cEventsMax > 0);
	#ifdef DEBUG
	int cbStart = pbinHtml->CbGetData();
	#endif
	int cEvents = ppEventStop - ppEventStart;
	if (cEvents <= 0)
		return;	// Nothing to do (this is the typical case of an empty Chat Log
	IEvent ** ppEventTemp = ppEventStop - m_cEventsMax;	// Limit the number of events to display
	if (ppEventTemp > ppEventStart)
		{
		ppEventStart = ppEventTemp;
		pbinHtml->BinAppendText("<div style='text-align: center; height: 25; margin-top: 20;'><a class='" d_szClassForChatLog_ButtonHtml "' href='" d_SzMakeCambrianAction(d_szCambrianAction_DisplayAllHistory) "'>");
		int cEventsMaxNext = m_cEventsMax * d_nEventsMultiplyBy;
		if (cEventsMaxNext > cEvents)
			pbinHtml->BinAppendText_VE("Display complete chat history of $I messages", cEvents);
		else
			pbinHtml->BinAppendText_VE("Display $I more messages (total $I)", cEventsMaxNext - m_cEventsMax, cEvents);
		pbinHtml->BinAppendText("</a></div>");
		cEvents = m_cEventsMax;
		}

	(void)pbinHtml->PbbAllocateMemoryToGrowBy_NZ(cEvents * 256);	// Pre-allocate some memory, assuming each event will require about 256 bytes
	while (ppEventStart != ppEventStop)
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
			m_hSenderPreviousEvent = d_zNA;	// Force the sender to be re-displayed
			}

		m_hSenderPreviousEvent = pEvent->AppendHtmlForChatLog_HAppendHeader(IOUT pbinHtml, m_hSenderPreviousEvent);
		pbinHtml->BinAppendText_VE("<span class='m' id='{t_}'>", pEvent->m_tsEventID);
		pEvent->AppendHtmlForChatLog(IOUT pbinHtml);
		pbinHtml->BinAppendText("</span></div>");
		} // while

	#ifdef DEBUG
	int cbData = pbinHtml->CbGetData() - cbStart;
	MessageLog_AppendTextFormatSev(eSeverityNoise, "$I events required $I bytes of HTML code ($I bytes per event)\n", cEvents, cbData, cbData / cEvents);
	#endif
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
			"<div class='d a'>"
			"<div class='i i$i'></div>"
			"<span class='s'>{sH}</span>"
			 , !fuIsEventReceived, pTreeItemNickname_NZ->ChatLog_PszGetNickname());
		pbinHtml->BinAppendText_VE("<span class='t' title='^Q'>^Q</span><div class='b'></div>", &sDateTime, &sTime);
		}
	else
		{
		// Just append the timestamp
		pbinHtml->BinAppendText_VE("<div class='d'><span class='tl' title='^Q'>^Q", &sDateTime, &sTime);
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
	m_poFrame->scrollToAnchor("-c-");
	}

void
WChatLogHtml::SL_ScrollToDisplayLastEvent()
	{
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "SL_ScrollToDisplayLastEvent()\n");
	_ScrollToDisplayLastEvent();
	}

void
WChatLogHtml::SL_SizeChanged(const QSize & /*size*/)
	{
	//MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "SL_SizeChanged()\n");
	_ScrollToDisplayLastEvent();
	}

void
WChatLogHtml::ChatLog_EventsAppend(const CArrayPtrEvents & arraypEvents)
	{
	CBin binHtml;
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
	ChatLog_EventsAppend(IN arraypEvents);
	}

void
WChatLogHtml::ChatLog_EventsRepopulate()
	{
	m_oElementMessages.setPlainText(c_sEmpty);	// Clear the previous messages
	CArrayPtrEvents arraypEvents;
	m_pContactOrGroup->Vault_GetEventsForChatLog(OUT &arraypEvents);
	ChatLog_EventsAppend(IN arraypEvents);
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

/*
void
WChatLogHtml::SL_PageLoadFinished(bool)
	{
	//MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "SL_PageLoadFinished()\n");
	_ScrollToDisplayLastEvent();
	}
*/

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
				m_cEventsMax *= d_nEventsMultiplyBy;
				ChatLog_EventsRepopulate();
				}
			} // if...else
		return;
		} // if
	if (url.scheme() != QLatin1String("file") && !url.isRelative())
		QDesktopServices::openUrl(url);
	} // SL_HyperlinkClicked()

void
WChatLogHtml::SL_InitJavaScript()
	{
	m_poFrame->addToJavaScriptWindowObject("SocietyPro", &m_oJapi);
	}

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



