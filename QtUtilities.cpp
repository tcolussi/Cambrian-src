///////////////////////////////////////////////////////////////////////////////////////////////////
//	QtUtilities.cpp
//
//	Various function wrappers for the Qt framework.
//	This file also contains 'scratch' sample code to use later.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

WWidget::WWidget(PA_PARENT QWidget * pwParent) : QWidget(PA_PARENT pwParent)
	{
	}

//	Detach the widget from its parent.
//	If the widget was visible on the GUI, it will no longer be visible because it has no parent.
//	The caller is now responsible of deleting the object, otherwise there is a memory leak.
void
WWidget::Widget_DetachFromParent()
	{
	PA_CAST_FROM_P setParent(NULL);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLabel::WLabel()
	{
	_Init();
	}

WLabel::WLabel(PSZAC pszmLabelTextAndToolTip)
	{
	_Init();
	Label_SetTextWithToolTip(pszmLabelTextAndToolTip);
	}

WLabel::WLabel(const CStr & strLabelText)
	{
	_Init();
	Label_SetTextPlain(strLabelText);
	}

WLabel::WLabel(PA_PARENT QBoxLayout * poParentLayout)
	{
	Assert(poParentLayout);
	poParentLayout->addWidget(PA_CHILD this);
	_Init();
	}

void
WLabel::_Init()
	{
	setTextFormat(Qt::RichText);	// By default, a label displays HTML text
	setFocusPolicy(Qt::ClickFocus);	// By default, labels do not capture the focus by tabbing; only if the user explicitly clicks on the control
	}

void
WLabel::Label_SetTextSelectable()
	{
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
	}

void
WLabel::Label_SetTextSelectableAndWordWrap()
	{
	Label_SetTextSelectable();
	setWordWrap(true);
	}

void
WLabel::Label_SetTextPlain(const CStr & strText)
	{
	setTextFormat(Qt::PlainText);
	setText(strText);
	}

void
WLabel::Label_SetTextWithToolTip(PSZAC pszmLabelTextAndToolTip)
	{
	PSZAC pszToolTip = PszroGetToolTipNext(pszmLabelTextAndToolTip);
	setText(QString::fromUtf8(pszmLabelTextAndToolTip, pszToolTip - pszmLabelTextAndToolTip));
	if (pszToolTip != NULL && *pszToolTip != '\0')
		setToolTip(QString::fromUtf8(pszToolTip + 1));
	}

void
WLabel::Label_SetTextFormat_VE_Gsb(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	Label_SetTextFormat_VL_Gsb(pszFmtTemplate, vlArgs);
	}

void
WLabel::Label_SetTextFormat_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	setText(g_strScratchBufferStatusBar);
	}

void
WLabel::Label_SetTextColorError()
	{
	::Widget_SetTextColorError(INOUT this);
	}

void
WLabel::Label_SetTextColorGray()
	{
	::Widget_SetTextColorGray(INOUT this);
	}

/*
//	Unfortunately I have no idea how to adjust the indentation of an HTML list, so I will use a negative padding.
//	This method has to be used with care because any item which are not within the list will be clipped by 20 pixels (in other words, it works only for an HTML list, and no other combination)
void
WLabel::Label_SetStyleSheetForHtmlListItems()
	{
	//setStyleSheet("padding-left: -20px");
	}
*/
void
WLabel::Label_SetTextHtmlListItems_Gsb(const CArrayPsz & arraypszListItems)
	{
	/*
	Label_SetStyleSheetForHtmlListItems();
	g_strScratchBufferStatusBar.BinInitFromText("<div style='margin:-30px;'><ul>");
	g_strScratchBufferStatusBar.BinAppendHtmlListItems(arraypszListItems);
	g_strScratchBufferStatusBar.BinAppendStringWithNullTerminator("</ul></div>");
	setText(g_strScratchBufferStatusBar);
	*/
	Label_SetTextFormat_VE_Gsb("^L", &arraypszListItems);
	}

WLabelSelectable::WLabelSelectable()
	{
	Label_SetTextSelectable();
	}

WLabelSelectable::WLabelSelectable(PSZAC pszmLabelTextAndToolTip) : WLabel(pszmLabelTextAndToolTip)
	{
	Label_SetTextSelectable();
	}

WLabelSelectable::WLabelSelectable(const CStr & strLabelText) : WLabel(strLabelText)
	{
	Label_SetTextSelectable();
	}

WLabelSelectable::WLabelSelectable(PA_PARENT QBoxLayout * poParentLayout) : WLabel(poParentLayout)
	{
	Label_SetTextSelectable();
	}

WLabelSelectableWrap::WLabelSelectableWrap()
	{
	setWordWrap(true);
	}

WLabelSelectableWrap::WLabelSelectableWrap(PSZAC pszmLabelTextAndToolTip) : WLabelSelectable(pszmLabelTextAndToolTip)
	{
	setWordWrap(true);
	}

WLabelSelectableWrap::WLabelSelectableWrap(const CStr & strLabelText) : WLabelSelectable(strLabelText)
	{
	setWordWrap(true);
	}

WLabelSelectableWrap::WLabelSelectableWrap(PA_PARENT QBoxLayout * poParentLayout) : WLabelSelectable(poParentLayout)
	{
	setWordWrap(true);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	See also Widget_SetIconButton()
WButtonIconForToolbar::WButtonIconForToolbar(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon) : QToolButton(pwParent)
	{
	_Init(eMenuIcon);
	}
WButtonIconForToolbar::WButtonIconForToolbar(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon)
	{
	Assert(poParentLayout != NULL);
	poParentLayout->addWidget(PA_CHILD this);
	_Init(eMenuIcon);
	}

WButtonIconForToolbar::WButtonIconForToolbar(EMenuIcon eMenuIcon, PSZAC pszmToolTip)
	{
	_Init(eMenuIcon);
	Assert(!FTextContainsTooltip(pszmToolTip));	// The text is already the tooltip, so there is no need to include the tooltip separator
	setToolTip(pszmToolTip);
	}

void
WButtonIconForToolbar::_Init(EMenuIcon eMenuIcon)
	{
	const QIcon oIcon = OGetIcon(eMenuIcon);
	Assert(!oIcon.isNull());
	setIcon(oIcon);
	setIconSize(oIcon.actualSize(QSize(32, 32)));
	setStyleSheet("QAbstractButton { border: none; }"); //  :hover {border: 1px} :pressed {border: 1px}");
	setFocusPolicy(Qt::ClickFocus);	// Icons do not capture the focus by tabbing; only if the user explicitly clicks on the icon
	}

WButtonIconForToolbarWithDropDownMenu::WButtonIconForToolbarWithDropDownMenu(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon, PSZAC pszmButtonTextAndToolTip) : WButtonIconForToolbar(pwParent, eMenuIcon)
	{
	setText(pszmButtonTextAndToolTip);
	_InitStyleForDropdownMenu();
	}

WButtonIconForToolbarWithDropDownMenu::WButtonIconForToolbarWithDropDownMenu(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon, PSZAC pszmButtonTextAndToolTip)  : WButtonIconForToolbar(poParentLayout, eMenuIcon)
	{
	setText(pszmButtonTextAndToolTip);
	_InitStyleForDropdownMenu();
	}

void
WButtonIconForToolbarWithDropDownMenu::_InitStyleForDropdownMenu(BOOL fHideArrowOfDropDownMenu)
	{
	setStyleSheet(fHideArrowOfDropDownMenu ?
		"QToolButton { border: none; } QToolButton::menu-indicator { image: none; }" :
		"QToolButton { border: none; padding: 1 10 1 1; }");		// Add 10 pixels to the left for the drop down arrow
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	setPopupMode(QToolButton::InstantPopup);
	setMenu(PA_CHILD new WMenu);
	}

WButtonIconForToolbarWithDropDownMenuNoArrow::WButtonIconForToolbarWithDropDownMenuNoArrow(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon) : WButtonIconForToolbarWithDropDownMenu(pwParent, eMenuIcon, NULL)
	{
	_InitStyleForDropdownMenu(TRUE);
	}

WButtonIconForToolbarWithDropDownMenuNoArrow::WButtonIconForToolbarWithDropDownMenuNoArrow(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon) : WButtonIconForToolbarWithDropDownMenu(poParentLayout, eMenuIcon, NULL)
	{
	_InitStyleForDropdownMenu(TRUE);
	}


WButtonText::WButtonText(PSZAC pszmButtonTextAndToolTip)
	{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);	// By default buttons should take as little vertical space as possible
	//setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	Button_SetTextAndToolTip(pszmButtonTextAndToolTip);
	}

void
WButtonText::Button_SetTextAndToolTip(PSZAC pszmButtonTextAndToolTip)
	{
	WidgetButton_SetTextAndToolTip(INOUT this, pszmButtonTextAndToolTip);
	/*
	PSZAC pszToolTip = PszroGetToolTipNext(pszmButtonTextAndToolTip);
	setText(QString::fromUtf8(pszmButtonTextAndToolTip, pszToolTip - pszmButtonTextAndToolTip));
	if (pszToolTip != NULL && *pszToolTip != '\0')
		setToolTip(QString::fromUtf8(pszToolTip + 1));
	*/
	}

void
WButtonText::Button_SetIcon(EMenuIcon eMenuIcon)
	{
	Widget_SetIconButton(OUT this, eMenuIcon);
	}

WButtonTextWithIcon::WButtonTextWithIcon(PSZAC pszmButtonTextAndToolTip, EMenuIcon eMenuIcon) : WButtonText(pszmButtonTextAndToolTip)
	{
	Button_SetIcon(eMenuIcon);
	}

WButtonCheckbox::WButtonCheckbox()
	{
	}

