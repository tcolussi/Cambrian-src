///////////////////////////////////////////////////////////////////////////////////////////////////
//	WChatInput.h
//
//	Class to display an edit text so the user may input text for chat.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WCHATINPUT_H
#define WCHATINPUT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WChatInput : public WEditTextArea // QPlainTextEdit
{
protected:
	WLayoutChatLog * m_pwLayoutChatLog;	// Widget where input messages are relayed to be dispatched on the server (this pointer is typically the parent widget of WChatInput)
	CEventMessageTextSent * m_pEventEdit;				// Event currently being edited.  This pointer is NULL to send a new message.
	CStr m_strTextLastWritten;							// Cached copy of what was last typed by the user (this is useful in case the user wishes to re-edit the last message)

public:
	explicit WChatInput(WLayoutChatLog * pwLayoutChatLog);
	void ChatInput_UpdateWatermarkText();
	void EditEventText(CEventMessageTextSent * pEventEdit);
	
	virtual QSize minimumSizeHint() const;				// From QWidget
	virtual QSize sizeHint() const;						// From QWidget
	virtual bool event(QEvent * pEvent);				// From QObject
	virtual bool canInsertFromMimeData(const QMimeData * poMimeDataSource) const;
	virtual void insertFromMimeData(const QMimeData * poMimeDataSource);
}; // WChatInput

#endif // WCHATINPUT_H
