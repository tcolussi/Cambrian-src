///////////////////////////////////////////////////////////////////////////////////////////////////
//	WChatLog.cpp
//
//	Widget to display the chat history.
//	Typically the chat history looks very much like Skype, where the sender, message content and time is displayed in a grid format.
//
//	FUTURE IMPLEMENTATIONS: QTextTableFormat, QTextBlock, QTextFrame
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

const QBrush c_brushSilver(d_coSilver);			// Display messages from Cambrian in a silver (light gray) color
const QBrush c_brushGreenSuperPale(d_coGreenSuperPale);
const QBrush c_brushGreenSuperSuperPale(d_coGreenSuperSuperPale);
const QBrush c_brushFileTransfer(d_coBlueSky);
const QBrush c_brushDebugPurple(d_coPurpleLight);

#ifndef COMPILE_WITH_CHATLOG_HTML

WChatLog::WChatLog(QWidget * pwParent, ITreeItemChatLogEvents * pContactOrGroup) : WTextBrowser(pwParent)
	{
	Assert(pContactOrGroup != NULL);
	m_pContactOrGroup = pContactOrGroup;
	m_fDisplayAllMessages = FALSE;
	m_tsMidnightNext = d_ts_zNA;
	setMinimumSize(10, 50);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	setTextInteractionFlags((textInteractionFlags() & ~Qt::TextEditable) | Qt::LinksAccessibleByMouse);
	setUndoRedoEnabled(false);		// setMaximumBlockCount
	setCursorWidth(0);
	//document()->setDefaultStyleSheet("p {text-indent:-10px; margin-left:10px; margin-top:5; margin-bottom:0; white-space:pre-wrap;}");
	document()->setDefaultStyleSheet(
		"a." d_szClassForChatLog_ButtonHtml " { color: black; background-color: silver; text-decoration: none; font-weight: bold } "
		"a." d_szClassForChatLog_HyperlinkDisabled " { color: gray; text-decoration: none; } "

		"a.ha { color: navy; text-decoration: none; } "	// Active Hyperlink (displayed in the normal blue color)
		".d { color: green } "
		//"a.hb { padding: 6px; margin-left:10px; background-color: silver; border-style: outset; border-width: 2px; border-radius: 10px; border-color: black; } "
		//"a.hb:hover { background-color: rgb(224, 0, 0); border-style: inset; }");
		);

	/*
	QTextCursor oTextCursor = textCursor();
	//oTextCursor.select(QTextCursor::Document);
	//oTextCursor.setBlockFormat(QTextBlockFormat());
	oTextCursor.insertBlock();
	m_oTextBlockComposing = oTextCursor.block();

	CArrayPtrEvents arraypEvents;
	pContactOrGroup->Vault_GetEventsForChatLog(OUT &arraypEvents);
	ChatLog_EventsDisplay(IN arraypEvents);
	*/
	ChatLog_EventsRepopulate();

	setOpenLinks(false);	// Prevent the chat history to automatically open the hyperlinks
	connect(this, SIGNAL(highlighted(QUrl)), this, SLOT(SL_HyperlinkMouseHovering(QUrl)));
	connect(this, SIGNAL(anchorClicked(QUrl)), this, SLOT(SL_HyperlinkClicked(QUrl)));
	}

WChatLog::~WChatLog()
	{
	}

//	Flush the message history by flushing all current events
void
WChatLog::ChatLog_EventsRemoveAll()
	{
	clear();
	QTextCursor oTextCursor = textCursor();
	oTextCursor.insertBlock();
	m_oTextBlockComposing = oTextCursor.block();
	}

void
WChatLog::ChatLog_EventUpdate(IEvent * pEvent)
	{
	// Find the text block of the event
	OCursorSelectBlock oCursorEvent(pEvent, this);
	pEvent->ChatLogUpdateTextBlock(INOUT &oCursorEvent);
	ChatLog_ChatStateTextRefresh();
	}

void
WChatLog::ChatLog_EventDisplay(IEvent * pEvent)
	{
	if (pEvent == NULL)
		return;
	CArrayPtrEvents arraypEvents;
	arraypEvents.EventAdd(pEvent);
	ChatLog_EventsDisplay(arraypEvents);
	}