WButtonCheckbox::WButtonCheckbox(PSZAC pszmButtonTextAndToolTip, BOOL fChecked)
	{
	WidgetButton_SetTextAndToolTip(INOUT this, pszmButtonTextAndToolTip);
	setChecked(fChecked);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WEdit::WEdit()
	{
	}

WEdit::WEdit(PSZUC pszText)
	{
	Edit_SetTextU(pszText);
	}

WEdit::WEdit(const CStr & strText)
	{
	Edit_SetTextU(strText);
	}

WEdit::WEdit(const QString & sText) : QLineEdit(sText)
	{
	setAttribute(Qt::WA_LayoutUsesWidgetRect); // For Mac
	}

void
WEdit::Edit_SetText(const CStr & strText)
	{
	setText(strText);
	}

void
WEdit::Edit_SetTextU(PSZUC pszText)
	{
	setText(QString::fromUtf8((const char *)pszText));
	}
void
WEdit::Edit_SetTextFromNumber(int nNumber)
	{
	setText(QString::number(nNumber));
	}

void
WEdit::Edit_SetToolTip(PSZUC pszToolTip)
	{
	setToolTip(QString::fromUtf8((const char *)pszToolTip));
	}

void
WEdit::Edit_SetWatermark(PSZAC pszWatermark)
	{
	setPlaceholderText(QString::fromUtf8((const char *)pszWatermark));
	}

void
WEdit::EditFingerprint_Set(const CBin & binFingerprint)
	{
	g_strScratchBufferStatusBar.InitFromFingerprintBinary(binFingerprint);
	Edit_SetText(g_strScratchBufferStatusBar);
	}

BOOL
WEdit::EditFingerprint_FGetMB(OUT_F_UNCH CBin * pbinFingerprint)
	{
	Assert(pbinFingerprint != NULL);
	//MessageLog_AppendTextFormatCo(d_coBlack, "EditFingerprint__FGetUI() $S\n", &strFingerprint);
	CBin binFingerprint;
	if (binFingerprint.BinAppendBinaryFingerprintF_MB(IN this))
		{
		pbinFingerprint->BinInitFromCBinStolen(INOUT &binFingerprint);
		return TRUE;
		}
	setFocus();
	return FALSE;
	}

BOOL
WEdit::EditUsername_FGet_MB(OUT_F_UNCH CStr * pstrUsername)
	{
	Assert(pstrUsername != NULL);
	CStr strUsername = *this;
	int cchUsername = strUsername.CchGetLength();
	if (cchUsername > 0)
		{
		if (cchUsername < 256)
			{
			PCHERROR pcheError = PcheValidateUsername(strUsername);
			if (pcheError == NULL)
				{
				*pstrUsername = strUsername;
				return TRUE;
				}
			EMessageBoxWarning("The username <b>^S</b> contains an illegal character <b>{s1}</b>", &strUsername, pcheError);
			}
		else
			EMessageBoxWarning("The username must be less than 256 characters");
		}
	else
		EMessageBoxWarning("Please enter a username");
	setFocus();
	return FALSE;
	}

BOOL
WEdit::EditPassword_FGet_MB(OUT_F_UNCH CStr * pstrPassword)
	{
	Assert(pstrPassword != NULL);
	CStr strPassword = *this;
	if (strPassword.CchGetLength() >= 6)
		{
		pstrPassword->BinInitFromCBinStolen(INOUT &strPassword);
		return TRUE;
		}
	EMessageBoxWarning("The password must be at least 6 characters");
	setFocus();	// Set the focus to the edit control so the user may type the password
	return FALSE;
	}

BOOL
WEdit::EditServerName_FGet_MB(OUT_F_UNCH CStr * pstrServerName)
	{
	CStr strServerName = *this;
	if (strServerName.FValidateServerName_MB())
		{
		pstrServerName->BinInitFromCBinStolen(INOUT &strServerName);
		return TRUE;
		}
	setFocus();
	return FALSE;
	}

BOOL
CStr::FValidateServerName_MB() const
	{
	PSZUC pszServerName = PszuGetStringNullIfEmpty();
	if (pszServerName != NULL)
		{
		PCHERROR pcheError = PcheValidateServername(pszServerName);
		if (pcheError == NULL)
			return TRUE;
		EMessageBoxWarning("The servername <b>^s</b> contains an illegal character <b>{s1}</b>", pszServerName, pcheError);
		}
	else
		EMessageBoxWarning("Please enter the name or IP address of the server");
	return FALSE;
	}

BOOL
CStr::FValidateJID_MB() const
	{
	Assert(FALSE && "NYI");
	return TRUE;
	}


WEditReadOnly::WEditReadOnly()
	{
	setReadOnly(true);
	}

WEditReadOnly::WEditReadOnly(PSZUC pszText) : WEdit(pszText)
	{
	setReadOnly(true);
	}

WEditReadOnly::WEditReadOnly(const CStr & strText) : WEdit(strText)
	{
	setReadOnly(true);
	}

WEditReadOnly::WEditReadOnly(const QString & sText) : WEdit(sText)
	{
	setReadOnly(true);
	}

WEditPassword::WEditPassword()
	{
	_Init();
	}

WEditPassword::WEditPassword(PSZUC pszPassword)
	{
	_Init();
	Edit_SetTextU(pszPassword);
	}
void
WEditPassword::_Init()
	{
	setEchoMode(Password);
	setFixedWidth(d_cxWidthEditPassword);
	}

WEditFingerprint::WEditFingerprint()
	{
	_Init();
	}

WEditFingerprint::WEditFingerprint(const CBin & binFingerprint, PSZAC pszToolTip)
	{
	_Init();
	EditFingerprint_Set(binFingerprint);
	Edit_SetToolTip((PSZUC)pszToolTip);
	}

void
WEditFingerprint::_Init()
	{
	setMinimumWidth(300);	// 300 pixels is enough to display an entire SHA-1 fingerprint with separators
	}


WEditNumber::WEditNumber()
	{
	_Init();
	}

WEditNumber::WEditNumber(int nValue)
	{
	_Init();
	Edit_SetTextFromNumber(nValue);
	}

void
WEditNumber::_Init()
	{
	setFixedWidth(d_cxWidthEditNumber);
	}

WEditSearch::WEditSearch()
	{
	QToolButton * m_pwButtonSearch;
	m_pwButtonSearch = new QToolButton(this);
	QPixmap pixmap(":/ico/Find");
	m_pwButtonSearch->setIcon(QIcon(pixmap));
//	m_pwButtonSearch->setIconSize(pixmap.size());
	m_pwButtonSearch->setCursor(Qt::ArrowCursor);
	m_pwButtonSearch->setStyleSheet("QToolButton { border: none; padding: 1 1 1 3; }");	// Pad 3 pixels to the left
	m_pwButtonSearch->setFocusPolicy(Qt::NoFocus);

	//int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	//setStyleSheet(QString("QLineEdit { border: none; padding-left: %1px; } ").arg(m_pwButtonSearch->sizeHint().width() + frameWidth + 1));
	setStyleSheet("QLineEdit { border: none; padding: 2px 2px 2px 20px; } ");
	//setStyleSheet("QLineEdit { padding-left: 5px; } ");
	setPlaceholderText("Search");
//	setFocusPolicy(Qt::ClickFocus);	// Prevent the edit control to receive the focus when the application starts
	//setMinimumSize(40, 30);

	// See http://aseigo.blogspot.com/2006/08/sweep-sweep-sweep-ui-floor.html for improvement of the search
	}

WEditSearch::WEditSearch(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon, PSZAC pszWatermark)
	{
	Assert(poParentLayout != NULL);
	poParentLayout->addWidget(PA_CHILD this);
	QToolButton * pwButtonSearch = new WButtonIconForToolbar(this, eMenuIcon);
	pwButtonSearch->setCursor(Qt::ArrowCursor);
	pwButtonSearch->setStyleSheet("QToolButton { border: none; padding: 3 1 1 3; }");	// Pad 3 pixels to the left
	//pwButtonSearch->setFocusPolicy(Qt::NoFocus);

	setStyleSheet("QLineEdit { padding: 2px 2px 2px 20px; } ");
	Edit_SetWatermark(pszWatermark);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WEditTextArea::WEditTextArea()
	{
	_InitTextArea();
	}

WEditTextArea::WEditTextArea(QWidget * pwParent) : QTextEdit(pwParent)
	{
	_InitTextArea();
	}

WEditTextArea::WEditTextArea(const CStr & strText)
	{
	_InitTextArea();
	Edit_SetText(strText);
	}

WEditTextArea::WEditTextArea(const QString & sText)
	{
	_InitTextArea();
	setText(sText);	// We must call the method setText() and not use the constructor of QTextEdit() otherwise the new line characters ('\n') are lost
	}

WEditTextArea::WEditTextArea(int cLines)
	{
	_InitTextArea();
	Edit_SetHeightLines(cLines);
	}

void
WEditTextArea::_InitTextArea()
	{
	setTabChangesFocus(true);
	Edit_SetHeightRecommended(d_cyHeightEditTextArea);
	setTabStopWidth(16);
	}

void
WEditTextArea::Edit_SetColorBackgroundToGray()
	{
	setStyleSheet("background-color: #EEE");
	}

void
WEditTextArea::Edit_SetHeightLines(int cLines)
	{
	setFixedHeight(1 + cLines * 16);	// Assume 16 pixels per text line
	}

void
WEditTextArea::Edit_SetHeightRecommended(int cyHeightRecommended)
	{
	m_cyHeightRecommended = cyHeightRecommended;
	//MessageLog_AppendTextFormatCo(d_coBlack, "minimumHeight = $i\n", minimumHeight());
	setMinimumHeight(cyHeightRecommended / 2);	// By default, set the minimum height to half of the recommended height
	}

//	WEditTextArea::QWidget::sizeHint()
QSize
WEditTextArea::sizeHint() const
	{
	QSize sizeRecommended = QTextEdit::sizeHint();
	if (m_cyHeightRecommended > 0)
		sizeRecommended.setHeight(m_cyHeightRecommended);
	return sizeRecommended;
	}


/*
	if (d->control->text().isEmpty()) {
		if (!hasFocus() && !d->placeholderText.isEmpty()) {
			QColor col = pal.text().color();
			col.setAlpha(128);
			QPen oldpen = p.pen();
			p.setPen(col);
			lineRect.adjust(minLB, 0, 0, 0);
			QString elidedText = fm.elidedText(d->placeholderText, Qt::ElideRight, lineRect.width());
			p.drawText(lineRect, va, elidedText);
			p.setPen(oldpen);
			return;
		}
*/

//	WEditTextArea::QWidget::paintEvent()
void
WEditTextArea::paintEvent(QPaintEvent * pEvent)
	{
	if (!m_strWatermark.FIsEmptyString())
		{
		// This code was inspired from Src\qtbase\src\widgets\widgets\qlineedit.cpp, QLineEdit::paintEvent()
		if (toPlainText().isEmpty())
			{
			QPainter oPainter(viewport());
			oPainter.setPen(d_coGray);
//			oPainter.setBackgroundMode(Qt::TransparentMode);
			oPainter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_strWatermark);	// Draw plain text

			/*
			// This is grossly inefficient, however it enables the drawing of HTML text, however the text is not centered nor displayed in gray
			QTextDocument oDocument;
			oDocument.setHtml(m_strWatermark);
			oDocument.drawContents(&oPainter, rect());
			*/
	/*
	QString text_x = "Hello World!";
	doc = new QTextDocument(this);
	doc->setUndoRedoEnabled(false);
	doc->setHtml("<p><span style=\"color:#000000; font-weight:600\">"+text_x+"</span></p>");
	doc->setTextWidth(width());
	doc->setUseDesignMetrics(true);
	doc->setDefaultTextOption ( QTextOption (Qt::AlignHCenter )  );
	//// height from doc QTextDocument
	//// http://fop-miniscribus.googlecode.com/svn/trunk/fop_miniscribus.1.0.0/src/floating_box/floatdiagram.cpp
	//////setMaximumHeight(DocumentHighgtActual());
	QPainter *p = new QPainter(&PicsDocument);
	p->setRenderHint(QPainter::Antialiasing, true);
	doc->drawContents(p,clip);
	p->end();
	///// QPixmap PicsDocument contains image as html
	*/
/*
QTextLayout l( "test" );
l.beginLayout();
QTextLine line = l.createLine();
line.setLineWidth( 100 );
line.setPosition( QPointF( 0, 14 ) );
l.endLayout();
l.draw( qPainter, QPointF( 100, 100 ) );
*/			if (!hasFocus())
				return;	// If it has the focus, continue to have the blinking cursor
			}
		}
	QTextEdit::paintEvent(pEvent);
	}

void
WEditTextArea::Edit_SetText(const CStr & strText)
	{
	setText(strText);
	}

void
WEditTextArea::Edit_SetTextU(PSZUC pszText)
	{
	setText(QString::fromUtf8((const char *)pszText));
	}

void
WEditTextArea::Edit_SetWatermark(PSZAC pszWatermark)
	{
	m_strWatermark.InitFromStringU((PSZUC)pszWatermark);
	}

void
WEditTextArea::Widget_SetHeightEqualToLayout(QLayout * poLayout)
	{
	setFixedHeight(poLayout->sizeHint().height());
	}

WEditTextAreaReadOnly::WEditTextAreaReadOnly()
	{
	_InitReadOnly();
	}

WEditTextAreaReadOnly::WEditTextAreaReadOnly(const QString & sText) : WEditTextArea(sText)
	{
	_InitReadOnly();
	}

void
WEditTextAreaReadOnly::_InitReadOnly()
	{
	setReadOnly(true);
	setFocusPolicy(Qt::ClickFocus);	// By default, read-only text area do not capture the focus by tabbing; only if the user explicitly clicks on the control
	}

WEditTextAreaReadOnlyGray::WEditTextAreaReadOnlyGray()
	{
	Edit_SetColorBackgroundToGray();
	/*
	QPalette Pal(palette());
	// set black background
	Pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(Pal);
	*/
	}

WEditMessageLog::WEditMessageLog()
	{
	Edit_SetWatermark("Message Log");
	}

WEditMessageLogHidden::WEditMessageLogHidden()
	{
	hide();
	}

WEditMessageLogGrayHidden::WEditMessageLogGrayHidden()
	{
	hide();
	Edit_SetColorBackgroundToGray();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
const QTextBlockFormat c_oTextBlockFormatBlank;		// Empty/blank QTextBlockFormat.  This is necessary because otherwise the method insertBlock() will use the same formatting as the previous block

OCursor::OCursor(const QTextCursor & oCursor) : QTextCursor(oCursor)
	{
	}
OCursor::OCursor(const QTextBlock & oTextBlock) : QTextCursor(oTextBlock)
	{
	}
void
OCursor::PositionMoveToEnd()
	{
	movePosition(QTextCursor::End);
	}
void
OCursor::AppendBlockBlank()
	{
	movePosition(QTextCursor::End);
	insertBlock(c_oTextBlockFormatBlank);
	}
void
OCursor::InsertBlockBlank()
	{
	insertBlock(c_oTextBlockFormatBlank);
	}

void OCursor::InsertHtmlBin(const CBin & binHtml)
	{
	insertHtml(binHtml.ToQString());
	}
void
OCursor::InsertHtmlStr(const CStr & strHtml)
	{
	insertHtml(strHtml);
	}

void
OCursor::InsertHtmlBin(const CBin & binHtml, const QBrush & oBrushColorBackground)
	{
	QTextBlockFormat oTextBlockFormat;
	oTextBlockFormat.setBackground(oBrushColorBackground);
	setBlockFormat(oTextBlockFormat);
	insertHtml(binHtml.ToQString());
	}

void
OCursor::InsertHtmlStr(const CStr & strHtml, const QBrush & oBrushColorBackground)
	{
	QTextBlockFormat oTextBlockFormat;
	oTextBlockFormat.setBackground(oBrushColorBackground);
	setBlockFormat(oTextBlockFormat);
	insertHtml(strHtml);
	}

void
OCursor::InsertHtmlCoFormat_VE(QRGBX coxBackground, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	InsertHtmlStr(g_strScratchBufferStatusBar, QBrush(coxBackground));
	}

OCursorSelectBlock::OCursorSelectBlock(const QTextBlock & oTextBlock) : OCursor(oTextBlock)
	{
	movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	}

#ifndef COMPILE_WITH_CHATLOG_HTML
OCursorSelectBlock::OCursorSelectBlock(IEvent * pEvent, QTextEdit * pwEdit) : OCursor(pEvent->ChatLog_GetTextBlockRelatedToWidget(pwEdit))
	{
	movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	}
#endif

OCursorMoveToEnd::OCursorMoveToEnd(const QTextCursor & oCursor) : OCursor(oCursor)
	{
	movePosition(QTextCursor::End);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WGroupBox::WGroupBox(PA_PARENT QWidget * pwParentWidget) : QGroupBox(PA_PARENT pwParentWidget)
	{
	}
WGroupBox::WGroupBox(PA_PARENT QSplitter * pwParentSplitter)
	{
	pwParentSplitter->addWidget(PA_CHILD this);
	}
WGroupBox::WGroupBox(PA_PARENT QBoxLayout * poParentLayout)
	{
	poParentLayout->addWidget(PA_CHILD this);
	}

void
WGroupBox::Widget_SetTitleFormat_VE_Gsb(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	Widget_SetTitleFormat_VL_Gsb(pszFmtTemplate, vlArgs);
	}
void
WGroupBox::Widget_SetTitleFormat_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	setTitle(g_strScratchBufferStatusBar);
	}

//	Set the title color to display an error
void
WGroupBox::Widget_SetTitleColorError()
	{
	setStyleSheet("QGroupBox { color: maroon; }");	// The maroon color is better than red because it is a bit darker
	//setStyleSheet("color: maroon");
	//setStyleSheet("QGroupBox::title { color: maroon; }");
/*
pwGroupBox->setStyleSheet(
	"QGroupBox { "
		"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);"
		"border: 2px solid gray;"
		"border-radius: 5px;"
		"margin-top: 1ex; } " // leave space at the top for the title

	 "QGroupBox::title { "
		 "subcontrol-origin: margin;"
		 "subcontrol-position: top center;" // position at the top center
		 "padding: 0 3px;"
		 "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF0ECE, stop: 1 #FFFFFF); }");
*/
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WSplitter::Splitter_SetMarginForCustomLayout()
	{
	setContentsMargins(5, 5, 5, 0);
	}

WGroupBox *
WSplitter::Splitter_PwAddGroupBox_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	Splitter_SetMarginForCustomLayout();
	WGroupBox * pawGroupBox = new WGroupBox(PA_PARENT this);
	pawGroupBox->Widget_SetTitleFormat_VL_Gsb(pszFmtTemplate, vlArgs);
//	addWidget(PA_CHILD pawGroupBox);
	return pawGroupBox;
	}

WGroupBox *
WSplitter::Splitter_PwAddGroupBox_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	return Splitter_PwAddGroupBox_VL(pszFmtTemplate, vlArgs);
	}

OLayoutForm *
WSplitter::Splitter_PoAddGroupBoxAndLayoutForm_VL(OUT_OPTIONAL WGroupBox ** ppwGroupBox, PSZAC pszFmtTemplate, va_list vlArgs)
	{
	WGroupBox * pwGroupBox = Splitter_PwAddGroupBox_VL(pszFmtTemplate, vlArgs);
	if (ppwGroupBox != NULL)
		*ppwGroupBox = pwGroupBox;
	return new OLayoutForm(PA_PARENT pwGroupBox);
	}

OLayoutForm *
WSplitter::Splitter_PoAddGroupBoxAndLayoutForm_VE(OUT_OPTIONAL WGroupBox ** ppwGroupBox, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	return Splitter_PoAddGroupBoxAndLayoutForm_VL(OUT_OPTIONAL ppwGroupBox, pszFmtTemplate, vlArgs);
	}

OLayoutForm *
WSplitter::Splitter_PoAddGroupBoxAndLayoutForm_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	return Splitter_PoAddGroupBoxAndLayoutForm_VL(NULL, pszFmtTemplate, vlArgs);
	}

