///////////////////////////////////////////////////////////////////////////////////////////////////
//	QtUtilities.h
//
//	Various wrappers for the Qt framework.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef QTUTILITIES_H
#define QTUTILITIES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_chSeparatorTextAndToolTip		'|'
#define d_chSeparatorTextAndWatermark	'~'

#define d_cxWidthEditNumber		65	// 65 pixels is enough to enter any 10-digit number
#define d_cxWidthEditPassword	100	// 100 pixels is enough to enter a password of 16 digits
#define d_cxWidthEditUsername	100
#define d_cyHeightEditTextArea	90	// 90 pixels is good for 6 lines of text (which is a good default)

//	Wrapper to the QWidget class
class WWidget : public QWidget
{
public:
	WWidget(PA_PARENT QWidget * pwParent);
	void Widget_DetachFromParent() PA_CAST_FROM_P;
	void Widget_SetTextFormat_VE_Gsb(PSZAC pszFmtTemplate, ...);
	void Widget_SetTextFormat_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs);
};

//	By default, the text of QLabel is not selectable.
//	Since we want the text to be selectable by default, we give a long name to the base class which will encourage using the name WLabelSelectable
class WLabel : public QLabel
{
public:
	WLabel();
	WLabel(PSZAC pszmLabelTextAndToolTip);
	WLabel(const CStr & strLabelText);
	WLabel(PA_PARENT QBoxLayout * poParentLayout);
	void _Init();
	void Label_SetTextSelectable();
	void Label_SetTextSelectableAndWordWrap();
	void Label_SetTextFormatPlain();
	void Label_SetTextPlain(const CStr & strText);

	void Label_SetTextWithToolTip(PSZAC pszmLabelTextAndToolTip);
	void Label_SetTextFormat_VE_Gsb(PSZAC pszFmtTemplate, ...);
	void Label_SetTextFormat_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs);
	void Label_SetTextColorError();
	void Label_SetTextColorGray();
	void Label_SetStyleSheetForHtmlListItems();
	void Label_SetTextHtmlListItems_Gsb(const CArrayPsz & arraypszListItems);
};

//	Interactive label where the text is selectable and automatically wraps if too long
class WLabelSelectable : public WLabel
{
public:
	WLabelSelectable();
	WLabelSelectable(PSZAC pszmLabelTextAndToolTip);
	WLabelSelectable(const CStr & strLabelText);
	WLabelSelectable(PA_PARENT QBoxLayout * poParentLayout);
};

class WLabelSelectableWrap : public WLabelSelectable
{
public:
	WLabelSelectableWrap();
	WLabelSelectableWrap(PSZAC pszmLabelTextAndToolTip);
	WLabelSelectableWrap(const CStr & strLabelText);
	WLabelSelectableWrap(PA_PARENT QBoxLayout * poParentLayout);
};

class WLabelIcon : public WLabel
{
public:
	WLabelIcon(PA_PARENT QWidget * pwParent, EMenuAction eMenuIcon);
};

//	Invisible widget, typically to add a space between buttons
class WSpacer : public QLabel
{

};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Button displaying an icon.  The difference between WButtonIcon and WLabelIcon is the button may be clicked while the label solely display the icon/pixmap.
class WButtonIcon : public QToolButton
{
public:
	WButtonIcon(PA_PARENT QWidget * pwParent, EMenuAction eMenuIcon);
	WButtonIcon(EMenuAction eMenuIcon, PSZAC pszmToolTip = NULL);
	void _Init(EMenuAction eMenuIcon);
};

class WButtonText : public QPushButton
{
public:
	WButtonText(PSZAC pszmButtonTextAndToolTip);
	void Button_SetTextAndToolTip(PSZAC pszmButtonTextAndToolTip);
	void Button_SetIcon(EMenuAction eMenuIcon);
};

