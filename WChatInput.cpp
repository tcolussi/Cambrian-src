///////////////////////////////////////////////////////////////////////////////////////////////////
//	WChatInput.cpp
//
//	Display a widget for the user to type text for a Chat Log.
//	Depending on what the user typed, the text may be interpreted as a text message to the user or group, or a command line.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

WChatInput::WChatInput(WLayoutChatLog * pwLayoutChatLog) : WEditTextArea(pwLayoutChatLog)
	{
	Assert(pwLayoutChatLog != NULL);	// In the future, this may be NULL
	m_pwLayoutChatLog = pwLayoutChatLog;
	ChatInput_UpdateWatermarkText();
	m_pEventEdit = NULL;
	/*
	setMinimumSize(10, 10);
	resize(100, 10);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	*/
	setAcceptDrops(false);	// Prevent the input widget to accept drag & drops because its parent, the WLayoutChatLog, already handles drag & drops for both the input widget and the message history.
	}

void
WChatInput::ChatInput_UpdateWatermarkText()
	{
	m_strWatermark.Format((m_pwLayoutChatLog->m_pContactParent_YZ != NULL) ? "Send a message to $s" : "Send a message to group $s", m_pwLayoutChatLog->m_pContactOrGroup_NZ->ChatLog_PszGetNickname());
	}

//	Edit an existing event
void
WChatInput::EditEventText(CEventMessageTextSent * pEventEdit)
	{
	m_pEventEdit = pEventEdit;
	if (m_pEventEdit != NULL)
		{
		Assert(pEventEdit->EGetEventClass() == eEventClass_eMessageTextSent);
		m_pEventEdit = pEventEdit->PFindEventMostRecent_NZ();
		Assert(m_pEventEdit->EGetEventClass() == eEventClass_eMessageTextSent);
		setPlainText(m_pEventEdit->m_strMessageText);	// Use text from the most recent message
		moveCursor(QTextCursor::End);
		setFocus();
		}
	}

//	WChatInput::QWidget::minimumSizeHint()
QSize
WChatInput::minimumSizeHint() const
	{
	return QSize(0, 32); // fontMetrics().height() * 10);
	}

//	WChatInput::QWidget::sizeHint()
QSize
WChatInput::sizeHint() const
	{
	return minimumSizeHint();
	}

#define d_kmKeyboardSpecialKeys		0xFF000000	// Mask of the special keys (escape, arrows, F1..F12) from the enum Qt::Key

//	WChatInput::QObject::event()
bool
WChatInput::event(QEvent * pEvent)
	{
	QEvent::Type eEventType = pEvent->type();
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "WChatInput::event(type=$i)\n", eEventType);
	if (eEventType == QEvent::KeyPress)
		{
		QKeyEvent * pEventKey = static_cast<QKeyEvent *>(pEvent);
		const Qt::Key eKey = (Qt::Key)pEventKey->key();
		//MessageLog_AppendTextFormatSev(eSeverityNoise, "WChatInput::event(QEvent::KeyPress, key=0x$p, modifiers=$x)\n", eKey, pEventKey->modifiers());
		if ((eKey & d_kmKeyboardSpecialKeys) == 0)
			m_pwLayoutChatLog->ChatStateComposingTimerStart();	// The user typed something other than a special key, therefore assume he/she started typing
		if ((pEventKey->modifiers() & ~Qt::KeypadModifier) == Qt::NoModifier)
			{
			ITreeItemChatLogEvents * pContactOrGroup = m_pwLayoutChatLog->PGetContactOrGroup_NZ();
			pContactOrGroup->TreeItem_IconUpdateOnMessagesRead();	// Any key pressed in the message input assumes the user read the message history
			//m_pwLayoutChatLog->TreeItem_UpdateIconMessageRead();	// Any key pressed in the message input assumes the user read the message history
			if (eKey == Qt::Key_Enter || eKey == Qt::Key_Return)
				{
				EUserCommand eUserCommand = eUserCommand_ComposingStopped;	// Pretend the text message was sent
				CStr strText = *this;
				if (!strText.FIsEmptyString())
					{
					m_strTextLastWritten = strText;
					if (m_pEventEdit == NULL)
						eUserCommand = pContactOrGroup->Xmpp_EParseUserCommandAndSendEvents(IN_MOD_INV strText);	// This is the typical case of a new message
					else
						{
						// The user was editing an existing message
						m_pEventEdit->EventUpdateMessageText(strText, INOUT m_pwLayoutChatLog);
						m_pEventEdit = NULL;
						}
					m_pwLayoutChatLog->ChatStateComposingTimerCancel(eUserCommand);	// After sending a message, cancel (reset) the timer to, so a new 'composing' notification will be sent when the user starts typing again. BTW, there is no need to send a 'pause' command since receiving a text message automatically implies a pause.
					} // if

				if (eUserCommand != eUserCommand_Error)
					{
					clear();	// Clear the chat text
					setCurrentCharFormat(QTextCharFormat());
					}
				return true;
				} // if (enter)
			if (eKey == Qt::Key_Up)
				{
				// Set the previous text if there is notning in the edit box
				CStr strText = *this;
				if (strText.FIsEmptyString())
					{
					//EditEventText(pContactOrGroup->Vault_PGetEventLastMessageSentEditable_YZ());
					m_pEventEdit = pContactOrGroup->Vault_PFindEventLastMessageTextSentMatchingText(IN m_strTextLastWritten);
					if (m_pEventEdit != NULL)
						{
						MessageLog_AppendTextFormatSev(eSeverityNoise, "Editing Event ID $t\n", m_pEventEdit->m_tsEventID);
						}
					setPlainText(m_strTextLastWritten);
					moveCursor(QTextCursor::End);
					return true;
					}
				}
			if (eKey == Qt::Key_Escape)
				{
				m_pEventEdit = NULL;
				clear();	// Clear the chat text
				return TRUE;
				}
			} // if
		}
	else if (eEventType == QEvent::FocusIn)
		{
		//m_pwLayoutChatLog->OnEventFocusIn();
		m_pwLayoutChatLog->PGetContactOrGroup_NZ()->TreeItem_IconUpdateOnMessagesRead();
		}
	return WEditTextArea::event(pEvent);
	} // event()

bool
WChatInput::canInsertFromMimeData(const QMimeData * poMimeDataSource) const
	{
	//return poMimeDataSource->hasImage() || WEditTextArea::canInsertFromMimeData(poMimeDataSource);
	return WEditTextArea::canInsertFromMimeData(poMimeDataSource);
	}

void
WChatInput::insertFromMimeData(const QMimeData * poMimeDataSource)
	{
	if (poMimeDataSource->hasImage())
		{
		textCursor().insertImage(qvariant_cast<QImage>(poMimeDataSource->imageData()));
		}
	if (poMimeDataSource->hasText())
		{
		CStr strText = poMimeDataSource->text();
		MessageLog_AppendTextFormatCo(d_coBlack, "Paste: '{Sf}'\n", &strText);
		}
	return WEditTextArea::insertFromMimeData(poMimeDataSource);
	}