OLayoutForm *
WSplitter::Splitter_PoAddGroupBoxAndLayoutFormError_V(PSZAC pszFmtTemplate, ...)
	{
	WGroupBox * pwGroupBox;
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	OLayoutForm * poLayout = Splitter_PoAddGroupBoxAndLayoutForm_VL(OUT &pwGroupBox, pszFmtTemplate, vlArgs);
	pwGroupBox->Widget_SetTitleColorError();
	return poLayout;
	}

OLayoutHorizontal *
WSplitter::Splitter_PoAddGroupBoxAndLayoutHorizontal_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	WGroupBox * pwGroupBox = Splitter_PwAddGroupBox_VL(pszFmtTemplate, vlArgs);
	return new OLayoutHorizontal(PA_PARENT pwGroupBox);
	}

OLayoutVerticalAlignTop *
WSplitter::Splitter_PoAddGroupBoxAndLayoutVertical_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	WGroupBox * pwGroupBox = Splitter_PwAddGroupBox_VL(pszFmtTemplate, vlArgs);
	return new OLayoutVerticalAlignTop(PA_PARENT pwGroupBox);
	}


OLayoutForm *
WSplitter::Splitter_PoAddWidgetAndLayoutForm()
	{
	QWidget * pawWidgetParent = new QWidget;	// We need to create a widget first, and add the form within, otherwise it will work
	addWidget(PA_CHILD pawWidgetParent);
	OLayoutForm * paLayout = new OLayoutForm(PA_PARENT pawWidgetParent);
	paLayout->Layout_RemoveMargins();	// By default, a layout within a QSplitter does not have any margins
	return paLayout;
	}