class WButtonTextWithIcon : public WButtonText
{
public:
	WButtonTextWithIcon(PSZAC pszmButtonTextAndToolTip, EMenuAction eMenuIcon);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WEdit : public QLineEdit
{
public:
	WEdit();
	WEdit(PSZUC pszText);
	WEdit(const CStr & strText);
	WEdit(const QString & sText);
	void Edit_SetText(const CStr & strText);
	void Edit_SetTextU(PSZUC pszText);
	void Edit_SetTextFromNumber(int nNumber);
	void Edit_SetToolTip(PSZUC pszToolTip);
	void Edit_SetWatermark(PSZAC pszWatermark);
	BOOL EditUsername_FGet_MB(OUT_F_UNCH CStr * pstrUsername);
	BOOL EditPassword_FGet_MB(OUT_F_UNCH CStr * pstrPassword);
	BOOL EditServerName_FGet_MB(OUT_F_UNCH CStr * pstrServerName);
	void EditFingerprint_Set(const CBin & binFingerprint);
	BOOL EditFingerprint_FGetMB(OUT_F_UNCH CBin * pbinFingerprint);
};

class WEditReadOnly : public WEdit
{
public:
	WEditReadOnly();
	WEditReadOnly(PSZUC pszText);
	WEditReadOnly(const QString & sText);
};

//	Class to edit and validate a username
class WEditUsername : public WEdit
{
public:
	WEditUsername() : WEdit() { setFixedWidth(d_cxWidthEditUsername); }
};

class WEditPassword : public WEdit
{
public:
	WEditPassword();
	WEditPassword(PSZUC pszPassword);
	void _Init();
};

class WEditFingerprint : public WEdit
{
public:
	WEditFingerprint();
	WEditFingerprint(const CBin & binFingerprint, PSZAC pszToolTip);
	void _Init();
};

class WEditNumber : public WEdit
{
public:
	WEditNumber();
	WEditNumber(int nValue);
	void _Init();
};

class WEditNumberReadOnly : public WEditNumber
{
public:
	WEditNumberReadOnly() { setReadOnly(true); }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	The class WEditTextArea is more than just a wrapper to the class QTextEdit.
class WEditTextArea : public QTextEdit
{
protected:
	int m_cyHeightRecommended;	// Set the recommended height for the text area.  Unfortunately Qt does not provide any mechanism to specify what is the recommended height, unless implementing the virtual method sizeHint() for every WEditTextArea.
public:
	CStr m_strWatermark;		// Text to display in gray when the content of the WEditTextArea is empty.  This is an equivalent of QLineEdit::setPlaceholderText().

public:
	WEditTextArea();
	WEditTextArea(QWidget * pwParent);
	WEditTextArea(const CStr & strText);
	WEditTextArea(const QString & sText);
	void _InitTextArea();
	void Edit_SetColorBackgroundToGray();
	void Edit_SetHeightLines(int cLines);
	void Edit_SetHeightRecommended(int cyHeightRecommended);
	virtual QSize sizeHint() const;
	virtual void paintEvent(QPaintEvent *);
	void Edit_SetText(const CStr & strText);
	void Edit_SetTextU(PSZUC pszText);
	void Edit_SetWatermark(PSZAC pszWatermark);
	void Widget_SetHeightEqualToLayout(QLayout * poLayout);
};

class WEditTextAreaReadOnly : public WEditTextArea
{
public:
	WEditTextAreaReadOnly();
	WEditTextAreaReadOnly(const QString & sText);
	void _InitReadOnly();
};

class WEditTextAreaReadOnlyGray : public WEditTextAreaReadOnly
{
public:
	WEditTextAreaReadOnlyGray();

};

class WEditMessageLog : public WEditTextAreaReadOnly
{
public:
	WEditMessageLog();
};

class WEditMessageLogHidden : public WEditMessageLog
{
public:
	WEditMessageLogHidden();
};

class WEditMessageLogGrayHidden : public WEditMessageLog
{
public:
	WEditMessageLogGrayHidden();
};

//	Widget to browse rich HTML text
//	In the future this widget will have a built in context menu to perform operations, including searching for text.
class WTextBrowser : public QTextBrowser	// QTextBrowser inherits QTextEdit
{
public:
	WTextBrowser(QWidget * pwParent) : QTextBrowser(pwParent) { }

};

///////////////////////////////////////////////////////////////////////////////////////////////////

class OCursor : public QTextCursor
{
public:
	OCursor(const QTextCursor & oCursor);
	OCursor(const QTextBlock & oTextBlock);
	void PositionMoveToEnd();
	void AppendBlockBlank();
	void InsertBlockBlank();
	void InsertHtmlBin(const CBin & binHtml);
	void InsertHtmlStr(const CStr & strHtml);
	void InsertHtmlBin(const CBin & binHtml, const QBrush & oBrushColorBackground);
	void InsertHtmlStr(const CStr & strHtml, const QBrush & oBrushColorBackground);
	void InsertHtmlCoFormat_VE(QRGBX coxBackground, PSZAC pszFmtTemplate, ...);
};

class OCursorSelectBlock : public OCursor
{
public:
	OCursorSelectBlock(const QTextBlock & oTextBlock);
	OCursorSelectBlock(IEvent * pEvent, QTextEdit * pwEdit);
};

class OCursorMoveToEnd : public OCursor
{
public:
	OCursorMoveToEnd(const QTextCursor & oCursor);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WGroupBox : public QGroupBox
{
public:
	explicit WGroupBox() : QGroupBox() { }
	explicit WGroupBox(PA_PARENT QWidget * pwParentWidget);
	explicit WGroupBox(PA_PARENT QSplitter * pwParentSplitter);
	explicit WGroupBox(PA_PARENT QBoxLayout * poParentLayout);
	void Widget_SetTitleFormat_VE_Gsb(PSZAC pszFmtTemplate, ...);
	void Widget_SetTitleFormat_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs);
	void Widget_SetTitleColorError();
};

class WSplitter : public QSplitter
{
public:
	WSplitter() : QSplitter() { }
	WSplitter(Qt::Orientation eOrientation) : QSplitter(eOrientation) { }
	void Splitter_SetMarginForCustomLayout();
	WGroupBox * Splitter_PwAddGroupBox_VL(PSZAC pszFmtTemplate, va_list vlArgs);
	WGroupBox * Splitter_PwAddGroupBox_VE(PSZAC pszFmtTemplate, ...);
	OLayoutForm * Splitter_PoAddGroupBoxAndLayoutForm_VL(OUT_OPTIONAL WGroupBox ** ppwGroupBox, PSZAC pszFmtTemplate, va_list vlArgs);
	OLayoutForm * Splitter_PoAddGroupBoxAndLayoutForm_VE(OUT_OPTIONAL WGroupBox ** ppwGroupBox, PSZAC pszFmtTemplate, ...);
	OLayoutForm * Splitter_PoAddGroupBoxAndLayoutForm_VE(PSZAC pszFmtTemplate, ...);
	OLayoutForm * Splitter_PoAddGroupBoxAndLayoutFormError_V(PSZAC pszFmtTemplate, ...);
	OLayoutHorizontal * Splitter_PoAddGroupBoxAndLayoutHorizontal_VE(PSZAC pszFmtTemplate, ...);
	OLayoutVerticalAlignTop * Splitter_PoAddGroupBoxAndLayoutVertical_VE(PSZAC pszFmtTemplate, ...);