void
WChatLog::ChatLog_EventsDisplay(const CArrayPtrEvents & arraypEvents, int iEventStart)
	{
	Assert(iEventStart >= 0);
	Assert(m_oTextBlockComposing.isValid());
//	CSocketXmpp * pSocket = m_pContactOrGroup->Xmpp_PGetSocketOnlyIfReady();

	OCursorSelectBlock oCursor(m_oTextBlockComposing);
	QTextBlock oTextBlockEvent;	// Text block for each event to insert
	IEvent ** ppEventStop;
	IEvent ** ppEventFirst = arraypEvents.PrgpGetEventsStop(OUT &ppEventStop) + iEventStart;
	IEvent ** ppEvent = ppEventFirst;
	if (!m_fDisplayAllMessages)
		{
		// For performance, limit the display to the last 100 events
		if (ppEventStop != NULL)
			{
			IEvent ** ppEventStart = ppEventStop - 100;
			if (ppEventStart > ppEvent)
				{
				ppEvent = ppEventStart;

				QTextBlockFormat oFormat;
				oFormat.setAlignment(Qt::AlignHCenter);
				oFormat.setBackground(c_brushSilver);	// Use a silver color
				oCursor.setBlockFormat(oFormat);
				g_strScratchBufferStatusBar.Format("<a href='" d_SzMakeCambrianAction(d_szCambrianAction_DisplayAllHistory) "'>Display complete history ($I messages)</a>", arraypEvents.GetSize());
				oCursor.insertHtml(g_strScratchBufferStatusBar);
				oCursor.AppendBlockBlank();
				}
			}
		}

	while (ppEvent < ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		AssertValidEvent(pEvent);
		if (pEvent->m_tsEventID >= m_tsMidnightNext)
			{
			QDateTime dtl = QDateTime::fromMSecsSinceEpoch(pEvent->m_tsEventID).toLocalTime();
			QDate date = dtl.date();	// Strip the time of the day
			m_tsMidnightNext = QDateTime(date).toMSecsSinceEpoch() + d_ts_cDays;	// I am sure there is a more elegant way to strip the time from a date, however at the moment I don't have time to investigate a better solution (and this code works)

			QTextBlockFormat oFormatBlock;
			oFormatBlock.setAlignment(Qt::AlignHCenter);
			oFormatBlock.setBackground(c_brushSilver);
			oCursor.setBlockFormat(oFormatBlock);
			QTextCharFormat oFormatChar; // = oCursor.charFormat();
			oFormatChar.setFontWeight(QFont::Bold);
			//oFormatChar.setFontItalic(true);
			//oCursor.setCharFormat(oFormatChar);
			oCursor.insertText(date.toString("dddd MMMM d, yyyy"), oFormatChar);
			oCursor.AppendBlockBlank();
			}
		oTextBlockEvent = oCursor.block();	// Get the current block under the cursor
		Endorse(oTextBlockEvent.userData() == NULL);

		if (pEvent->m_uFlagsEvent & IEvent::FE_kfReplacing)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "Attempting to replace Event ID $t\n", pEvent->m_tsEventID);
			QTextBlock oTextBlockUpdate;
			QTextBlock oTextBlockTemp = document()->lastBlock();
			IEvent * pEventOld = pEvent;
			const EEventClass eEventClassUpdater = pEvent->Event_FIsEventTypeSent() ? CEventUpdaterSent::c_eEventClass : CEventUpdaterReceived::c_eEventClass;	// Which updater to search for
			// The event is replacing an older event.  This code is a bit complex because the Chat Log may not display all events and therefore we need to find the most recent block displaying the most recent event.
			IEvent ** ppEventStop;
			IEvent ** ppEventFirst = pEvent->m_pVaultParent_NZ->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
			IEvent ** ppEventCompare = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
			while (ppEventFirst < ppEventCompare)
				{
				// Find the updater which should be right before the replacing event
				IEvent * pEventTemp = *--ppEventCompare;
				TryAgain:
				if (pEventTemp == pEventOld && ppEventFirst < ppEventCompare)
					{
					CEventUpdaterSent * pEventUpdater = (CEventUpdaterSent *)*--ppEventCompare;	// Get the updater which is just before the event
					if (pEventUpdater->EGetEventClass() != eEventClassUpdater)
						{
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Missing Updater for Event ID $t ({tL}); instead found class '$U' with Event ID $t.\n", pEventTemp->m_tsEventID, pEventTemp->m_tsEventID, pEventUpdater->EGetEventClass(), pEventUpdater->m_tsEventID);
						pEvent->m_uFlagsEvent &= ~IEvent::FE_kfReplacing;	// Remove the bit to avoid displaying the error again and again
						pEvent->m_pVaultParent_NZ->SetModified();			// Save the change
						continue;
						}
					const TIMESTAMP tsEventIdOld = pEventUpdater->m_tsEventIdOld;
					MessageLog_AppendTextFormatSev(eSeverityNoise, "\t [$i] Found updater: $t  ->  $t\n", ppEventCompare - ppEventFirst, pEventUpdater->m_tsEventIdNew, tsEventIdOld);
					// Now, search for the block containing the replacement event
					while (oTextBlockTemp.isValid())
						{
						OTextBlockUserDataEvent * pUserData = (OTextBlockUserDataEvent *)oTextBlockTemp.userData();
						if (pUserData != NULL)
							{
							TIMESTAMP_DELTA dtsEvent = (pUserData->m_pEvent->m_tsEventID - tsEventIdOld);
							MessageLog_AppendTextFormatCo(d_coPurple, "Comparing block Event ID $t with $t: dtsEvent = $T\n", pUserData->m_pEvent->m_tsEventID, tsEventIdOld, dtsEvent);
							if (dtsEvent <= 0)
								{
								if (dtsEvent == 0)
									{
									MessageLog_AppendTextFormatSev(eSeverityNoise, "\t Found matching textblock for replacement: Event ID $t  ->  $t\n", pEventOld->m_tsEventID, tsEventIdOld);
									oTextBlockUpdate = oTextBlockTemp;
									}
								break;
								}
							}
						oTextBlockTemp = oTextBlockTemp.previous();
						} // while
					// Keep searching in case there are chained updated events
					while (ppEventFirst < ppEventCompare)
						{
						pEventTemp = *--ppEventCompare;
						if (pEventTemp->m_tsEventID == tsEventIdOld)
							{
							MessageLog_AppendTextFormatSev(eSeverityNoise, "\t [$i] Found chained replacement: Event ID $t  -> $t\n", ppEventCompare - ppEventFirst, pEvent->m_tsEventID, tsEventIdOld);
							pEventTemp->m_uFlagsEvent |= IEvent::FE_kfReplaced;
							pEventOld = pEventTemp;
							goto TryAgain;
							}
						} // while
					} // if
				} // while
			if (oTextBlockUpdate.isValid())
				{
				MessageLog_AppendTextFormatSev(eSeverityNoise, "\t Event ID $t is updating its old Event ID $t\n", pEvent->m_tsEventID, pEventOld->m_tsEventID);
				OCursorSelectBlock oCursorEventOld(oTextBlockUpdate);
				pEvent->ChatLogUpdateTextBlock(INOUT &oCursorEventOld);
				continue;
				}
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Event ID $t is replacing another event which cannot be found\n", pEvent->m_tsEventID);
			} // if (replacing)
		oTextBlockEvent.setUserData(PA_CHILD new OTextBlockUserDataEvent(pEvent));		// Assign an event for each text block
		pEvent->ChatLogUpdateTextBlock(INOUT &oCursor);
		if ((pEvent->m_uFlagsEvent & IEvent::FE_kfEventHidden) == 0)
			oCursor.AppendBlockBlank();	// If the event is visible, then add a new text block (otherwise it will reuse the same old block)
		else
			oTextBlockEvent.setUserData(NULL);	// Since we are reusing the same block, delete its userdata so we may assing another OTextBlockUserDataEvent
		} // while
	m_oTextBlockComposing = oCursor.block();
	ChatLog_ChatStateTextAppend(INOUT oCursor);
	Widget_ScrollToEnd(INOUT this);
	} // ChatLog_EventsDisplay()