WEditTextAreaReadOnly *
WSplitter::Splitter_PwAddEditTextAreaReadOnly(PSZAC pszLabel, const CString & sEditText)
	{
	WEditTextAreaReadOnly * pawEdit = new WEditTextAreaReadOnly(sEditText);
//	setMaximumHeight(500);
//	pawEdit->setMaximumHeight(500);
	if (pszLabel != NULL)
		{
		// We have a label, so we must add it as a layout
		OLayoutForm * pLayout = Splitter_PoAddWidgetAndLayoutForm();
//		pLayout->setSizeConstraint(QLayout::SetNoConstraint);
		pLayout->addRow(PA_CHILD new WLabelSelectable(pszLabel));
		pLayout->addRow(PA_CHILD pawEdit);
		}
	else
		{
		addWidget(PA_CHILD pawEdit);
		}
	return pawEdit;
	}

void
WSplitter::Splitter_AddWidgetSpacer()
	{
	addWidget(PA_CHILD new QWidget);	// Add an empty widget to fill the bottom space
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WDocking::WDocking() : QDockWidget(g_pwMainWindow)
	{
	}

void
WDocking::Widget_SetTitleFormat_VE_Gsb(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	setWindowTitle(g_strScratchBufferStatusBar);
	}

WDockingSplitter::WDockingSplitter()
	{
	m_pSplitter = new WSplitterVertical;
	setWidget(PA_CHILD m_pSplitter);
	}
void
WDockingSplitter::Widget_ResizeTocontentOfSplitter()
	{
	resize(m_pSplitter->size());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
OLayoutHorizontal::OLayoutHorizontal(PA_PARENT QWidget * pwParent) : QHBoxLayout(PA_PARENT pwParent)
	{
	}
OLayoutHorizontal::OLayoutHorizontal(PA_PARENT QBoxLayout * poParentLayout)
	{
	Assert(poParentLayout != NULL);
	poParentLayout->addLayout(PA_CHILD this);
	}
void
OLayoutHorizontal::Layout_MarginsClear()
	{
	::Layout_MarginsClear(INOUT this);
	}

OLayoutVertical *
OLayoutHorizontal::Layout_PoAddLayoutVerticalWithWidgets_VLZA(QWidget * pawWidgetFirst, va_list vlaArgs)
	{
	OLayoutVertical * pLayout = new OLayoutVerticalAlignTop(PA_PARENT this);
	pLayout->Layout_AddWidgetsAndResizeWidths_VLZA(pawWidgetFirst, vlaArgs);
	return pLayout;
	}

OLayoutVertical *
OLayoutHorizontal::Layout_PoAddLayoutVerticalWithWidgets_VEZA(QWidget * pawWidgetFirst, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pawWidgetFirst);
	return Layout_PoAddLayoutVerticalWithWidgets_VLZA(pawWidgetFirst, vlArgs);
	}

/*
//	This method adds all the buttons at once, and a spacer at the bottom to prevent the buttons to stretch when the layout is resized
//	INTERFACE NOTES
//	The parameter pawButtonFirst is of type QWidget because the method works for widget, however mostly used for buttons.
OLayoutVertical *
OLayoutHorizontal::Layout_PoAddLayoutVerticalWithButtons_VEZA(QWidget * pawButtonFirst, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pawButtonFirst);
	OLayoutVertical * pLayout = Layout_PoAddLayoutVerticalWithWidgets_VLZA(pawButtonFirst, vlArgs);
	pLayout->Layout_AddWidgetSpacer();	// This line may no longer be necessary because the layout may be top aligned
	return pLayout;
	}
*/

void
OLayoutHorizontal::Layout_AddWidgetsH_VEZA(QWidget * pawWidgetFirst, ...)
	{
	Assert(pawWidgetFirst != NULL);
	va_list vlArgs;
	va_start(OUT vlArgs, pawWidgetFirst);
	while (TRUE)
		{
		addWidget(PA_CHILD pawWidgetFirst);
		pawWidgetFirst = va_arg(vlArgs, QWidget *);
		if (pawWidgetFirst == NULL)
			break;
		}
	}

void
OLayoutHorizontal::Layout_AddLabelAndWidgetH_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget)
	{
	Assert(pawWidget != NULL);
	addWidget(PA_CHILD new WLabel(pszmLabelAndToolTip));
	addWidget(PA_CHILD pawWidget);
	Widget_SetToolTipNext(INOUT pawWidget, pszmLabelAndToolTip);
	}

void
OLayoutHorizontal::Layout_AddLabelAndWidgetV_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget)
	{
	Assert(pawWidget != NULL);
	OLayoutVertical * pLayout = new OLayoutVerticalAlignTop(PA_PARENT this);
	pLayout->setSpacing(2);
	pLayout->addWidget(PA_CHILD new WLabel(pszmLabelAndToolTip));
	pLayout->addWidget(PA_CHILD pawWidget);
	Widget_SetToolTipNext(INOUT pawWidget, pszmLabelAndToolTip);
	}

void
OLayoutHorizontal::Layout_AddLabelsAndWidgetsV_VLZA(PSZAC pszmFirstLabelAndToolTip, va_list vlaArgs)
	{
	while (TRUE)
		{
		Layout_AddLabelAndWidgetV_PA(pszmFirstLabelAndToolTip, va_arg(vlaArgs, QWidget *));
		pszmFirstLabelAndToolTip = va_arg(vlaArgs, PSZAC);
		if (pszmFirstLabelAndToolTip == NULL)
			break;
		}
	}

void
OLayoutHorizontal::Layout_AddLabelsAndWidgetsV_VEZA(PSZAC pszmFirstLabelAndToolTip, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszmFirstLabelAndToolTip);
	Layout_AddLabelsAndWidgetsV_VLZA(pszmFirstLabelAndToolTip, vlArgs);
	}

OLayoutHorizontalAlignLeft::OLayoutHorizontalAlignLeft(PA_PARENT QWidget * pwParent) : OLayoutHorizontal(PA_PARENT pwParent)
	{
	setAlignment(Qt::AlignLeft);
	}
OLayoutHorizontalAlignLeft::OLayoutHorizontalAlignLeft(PA_PARENT QBoxLayout * poParentLayout) : OLayoutHorizontal(PA_PARENT poParentLayout)
	{
	setAlignment(Qt::AlignLeft);
	}
OLayoutHorizontalAlignLeft0::OLayoutHorizontalAlignLeft0(PA_PARENT QWidget * pwParent) : OLayoutHorizontalAlignLeft(PA_PARENT pwParent)
	{
	Layout_MarginsClear();
	}
OLayoutHorizontalAlignLeft0::OLayoutHorizontalAlignLeft0(PA_PARENT QBoxLayout * poParentLayout) : OLayoutHorizontalAlignLeft(PA_PARENT poParentLayout)
	{
	Layout_MarginsClear();
	}

OLayoutHorizontalAlignRight::OLayoutHorizontalAlignRight(PA_PARENT QBoxLayout * poParentLayout) : OLayoutHorizontal(PA_PARENT poParentLayout)
	{
	setAlignment(Qt::AlignRight);
	}

OLayoutVertical::OLayoutVertical(PA_PARENT QWidget * pwParent) : QVBoxLayout(PA_PARENT pwParent)
	{
	}

OLayoutVertical::OLayoutVertical(PA_PARENT QBoxLayout * poParentLayout)
	{
	Assert(poParentLayout != NULL);
	poParentLayout->addLayout(PA_CHILD this);
	}
void
OLayoutVertical::Layout_MarginsClear()
	{
	::Layout_MarginsClear(INOUT this);
	}
void
OLayoutVertical::Layout_MarginsClearAndSpacing()
	{
	::Layout_MarginsClear(INOUT this);
	setSpacing(0);
	}

QWidget *
OLayoutVertical::Layout_PwGetWidgetAt(int iWidget) const
	{
	QLayoutItem * pItem = itemAt(iWidget);
	if (pItem != NULL)
		return pItem->widget();
	return NULL;
	}

//	 Maybe use QDialogButtonBox
#define d_cxWidthExtra		4	// Add 2 pixels on each end of the widget/button

//	See also: Widgets_ResizeWidths_VEZ();
void
OLayoutVertical::Layout_ResizeWidthsOfAllWidgets()
	{
	QWidget * pwWidget = Layout_PwGetWidgetAt(0);
	if (pwWidget == NULL)
		return;	// There is nothing to resize
	int cxWidthMinimum = Widget_GetWidthMinimumRecommended(pwWidget);
	int iWidget = 0;
	while (TRUE)
		{
		pwWidget = Layout_PwGetWidgetAt(++iWidget);
		if (pwWidget == NULL)
			break;
		int cxWidth = Widget_GetWidthMinimumRecommended(pwWidget);
		if (cxWidthMinimum < cxWidth)
			cxWidthMinimum = cxWidth;	// Expand to fit the largest widget
		}
	iWidget = 0;
	while (TRUE)
		{
		pwWidget = Layout_PwGetWidgetAt(iWidget++);
		if (pwWidget == NULL)
			break;
		pwWidget->setFixedWidth(cxWidthMinimum + d_cxWidthExtra);
		}
	}

