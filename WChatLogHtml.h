//	WChatLogHtml.h
#ifndef WCHATLOGHTML_H
#define WCHATLOGHTML_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef COMPILE_WITH_CHATLOG_HTML

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

public:
	explicit WChatLogHtml(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup);

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
	void SL_HyperlinkMouseHovering(const QUrl & url);
	void SL_HyperlinkClicked(const QUrl & url);
};
#endif

#endif // WCHATLOGHTML_H
