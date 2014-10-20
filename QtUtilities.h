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
	void Widget_Invalidate() { update(); }			// Schedule a paint event for processing when Qt returns to the main event loop.
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
	WLabelIcon(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon);
};

//	Invisible widget, typically to add a space between buttons
class WSpacer : public QLabel
{

};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Button displaying an icon.  The difference between WButtonIconForToolbar and WLabelIcon is the button may be clicked while the label solely displays the icon/pixmap.
class WButtonIconForToolbar : public QToolButton
{
public:
	WButtonIconForToolbar(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon);
	WButtonIconForToolbar(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon);
	WButtonIconForToolbar(EMenuIcon eMenuIcon, PSZAC pszmToolTip = NULL);
	void _Init(EMenuIcon eMenuIcon);

	WMenu * PwGetMenu() const { return (WMenu *)menu(); }
};

//	Button displaying an icon and a menu when clicked
class WButtonIconForToolbarWithDropDownMenu : public WButtonIconForToolbar
{
public:
	WButtonIconForToolbarWithDropDownMenu(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon, PSZAC pszmButtonTextAndToolTip);
	WButtonIconForToolbarWithDropDownMenu(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon, PSZAC pszmButtonTextAndToolTip);

	void _InitStyleForDropdownMenu(BOOL fHideArrowOfDropDownMenu = FALSE);
};

class WButtonIconForToolbarWithDropDownMenuNoArrow : public WButtonIconForToolbarWithDropDownMenu
{
public:
	WButtonIconForToolbarWithDropDownMenuNoArrow(PA_PARENT QWidget * pwParent, EMenuIcon eMenuIcon);
	WButtonIconForToolbarWithDropDownMenuNoArrow(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon);
};

class WButtonText : public QPushButton
{
public:
	WButtonText(PSZAC pszmButtonTextAndToolTip);
	void Button_SetTextAndToolTip(PSZAC pszmButtonTextAndToolTip);
	void Button_SetIcon(EMenuIcon eMenuIcon);
};

//	This is a text button with an icon on the left of the text.  This is different than WButtonIconForToolbarWithDropDownMenu because no menu will be displayed when clicked.
class WButtonTextWithIcon : public WButtonText
{
public:
	WButtonTextWithIcon(PSZAC pszmButtonTextAndToolTip, EMenuIcon eMenuIcon);
};

//	A normal checkbox where the user may switched on (checked) or off (unchecked)
class WButtonCheckbox : public QCheckBox
{
public:
	WButtonCheckbox();
	WButtonCheckbox(PSZAC pszmButtonTextAndToolTip, BOOL fChecked);
};

//	A radio button where only one button may be selected (somewhat complement to a checkbox)
class WButtonRadio : public QRadioButton
{
public:

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
	WEditReadOnly(const CStr & strText);
	WEditReadOnly(const QString & sText);
};

//	Widget to edit and validate a username
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

//	Widget for the user to enter text to search.  This widget displays the 'search icon'
class WEditSearch : public WEdit
{
/*
	Q_OBJECT
protected:
	QToolButton * m_pwButtonSearch;		// Button to display the search icon located on the left of widget
*/

public:
	WEditSearch();
	WEditSearch(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon, PSZAC pszWatermark);
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
	WEditTextArea(int cLines);
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
	#ifndef COMPILE_WITH_CHATLOG_HTML
	OCursorSelectBlock(IEvent * pEvent, QTextEdit * pwEdit);
	#endif
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
	void Layout_MarginsClear();
	OLayoutVertical * Layout_PoAddLayoutVerticalWithWidgets_VLZA(QWidget * pawWidgetFirst, va_list vlaArgs);
	OLayoutVertical * Layout_PoAddLayoutVerticalWithWidgets_VEZA(QWidget * pawWidgetFirst, ...);
	OLayoutVertical * Layout_PoAddLayoutVerticalWithButtons_VEZA(QWidget * pawButtonFirst, ...);
	void Layout_AddWidgetsH_VEZA(QWidget * pawWidgetFirst, ...);
	void Layout_AddIconLabelAndWidgetsH_VEZA(EMenuIcon eMenuIcon, PSZAC pszmFirstLabelAndToolTip, QWidget * pawWidgetFirst, ...);
	void Layout_AddLabelAndWidgetH_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget);
	void Layout_AddLabelAndWidgetV_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget);
	void Layout_AddLabelsAndWidgetsV_VEZA(PSZAC pszmFirstLabelAndToolTip, ...);
	void Layout_AddLabelsAndWidgetsV_VLZA(PSZAC pszmFirstLabelAndToolTip, va_list vlaArgs);
};