//	Display several widgets vertically, all having the same width.
//	This method is very useful to display several buttons and making sure they are all the same width.
void
OLayoutVertical::Layout_AddWidgetsAndResizeWidths_VLZA(QWidget * pawWidgetFirst, va_list vlArgs)
	{
	setSpacing(3);	// Separate the widgets/buttons by 3 pixels
	va_list vaListCopy;	// Make a copy of the original arguments because we need to iterate twice
	va_copy(OUT vaListCopy, IN vlArgs);
	int cxWidthMinimum = Widget_GetWidthMinimumRecommended(pawWidgetFirst);
	QWidget * pwWidget = pawWidgetFirst;
	while (TRUE)
		{
		addWidget(PA_CHILD pwWidget); // , 0, Qt::AlignTop);
		pwWidget = va_arg(vlArgs, QWidget *);
		if (pwWidget == NULL)
			break;
		int cxWidth = Widget_GetWidthMinimumRecommended(pwWidget);
		//MessageLog_AppendTextFormatCo(d_coBlack, "cxWidth = $i\n", cxWidth);
		if (cxWidthMinimum < cxWidth)
			cxWidthMinimum = cxWidth;	// Expand to fit the largest widget
		} // while
	pwWidget = pawWidgetFirst;
	while (TRUE)
		{
		//MessageLog_AppendTextFormatCo(d_coBlack, "Setting widget 0x$p width to $i\n", pwWidget, cxWidthMinimum);
		//WidgetSetWidth(INOUT pwWidget, cxWidthMinimum);
		pwWidget->setFixedWidth(cxWidthMinimum + d_cxWidthExtra);
		pwWidget = va_arg(vaListCopy, QWidget *);
		if (pwWidget == NULL)
			break;
		} // while
	} // Layout_AddWidgetsAndResizeWidths_VLZA()

void
OLayoutVertical::Layout_AddWidgetsAndResizeWidths_VEZA(QWidget * pawWidgetFirst, ...)
	{
	Assert(pawWidgetFirst != NULL);
	va_list vlArgs;
	va_start(OUT vlArgs, pawWidgetFirst);
	Layout_AddWidgetsAndResizeWidths_VLZA(pawWidgetFirst, vlArgs);
	}

OLayoutHorizontalAlignLeft *
OLayoutVertical::Layout_PoAddRowLabelsAndWidgets_VEZA(PSZAC pszmFirstLabelAndToolTip, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszmFirstLabelAndToolTip);
	return Layout_PoAddRowLabelsAndWidgets_VLZA(pszmFirstLabelAndToolTip, vlArgs);
	}

OLayoutHorizontalAlignLeft *
OLayoutVertical::Layout_PoAddRowLabelsAndWidgets_VLZA(PSZAC pszmFirstLabelAndToolTip, va_list vlaArgs)
	{
	OLayoutHorizontalAlignLeft * pLayout = new OLayoutHorizontalAlignLeft(PA_PARENT this);
	pLayout->Layout_AddLabelsAndWidgetsV_VLZA(pszmFirstLabelAndToolTip, vlaArgs);
	return pLayout;
	}

WButtonTextWithIcon *
OLayoutVertical::Layout_PwAddRowButtonAndLabel(PSZAC pszmButtonTextAndToolTip, EMenuIcon eMenuIconButton, PSZAC pszmLabelTextAndToolTip)
	{
	WButtonTextWithIcon * pwButton = new WButtonTextWithIcon(pszmButtonTextAndToolTip, eMenuIconButton);
	WLabel * pwLabel = new WLabel(pszmLabelTextAndToolTip);
	OLayoutHorizontalAlignLeft * pLayout = new OLayoutHorizontalAlignLeft(PA_PARENT this);
	pLayout->Layout_AddWidgetsH_VEZA(pwButton, pwLabel, NULL);
	return pwButton;
	}

WEdit *
OLayoutVertical::Layout_PwAddRowLabelEditReadOnly(PSZAC pszmLabelTextAndToolTipEdit, PSZUC pszEditText)
	{
	WEdit * pwEdit = new WEditReadOnly(pszEditText);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(this);
	poLayout->Layout_AddLabelAndWidgetH_PA(pszmLabelTextAndToolTipEdit, pwEdit);
	return pwEdit;
	}

WEdit *
OLayoutVertical::Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszmLabelTextAndToolTipEdit, const CBin & bin)
	{
	return Layout_PwAddRowLabelEditReadOnly(pszmLabelTextAndToolTipEdit, IN g_strScratchBufferStatusBar.InitFromBinaryToHexPsz(bin));
	}

WLabel *
OLayoutVertical::Layout_PwAddRowLabelEditButton(PSZAC pszmLabelTextAndToolTipEdit, PA_CHILD WEdit * pwEdit, PA_CHILD QWidget * pawButton)
	{
	OLayoutHorizontalAlignLeft * pLayout = new OLayoutHorizontalAlignLeft(PA_PARENT this);
	WLabel * pLabel = new WLabel(PA_PARENT pLayout);
	pLabel->Label_SetTextWithToolTip(pszmLabelTextAndToolTipEdit);
	pLayout->addWidget(PA_CHILD pwEdit);
	pLayout->addWidget(PA_CHILD pawButton);
	return pLabel;

	}

WLabel *
OLayoutVertical::Layout_PwAddRowLabelAndButton(PSZAC pszmLabelTextAndToolTip, PA_CHILD QWidget * pawButton)
	{
	OLayoutHorizontalAlignLeft * pLayout = new OLayoutHorizontalAlignLeft(PA_PARENT this);
	WLabel * pLabel = new WLabel(PA_PARENT pLayout);
	pLabel->Label_SetTextWithToolTip(pszmLabelTextAndToolTip);
	pLayout->addWidget(PA_CHILD pawButton);
	return pLabel;
	}

WLabel *
OLayoutVertical::Layout_PwAddRowLabel(PSZAC pszmLabelTextAndToolTip)
	{
	WLabel * pLabel = new WLabelSelectable(PA_PARENT this);
	pLabel->Label_SetTextWithToolTip(pszmLabelTextAndToolTip);
	return pLabel;
	}

WLabel *
OLayoutVertical::Layout_PwAddRowLabel_VE(PSZAC pszmFmtLabelTextAndToolTip, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszmFmtLabelTextAndToolTip);
	return Layout_PwAddRowLabel((PSZAC)g_strScratchBufferStatusBar.Format_VL(pszmFmtLabelTextAndToolTip, vlArgs));
	}

WLabel *
OLayoutVertical::Layout_PwAddRowLabelWrap(PSZAC pszmLabelTextAndToolTip)
	{
	WLabel * pwLabel = Layout_PwAddRowLabel(pszmLabelTextAndToolTip);
	pwLabel->setWordWrap(true);
	return pwLabel;
	}

QFrame *
OLayoutVertical::Layout_PwAddRowHorizonalLine()
	{
	QFrame * pwFrame = new QFrame;
	pwFrame->setFrameShape(QFrame::HLine);
	pwFrame->setFrameShadow(QFrame::Raised);
	//pwFrame->setFrameShadow(QFrame::Sunken);
	addWidget(PA_CHILD pwFrame);
	return pwFrame;
	}

//	Add an array of widgets
void
OLayoutVertical::Layout_AddWidgets(PA_CHILD QWidget * prgpawWidgets[], int cWidgets)
	{
	Assert(prgpawWidgets != NULL);
	Assert(cWidgets >= 0);
	QWidget ** ppwWidgetStop = prgpawWidgets + cWidgets;
	while (prgpawWidgets < ppwWidgetStop)
		addWidget(PA_CHILD *prgpawWidgets++);
	}

void
OLayoutVertical::Layout_AddWidgetSpacer()
	{
	addWidget(PA_CHILD new QWidget, 1);
	}

OLayoutVerticalAlignTop::OLayoutVerticalAlignTop(PA_PARENT QWidget * pwParent) : OLayoutVertical(PA_PARENT pwParent)
	{
	setAlignment(Qt::AlignTop);
	}

OLayoutVerticalAlignTop::OLayoutVerticalAlignTop(PA_PARENT QBoxLayout * poParentLayout) : OLayoutVertical(PA_PARENT poParentLayout)
	{
	setAlignment(Qt::AlignTop);
	}
OLayoutVerticalAlignTop0::OLayoutVerticalAlignTop0(PA_PARENT QWidget * pwParent) : OLayoutVerticalAlignTop(PA_PARENT pwParent)
	{
	Layout_MarginsClearAndSpacing();
	}
