///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutChatLog.h

#ifndef WLAYOUTMESSAGELOG_H
#define WLAYOUTMESSAGELOG_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WLayoutChatLog : public WLayout
{
	Q_OBJECT
public:
	// Since the layout is used for contacts and groups, the pointers m_pContactParent_YZ and m_pGroupParent_YZ are mutually exclusive
	ITreeItemChatLogEvents * m_pContactOrGroup_NZ;
	TContact * m_pContactParent_YZ;
	#ifdef COMPILE_WITH_CHATLOG_HTML
	WChatLogHtml * m_pwChatLog_NZ;
	#else
	WChatLog * m_pwChatLog_NZ;	// Widget displaying the chat history between the user and the contact
	#endif
	WChatInput * m_pwChatInput;	// Widget for the user to enter text to send to the contact
protected:
	class WFindText * m_pwFindText;			// Widget to search text within the chat history.  Eventually this widget should be built-in to WChatLog
	WButtonIconForToolbar * m_pwButtonSendBitcoin;
	//WButtonIconForToolbar * m_pwButtonSendFile;
	int m_tidChatStateComposing;						// Timer identifier for the 'chat state'
	int m_ttcBeforeChatStatePaused;						// Number of timer ticks before considering the 'chat state' as paused (when the user stopped typing)

public:
	explicit WLayoutChatLog(ITreeItemChatLogEvents * pContactOrGroupParent);
	inline ITreeItemChatLogEvents * PGetContactOrGroup_NZ() const { return m_pContactOrGroup_NZ; }

	void OnEventFocusIn();
	BOOL FGotFocus();

	PSZUC ChatLog_DisplayStanzaToUser(const CXmlNode * pXmlNodeMessageStanza);
	void ChatLog_EventAppend(IEvent * pEvent);
	void ChatLog_EventsRepopulateUpdateUI();
	void ChatLog_ScrollToDisplayLastMessage();
	void WidgetFindText_Show();
	void WidgetFindText_Hide();
	void WidgetContactInvitation_Show();

	//void Socket_WriteXmlChatState(EChatState eChatState) const;
	void ChatStateComposingTimerStart();
	void ChatStateComposingTimerCancel(EUserCommand eUserCommand);
	virtual void timerEvent(QTimerEvent * pTimerEvent);	// From QObject

//	virtual void paintEvent(QPaintEvent * pPaintEvent);
	virtual void dragEnterEvent(QDragEnterEvent * pDragEnterEvent);
	virtual void dropEvent(QDropEvent * pDropEvent);

	virtual void Layout_NoticeDisplay(IN INotice * piwNotice);	// From WLayout

public slots:
	void SL_ButtonSendBitcoin();
	void SL_ButtonSendFile();
	void SL_ButtonAddContacts();
	void SL_ButtonSendBallot();
}; // WLayoutChatLog


class DDialogSendBitcoin : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
protected:
	WEditNumber * m_pwEditQuantity;
	WEditTextArea * m_pwEditComment;
public:
	DDialogSendBitcoin();
public slots:
	void SL_ButtonSendBitcoins();
};

#define SL_DDialogSendBitcoin(pfmSlot)		SL_DDialog(pfmSlot, DDialogSendBitcoin)


#endif // WLAYOUTMESSAGELOG_H