	OLayoutForm * Splitter_PoAddWidgetAndLayoutForm();
	WEditTextAreaReadOnly * Splitter_PwAddEditTextAreaReadOnly(PSZAC pszLabel, const CString & sEditText);
	void Splitter_AddWidgetSpacer();
	void Splitter_AddCertificateInfo(TCertificate * pCertificate);
}; // WSplitter

class WSplitterVertical : public WSplitter
{
public:
	WSplitterVertical() : WSplitter(Qt::Vertical) { }
};

class WDocking : public QDockWidget
{
public:
	WDocking();
	void Widget_SetTitleFormat_VE_Gsb(PSZAC pszFmtTemplate, ...);
};

class WDockingSplitter : public WDocking
{
protected:
	WSplitterVertical * m_pSplitter;
public:
	WDockingSplitter();
	void Widget_ResizeTocontentOfSplitter();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Display widgets in a horizontal row (by default those widgets will be centered horizontally)
class OLayoutHorizontal : public QHBoxLayout
{
public:
	OLayoutHorizontal(PA_PARENT QWidget * pwParent);
	OLayoutHorizontal(PA_PARENT QBoxLayout * poParentLayout);
	OLayoutVertical * Layout_PoAddLayoutVerticalWithWidgets_VLZA(QWidget * pawWidgetFirst, va_list vlaArgs);
	OLayoutVertical * Layout_PoAddLayoutVerticalWithWidgets_VEZA(QWidget * pawWidgetFirst, ...);
	OLayoutVertical * Layout_PoAddLayoutVerticalWithButtons_VEZA(QWidget * pawButtonFirst, ...);
	void Layout_AddWidgetsH_VEZA(QWidget * pawWidgetFirst, ...);
	void Layout_AddIconLabelAndWidgetsH_VEZA(EMenuAction eMenuActionIcon, PSZAC pszmFirstLabelAndToolTip, QWidget * pawWidgetFirst, ...);
	void Layout_AddLabelAndWidgetH_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget);
	void Layout_AddLabelAndWidgetV_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget);
	void Layout_AddLabelsAndWidgetsV_VEZA(PSZAC pszmFirstLabelAndToolTip, ...);
	void Layout_AddLabelsAndWidgetsV_VLZA(PSZAC pszmFirstLabelAndToolTip, va_list vlaArgs);
};

//	Display widgets in an horizontal row aligned to the left (as opposed to be centered horizontally)
class OLayoutHorizontalAlignLeft : public OLayoutHorizontal
{
public:
	OLayoutHorizontalAlignLeft(PA_PARENT QBoxLayout * poParentLayout);
};

//	Display widgets on top of each other centered vertically
class OLayoutVertical : public QVBoxLayout
{
public:
	OLayoutVertical(PA_PARENT QWidget * pwParent);
	OLayoutVertical(PA_PARENT QBoxLayout * poParentLayout);
	QWidget * Layout_PwGetWidgetAt(int iWidget) const;
	void Layout_ResizeWidthsOfAllWidgets();
	void Layout_AddWidgetsAndResizeWidths_VEZA(QWidget * pawWidgetFirst, ...);
	void Layout_AddWidgetsAndResizeWidths_VLZA(QWidget * pawWidgetFirst, va_list vlArgs);
	OLayoutHorizontalAlignLeft * Layout_PoAddRowLabelsAndWidgets_VEZA(PSZAC pszmFirstLabelAndToolTip, ...);
	OLayoutHorizontalAlignLeft * Layout_PoAddRowLabelsAndWidgets_VLZA(PSZAC pszmFirstLabelAndToolTip, va_list vlArgs);
	WButtonTextWithIcon * Layout_PwAddRowButtonAndLabel(PSZAC pszmButtonTextAndToolTip, EMenuAction eMenuIconButton, PSZAC pszmLabelTextAndToolTip);
	WLabel * Layout_PwAddRowLabelEditButton(PSZAC pszmLabelTextAndToolTipEdit, PA_CHILD WEdit * pwEdit, PA_CHILD QWidget * pawButton);
	WLabel * Layout_PwAddRowLabelAndButton(PSZAC pszmLabelTextAndToolTip, PA_CHILD QWidget * pawButton);
	WLabel * Layout_PwAddRowLabel(PSZAC pszmLabelTextAndToolTip);
	WLabel * Layout_PwAddRowLabel_VE(PSZAC pszmFmtLabelTextAndToolTip, ...);
	WLabel * Layout_PwAddRowLabelWrap(PSZAC pszmLabelTextAndToolTip);
	QFrame * Layout_PwAddRowHorizonalLine();
	void Layout_AddWidgetSpacer();
};

//	Display widgets on top of each other and align them at the top (do not center them vertically)
class OLayoutVerticalAlignTop : public OLayoutVertical
{
public:
	OLayoutVerticalAlignTop(PA_PARENT QWidget * pwParent);
	OLayoutVerticalAlignTop(PA_PARENT QBoxLayout * poParentLayout);
};

class OLayoutForm : public QFormLayout
{
public:
	explicit OLayoutForm(PA_PARENT QWidget * pwParent);
	explicit OLayoutForm(PA_PARENT QBoxLayout * poParentLayout);
	void Layout_RemoveMargins();
	void Layout_AddRowLabelFormat_VE_Gsb(PSZAC pszFmtTemplate, ...);
	WEdit * Layout_PwAddRowLabelEdit(PSZAC pszLabel, const CString & sEditText);
	WEdit * Layout_PwAddRowLabelEditReadOnly(PSZAC pszLabel, const CString & sEditText);
	WEdit * Layout_PwAddRowLabelEditReadOnlyInt(PSZAC pszLabel, int nValue);
	WEdit * Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszLabel, const QByteArray & arrayb);
	WEdit * Layout_PwAddRowLabelEditReadOnlyFingerprint(PSZAC pszLabel, const QByteArray & arraybFingerprint);
	WEdit * Layout_PwAddRowLabelEditReadOnlyFingerprint(PSZAC pszLabel, const CBin & binFingerprint);
	WEditTextArea * Layout_PwAddRowLabelEditTextAreaH(PSZAC pszLabel, const CStr & strEditText, int cLines);
	WEditTextArea * Layout_PwAddRowLabelEditTextArea(PSZAC pszLabel, const CString & sEditText, int cLines);
	WEditTextArea * Layout_PwAddRowLabelEditTextAreaReadOnly(PSZAC pszLabel, const CString & sEditText, int cLines);
	WEditPassword * Layout_PwAddRowLabelEditPassword(PSZAC pszLabel);
	WEdit * Layout_PwAddRowLabelEditAndPushButton(PSZAC pszLabel, const QString & sEditText, PSZAC pszButtonText, OUT QPushButton ** ppwButton);
	WButtonTextWithIcon * Layout_PwAddRowLabelAndPushButton(PSZAC pszmLabelTextAndToolTip, PSZAC pszmButtonTextAndToolTip, EMenuAction eMenuIconButton);