void
WChatLog::ChatLog_EventsRepopulate()
	{
	ChatLog_EventsRemoveAll();
	CArrayPtrEvents arraypEvents;
	m_pContactOrGroup->Vault_GetEventsForChatLog(OUT &arraypEvents);
	ChatLog_EventsDisplay(IN arraypEvents);
	}

void
TContact::ChatLogContact_AppendExtraTextToChatState(INOUT OCursor & oTextCursor)
	{
	if ((m_uFlagsContact & FC_kfContactNeedsInvitation) == 0)
		return;
	CStr strInvitationLink;
	PSZUC pszInvitationLink = m_pAccount->ChatLog_PszGetInvitationLink(OUT_IGNORED &strInvitationLink);
	PSZUC pszNickname = ChatLog_PszGetNickname();
	if (m_uFlagsContact & FC_kfContactNeedsInvitation)
		{
		// If the user never received any message from the remote contact, then display a notification
		oTextCursor.InsertHtmlCoFormat_VE(0xFFFF80,
			"<br/>So far, you <i>never</i> received any message from <b>^s</b>. "
			"If you believe <b>^s</b> needs your contact information, please send to <b>^s</b> the following invitation link:<br/><br/></br>^s<br/> ",
			pszNickname, pszNickname, pszNickname, pszInvitationLink);
		}
	else
		{
		/*
		oTextCursor.InsertHtmlCoFormat_VE(0xFFAA11, "<br/><b>^s</b> uses a non-secure communication software.  If you wish to send secure messages please recommend <b>^s</b> to download SocietyPro with the following invitation link:<br/><br/></br>^s<br/> ",
			pszNickname, pszNickname, pszInvitationLink);
		*/
		}
	}