OLayoutVerticalAlignTop0::OLayoutVerticalAlignTop0(PA_PARENT QBoxLayout * poParentLayout) : OLayoutVerticalAlignTop(PA_PARENT poParentLayout)
	{
	Layout_MarginsClearAndSpacing();
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
OLayoutForm::OLayoutForm(PA_PARENT QWidget * pwParent)
	{
	Assert(pwParent != NULL);
	pwParent->setLayout(PA_CHILD this);
	}

OLayoutForm::OLayoutForm(PA_PARENT QBoxLayout * poParentLayout)
	{
	Assert(poParentLayout != NULL);
	poParentLayout->addLayout(PA_CHILD this);
	}

//	Make sure there are no more margins around the layout.
//	By default Qt sets the margins to about 5 pixels, however if we want to put layouts within layouts, we have to remove the margins.
void
OLayoutForm::Layout_RemoveMargins()
	{
	setContentsMargins(0, 0, 0, 0);
	}

WLabel *
OLayoutForm::Layout_PwAddRowLabelFormat_VE_Gsb(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	WLabel * pwLabel = new WLabel(g_strScratchBufferStatusBar);
	addRow(PA_CHILD pwLabel);
	return pwLabel;
	}


WLabel *
OLayoutForm::Layout_PwAddRowBlankAndLabelDescription(PSZAC pszLabel)
	{
	WLabel * pwLabel = new WLabel(pszLabel);
	pwLabel->setStyleSheet("color: #9E9EA6");	// Display the text in a grey color
	addRow(c_sEmpty, PA_CHILD pwLabel);
	return pwLabel;
	}

WLabel *
OLayoutForm::Layout_PwAddRowLabelAndWidget(PSZAC pszLabel, PA_CHILD QWidget * pawWidget)
	{
	WLabel * pwLabel = new WLabel(pszLabel);
	pwLabel->setStyleSheet("color: #555459; font-weight: bold;");
	addRow(PA_CHILD pwLabel, PA_CHILD pawWidget);
	return pwLabel;
	}

WEditPassword *
OLayoutForm::Layout_PwAddRowLabelEditPassword(PSZAC pszLabel)
	{
	WEditPassword * pwEdit = new WEditPassword;
	addRow(PA_CHILD new WLabel(pszLabel), PA_CHILD pwEdit);
	return pwEdit;
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEdit(PSZAC pszLabel, const CString & sEditText)
	{
	/* Mac stuff
	 formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
 formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
 formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
 formLayout->setLabelAlignment(Qt::AlignLeft);
	*/
//	QTextLayout
	WEdit * pwEdit =  new WEdit(sEditText);
	// pwEdit->setText(sEditText);
	//pwEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	addRow(PA_CHILD new WLabel(pszLabel), PA_CHILD pwEdit);
	return pwEdit;
	}
WEdit *
OLayoutForm::Layout_PwAddRowLabelEditReadOnly(PSZAC pszLabel, const CString & sEditText)
	{
	WEdit * pwEdit = Layout_PwAddRowLabelEdit(pszLabel, sEditText);
	pwEdit->setReadOnly(true);
	return pwEdit;
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEditReadOnlyInt(PSZAC pszLabel, int nValue)
	{
	return Layout_PwAddRowLabelEditReadOnly(pszLabel, QString::number(nValue));
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszLabel, const CBin & bin)
	{
	return Layout_PwAddRowLabelEditReadOnly(pszLabel, IN g_strScratchBufferStatusBar.InitFromBinaryToHexPsz(bin));
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszLabel, const QByteArray & arrayb)
	{
	return Layout_PwAddRowLabelEditReadOnly(pszLabel, CString(arrayb.toHex()));
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEditReadOnlyFingerprint(PSZAC pszLabel, const QByteArray & arraybFingerprint)
	{
	g_strScratchBufferStatusBar.InitFromFingerprintBinary(arraybFingerprint);
	return Layout_PwAddRowLabelEditReadOnly(pszLabel, g_strScratchBufferStatusBar);
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEditReadOnlyFingerprint(PSZAC pszLabel, const CBin & binFingerprint)
	{
	g_strScratchBufferStatusBar.InitFromFingerprintBinary(binFingerprint);
	return Layout_PwAddRowLabelEditReadOnly(pszLabel, g_strScratchBufferStatusBar);
	}

WEditTextArea *
OLayoutForm::Layout_PwAddRowLabelEditTextAreaH(PSZAC pszLabel, const CStr & strEditText, int cLines)
	{
	WEditTextArea * pwEdit = new WEditTextArea(strEditText);
	pwEdit->Edit_SetHeightLines(cLines);
	pwEdit->setStyleSheet("background-color: #FFF");	// This is a workaround of a Qt bug where the background color turns black if nothing is specified when adding a new widget in the dialog
	addRow(PA_CHILD new WLabel(pszLabel), PA_CHILD pwEdit);
	return pwEdit;
	}


// Add two rows: one with a label and another with a multi-line text edit
WEditTextArea *
OLayoutForm::Layout_PwAddRowLabelEditTextArea(PSZAC pszLabel, const CString & sEditText, int cLines)
	{
	addRow(PA_CHILD new WLabelSelectable(pszLabel));
	WEditTextArea * pwEdit = new WEditTextArea(sEditText);
	// pwEdit->setText(sEditText);
	pwEdit->setFixedHeight(cLines * 16);
	addRow(PA_CHILD pwEdit);
	return pwEdit;
	}


WEditTextArea *
OLayoutForm::Layout_PwAddRowLabelEditTextAreaReadOnly(PSZAC pszLabel, const CString & sEditText, int cLines)
	{
	WEditTextArea * pwEdit = Layout_PwAddRowLabelEditTextAreaH(pszLabel, sEditText, cLines);
	pwEdit->setReadOnly(true);
	return pwEdit;
	}


WEditTextArea *
OLayoutForm::Layout_PwAddRowLabelEditTextAreaReadOnlyToHex(PSZAC pszLabel, const CBin & bin, int cLines)
	{
	g_strScratchBufferStatusBar.InitFromBinaryToHexPsz(bin);
	return Layout_PwAddRowLabelEditTextAreaH(pszLabel, IN g_strScratchBufferStatusBar, cLines);
	}

WEdit *
OLayoutForm::Layout_PwAddRowLabelEditAndPushButton(PSZAC pszLabel, const QString & sEditText, PSZAC pszButtonText, OUT QPushButton ** ppwButton)
	{
	Assert(ppwButton != NULL);
	QHBoxLayout * pLayout = new QHBoxLayout;
	WEdit * pwEdit =  new WEdit(sEditText);
	QPushButton * pwButton = new QPushButton(pszButtonText);
	//pwButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	pLayout->addWidget(PA_CHILD pwEdit);
	pLayout->addWidget(PA_CHILD pwButton);
	addRow(PA_CHILD new WLabel(pszLabel), PA_CHILD pLayout);
	*ppwButton = pwButton;
	return pwEdit;
	}

WButtonTextWithIcon *
OLayoutForm::Layout_PwAddRowLabelAndPushButton(PSZAC pszmLabelTextAndToolTip, PSZAC pszmButtonTextAndToolTip, EMenuIcon eMenuIconButton)
	{
	WButtonTextWithIcon * pwButton = new WButtonTextWithIcon(pszmButtonTextAndToolTip, eMenuIconButton);
	addRow(PA_CHILD new WLabel(pszmLabelTextAndToolTip), pwButton);
	return pwButton;
	}

void
OLayoutForm::Layout_AddRowLabelError(PSZAC pszLabel, const QString & sError)
	{
	WLabelSelectable * pwLabelError = new WLabelSelectable;
	pwLabelError->setText(sError);
	pwLabelError->Label_SetTextColorError();
	addRow(pszLabel, PA_CHILD pwLabelError);
	}

void
OLayoutForm::Layout_AddRowErrors(const CArrayPsz & arraypszErrors)
	{
	WLabelSelectable * pwLabel = new WLabelSelectable;
	addRow(PA_CHILD pwLabel);
	pwLabel->Label_SetTextHtmlListItems_Gsb(arraypszErrors);
	}

WSplitter *
OLayoutForm::Layout_PwAddSplitterHorizontal()
	{
	WSplitter * pwSplitter = new WSplitter;
	addRow(PA_CHILD pwSplitter);
	return pwSplitter;
	}



///////////////////////////////////////////////////////////////////////////////////////////////////
void OTableRow::AddData(PSZUC pszuData)
	{
	AddData(CString(pszuData));
	}
void OTableRow::AddData(const CStr & strData)
	{
	append(new QStandardItem(strData));
	}
void OTableRow::AddData(const QString & sData)
	{
	append(new QStandardItem(sData));
	}
void OTableRow::AddData(const QByteArray & arraybData)
	{
	append(new QStandardItem(QString(arraybData)));
	}
void OTableRow::AddDataBinaryHex(const QByteArray & arraybData)
	{
	append(new QStandardItem(QString(arraybData.toHex())));
	}
void OTableRow::AddDataFingerprint(const QByteArray & arraybFingerprint)
	{
	g_strScratchBufferStatusBar.InitFromFingerprintBinary(arraybFingerprint);
	AddData(g_strScratchBufferStatusBar);
	}
void OTableRow::AddDate(const QDateTime & dtu)
	{
	append(new QStandardItem(dtu.date().toString(Qt::DefaultLocaleShortDate)));
	}
void OTableRow::AddDateTime(const QDateTime & dtu)
	{
	append(new QStandardItem(dtu.toString(Qt::DefaultLocaleShortDate)));
	}
void OTableRow::AddDateTime(TIMESTAMP ts)
	{
	AddDateTime(QDateTime::fromMSecsSinceEpoch(ts));
	}
void OTableRow::AddAmount_BTC(AMOUNT amtSatoshis)
	{
	AddData(Amount_SFormat_BTC(amtSatoshis));
	}
void OTableRow::AddAmount_mBTC(AMOUNT amtSatoshis)
	{
	AddData(Amount_SFormat_mBTC(amtSatoshis));
	}

void OTableRow::AddInt(int nValue)
	{
	if (nValue != 0)
		AddData(QString::number(nValue));
	else
		AddData(c_sEmpty);
	}
void OTableRow::AddData_VEZ(PSZAC pszData, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszData);
	while (pszData != NULL)
		{
		AddData((PSZUC)pszData);
		pszData = va_arg(vlArgs, PSZAC);
		}
	}


WTable::WTable(QWidget * pwParent) : QTableView(pwParent)
	{
	_Init();
	}

WTable::WTable(PA_PARENT QFormLayout * poParentLayout)
	{
	Assert(poParentLayout != NULL);
	_Init();
	poParentLayout->addRow(PA_CHILD this);
	}

void
WTable::_Init()
	{
	m_poModel = new QStandardItemModel(PA_PARENT this);
	setModel(PA_CHILD m_poModel);
	setEditTriggers(NoEditTriggers);
	verticalHeader()->hide();
	// setShowGrid(false);
	}

void
WTable::SetColumns_VEZ(PSZAC pszNameColumn, ...)
	{
	QStringList listNames;
	va_list vlArgs;
	va_start(OUT vlArgs, pszNameColumn);
	while (TRUE)
		{
		listNames << pszNameColumn;
		pszNameColumn = va_arg(vlArgs, PSZAC);
		if (pszNameColumn == NULL)
			break;
		}
	m_poModel->setHorizontalHeaderLabels(listNames);
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	//horizontalHeader()->setStretchLastSection(true);
	}

LPARAM
WTable::LParamGetFromRow(const QModelIndex & oIndex)
	{
	QModelIndex oIndexFirstColumn = m_poModel->index(oIndex.row(), 0);
	return (LPARAM)m_poModel->data(oIndexFirstColumn, Qt::UserRole).toLongLong();
	}

LPARAM
WTable::LParamGetFromRowSelected()
	{
	return LParamGetFromRow(currentIndex());
	}

void
WTable::AppendRow(OTableRow & oRow, LPARAM lParam)
	{
	m_poModel->appendRow(oRow);
	if (lParam != d_zNA)
		{
		// Attach some data to the row
		int cRows = m_poModel->rowCount();
		QModelIndex oIndex = m_poModel->index(cRows - 1, 0);
		m_poModel->setData(oIndex, (qlonglong)lParam, Qt::UserRole);
		}
	}

void
WTable::AppendRow_VEZ(PSZAC pszData, ...)
	{
	OTableRow oRow;
	va_list vlArgs;
	va_start(OUT vlArgs, pszData);
	while (pszData != NULL)
		{
		oRow.AddData((PSZUC)pszData);
		pszData = va_arg(vlArgs, PSZAC);
		}
	AppendRow(oRow);
	}

void
WTable::clear()
	{
		m_poModel->clear();
	}

//	So far, I have not found any friendly method to adjust the height of a table
void
WTable::AdjustHeightToFitRows()
	{
	setMinimumHeight(horizontalHeader()->height() + (m_poModel->rowCount() * rowHeight(0)) + (frameWidth() * 2) + horizontalScrollBar()->height());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WTreeWidget::DeleteAllRootNodesWhichAreEmpty()
	{
	QTreeWidgetItemIterator oIterator(this);
	while (TRUE)
		{
		CTreeItemW * pTreeWidgetItem = (CTreeItemW *)*oIterator++;
		if (pTreeWidgetItem == NULL)
			return;
		if (pTreeWidgetItem->parent() == NULL && pTreeWidgetItem->childCount() == 0)
			delete pTreeWidgetItem;	// Delete any empty root node
		}
	}

void
WTreeWidget::ShowAllTreeItems()
	{
	// Show every tree item
	QTreeWidgetItemIterator oIterator(this);
	while (TRUE)
		{
		CTreeWidgetItem * pTreeWidgetItem = (CTreeWidgetItem *)*oIterator++;
		if (pTreeWidgetItem == NULL)
			return;
		pTreeWidgetItem->setHidden(false);
		}
	}

//	Search text only in the first three (3) columns
void
WTreeWidget::ShowAllTreeItemsContainingText(const QString & sTextSearch)
	{
	if (sTextSearch.isEmpty())
		{
		ShowAllTreeItems();
		return;
		}
	QTreeWidgetItemIterator oIterator(this);
	while (TRUE)
		{
		CTreeWidgetItem * pTreeWidgetItem = (CTreeWidgetItem *)*oIterator++;
		if (pTreeWidgetItem == NULL)
			return;
		BOOL fShowRow = (pTreeWidgetItem->text(0).contains(sTextSearch, Qt::CaseInsensitive) ||
						 pTreeWidgetItem->text(1).contains(sTextSearch, Qt::CaseInsensitive) ||
						 pTreeWidgetItem->text(2).contains(sTextSearch, Qt::CaseInsensitive));
		pTreeWidgetItem->SetItemVisibleAlongWithItsParents(fShowRow);
		}
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Make sure the Tree Item is visible and so its parent
void
CTreeWidgetItem::SetItemVisibleAlongWithItsParents(BOOL fVisible)
	{
	setVisible(fVisible);
	if (fVisible)
		{
		CTreeWidgetItem * pParent = (CTreeWidgetItem *)parent();
		if (pParent != NULL)
			{
			// Recursively expand and make the parent visible
			pParent->setExpanded(true);
			pParent->SetItemVisibleAlongWithItsParents(TRUE);
			}
		}
	}

//	We need this method, otherwise the Qt framework will blindly send the signal QTreeWidget::itemChanged() regardless if the flags have been changed, and
//	this will cause a stack overflow in slot SL_TreeItemEdited().
void
CTreeWidgetItem::ItemFlagsAdd(Qt::ItemFlag efItemFlagsAdd)
	{
	const Qt::ItemFlags eFlags = flags();
	const Qt::ItemFlags eFlagsNew = (eFlags | efItemFlagsAdd);
	if (eFlagsNew != eFlags)
		setFlags(eFlagsNew);
	}

void
CTreeWidgetItem::ItemFlagsRemove(Qt::ItemFlag efItemFlagsRemove)
	{
	const Qt::ItemFlags eFlags = flags();
	const Qt::ItemFlags eFlagsNew = (eFlags & ~efItemFlagsRemove);
	if (eFlagsNew != eFlags)
		setFlags(eFlagsNew);
	}

void
CTreeWidgetItem::InitIconAndText(EMenuIcon eMenuIcon, PSZUC pszTextColumn1, PSZUC pszTextColumn2, PSZUC pszTextColumn3, PSZUC pszTextColumn4)
	{
	setIcon(0, OGetIcon(eMenuIcon));
	setText(0, CString(pszTextColumn1));
	setText(1, CString(pszTextColumn2));
	setText(2, CString(pszTextColumn3));
	setText(3, CString(pszTextColumn4));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
Widget_SetText(INOUT QWidget * pwWidget, PSZAC pszText)
	{
	pwWidget->setWindowTitle(pszText);
	}

void
Widget_SetText(INOUT QWidget * pwWidget, const CStr & strText)
	{
	pwWidget->setWindowTitle(strText);
	}

void
Widget_SetTextFormat_VE_Gsb(INOUT QWidget * pwWidget, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	Widget_SetTextFormat_VL_Gsb(INOUT pwWidget, pszFmtTemplate, vlArgs);
	}

void
Widget_SetTextFormat_VL_Gsb(INOUT QWidget * pwWidget, PSZAC pszFmtTemplate, va_list vlArgs)
	{
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	pwWidget->setWindowTitle(g_strScratchBufferStatusBar);
	}

void
Widget_SetTextAndToolTip(INOUT QWidget * pwWidget, PSZAC pszmTextAndToolTip)
	{
	PSZAC pszToolTip = PszroGetToolTipNext(pszmTextAndToolTip);
	pwWidget->setWindowTitle(QString::fromUtf8(pszmTextAndToolTip, pszToolTip - pszmTextAndToolTip));
	if (pszToolTip != NULL && *pszToolTip != '\0')
		pwWidget->setToolTip(pszToolTip + 1);
	}

void
Widget_SetToolTipNext(INOUT QWidget * pwWidget, PSZAC pszmTextAndToolTip)
	{
	Assert(pwWidget != NULL);
	pszmTextAndToolTip = PszroGetToolTipNext(pszmTextAndToolTip);
	if (pszmTextAndToolTip != NULL && *pszmTextAndToolTip != '\0')
		pwWidget->setToolTip(pszmTextAndToolTip + 1);
	}

void
Widget_SetToolTipFormat_VE_Gsb(INOUT QWidget * pwWidget, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	pwWidget->setToolTip(g_strScratchBufferStatusBar);
	}

void
Widget_SetTextColorError(INOUT QWidget * pwWidget)
	{
	pwWidget->setStyleSheet("color: maroon");	// The maroon color is better than red because it is a bit darker
	}

void
Widget_SetTextColorGray(INOUT QWidget * pwWidget)
	{
	pwWidget->setStyleSheet("color: #808080");
	}

int
Widget_GetWidthMinimumRecommended(const QWidget * pwWidget)
	{
	//return pwWidget->minimumWidth();
	return pwWidget->sizeHint().width();
	}

void
Widget_SetWidth(INOUT QWidget * pwWidget, int nWidth)
	{
	pwWidget->resize(nWidth, pwWidget->height());
	}

void
Widget_ScrollToEnd(QAbstractScrollArea * pwWidget)
	{
	//pWidget->update();
	QScrollBar * pScrollBarVertical = pwWidget->verticalScrollBar();
	// pScrollBarVertical->setValue(pScrollBarVertical->maximum());
	pScrollBarVertical->setValue(pScrollBarVertical->maximum() + 100);
	pScrollBarVertical->setSliderPosition(pScrollBarVertical->maximum() + 100);
	}

void
WidgetButton_SetTextAndToolTip(QAbstractButton * pwButton, PSZAC pszmButtonTextAndToolTip)
	{
	PSZAC pszToolTip = PszroGetToolTipNext(pszmButtonTextAndToolTip);
	pwButton->setText(QString::fromUtf8(pszmButtonTextAndToolTip, pszToolTip - pszmButtonTextAndToolTip));
	if (pszToolTip != NULL && *pszToolTip != '\0')
		pwButton->setToolTip(QString::fromUtf8(pszToolTip + 1));
	}

//	See also class WButtonIconForToolbar
void
Widget_SetIconButton(INOUT QAbstractButton * pwButton, EMenuIcon eMenuIcon)
	{
	Assert(pwButton != NULL);
	pwButton->setIcon(OGetIcon(eMenuIcon));
	}

//	Set the icon of a widget (typically a window or a dialog) using the same icon of a menu action.
void
Widget_SetIcon(INOUT QWidget * pwWidget, EMenuIcon eMenuIcon)
	{
	Assert(pwWidget != NULL);
	pwWidget->setWindowIcon(OGetIcon(eMenuIcon));
	}


const QMargins c_oMarginsEmpty;
void
Layout_MarginsClear(INOUT QLayout * poLayout)
	{
	poLayout->setContentsMargins(c_oMarginsEmpty);
	}

void
CStr::CopyStringToClipboard() const
	{
	Clipboard_SetText(*this);
	}

void
CStr::InitFromClipboard()
	{
	InitFromStringQTrimmed(QApplication::clipboard()->text());
	}

void
Clipboard_SetText(const QString & sText)
	{
	QApplication::clipboard()->setText(sText);
	}

void
Clipboard_SetText(const CStr & strText)
	{
	Clipboard_SetText(strText.ToQString());
	}

void
Clipboard_SetText(const QLineEdit * pwEdit)
	{
	Clipboard_SetText(pwEdit->text());
	}

/* Copied from /src/widgets/widgets/qwidgettextcontrol.cpp, line 3120
void
QWidgetTextControlPrivate::_q_copyLink()
	{
	QMimeData *md = new QMimeData;
	md->setText(linkToCopy);
	QApplication::clipboard()->setMimeData(md);
	}
*/

/*
class LineEdit : public QLineEdit
{
	Q_OBJECT

public:
	LineEdit(QWidget *parent = 0);

protected:
	void resizeEvent(QResizeEvent *);

private slots:
	void updateCloseButton(const QString &text);

private:
	QToolButton *clearButton;
};

LineEdit::LineEdit(QWidget *parent)	: QLineEdit(parent)
{
	clearButton = new QToolButton(this);
	//QPixmap pixmap("fileclose.png");
	QPixmap pixmap(":/ico/Cancel");
	clearButton->setIcon(QIcon(pixmap));
	clearButton->setIconSize(pixmap.size());
	clearButton->setCursor(Qt::ArrowCursor);
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	clearButton->hide();
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth + 1));
	QSize msz = minimumSizeHint();
	setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
				   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void LineEdit::resizeEvent(QResizeEvent *)
{
	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move(rect().right() - frameWidth - sz.width(),  (rect().bottom() + 1 - sz.height())/2);
}

void LineEdit::updateCloseButton(const QString& text)
{
	clearButton->setVisible(!text.isEmpty());
}
*/


/*
class TitleBar : public QWidget
{
	Q_OBJECT
private:
	QToolButton * minimize;
	QToolButton * maximize;
	QToolButton * close;
	QPixmap restorePix, maxPix;
	bool maxNormal;
	QPoint startPos;
	QPoint clickPos;

public:
	TitleBar(QWidget *parent)
	{
		// Don't let this widget inherit the parent's backround color
		setAutoFillBackground(true);
		// Use a brush with a Highlight color role to render the background
		setBackgroundRole(QPalette::Highlight);

		minimize = new QToolButton(this);
		maximize = new QToolButton(this);
		close= new QToolButton(this);

		// Use the style to set the button pixmaps
		QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
		close->setIcon(pix);

		maxPix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
		maximize->setIcon(maxPix);

		pix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
		minimize->setIcon(pix);

		restorePix = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);

		minimize->setMinimumHeight(16);
		close->setMinimumHeight(16);
		maximize->setMinimumHeight(16);


		QLabel *label = new QLabel(this);
		label->setText("Window Title");
		parent->setWindowTitle("Window Title");

		QHBoxLayout *hbox = new QHBoxLayout(this);

		hbox->addWidget(label);
		hbox->addWidget(minimize);
		hbox->addWidget(maximize);
		hbox->addWidget(close);
		hbox->insertStretch(1, 500);
		hbox->setSpacing(0);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		maxNormal = false;
		connect(close, SIGNAL( clicked() ), parent, SLOT(close() ) );
		connect(minimize, SIGNAL( clicked() ), this, SLOT(showSmall() ) );
		connect(maximize, SIGNAL( clicked() ), this, SLOT(showMaxRestore() ) );
	}

public slots:
	void showSmall()
	{
		parentWidget()->showMinimized();
	}

	void showMaxRestore()
	{
		if (maxNormal) {
			parentWidget()->showNormal();
			maxNormal = !maxNormal;
			maximize->setIcon(maxPix);
		} else {
			parentWidget()->showMaximized();
			maxNormal = !maxNormal;
			maximize->setIcon(restorePix);
		}
	}
protected:
	void mousePressEvent(QMouseEvent *me)
	{
		startPos = me->globalPos();
		clickPos = mapToParent(me->pos());
	}
	void mouseMoveEvent(QMouseEvent *me)
	{
		if (maxNormal)
			return;
		QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
		dw->setFloating(true);
		me->accept();
		//parentWidget()->move(me->globalPos() - clickPos);
	}
};
*/

void setLineEditTextFormat(QLineEdit* lineEdit, const QList<QTextLayout::FormatRange>& formats)
{
	if(!lineEdit)
		return;

	QList<QInputMethodEvent::Attribute> attributes;
	foreach(const QTextLayout::FormatRange& fr, formats)
	{
		QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
		int start = fr.start - lineEdit->cursorPosition();
		int length = fr.length;
		QVariant value = fr.format;
		attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
	}
	QInputMethodEvent event(QString(), attributes);
	QCoreApplication::sendEvent(lineEdit, &event);
}

void clearLineEditTextFormat(QLineEdit* lineEdit)
{
	setLineEditTextFormat(lineEdit, QList<QTextLayout::FormatRange>());
}

void LineEditExample(QLineEdit* lineEdit)
	{
	Assert(!lineEdit->isReadOnly());	//Somehow it does not work if the edit is in read-only
	// Usage example:
	//QLineEdit* lineEdit = new QLineEdit;
	lineEdit->setText("Task Tracker - Entry");

	QList<QTextLayout::FormatRange> formats;

	QTextCharFormat f;

	f.setFontWeight(QFont::Bold);
	QTextLayout::FormatRange fr_task;
	fr_task.start = 0;
	fr_task.length = 4;
	fr_task.format = f;

	f.setFontItalic(true);
	f.setBackground(Qt::darkYellow);
	f.setForeground(Qt::white);
	QTextLayout::FormatRange fr_tracker;
	fr_tracker.start = 5;
	fr_tracker.length = 7;
	fr_tracker.format = f;

	formats.append(fr_task);
	formats.append(fr_tracker);

	setLineEditTextFormat(lineEdit, formats);
	}

	/*
	// Set a scrolling area with a dialog layout
	QWidget * pWidget = new QWidget(PA_PARENT this);
	ui = new Ui::WLayoutCertificate;
	ui->setupUi(pWidget);
	QScrollArea * pScrollArea = new QScrollArea(this);
	pScrollArea->setWidget(pWidget)	;
	*/
	/*
	QWidget * pWidget = new QWidget(PA_PARENT this);
	ui = new Ui::WLayoutCertificate;
	ui->setupUi(pWidget);
	*/
	/*
	ui = new Ui::WLayoutCertificate;
	ui->setupUi(this);
	*/


void
SetCursorWait()
	{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	}

void
SetCursorRestoreDefault()
	{
	QApplication::restoreOverrideCursor();
	}

//	Return a pseudo-random value.
//
//	WARNING
//	Before calling this function, the random number generator must be initialized with qsrand().
//	The functions UGenerateRandomValueRandomUntil() and LGenerateRandomValueBetween() do provide random
//	numbers, however those numbers should not be used for cryptography.
//	For a truly random number, use HashSha1_InitRandom() and pick how many bits (32, 64, or whatever) you need as your random number.
UINT
UGenerateRandomValueRandomUntil(int nMax)
	{
	if (nMax > 0)
		return qrand() % nMax;
	return 0;
	}

#define d_wRandomMax		RAND_MAX // Typically 0x7fff
L64
LGenerateRandomValueBetween(L64 lMin, L64 lMax)
	{
	Assert(lMin <= lMax);
	L64 lRandom;
	L64 lDifference = lMax - lMin;
	if (lDifference < d_wRandomMax)
		{
		lRandom = UGenerateRandomValueRandomUntil(lDifference);
		}
	else
		{
		// The random generator is out of reach
		lDifference >>= 15;		// Divide by 0x7fff
		if (lDifference < d_wRandomMax)
			lRandom = (UGenerateRandomValueRandomUntil(lDifference) << 15) | qrand();
		else
			{
			lDifference >>= 15;		// Divide again by 0x7fff
			Assert(lDifference < d_wRandomMax);
			lRandom = ((LU64)UGenerateRandomValueRandomUntil(lDifference) << 30) | (qrand() << 15) | qrand();
			}
		}
	Assert(lRandom >= 0);
	Assert(lMin + lRandom <= lMax);
	return lMin + lRandom;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
CFile::CFile(const CStr & strFileName) : QFile(strFileName)
	{
	}

L64
CFile::CbDataReadAtOffset(L64 ibData, L64 cbData, OUT BYTE prgbBuffer[])
	{
	Assert(prgbBuffer != NULL);
	(void)seek(ibData);	// Attempt to seek to the desired offset (if the device is squential, then it will do nothing and get the next bytes)
	return read(OUT (char *)prgbBuffer, cbData);
	}

int
CFile::CbDataWriteBinaryFromBase85(PSZUC pszuBase85)
	{
	// First, decode the Base85 into binary
	CBin bin;
	const char * prgbData = (char *)bin.BinAppendBinaryDataFromBase85Szv_ML(pszuBase85);
	return write(IN prgbData, bin.CbGetData());
	}

CFileOpenRead::CFileOpenRead(const CStr & strFileName) : CFile(strFileName)
	{
	(void)open(QIODevice::ReadOnly);	// Attempt to read the file
	}

CFileOpenWrite::CFileOpenWrite(const CStr & strFileName) : CFile(strFileName)
	{
	(void)open(QIODevice::WriteOnly);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
CPainter::DrawLineHorizontal(int xLeft, int xRight, int yPos)
	{
	drawLine(xLeft, yPos, xRight, yPos);
	}

void
CPainter::DrawLineVertical(int xPos, int yTop, int yBottom)
	{
	drawLine(xPos, yTop, xPos, yBottom);
	}

void
CPainter::FillRectWithGradientVertical(const QRect & rcFill, QRGB coTop, QRGB coBottom)
	{
	QLinearGradient oGradient(0, 0, 0, rcFill.height());
	oGradient.setColorAt(0, coTop);
	oGradient.setColorAt(1, coBottom);
	fillRect(rcFill, oGradient);
	}

CPainterCell::CPainterCell(QWidget * pwWidget) : CPainter(pwWidget)
	{
	Assert(pwWidget != NULL);
	m_rcCell = pwWidget->rect();
	}

void
CPainterCell::DrawTextWithinCell(const QString & sText)
	{
	drawText(IN m_rcCell, Qt::AlignVCenter, sText);
	}

void
CPainterCell::DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	DrawTextWithinCell(g_strScratchBufferStatusBar);
	}

void
CPainterCell::DrawIconAdjustLeft(const QIcon & oIcon)
	{
	QRect rcIcon = m_rcCell;
	rcIcon.setWidth(16);		// This assumes icons are 16 pixels
	oIcon.paint(this, rcIcon);
	m_rcCell.setLeft(m_rcCell.left() + 19);
	}

void
CPainterCell::DrawIconAdjustLeft(EMenuIcon eMenuIcon)
	{
	DrawIconAdjustLeft(OGetIcon(eMenuIcon));
	}

void
CPainterCell::DrawIconAlignment(const QIcon & oIcon, Qt::Alignment eAlignment)
	{
	oIcon.paint(this, m_rcCell, eAlignment);
	}

void
CPainterCell::DrawIconAlignment(EMenuIcon eMenuIcon, Qt::Alignment eAlignment)
	{
	DrawIconAlignment(OGetIcon(eMenuIcon), eAlignment);
	}

void
CPainterCell::DrawIconAlignmentLeftBottom(EMenuIcon eMenuIcon)
	{
	DrawIconAlignment(eMenuIcon, Qt::AlignLeft | Qt::AlignBottom);
	}

void
CPainterCell::DrawIconAlignmentRightBottom(EMenuIcon eMenuIcon)
	{
	DrawIconAlignment(eMenuIcon, Qt::AlignRight | Qt::AlignBottom);
	}
