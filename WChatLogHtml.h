//	WChatLogHtml.h
#ifndef WCHATLOGHTML_H
#define WCHATLOGHTML_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef COMPILE_WITH_CHATLOG_HTML

//	JavaScript APIs available for the HTML Chat Log
class OJapiChatLog : public OJapi
{
	Q_OBJECT
protected:
	WChatLogHtml * m_pwChatLog;

public:
	OJapiChatLog(WChatLogHtml * pwChatLog);

public slots:
	void pin();
	void sendMessage(const QString & sMessage);
};

class WChatLogHtml : public QWebView
{
	Q_OBJECT
protected:
	ITreeItemChatLogEvents * m_pContactOrGroup;
	CArrayPtrContacts m_arraypContactsComposing;	// All the users currently composing text (typically there is only one, however for group chat, there may be several)
	QWebFrame * m_poFrame;
	QWebElement m_oElementMessages;			// Reference of the HTML node to append the HTML messages
	QWebElement m_oElementComposing;		// Where to display the HTML text of the user(s) composing text
	TIMESTAMP m_tsMidnightNext;				// Timestamp to determine when to display a cate change in the Chat Log.
	HOBJECT m_hSenderPreviousEvent;			// Handle of the sender who wrote the last event.
	int m_cEventsMax;						// Maximum number of events to display.  This variable is important for performance to quickly the Chat Log without the entire history.
	OJapiChatLog m_oJapi;

public:
	explicit WChatLogHtml(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup);
	virtual ~WChatLogHtml();

	void _BinAppendHtmlForEvents(IOUT CBin * pbinHtml, IEvent ** ppEventStart, IEvent ** ppEventStop);
	void _ScrollToDisplayLastEvent();

	void ChatLog_EventsAppend(const CArrayPtrEvents & arraypEvents);
	void ChatLog_EventAppend(IEvent * pEvent);
	void ChatLog_EventUpdate(IEvent * pEvent);
	void ChatLog_EventsRepopulate();
	void ChatLog_ChatStateTextRefresh();
	void ChatLog_ChatStateTextUpdate(INOUT TContact * pContact, EChatState eChatState);
	inline void ChatLog_ChatStateComposerRemovePointerOnly(TContact * pContact) { m_arraypContactsComposing.RemoveElementI(pContact); }

	virtual bool event(QEvent * pEvent);	// From QObject
	virtual void contextMenuEvent(QContextMenuEvent * pEventContextMenu); // From QTextEdit

public slots:
	void SL_ScrollToDisplayLastEvent();
	void SL_SizeChanged(const QSize & size);
	void SL_HyperlinkMouseHovering(const QUrl & url);
	void SL_HyperlinkClicked(const QUrl & url);
	void SL_InitJavaScript();

	friend class OJapiChatLog;
}; // WChatLogHtml

#endif

#endif // WCHATLOGHTML_H