QImage * g_poImageComposing;

//	Append the 'chat state' to the cursor
void
WChatLog::ChatLog_ChatStateTextAppend(INOUT OCursor & oTextCursor)
	{
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypContactsComposing.PrgpGetContactsStop(OUT &ppContactStop);
	if (ppContact == ppContactStop)
		{
		oTextCursor.removeSelectedText();
		}
	else
		{
		if (g_poImageComposing == NULL)
			g_poImageComposing = new QImage(":/ico/Pencil");
		while (TRUE)
			{
			oTextCursor.insertImage(*g_poImageComposing);
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			g_strScratchBufferStatusBar.Format(d_szu_nbsp " <b>^s</b> is typing...", pContact->ChatLog_PszGetNickname());
			oTextCursor.insertHtml(g_strScratchBufferStatusBar);
			if (ppContact == ppContactStop)
				break;
			} // while
		}
	if (m_pContactOrGroup->EGetRuntimeClass() == RTI(TContact))
		((TContact *)m_pContactOrGroup)->ChatLogContact_AppendExtraTextToChatState(INOUT oTextCursor);
	Widget_ScrollToEnd(INOUT this);
	} // ChatLog_ChatStateTextAppend()

void
WChatLog::ChatLog_ChatStateTextRefresh()
	{
	OCursorSelectBlock oTextCursor(m_oTextBlockComposing);
	ChatLog_ChatStateTextAppend(INOUT oTextCursor);
	}

void
WChatLog::ChatLog_ChatStateTextUpdate(INOUT TContact * pContact, EChatState eChatState)
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

//	WChatLog::QObject::event()
bool
WChatLog::event(QEvent * pEvent)
	{
	if (pEvent->type() == QEvent::FocusIn)
		m_pContactOrGroup->TreeItem_IconUpdateOnMessagesRead();
	return WTextBrowser::event(pEvent);
	}

#pragma GCC diagnostic ignored "-Wswitch"

