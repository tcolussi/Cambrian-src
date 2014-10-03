#ifndef WMESSAGELOG_H
#define WMESSAGELOG_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WChatLog : public WTextBrowser
{
	Q_OBJECT
protected:
	ITreeItemChatLogEvents * m_pContactOrGroup;
	CArrayPtrContacts m_arraypContactsComposing;	// All the users currently composing text (typically there is only one, however for group chat, there may be several)
	QTextBlock m_oTextBlockComposing;				// Text block displaying the names of users composing text
	BOOL m_fDisplayAllMessages;
	TIMESTAMP m_tsMidnightNext;						// Timestamp to determine when to display the next data in the Chat Log.

public:
	explicit WChatLog(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup);
	virtual ~WChatLog();

	void ChatLog_EventsRemoveAll();
	void ChatLog_EventUpdate(IEvent * pEvent);
	void ChatLog_EventDisplay(IEvent * pEvent);
	void ChatLog_EventsDisplay(const CArrayPtrEvents & arraypEvents, int iEventStart = 0);
	void ChatLog_EventsRepopulate();
	void ChatLog_ChatStateTextAppend(INOUT OCursor & oTextCursor);
	void ChatLog_ChatStateTextRefresh();
	void ChatLog_ChatStateTextUpdate(INOUT TContact * pContact, EChatState eChatState);
	inline void ChatLog_ChatStateComposerRemovePointerOnly(TContact * pContact) { m_arraypContactsComposing.RemoveElementI(pContact); }

	virtual bool event(QEvent * pEvent);	// From QObject	
	virtual void contextMenuEvent(QContextMenuEvent * pEventContextMenu); // From QTextEdit

public slots:
	void SL_HyperlinkMouseHovering(const QUrl & url);
	void SL_HyperlinkClicked(const QUrl & url);
}; // WChatLog


	#define d_coBluePastel			MAKE_QRGB(220, 220, 255)
	#define d_coBluePastelLight		MAKE_QRGB(235, 235, 255)

	#define d_coGreenPale			MAKE_QRGB(152, 251, 152)
	#define d_coGreenPastel			MAKE_QRGB(119, 221, 119)
	/*
	#define d_coGreenSuperPale		MAKE_QRGB(182, 251, 182)
	#define d_coGreenSuperSuperPale		MAKE_QRGB(200, 251, 200)
	*/
	/*
	#define d_coGreenSuperPale		MAKE_QRGB(220, 251, 220)
	#define d_coGreenSuperSuperPale		MAKE_QRGB(210, 251, 210)
	*/
	/*
	#define d_coGreenSuperPale		MAKE_QRGB(220, 255, 220)
	#define d_coGreenSuperSuperPale	MAKE_QRGB(210, 255, 210)
	*/
	/*
	#define d_coGreenSuperPale		MAKE_QRGB_(0xE2FFD2) //0xB6FF8C)
	#define d_coGreenSuperSuperPale	MAKE_QRGB_(0xDBFFC7)
	*/
	#define d_coGreenSuperPale		MAKE_QRGB_(0xDBFFC7)
	#define d_coGreenSuperSuperPale	MAKE_QRGB_(0xE4FFCF)

	extern const QBrush c_brushSilver;

#endif // WMESSAGELOG_H
