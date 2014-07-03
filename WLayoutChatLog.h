///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutChatLog.h

#ifndef WLAYOUTMESSAGELOG_H
#define WLAYOUTMESSAGELOG_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WChatLog.h"
#include "WChatInput.h"


class WLayoutChatLog : public WLayout
{
	Q_OBJECT
public:
	// Since the layout is used for contacts and groups, the pointers m_pContactParent_YZ and m_pGroupParent_YZ are mutually exclusive
	TContact * m_pContactParent_YZ;
	TGroup * m_pGroupParent_YZ;
	WChatLog * m_pwChatLog_NZ;	// Widget displaying the chat history between the user and the contact
	WChatInput * m_pwChatInput;	// Widget for the user to enter text to send to the contact
protected:
	class WFindText * m_pwFindText;			// Widget to search text within the chat history.  Eventually this widget should be built-in to WChatLog
	WButtonIcon * m_pwButtonSendBitcoin;
	//WButtonIcon * m_pwButtonSendFile;

public:
	explicit WLayoutChatLog(ITreeItemChatLogEvents * pParent);
	inline ITreeItemChatLogEvents * PGetContactOrGroup_NZ() const { return (ITreeItemChatLogEvents *)((INT_P)m_pContactParent_YZ | (INT_P)m_pGroupParent_YZ); }	// // This cast is not elegant, however it is my response to the compiler error: conditional expression between distinct pointer types lacks a cast

	void OnEventFocusIn();
	BOOL FGotFocus();

	PSZUC ChatLog_DisplayStanzaToUser(const CXmlNode * pXmlNodeMessageStanza);
	void ChatLog_EventAppend(IEvent * pEvent);
	void ChatLog_EventsRepopulateUpdateUI();
	void ChatLog_ScrollToDisplayLastMessage();
	void WidgetFindText_Show();
	void WidgetFindText_Hide();
	void WidgetContactInvitation_Show();

	void Socket_WriteXmlChatState(EChatState eChatState) const;
	void TreeItem_UpdateIconMessageRead();

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