//	WChatLog::QTextEdit::contextMenuEvent()
void
WChatLog::contextMenuEvent(QContextMenuEvent * pEventContextMenu)
	{
	//WTextBrowser::contextMenuEvent(pEvent);
	IEvent * pEventSelected = NULL;

	WMenu oMenu;

	// Find out which event the context menu is for
	const QPoint ptEvent = pEventContextMenu->pos();
	QTextCursor oCursor = cursorForPosition(ptEvent);
	OTextBlockUserDataEvent * pUserData = (OTextBlockUserDataEvent *)oCursor.block().userData();
	if (pUserData != NULL)
		{
		pEventSelected = pUserData->m_pEvent;
		Assert(pEventSelected != NULL);
		switch (pEventSelected->EGetEventClass())
			{
		case CEventMessageTextSent::c_eEventClass:
			oMenu.ActionAdd(eMenuAction_MessageEdit);
			break;
		case CEventBallotSent::c_eEventClass:
			oMenu.ActionAdd(eMenuAction_BallotReSend);
			oMenu.ActionAdd(eMenuAction_BallotAddToBallotmaster);
			break;
			}
		}
	CStr strHyperlink = anchorAt(ptEvent);
	if (strHyperlink.PathUrl_FIsValidHyperlinkNonCambrian())
		oMenu.ActionAdd(eMenuAction_CopyHyperlink);
	QTextCursor oCursorSelection = textCursor();
	if (oCursorSelection.hasSelection())
		oMenu.ActionAdd(eMenuAction_Copy);
	oMenu.ActionAdd(eMenuAction_SelectAll);
	oMenu.ActionAdd(eMenuAction_DebugSendChatLog);

	EMenuAction eMenuAction = oMenu.EDisplayContextMenu();
	switch (eMenuAction)
		{
	case eMenuAction_MessageEdit:
		m_pContactOrGroup->ChatLog_EventEditMessageSent((CEventMessageTextSent *)pEventSelected);
		break;
	case eMenuAction_BallotReSend:
		m_pContactOrGroup->DisplayDialogBallotSend((CEventBallotSent *)pEventSelected);
		break;
	case eMenuAction_BallotAddToBallotmaster:
		m_pContactOrGroup->m_pAccount->m_pProfileParent->BallotMaster_EventBallotAddAsTemplate((IEventBallot *)pEventSelected);
		break;
	case eMenuAction_CopyHyperlink:
		Clipboard_SetText(strHyperlink);
		break;
	case eMenuAction_Copy:
		copy();
		break;
	case eMenuAction_SelectAll:
		selectAll();
		break;
	case eMenuAction_DebugSendChatLog:
		m_pContactOrGroup->Vault_SendToJID("chatlogs@xmpp.cambrian.org");
		} // switch

	} // contextMenuEvent()


#if 0
QMenu *QWidgetTextControl::createStandardContextMenu(const QPointF &pos, QWidget *parent)
{
	Q_D(QWidgetTextControl);

	const bool showTextSelectionActions = d->interactionFlags & (Qt::TextEditable | Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

	d->linkToCopy = QString();
	if (!pos.isNull())
		d->linkToCopy = anchorAt(pos);

	if (d->linkToCopy.isEmpty() && !showTextSelectionActions)
		return 0;

	QMenu *menu = new QMenu(parent);
	QAction *a;

	if (d->interactionFlags & Qt::TextEditable) {
		a = menu->addAction(tr("&Undo") + ACCEL_KEY(QKeySequence::Undo), this, SLOT(undo()));
		a->setEnabled(d->doc->isUndoAvailable());
		setActionIcon(a, QStringLiteral("edit-undo"));
		a = menu->addAction(tr("&Redo") + ACCEL_KEY(QKeySequence::Redo), this, SLOT(redo()));
		a->setEnabled(d->doc->isRedoAvailable());
		setActionIcon(a, QStringLiteral("edit-redo"));
		menu->addSeparator();

		a = menu->addAction(tr("Cu&t") + ACCEL_KEY(QKeySequence::Cut), this, SLOT(cut()));
		a->setEnabled(d->cursor.hasSelection());
		setActionIcon(a, QStringLiteral("edit-cut"));
	}

	if (showTextSelectionActions) {
		a = menu->addAction(tr("&Copy") + ACCEL_KEY(QKeySequence::Copy), this, SLOT(copy()));
		a->setEnabled(d->cursor.hasSelection());
		setActionIcon(a, QStringLiteral("edit-copy"));
	}

	if ((d->interactionFlags & Qt::LinksAccessibleByKeyboard)
			|| (d->interactionFlags & Qt::LinksAccessibleByMouse)) {

		a = menu->addAction(tr("Copy &Link Location"), this, SLOT(_q_copyLink()));
		a->setEnabled(!d->linkToCopy.isEmpty());
	}

	if (d->interactionFlags & Qt::TextEditable) {
#if !defined(QT_NO_CLIPBOARD)
		a = menu->addAction(tr("&Paste") + ACCEL_KEY(QKeySequence::Paste), this, SLOT(paste()));
		a->setEnabled(canPaste());
		setActionIcon(a, QStringLiteral("edit-paste"));
#endif
		a = menu->addAction(tr("Delete"), this, SLOT(_q_deleteSelected()));
		a->setEnabled(d->cursor.hasSelection());
		setActionIcon(a, QStringLiteral("edit-delete"));
	}


	if (showTextSelectionActions) {
		menu->addSeparator();
		a = menu->addAction(tr("Select All") + ACCEL_KEY(QKeySequence::SelectAll), this, SLOT(selectAll()));
		a->setEnabled(!d->doc->isEmpty());
	}

	if ((d->interactionFlags & Qt::TextEditable) && qApp->styleHints()->useRtlExtensions()) {
		menu->addSeparator();
		QUnicodeControlCharacterMenu *ctrlCharacterMenu = new QUnicodeControlCharacterMenu(this, menu);
		menu->addMenu(ctrlCharacterMenu);
	}

	return menu;
}
#endif

void
WChatLog::SL_HyperlinkMouseHovering(const QUrl & url)
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
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "WChatLog::SL_HyperlinkMouseHovering() - Unable to find matching tsEventID $t from hyperlink $s\n", tsEventID, pszUrl);
			} // if
		sUrl = strTip.ToQString();
		}
	QToolTip::showText(QCursor::pos(), sUrl, this);
	} // SL_HyperlinkMouseHovering()