	void Layout_AddRowLabelError(PSZAC pszLabel, const QString & sError);
	void Layout_AddRowErrors(const CArrayPsz & arraypszErrors);
	WSplitter * Layout_PwAddSplitterHorizontal();

	// Wrappers
	void Layout_AddFormCertificate(const CSslCertificate & oCertificate);
	void Layout_AddFormCertificate(const TCertificate * pCertificate);
}; // OLayoutForm


///////////////////////////////////////////////////////////////////////////////////////////////////
//	This class is a helper to add a row to a table.
class OTableRow : public QList<QStandardItem *>
{
public:
	void AddData(PSZUC pszuData);
	void AddData(const CStr & strData);
	void AddData(const QString & sData);
	void AddData(const QByteArray & arraybData);
	void AddDataBinaryHex(const QByteArray & arraybData);
	void AddDataFingerprint(const QByteArray & arraybFingerprint);
	void AddDate(const QDateTime & dtu);
	void AddDateTime(const QDateTime & dtu);
	void AddDateTime(TIMESTAMP ts);
	void AddAmount_BTC(AMOUNT amtSatoshis);
	void AddAmount_mBTC(AMOUNT amtSatoshis);
	void AddInt(int nValue);
	void AddData_VEZ(PSZAC pszData, ...);
};