//	Display widgets in an horizontal row aligned to the left (as opposed to be centered horizontally)
class OLayoutHorizontalAlignLeft : public OLayoutHorizontal
{
public:
	OLayoutHorizontalAlignLeft(PA_PARENT QWidget * pwParent);
	OLayoutHorizontalAlignLeft(PA_PARENT QBoxLayout * poParentLayout);
};
class OLayoutHorizontalAlignLeft0 : public OLayoutHorizontalAlignLeft
{
public:
	OLayoutHorizontalAlignLeft0(PA_PARENT QWidget * pwParent);
	OLayoutHorizontalAlignLeft0(PA_PARENT QBoxLayout * poParentLayout);
};

class OLayoutHorizontalAlignRight : public OLayoutHorizontal
{
public:
	OLayoutHorizontalAlignRight(PA_PARENT QBoxLayout * poParentLayout);
};

//	Display widgets on top of each other centered vertically
class OLayoutVertical : public QVBoxLayout
{
public:
	OLayoutVertical(PA_PARENT QWidget * pwParent);
	OLayoutVertical(PA_PARENT QBoxLayout * poParentLayout);
	void Layout_MarginsClear();
	void Layout_MarginsClearAndSpacing();
	QWidget * Layout_PwGetWidgetAt(int iWidget) const;
	void Layout_ResizeWidthsOfAllWidgets();
	void Layout_AddWidgetsAndResizeWidths_VEZA(QWidget * pawWidgetFirst, ...);
	void Layout_AddWidgetsAndResizeWidths_VLZA(QWidget * pawWidgetFirst, va_list vlArgs);
	OLayoutHorizontalAlignLeft * Layout_PoAddRowLabelsAndWidgets_VEZA(PSZAC pszmFirstLabelAndToolTip, ...);
	OLayoutHorizontalAlignLeft * Layout_PoAddRowLabelsAndWidgets_VLZA(PSZAC pszmFirstLabelAndToolTip, va_list vlArgs);
	WButtonTextWithIcon * Layout_PwAddRowButtonAndLabel(PSZAC pszmButtonTextAndToolTip, EMenuIcon eMenuIconButton, PSZAC pszmLabelTextAndToolTip);
	WEdit * Layout_PwAddRowLabelEditReadOnly(PSZAC pszmLabelTextAndToolTipEdit, PSZUC pszEditText);
	WEdit * Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszmLabelTextAndToolTipEdit, const CBin & bin);
	WLabel * Layout_PwAddRowLabelEditButton(PSZAC pszmLabelTextAndToolTipEdit, PA_CHILD WEdit * pwEdit, PA_CHILD QWidget * pawButton);
	WLabel * Layout_PwAddRowLabelAndButton(PSZAC pszmLabelTextAndToolTip, PA_CHILD QWidget * pawButton);
	WLabel * Layout_PwAddRowLabel(PSZAC pszmLabelTextAndToolTip);
	WLabel * Layout_PwAddRowLabel_VE(PSZAC pszmFmtLabelTextAndToolTip, ...);
	WLabel * Layout_PwAddRowLabelWrap(PSZAC pszmLabelTextAndToolTip);
	QFrame * Layout_PwAddRowHorizonalLine();
	void Layout_AddWidgets(QWidget * prgpWidgets[], int cWidgets);
	void Layout_AddWidgetSpacer();
};

//	Display widgets on top of each other and align them at the top (do not center them vertically)
class OLayoutVerticalAlignTop : public OLayoutVertical
{
public:
	OLayoutVerticalAlignTop(PA_PARENT QWidget * pwParent);
	OLayoutVerticalAlignTop(PA_PARENT QBoxLayout * poParentLayout);
};
class OLayoutVerticalAlignTop0 : public OLayoutVerticalAlignTop
{
public:
	OLayoutVerticalAlignTop0(PA_PARENT QWidget * pwParent);
	OLayoutVerticalAlignTop0(PA_PARENT QBoxLayout * poParentLayout);
};