void
WChatLog::SL_HyperlinkClicked(const QUrl & url)
	{
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
				{
				#ifndef COMPILE_WITH_CHATLOG_HTML
				OCursorSelectBlock oCursor(pEvent, this);
				pEvent->HyperlinkClicked(pszAction + 1, INOUT &oCursor);
				#endif
				return;
				}
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "WChatLog::SL_HyperlinkClicked() - Unable to find matching tsEventID $t from hyperlink $s\n", tsEventID, pszUrl);
			}
		else
			{
			if (chCambrianAction == d_chCambrianAction_DisplayAllHistory)
				{
				CWaitCursor wait;
				m_fDisplayAllMessages = TRUE;
				ChatLog_EventsRepopulate();
				}
			} // if...else
		return;
		} // if
	if (url.scheme() != QLatin1String("file") && !url.isRelative())
		QDesktopServices::openUrl(url);
	} // SL_HyperlinkClicked()


///////////////////////////////////////////////////////////////////////////////////////////////////

/*void WMainWindow::highlightErrors(int num) {
  QTextCursor tc = ui_.textEdit->textCursor();
  tc.select(QTextCursor::Document);
  QTextCharFormat format;
  QColor lineColor = QColor(Qt::red);
  format.setBackground(lineColor);
  // format.setFontCapitalization(QFont::AllLowercase);
  QList<QTextEdit::ExtraSelection> extraSelections;
  for(int i = 0; i < num; ++i) {
	QTextEdit::ExtraSelection selection;
	selection.format = format;
	selection.format.setToolTip(QString::number(i));
	selection.cursor = tc;
	selection.cursor.clearSelection();
	selection.cursor.setPosition(i);
	selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
	// selection.cursor.mergeCharFormat(selection.format);
	extraSelections.append(selection);
  }
  ui_.textEdit->setExtraSelections(extraSelections);
}
*/
/*
int SpellChecker::spellCheckDocument(QTextEdit* textEdit)
{
   QStringList suggestions;
   QTextCharFormat normalFormat;
   QTextCursor cur = textEdit->textCursor();

   cur.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

   while (getNextWord(&cur) == success)
   {
	  suggestions.clear();

	  if (spellCheckWord(cur.selectedText(), &suggestions) != success)
	  {
		 QTextCharFormat errorFormat;
		 errorFormat.setUnderlineColor(Qt::red);
		 errorFormat.setUnderlineStyle(QTextCharFormat::DotLine);
		 // store current underline info
		 if (cur.charFormat().fontUnderline())
			errorFormat.setProperty(-100, 1);

		 errorFormat.setToolTip(suggestions.join(", "));
		 cur.mergeCharFormat(errorFormat);
	  }
	  else
	  {
		 normalFormat = cur.charFormat();
		 normalFormat.setToolTip("");
		 normalFormat.setUnderlineColor(normalFormat.foreground().color());

		 // restore previous underline, or store current underline if word is correct

		 if (normalFormat.fontUnderline())
			normalFormat.setProperty(-100, 1);
		 else if (normalFormat.hasProperty(-100))
			normalFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
		 else
			normalFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);

		 cur.setCharFormat(normalFormat);
	  }
   }
   return success;
   }
*/