//	Class containing friendly wrappers to the QTableView
class WTable : public QTableView
{
protected:
	QStandardItemModel * m_poModel;

public:
	explicit WTable(PA_PARENT QWidget * pwParent);
	explicit WTable(PA_PARENT QFormLayout * poParentLayout);
	void _Init();
	void SetColumns_VEZ(PSZAC pszNameColumn, ...);
	void AppendRow(OTableRow & oRow, LPARAM lParam = d_zNA);
	void AppendRow_VEZ(PSZAC pszData, ...);
	void AdjustHeightToFitRows();
	void clear();
	LPARAM LParamGetFromRow(const QModelIndex & oIndex);
	LPARAM LParamGetFromRowSelected();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	The class CSslCertificate adds friendly wrappers to the class QSslCertificate.
class CSslCertificate : public QSslCertificate
{
public:
	void GetCertificateName(OUT CStr * pstrName) const;
	PSZUC GetCertificateIssuer(OUT CStr * pstrIssuer) const;
	QByteArray GetHashValue(EHashAlgorithm eHashAlgorithm) const;
};

void Widget_SetText(INOUT QWidget * pwWidget, PSZAC pszText);
void Widget_SetText(INOUT QWidget * pwWidget, const CStr & strText);
void Widget_SetTextFormat_VE_Gsb(INOUT QWidget * pwWidget, PSZAC pszFmtTemplate, ...);
void Widget_SetTextFormat_VL_Gsb(INOUT QWidget * pwWidget, PSZAC pszFmtTemplate, va_list vlArgs);
void Widget_SetTextAndToolTip(INOUT QWidget * pwWidget, PSZAC pszmTextAndToolTip);
void Widget_SetToolTipNext(INOUT QWidget * pwWidget, PSZAC pszmTextAndToolTip);
void Widget_SetToolTipFormat_VE_Gsb(INOUT QWidget * pwWidget, PSZAC pszFmtTemplate, ...);
void Widget_SetTextColorError(INOUT QWidget * pwWidget);
void Widget_SetTextColorGray(INOUT QWidget * pwWidget);
int  Widget_GetWidthMinimumRecommended(const QWidget * pwWidget);
void Widget_SetWidth(INOUT QWidget * pwWidget, int nWidth);
void Widget_ScrollToEnd(INOUT QAbstractScrollArea * pwWidget);


void SetCursorWait();
void SetCursorRestoreDefault();
class CWaitCursor
{
public:
	inline CWaitCursor()	{ SetCursorWait(); }
	inline ~CWaitCursor()	{ SetCursorRestoreDefault(); }
};

void Clipboard_SetText(const QString & sText);
void Clipboard_SetText(const CStr & strText);
void Clipboard_SetText(const QLineEdit * pwEdit);

UINT UGenerateRandomValueRandomUntil(int nMax);
L64 LGenerateRandomValueBetween(L64 lMin, L64 lMax);

///////////////////////////////////////////////////////////////////////////////////////////////////
class CFile : public QFile
{
public:
	CFile(const CStr & strFileName);
	L64 CbDataReadAtOffset(L64 ibData, L64 cbData, OUT BYTE prgbBuffer[]);
	int CbDataWriteBinaryFromBase85(PSZUC pszuBase85);
};

class CFileOpenRead : public CFile
{
public:
	CFileOpenRead(const CStr & strFileName);
};

class CFileOpenWrite : public CFile
{
public:
	CFileOpenWrite(const CStr & strFileName);
};

#endif // QTUTILITIES_H