class OLayoutForm : public QFormLayout
{
public:
	explicit OLayoutForm(PA_PARENT QWidget * pwParent);
	explicit OLayoutForm(PA_PARENT QBoxLayout * poParentLayout);
	void Layout_RemoveMargins();
	WLabel * Layout_PwAddRowLabelFormat_VE_Gsb(PSZAC pszFmtTemplate, ...);
	WLabel * Layout_PwAddRowBlankAndLabelDescription(PSZAC pszLabel);
	WLabel * Layout_PwAddRowLabelAndWidget(PSZAC pszLabel, PA_CHILD QWidget * pawWidget);
	WEdit * Layout_PwAddRowLabelEdit(PSZAC pszLabel, const CString & sEditText);
	WEdit * Layout_PwAddRowLabelEditReadOnly(PSZAC pszLabel, const CString & sEditText);
	WEdit * Layout_PwAddRowLabelEditReadOnlyInt(PSZAC pszLabel, int nValue);
	WEdit * Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszLabel, const CBin & bin);
	WEdit * Layout_PwAddRowLabelEditReadOnlyToHex(PSZAC pszLabel, const QByteArray & arrayb);
	WEdit * Layout_PwAddRowLabelEditReadOnlyFingerprint(PSZAC pszLabel, const QByteArray & arraybFingerprint);
	WEdit * Layout_PwAddRowLabelEditReadOnlyFingerprint(PSZAC pszLabel, const CBin & binFingerprint);
	WEditTextArea * Layout_PwAddRowLabelEditTextAreaH(PSZAC pszLabel, const CStr & strEditText, int cLines);
	WEditTextArea * Layout_PwAddRowLabelEditTextArea(PSZAC pszLabel, const CString & sEditText, int cLines);
	WEditTextArea * Layout_PwAddRowLabelEditTextAreaReadOnly(PSZAC pszLabel, const CString & sEditText, int cLines);
	WEditTextArea * Layout_PwAddRowLabelEditTextAreaReadOnlyToHex(PSZAC pszLabel, const CBin & bin, int cLines);
	WEditPassword * Layout_PwAddRowLabelEditPassword(PSZAC pszLabel);
	WEdit * Layout_PwAddRowLabelEditAndPushButton(PSZAC pszLabel, const QString & sEditText, PSZAC pszButtonText, OUT QPushButton ** ppwButton);
	WButtonTextWithIcon * Layout_PwAddRowLabelAndPushButton(PSZAC pszmLabelTextAndToolTip, PSZAC pszmButtonTextAndToolTip, EMenuIcon eMenuIconButton);

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

class WTreeWidget : public QTreeWidget
{
public:
	BOOL FIsEditingTreeWidgetItem() const { return (state() == EditingState); }	// This wrapper is necessary because the method state() is protected
	void DisableEditingTreeWidgetItem() { setState(NoState); }
	void DeleteAllRootNodesWhichAreEmpty();
	void ShowAllTreeItems();
	void ShowAllTreeItemsContainingText(const QString & sTextSearch);
};


//	This class is named CTreeWidgetItem because it is a wrapper to QTreeWidgetItem, however QTreeWidgetItem is NOT inheriting from QWidget nor from QObject.
class CTreeWidgetItem : public QTreeWidgetItem
{
public:
	inline void setVisible(bool fVisible) { setHidden(!fVisible); }
	void SetItemVisibleAlongWithItsParents(BOOL fVisible);
	void ItemFlagsAdd(Qt::ItemFlag efItemFlagsAdd);
	void ItemFlagsRemove(Qt::ItemFlag efItemFlagsRemove);
	void ItemFlagsEditingEnable() { ItemFlagsAdd(Qt::ItemIsEditable); }
	void ItemFlagsEditingDisable() { ItemFlagsRemove(Qt::ItemIsEditable); }
	void InitIconAndText(EMenuIcon eMenuIcon, PSZUC pszTextColumn1, PSZUC pszTextColumn2 = NULL, PSZUC pszTextColumn3 = NULL, PSZUC pszTextColumn4 = NULL);
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

void WidgetButton_SetTextAndToolTip(INOUT QAbstractButton * pwButton, PSZAC pszmButtonTextAndToolTip);
void Widget_SetIconButton(INOUT QAbstractButton * pwButton, EMenuIcon eMenuIcon);
void Widget_SetIcon(INOUT QWidget * pwWidget, EMenuIcon eMenuIcon);

void Layout_MarginsClear(INOUT QLayout * poLayout);

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


///////////////////////////////////////////////////////////////////////////////////////////////////
class CPainter : public QPainter
{
public:
	CPainter(QPaintDevice * pPaintDevice) : QPainter(pPaintDevice) { }
	void DrawLineHorizontal(int xLeft, int xRight, int yPos);
	void DrawLineVertical(int xPos, int yTop, int yBottom);
	void FillRectWithGradientVertical(const QRect & rcFill, QRGB coTop, QRGB coBottom);
};

//	Class having a 'boundary rectangle' where the painter is allowed to draw.
//	A typical use of this class is to paint a cell in a grid.
class CPainterCell : public CPainter
{
public:
	QRect m_rcCell;

public:
	CPainterCell(QWidget * pwWidget);
	void DrawTextWithinCell(const QString & sText);
	void DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...);

	void DrawIconAdjustLeft(const QIcon & oIcon);
	void DrawIconAdjustLeft(EMenuIcon eMenuIcon);

	void DrawIconAlignment(const QIcon & oIcon, Qt::Alignment eAlignment);
	void DrawIconAlignment(EMenuIcon eMenuIcon, Qt::Alignment eAlignment);
	void DrawIconAlignmentLeftBottom(EMenuIcon eMenuIcon);
	void DrawIconAlignmentRightBottom(EMenuIcon eMenuIcon);

	// Specific methods
	void DrawTextUnderlinedStyle(const QString & sText, Qt::PenStyle eStyle);
	int DrawNumberWithinCircle(int nNumber);
	void FillRect0(QRGB coBackgroundFill);
};


#endif // QTUTILITIES_H