/*
bool MyTextEdit::event(QEvent* event)
{
	if (event->type() == QEvent::ToolTip)
	{
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
		QTextCursor cursor = cursorForPosition(helpEvent->pos());
		cursor.select(QTextCursor::WordUnderCursor);
		if (!cursor.selectedText().isEmpty())
			QToolTip::showText(helpEvent->globalPos(), cursor.selectedText());
		else
			QToolTip::hideText();
		return true;
	}
	return QTextEdit::event(event);
}
*/
/*
#include <QtGui>

class TextEdit : public QTextEdit
{
		Q_OBJECT
public:
		TextEdit(QWidget *parent) : QTextEdit(parent)
		{
				setTextInteractionFlags(Qt::TextBrowserInteraction);
				setFrameStyle(QFrame::NoFrame);
				setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
				setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
				setText("This application only provides a tooltip for the following word: polymorphism");
				QPalette pal = palette();
				pal.setColor(QPalette::Base, QColor(1, 0.941176, 0.941176, 0.941176) );
				setReadOnly(true);
				setPalette(pal);
				setFixedHeight(18);
				setFixedWidth(400);
				setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		}

		bool event(QEvent* event) {
				if (event->type() == QEvent::ToolTip)
				{
						QHelpEvent* helpEvent = static_cast <QHelpEvent*>(event);
						QTextCursor cursor = cursorForPosition(helpEvent->pos());
						cursor.select(QTextCursor::WordUnderCursor);
						if (cursor.selectedText() == "polymorphism")
								QToolTip::showText(helpEvent->globalPos(), cursor.selectedText());
						else
								QToolTip::hideText();
						return true;
				}

				return QTextEdit::event(event);
		}

		void scrollContentsBy ( int dx, int dy  )
		{
		}
};
*/

//	http://www.mimec.org/node/337 (Tooltips for truncated items in a QTreeView)

/*
class AnimatedTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	AnimatedTextEdit(QWidget* parent = 0)
		: QTextEdit(parent) { }

	void addAnimation(const QUrl& url, const QString& fileName)
	{
		QMovie* movie = new QMovie(this);
		movie->setFileName(fileName);
		urls.insert(movie, url);
		connect(movie, SIGNAL(frameChanged(int)), this, SLOT(animate()));
		movie->start();
	}

private slots:
	void animate()
	{
		if (QMovie* movie = qobject_cast<QMovie*>(sender()))
		{
			document()->addResource(QTextDocument::ImageResource,
				urls.value(movie), movie->currentPixmap());
			setLineWrapColumnOrWidth(lineWrapColumnOrWidth()); // causes reload
		}
	}

private:
	QHash<QMovie*, QUrl> urls;
};

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	AnimatedTextEdit textEdit;
	textEdit.setHtml("<img src='bart-url'/><img src='homer-url'/>");
	textEdit.addAnimation(QUrl("bart-url"), "bart.gif");
	textEdit.addAnimation(QUrl("homer-url"), "homer.gif");
	textEdit.show();
	return app.exec();
}
*/


/*
void showInGraphicalShell(QWidget *parent, const QString &pathIn)
{
	// Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
	const QString explorer = QProcessEnvironment::systemEnvironment().searchInPath(QLatin1String("explorer.exe"));
	if (explorer.isEmpty()) {
		QMessageBox::warning(parent,
							 tr("Launching Windows Explorer failed"),
							 tr("Could not find explorer.exe in path to launch Windows Explorer."));
		return;
	}
	QString param;
	if (!QFileInfo(pathIn).isDir())
		param = QLatin1String("/select,");
	param += QDir::toNativeSeparators(pathIn);
	QProcess::startDetached(explorer, QStringList(param));
#elif defined(Q_OS_MAC)
	Q_UNUSED(parent)
	QStringList scriptArgs;
	scriptArgs << QLatin1String("-e")
			   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
									 .arg(pathIn);
	QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
	scriptArgs.clear();
	scriptArgs << QLatin1String("-e")
			   << QLatin1String("tell application \"Finder\" to activate");
	QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
	// we cannot select a file here, because no file browser really supports it...
	const QFileInfo fileInfo(pathIn);
	const QString folder = fileInfo.absoluteFilePath();
	const QString app = Utils::UnixUtils::fileBrowser(Core::ICore::instance()->settings());
	QProcess browserProc;
	const QString browserArgs = Utils::UnixUtils::substituteFileBrowserParameters(app, folder);
	if (debug)
		qDebug() <<  browserArgs;
	bool success = browserProc.startDetached(browserArgs);
	const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
	success = success && error.isEmpty();
	if (!success)
		showGraphicalShellError(parent, app, error);
#endif
}
*/

