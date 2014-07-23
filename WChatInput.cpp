///////////////////////////////////////////////////////////////////////////////////////////////////
//	WChatInput.cpp
//
//	Display a widget for the user to type text for a Chat Log.
//	Depending on what the user typed, the text may be interpreted as a text message to the user or group, or a command line.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_ttiChatState				1000	// Create a 1-second timer tick interval (tti)
#define d_ttcChatStateEventPaused	10		// Consider the user paused after 10 timer ticks (10 seconds)

WChatInput::WChatInput(WLayoutChatLog * pwLayoutChatLog) : WEditTextArea(pwLayoutChatLog)
	{
	Assert(pwLayoutChatLog != NULL);	// In the future, this may be NULL
	m_pwLayoutChatLog = pwLayoutChatLog;
	ChatInput_UpdateWatermarkText();
	m_tidChatStateComposing = d_zNA;
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
	TContact * pContact = m_pwLayoutChatLog->m_pContactParent_YZ;
	if (pContact != NULL)
		m_strWatermark.Format("Send a message to $s", pContact->ChatLog_PszGetNickname());
	else
		m_strWatermark.Format("Send a message to group $s", m_pwLayoutChatLog->m_pGroupParent_YZ->ChatLog_PszGetNickname());
	}

//	Edit an existing event
void
WChatInput::EditEventText(CEventMessageTextSent * pEventEdit)
	{
	m_pEventEdit = pEventEdit;
	if (pEventEdit != NULL)
		{
		Assert(pEventEdit->EGetEventClass() == eEventClass_eMessageTextSent);
		setPlainText(pEventEdit->m_strMessageText);
		moveCursor(QTextCursor::End);
		setFocus();
		}
	}

void
WChatInput::ChatStateComposingCancelTimer(BOOL fWriteXmlChatStatePaused)
	{
	if (m_tidChatStateComposing != d_zNA)
		{
		killTimer(m_tidChatStateComposing);
		m_tidChatStateComposing = d_zNA;
		}
	m_ttcBeforeChatStatePaused = 0;
	if (fWriteXmlChatStatePaused)
		m_pwLayoutChatLog->Socket_WriteXmlChatState(eChatState_fPaused);	// Notify the remote contact the user stopped typing
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
			{
			// The user typed something other than a special key
			m_ttcBeforeChatStatePaused = d_ttcChatStateEventPaused;
			if (m_tidChatStateComposing == d_zNA)
				{
				m_tidChatStateComposing = startTimer(d_ttiChatState);	// Create a timer to determine when the user 'stopped' typing
				m_pwLayoutChatLog->Socket_WriteXmlChatState(eChatState_zComposing);
				}
			}
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
						eUserCommand = pContactOrGroup->Xmpp_EParseUserCommandAndSendEvents(IN_MOD_INV strText);
					else
						{
						m_pEventEdit->MessageResendUpdate(strText, INOUT m_pwLayoutChatLog);
						m_pEventEdit = NULL;
						}
					ChatStateComposingCancelTimer((BOOL)eUserCommand);	// After sending a message, cancel (reset) the timer to, so a new 'composing' notification will be sent when the user starts typing again. BTW, there is no need to send a 'pause' command since receiving a text message automatically implies a pause.
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


//	WChatInput::QObject::timerEvent()
void
WChatInput::timerEvent(QTimerEvent * pTimerEvent)
	{
//	MessageLog_AppendTextFormatSev(eSeverityNoise, "WChatInput::timerEvent(id=$i)\n", e->timerId());
	if (pTimerEvent->timerId() == m_tidChatStateComposing)
		{
		Assert(m_tidChatStateComposing != d_zNA);
		if (--m_ttcBeforeChatStatePaused <= 0)
			ChatStateComposingCancelTimer(TRUE);	// If the user is idle for too long, then notify the remote contact he/she stopped typing
		}
	WEditTextArea::timerEvent(pTimerEvent);
	} // timerEvent()

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