const QStringList allowedSchemes()
	{
	QStringList qslAllowedSchemeNames;
	qslAllowedSchemeNames << QLatin1String("mumble");
	qslAllowedSchemeNames << QLatin1String("http");
	qslAllowedSchemeNames << QLatin1String("https");
	qslAllowedSchemeNames << QLatin1String("ftp");
	qslAllowedSchemeNames << QLatin1String("clientid");
	qslAllowedSchemeNames << QLatin1String("channelid");
	qslAllowedSchemeNames << QLatin1String("irc");
	qslAllowedSchemeNames << QLatin1String("mailto");
	qslAllowedSchemeNames << QLatin1String("skype");
	return qslAllowedSchemeNames;
	}

class ValidDocument : public QTextDocument
{
	protected:
		QStringList qslValidImage;
		bool bValid;
		QVariant loadResource(int, const QUrl &);
	public:
		ValidDocument(bool httpimages, QObject *p = NULL);
		bool isValid() const;
};

ValidDocument::ValidDocument(bool allowhttp, QObject *p) : QTextDocument(p) {
	bValid = true;
	qslValidImage << QLatin1String("data");
	if (allowhttp) {
		qslValidImage << QLatin1String("http");
		qslValidImage << QLatin1String("https");
	}
}

QVariant ValidDocument::loadResource(int type, const QUrl &url) {
	QVariant v = QLatin1String("PlaceHolder");
	if ((type == QTextDocument::ImageResource) && qslValidImage.contains(url.scheme()))
		return QTextDocument::loadResource(type, url);
	bValid = false;
	return v;
}

bool ValidDocument::isValid() const {
	return bValid;
}
QString validHtml(const QString &html, bool allowReplacement, QTextCursor *tc)
	{
	QDesktopWidget dw;
	ValidDocument oValidDocument(allowReplacement);

	QRectF qr = dw.availableGeometry();
	oValidDocument.setTextWidth(qr.width() / 2);
	oValidDocument.setDefaultStyleSheet(qApp->styleSheet());

	oValidDocument.setHtml(html);
	bool fIsValid = oValidDocument.isValid();

	QStringList qslAllowed = allowedSchemes();
	for (QTextBlock qtb = oValidDocument.begin(); qtb != oValidDocument.end(); qtb = qtb.next())
		{
		for (QTextBlock::iterator qtbi = qtb.begin(); qtbi != qtb.end(); ++qtbi)
			{
			const QTextFragment &qtf = qtbi.fragment();
			QTextCharFormat qcf = qtf.charFormat();
			if (! qcf.anchorHref().isEmpty())
				{
				QUrl url(qcf.anchorHref());
				if (! url.isValid() || ! qslAllowed.contains(url.scheme()))
					{
					QTextCharFormat qcfn = QTextCharFormat();
					QTextCursor qtc(&oValidDocument);
					qtc.setPosition(qtf.position(), QTextCursor::MoveAnchor);
					qtc.setPosition(qtf.position()+qtf.length(), QTextCursor::KeepAnchor);
					qtc.setCharFormat(qcfn);
					qtbi = qtb.begin();
					}
				}
			if (qcf.isImageFormat())
				{
				QTextImageFormat qtif = qcf.toImageFormat();
				QUrl url(qtif.name());
				if (! qtif.name().isEmpty() && ! url.isValid())
					fIsValid = false;
				}
			}
		}

	oValidDocument.adjustSize();
	QSizeF s = oValidDocument.size();

	if (!fIsValid || (s.width() > qr.width()) || (s.height() > qr.height())) {
		oValidDocument.setPlainText(html);
		oValidDocument.adjustSize();
		s = oValidDocument.size();

		if ((s.width() > qr.width()) || (s.height() > qr.height())) {
			QString errorMessage = "[[ Text object too large to display ]]";
			if (tc) {
				tc->insertText(errorMessage);
				return QString();
			} else {
				return errorMessage;
			}
		}
	}

	if (tc) {
		QTextCursor tcNew(&oValidDocument);
		tcNew.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
		tc->insertFragment(tcNew.selection());
		return QString();
	} else {
		return oValidDocument.toHtml();
	}
}

#endif // ~COMPILE_WITH_CHATLOG_HTML
