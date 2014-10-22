///////////////////////////////////////////////////////////////////////////////////////////////////
//	ErrorMessages.cpp
//
//	Routine to map error codes to a human readable message.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef Q_OS_WIN
	#define CONFIG_MESSAGE_LOG_WIN32	// Create a Win-32 modeless dialog for logging messages.  If this flag is not defined, create a portable widget with a QTextBrowser window.
#endif

const QString c_sMessageLog("Message Log");
const QString c_sErrorLog("Error Log");
BOOL g_fMessageLogsPaused;			// Pause all Message Logs (no new entries are added)

/////////////////////////////////////////////////////////////////////
//	Select the text color based on the severity of a message.
QRGBX
CoxFromSeverity(ESeverity eSeverity)
	{
	switch (eSeverity)
		{
	case eSeverityNoise:
		return d_coGray;
	case eSeverityComment:
		return d_coGreen;
	case eSeverityInfoTextBlue:
		return d_coBlue;
	case eSeverityInfoTextBlueDark:
		return d_coBlueDark;
	case eSeverityNull:				// A 'null' severity is displayed in black, although it is typically an empty message
	case eSeverityInfoTextBlack:
		return d_coBlack;
	case eSeverityWarning:
		return COX_MakeBold(d_coOrange);
	case eSeverityWarningToErrorLog:
		return COX_MakeBold(d_coOrangeDark);
	case eSeverityErrorWarning:
		return COX_MakeBold(d_coRed);
	case eSeverityErrorAssert:
	/*
	case eSeverityErrorCritical:
	*/
		return COX_MakeBold(d_coRedHot);
	default:
		Assert(FALSE && "Unknown severity level");
		return COX_MakeBold(d_coRedHot);
		} // switch
	} // CoxFromSeverity()

QRgb
CoFromSeverity(ESeverity eSeverity)
	{
	return COX_GetQRGB(CoxFromSeverity(eSeverity));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CErrorMessage::CErrorMessage()
	{
	m_err = errSuccess;
	// For convenience, the following two fields are initialized to avoid displaying garbage
	m_eSeverity = eSeverityErrorWarning;
	m_nLine = d_zNA;
	// DEBUG_CODE( m_strErrorDetails.BinInitFromStringWithNullTerminator("[Debug] Error Details"); )
	}

CErrorMessage::~CErrorMessage()
	{
	}

void
CErrorMessage::ClearError()
	{
	m_err = errSuccess;
	m_nLine = d_zNA;
	m_strErrorDescription.Empty();
	m_strErrorDetails.Empty();
	}


//	Generic method to set the error
void
CErrorMessage::SetErrorCode(EError err)
	{
	Endorse(err == errSuccess);	// This should do no harm
	if (err != m_err)
		{
		// Cascade the error description from the previous error (if any) to the error detail.  This is useful for chaining errors.
		m_strErrorDetails.AppendSeparatorCommaAndTextU(PszuGetErrorDescriptionReFormatted());
		m_strErrorDescription.Empty();
		m_err = err;
		m_eSeverity = eSeverityErrorWarning;
		}
	} // SetErrorCode()

//	Set the error code with a with a description formatted with parameters.
//	The formatting template is determined from the error code.
void
CErrorMessage::SetErrorCodeFormatted(EError err, ...)
	{
	PSZAC pszFmtTemplate;
	switch (err)
		{
	case errParseXmlPrematureEof_i:
		pszFmtTemplate = "Unexpected end of XML data at line $i";
		break;
	case errParseXmlPrematureEofForCDATA_i:
		pszFmtTemplate = "Premature end of XML data while looking for termination of <![CDATA[ from line $i";
		break;
	case errParseXmlElementMismatch_ssi:
		pszFmtTemplate = "The closing element <$s> does not match its opening element <$s> from line $i";
		break;
	case errParseXmlAttributeInvalidName_s:
		pszFmtTemplate = "The attribute name '$s' is invalid";
		break;
	case errParseXmlAttributeMissingOpeningQuote_s:
		pszFmtTemplate = "The attribute '$s' is missing an opening quote for its value";
		break;
	case errParseXmlAttributeMissingClosingQuote_s:
		pszFmtTemplate = "The attribute '$s' is missing its closing quote for its value";
		break;
	case errParseXmlAttributeMissingValue_s:
		pszFmtTemplate = "The attribute '$s' is missing a value";
		break;
	/*
	case ERROR_PARSE_XML_ROOT_ELEMENT_MISSING:
		pszFmtTemplate = "The root element is missing";
		break;
	*/
	default:
		Assert(FALSE && "Unknown error code");
		return;
		} // switch

	va_list vlArgs;
	va_start(OUT vlArgs, err);
	SetErrorCodeFormatted_VL(err, (PSZUC)pszFmtTemplate, vlArgs);
	} // SetErrorCodeFormatted()


void
CErrorMessage::SetErrorCodeFormatted_VL(EError err, PSZUC pszuFmtTemlate, va_list vlArgs)
	{
	Assert(pszuFmtTemlate != NULL);
	SetErrorCode(err);	// Make sure m_strErrorDescription is empty.
	Assert(m_strErrorDescription.FIsEmptyString());		// If this happens, it is because err is the same as m_err, and the method SetErrorCode() did nothing.
	// Format the new error
	m_strErrorDescription.Format_VL((PSZAC)pszuFmtTemlate, vlArgs);
	}

//	Return a pointer to the error description.
//	This method may re-format the error description to include the line number (if any)
PSZUC
CErrorMessage::PszuGetErrorDescriptionReFormatted()
	{
	if (m_err != errSuccess && m_strErrorDescription.FIsEmptyString())
		m_strErrorDescription.FormatErrorCode(m_err);
	if (m_nLine > 0)
		{
		PSZUC pszuDescription = m_strErrorDescription;	// Keep a copy of the error description on the stack
		pszuDescription = STR_UtoU(pszuDescription);
		m_strErrorDescription.FormatErrorAtLineNumber(m_nLine);
		m_nLine = -m_nLine;
		m_strErrorDescription.AppendTextU(pszuDescription);
		}
	return m_strErrorDescription;
	}

BOOL
CErrorMessage::FDisplayToMessageLog()
	{
	if (m_err == errSuccess || EError_FuIsSilentError(m_err))
		return FALSE;
	(void)PszuGetErrorDescriptionReFormatted();
	MessageLog_AppendTextFormatSev(m_eSeverity, "$S\n\t$S\n", &m_strErrorDescription, &m_strErrorDetails);
	if (m_eSeverity >= eSeverityWarningToErrorLog)
		{
		#ifdef DEBUG
		MessageLog_Show();
		#endif
		ErrorLog_AddNewMessage(m_strErrorDescription.PszaGetUtf8NZ(), m_strErrorDetails);
		}
	// Finally, make the error silent, so we don't display the same error twice
	m_err = EError_EMakeSilentError(m_err);
	Assert(EError_FuIsSilentError(m_err));
	return TRUE;
	} // FDisplayToMessageLog()

void
CStr::FormatErrorCode(EError err)
	{
	Assert(FALSE && "NYI!");
	Assert(err != errSuccess);
	}

void
CStr::FormatErrorAtLineNumber(int nLineNumberError)
	{
	Empty();
	if (nLineNumberError > 0)
		Format("Error at line $i:", nLineNumberError);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Widget/window displaying debugging messages to the user.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_MESSAGE_LOG_WIN32

const WCHAR c_szwRegPathMessageLog[] = L"Software\\Cambrian.org\\Chat";

#include <richedit.h>	// The MessageLog is implemented as a RichEdit

extern HWND g_hwndMainWindow;
HINSTANCE g_hInstance = ::GetModuleHandle(NULL);
ATOM g_atomMessageLogRegistration;

#define IDM_EDIT_COPY_SELECTION		1
#define IDM_EDIT_CUT_SELECTION		2
#define IDM_EDIT_CLEAR_SELECTION	3
#define IDM_EDIT_CLEAR_ALL			4
#define IDM_EDIT_SELECT_ALL			5
#define IDM_PAUSE					6

///////////////////////////////////////////////////////////////////////////////////////////////////
void
Menu_AddItem(INOUT HMENU hMenu, UINT idMenuItem, PSZAC pszMenuItemText)
	{
	Assert(hMenu != NULL);
	Assert(pszMenuItemText != NULL);
	MENUITEMINFO mii;
	InitToZeroes(OUT &mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_TYPE;
	mii.wID = idMenuItem ;
	mii.fType = MFT_STRING;
	mii.dwTypeData = (LPWSTR)STR_UtoW((PSZUC)pszMenuItemText);
	Verify( ::InsertMenuItem(hMenu, 9999, TRUE, IN &mii) );
	//PSZWC pszwError = STR_UtoW(pszuError);
	/*
	BOOL fSuccess = ::InsertMenuItem(hMenu, d_zNA, FALSE, IN &mii);
	Assert(fSuccess);
	*/
	} // Menu_AddItem()

void
Menu_AddSeparator(HMENU hMenu)
	{
	MENUITEMINFO mii;
	InitToGarbage(OUT &mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	Verify( ::InsertMenuItem(hMenu, 9999, TRUE, IN &mii) );
	}

#define Menu_EnableItem(hMenu, idMenuItem, fEnable)	::EnableMenuItem(hMenu, idMenuItem, (fEnable) ? MF_ENABLED : MF_GRAYED)

//	Return the handle of the RichEdit control within the MessageLog.
HWND
MessageLog_HGetRichEdit(HWND hwndMessageLog)
	{
	return ::GetWindow(hwndMessageLog, GW_CHILD);	// Since the MessageLog has only one child, we may use GetWindow()
	}

#define EnableMenuCommand(hMenu, idMenuItem, fEnable)	::EnableMenuItem(hMenu, idMenuItem, (fEnable) ? MF_ENABLED : MF_GRAYED)

///////////////////////////////////////////////////////////////////////////////////////////////////
void
MessageLog_DoContextMenu(HWND hwndMessageLog)
	{	
	HMENU haMenu = ::CreatePopupMenu();
	Menu_AddItem(INOUT haMenu, IDM_PAUSE, "&Pause Log");
	::CheckMenuItem(haMenu, IDM_PAUSE, g_fMessageLogsPaused ? MF_CHECKED : MF_UNCHECKED);
	Menu_AddSeparator(INOUT haMenu);
	Menu_AddItem(INOUT haMenu, IDM_EDIT_COPY_SELECTION, "&Copy");
	Menu_AddItem(INOUT haMenu, IDM_EDIT_CUT_SELECTION, "C&ut");
	Menu_AddItem(INOUT haMenu, IDM_EDIT_SELECT_ALL, "Select &All");
	Menu_AddSeparator(INOUT haMenu);
	Menu_AddItem(INOUT haMenu, IDM_EDIT_CLEAR_ALL, "Clear &Log");

	HWND hwndRichEdit = MessageLog_HGetRichEdit(hwndMessageLog);
	int ichSelectionStart;
	int ichSelectionEnd;
	SendMessage(hwndRichEdit, EM_GETSEL, OUT (WPARAM)&ichSelectionStart, OUT (LPARAM)&ichSelectionEnd);
	const int cchSelection = ichSelectionEnd - ichSelectionStart;
	Menu_EnableItem(haMenu, IDM_EDIT_COPY_SELECTION, cchSelection);
	Menu_EnableItem(haMenu, IDM_EDIT_CUT_SELECTION, cchSelection);

	POINT pt;
	::GetCursorPos(OUT &pt);
	const int nCmdId = ::TrackPopupMenu(haMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x + 2, pt.y + 2, d_zNA, g_hwndMainWindow, NULL);
	Verify( ::DestroyMenu(haMenu) );

	switch (nCmdId)
		{
	case 0:
		// The user selected nothing
		return;
	case IDM_EDIT_CUT_SELECTION:
		// Currint the selection cannot be achieved with the WM_CUT message because the RichEdit is in read-only.  Instead, the selection is copied, and the selection is replaced with an empty string.
		::SendMessage(hwndRichEdit, WM_COPY, d_zNA, d_zNA);
		::SendMessage(hwndRichEdit, EM_REPLACESEL, FALSE /* fCanUndo */, (LPARAM)NULL);
		break;
	case IDM_EDIT_COPY_SELECTION:
		::SendMessage(hwndRichEdit, WM_COPY, d_zNA, d_zNA);
		break;
	case IDM_EDIT_SELECT_ALL:
		::SendMessage(hwndRichEdit, EM_SETSEL, 0, -1);
		break;
	case IDM_EDIT_CLEAR_ALL:
		::SendMessage(hwndRichEdit, WM_SETTEXT, d_zNA, (LPARAM)NULL);
		break;
	case IDM_PAUSE:
		g_fMessageLogsPaused = ~g_fMessageLogsPaused;
		break;
		} // switch
	} // MessageLog_DoContextMenu()

#define d_cchLimitText		1024000		// Allow the RichEdit to contain about 1 MB of text rather than the default value of 32 KB. This value must be larger than d_cchLogMax
#define d_cchLogMax			1000000		// Limit the MessageLo to 1 million characters.  This should be enough to display a meaningful message.
#define d_cchEndOfLog		0x0FFF7FFF	// Seek to the 268th million character (this should seek to the end of the log).  This value is crafted to work on both 16-bit and 32-bit version of the EM_SETSEL.

#define d_nTabStopSpacing4Tabs	10	// Somewhow the value of 10 gives a tab stop length of exactly 4 spaces.  According to the documentation, this value should be 8, but it works

void
RichEdit_SelectionMoveToEnd(HWND hwndRichEdit)
	{
	Assert(IsWindow(hwndRichEdit));
	::SendMessage(hwndRichEdit, EM_SETSEL, (WPARAM)d_cchEndOfLog, (LPARAM)d_cchEndOfLog);	// Move the selection at the end of the log
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	RichEdit_SetTabSizeTo4Spaces()
//
//	Set the tab size to 4 spaces for the multi-line edit control.
//	By default, the Rich Edit control set the tab size to 16 characters, however when displaying certain text such as XML, this value is too much.
//	A tab size of 4 spaces is preferred.
//
//	This method uses the message EM_SETTABSTOPS which can set the individual tab stops or the tab size.
//		wParam is the number of tab stops contained in the array. If this parameter is zero, the lParam parameter is ignored and default tab stops are set at every 32 dialog template units. If this parameter is 1, tab stops are set at every n dialog template units, where n is the distance pointed to by the lParam parameter. If this parameter is greater than 1, lParam is a pointer to an array of tab stops.
//		lParam is a pointer to an array of unsigned integers specifying the tab stops, in dialog template units. If the wParam parameter is 1, this parameter is a pointer to an unsigned integer containing the distance between all tab stops, in dialog template units.
//			Windows 95/98/Me: The buffer pointed to by lParam must reside in writable memory, even though the message does not modify the array.
//
void
RichEdit_SetTabSizeTo4Spaces(HWND hwndRichEdit)
	{
	Assert(IsWindow(hwndRichEdit));
	int nTabStop = d_nTabStopSpacing4Tabs;	// This variable must be writable memory for Windows 95/98/Me.
	::SendMessage(hwndRichEdit, EM_SETTABSTOPS, 1, INOUT IGNORED (LPARAM)&nTabStop);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Window procedure of the MessageLog.
//	A RichEdit control is inside the MessageLog as a child window.
//
static LRESULT CALLBACK
WndProcMessageLog(HWND hwndMessageLog, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	Assert(IsWindow(hwndMessageLog));
	switch (uMsg)
		{
	case WM_CREATE:
		{
		const CREATESTRUCT * pCreateStruct = (const CREATESTRUCT *)lParam;
		HWND hwndEditLog = ::CreateWindowEx(
			WS_EX_NOPARENTNOTIFY,
			L"RichEdit20W",	// Create a Unicode RichEdit control (RichEdit20W creates a version 3.0 if available)
			NULL,			// No text yet
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
			0, 0, pCreateStruct->cx, pCreateStruct->cy,
			hwndMessageLog,
			0,				// No menu / No ID
			g_hInstance,
			d_zNA);
		Assert(hwndEditLog != NULL);
		::SendMessage(hwndEditLog, EM_EXLIMITTEXT, d_zNA, d_cchLimitText);	// This line  of code is necessary otherwise the log will be truncated to 32 KB, even if the documentation says otherwise.
		::SendMessage(hwndEditLog, EM_SETREADONLY, TRUE, d_zNA);			// Set the read-only without the gray background read-only color
		::SendMessage(hwndEditLog, EM_SETTEXTMODE, TM_RICHTEXT, d_zNA);	// Rich text enables the coloring
		::SendMessage(hwndEditLog, WM_SETFONT, (WPARAM)(HFONT)::GetStockObject(DEFAULT_GUI_FONT), d_zNA);
		RichEdit_SetTabSizeTo4Spaces(hwndEditLog);
		}
		return 0;
	case WM_DESTROY:
		{
		// Save the position of the window in the registry
		WCHAR szwName[100];
		GetWindowText(hwndMessageLog, OUT szwName, LENGTH(szwName));
		WINDOWPLACEMENT wp;
		::GetWindowPlacement(hwndMessageLog, OUT &wp);
		HKEY haKey = NULL;
		DWORD dwDisposition;
		::RegCreateKeyEx(HKEY_CURRENT_USER, c_szwRegPathMessageLog, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_QUERY_VALUE, NULL, OUT &haKey, OUT IGNORED &dwDisposition);
		Report(haKey != NULL);
		::RegSetValueEx(haKey, szwName, d_zNA, REG_BINARY, IN (const BYTE *)&wp.rcNormalPosition, sizeof(wp.rcNormalPosition));
		(void)::RegCloseKey(PA_DELETING haKey);
		}
		return 0;
	case WM_CLOSE:
		::ShowWindow(hwndMessageLog, SW_HIDE);	// Just hide the window, and prevent it from being destroyed
		return 0;
	case WM_SIZE:
		::MoveWindow(MessageLog_HGetRichEdit(hwndMessageLog), 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 200;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = 70;
		return 0;
	case WM_CONTEXTMENU:
		MessageLog_DoContextMenu(hwndMessageLog);
		return 0;
		} // switch
	return ::DefWindowProc(hwndMessageLog, uMsg, wParam, lParam);
	} // WndProcMessageLog()

// Minimum visible width
#define d_cxWidthVisibleMinimum	200
#define d_cyWidthVisibleMinimum	200

//	Ensure a window is partially visible within the screen.
//
//	- If the input prcWindowPos is completely out of the screen coordinates,
//	  then the routine will make sure the window rectangle becomes FULLY visible.
//	- If the input prcWindowPos is partially out of the screen coordinates,
//	  then the routine will make a reasonable size (200 x 200) of the window is visible.
VOID
WindowRectEnsurePartiallyVisibleWithinScreen(INOUT RECT * prcWindowPos)
	{
	Assert(prcWindowPos != NULL);
	RECT rcWindowPos = *prcWindowPos;	// Make a copy on the stack for efficiently
	int cxWindow = rcWindowPos.right - rcWindowPos.left;
	int cyWindow = rcWindowPos.bottom - rcWindowPos.top;
	Assert(cxWindow > 0 && cxWindow < 10000);
	Assert(cyWindow > 0 && cyWindow < 10000);

	HMONITOR hMonitor = ::MonitorFromRect(IN prcWindowPos, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	InitToGarbage(OUT &mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(hMonitor, OUT &mi);
	// Resize the window if larger than the screen (this is very rare)
	if (cxWindow > mi.rcWork.right)
		cxWindow = mi.rcWork.right;
	if (cyWindow > mi.rcWork.bottom)
		cyWindow = mi.rcWork.bottom;

	// If the window is completely out of screen coordinates, then make it fully visible
	if (rcWindowPos.left > mi.rcWork.right)
		rcWindowPos.left = mi.rcWork.right - cxWindow;
	if (rcWindowPos.top > mi.rcWork.bottom)
		rcWindowPos.top = mi.rcWork.bottom - cyWindow;

	// Then, if the window is partially visible, then
	// make sure there is enough of the window that is visible to the user
	if (rcWindowPos.left > mi.rcWork.right - d_cxWidthVisibleMinimum)
		rcWindowPos.left = mi.rcWork.right - d_cxWidthVisibleMinimum;
	if (rcWindowPos.top > mi.rcWork.bottom - d_cyWidthVisibleMinimum)
		rcWindowPos.top = mi.rcWork.bottom - d_cyWidthVisibleMinimum;
	if (rcWindowPos.left < mi.rcWork.left)
		rcWindowPos.left = mi.rcWork.left;
	if (rcWindowPos.top < mi.rcWork.top)
		rcWindowPos.top = mi.rcWork.top;

	rcWindowPos.right = rcWindowPos.left + cxWindow;
	rcWindowPos.bottom = rcWindowPos.top + cyWindow;
	*prcWindowPos = rcWindowPos;
	} // WindowRectEnsurePartiallyVisibleWithinScreen()

#else

class CTextBrowserMessageLog : public WTextBrowser
{
//	Q_OBJECT

public:
	explicit CTextBrowserMessageLog(QWidget * pwParent);
	virtual ~CTextBrowserMessageLog();
	virtual void contextMenuEvent(QContextMenuEvent * pEvent); // From QTextEdit
};

CTextBrowserMessageLog::CTextBrowserMessageLog(QWidget * pwParent) : WTextBrowser(pwParent)
	{
	document()->setMaximumBlockCount(10000);	// Keep the most recent 10,000 entries
	}
CTextBrowserMessageLog::~CTextBrowserMessageLog()
	{
	}

//	CTextBrowserMessageLog::QTextEdit::contextMenuEvent()
void
CTextBrowserMessageLog::contextMenuEvent(QContextMenuEvent * pEvent)
	{
	const QPoint ptMenu = pEvent->globalPos();
	QMenu * paMenu = createStandardContextMenu(ptMenu);
	QAction * pActionClearLog = paMenu->addAction("Clear Log");
	connect(pActionClearLog, SIGNAL(triggered()), this, SLOT(clear()));
	paMenu->exec(ptMenu);
	delete paMenu;
	}

class WChatDebugger : public QDockWidget
{
public:
	CTextBrowserMessageLog * m_pwTextBrowserMessageLog;
public:
	WChatDebugger(const QString & sName);
};


WChatDebugger::WChatDebugger(const QString & sName) : QDockWidget(sName, g_pwMainWindow)
	{
	setObjectName(sName);
	setVisible(false);
	setAllowedAreas(Qt::AllDockWidgetAreas);
	setFloating(true);

	QRect rc = QApplication::desktop()->screenGeometry();
	rc.adjust(rc.right() / 2, 50, -50, -100);
	int y = rc.bottom() * 3 / 4;
	if (sName == c_sMessageLog)
		rc.setBottom(y - 50);	// Position the MessageLog at the top
	else
		rc.setTop(y);		// Position the ErrorLog at the bottom
	setGeometry(rc);
	m_pwTextBrowserMessageLog = new CTextBrowserMessageLog(this);
	m_pwTextBrowserMessageLog->setTabStopWidth(16);	// Set the tab width to 16 pixels instead of 80 pixels
	setWidget(m_pwTextBrowserMessageLog);
	}

#endif // ~CONFIG_MESSAGE_LOG_WIN32

struct SErrorLogEntryHeaderOnly
{
	TIMESTAMP tsError;	// Date & time when the error occurred
};

struct SErrorLogEntry : SErrorLogEntryHeaderOnly
{
	CHU szmError[d_nLengthArrayDebug];	// Error Context + Error Description
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class containing a window displaying log messages.
//	This window is very useful for debugging.
//	This class exists becaue the Qt framework does not provide a window for logging,
//	and any normal window inheriting QWidget is not interactive when a dialog is visible.
class CMessageLog
{
protected:
	QString  m_sNameMessageLog;
	#ifdef CONFIG_MESSAGE_LOG_WIN32
	HWND m_hwndMessageLog;			// Global handle of the message log (this is not the handle to set the text, but the window containing the text control)
	HWND m_hwndRichEdit;			// This is the handle of the control displaying the RichEdit text of the message log
	#else
	WChatDebugger * m_pwChatDebugger;			// Pointer of the widget of the chat debugger (the widget may contain other debugging stuff, such as a toolbar)
	CTextBrowserMessageLog * m_pwTextBrowser;	// Pointer of actual chat log
	public:	void CreateWidget() { CreateLogWindow(); }	// Wrapper to create the MessageLog
	#endif

protected:
	void CreateLogWindow();
public:
	CMessageLog(const QString & sNameMessageLog);
	void Show();
	void Destroy();
	void Empty();
	void AppendTextU(QRGBX coxTextColor, PSZUC pszuTextAppend, PSZAC pszTextExtra = NULL);
	void AppendTextFormat(QRGBX coxTextColor, PSZAC pszFmtTemplate, ...);
	void AppendTextFormat_VL(QRGBX coxTextColor, PSZAC pszFmtTemplate, va_list vlArgs);
	void AppendTextError(const SErrorLogEntry * pError);
}; // CMessageLog

CMessageLog g_oMessageLog(c_sMessageLog);
CMessageLog g_oErrorLog(c_sErrorLog);

void
CMessageLog::CreateLogWindow()
	{
	#ifdef CONFIG_MESSAGE_LOG_WIN32
	LPCWSTR pszwClassMessageLog = (LPCWSTR)c_sMessageLog.utf16();
	LPCWSTR pszwWindowName = (LPCWSTR)m_sNameMessageLog.utf16();
	if (g_atomMessageLogRegistration == 0)
		{
		// Register the class of the MessageLog
		WNDCLASS wc = { CS_BYTEALIGNCLIENT | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, WndProcMessageLog, 0, 0, g_hInstance, NULL, NULL, NULL, NULL, pszwClassMessageLog };
		g_atomMessageLogRegistration = ::RegisterClass(IN &wc);
		Assert(g_atomMessageLogRegistration != 0);
		(void)::LoadLibrary(L"RICHED20");	// Make sure the RichEdit DLL is loaded, otherwise the MessageLog won't be able to create its RichEdit control
		}
	Assert(m_hwndMessageLog == NULL);
	Assert(m_hwndRichEdit == NULL);

	RECT rcLogWindow;
	HKEY haKey = NULL;
	(void)RegOpenKeyEx(HKEY_CURRENT_USER, c_szwRegPathMessageLog, 0, KEY_READ, PA_OUT &haKey);
	DWORD dwType;
	DWORD cbData = sizeof(rcLogWindow);
	LONG lRetCode = ::RegQueryValueEx(haKey, IN pszwWindowName, d_zNA, OUT IGNORED &dwType, OUT (BYTE *)&rcLogWindow, INOUT &cbData);
	(void)RegCloseKey(PA_DELETING haKey);
	if (lRetCode != ERROR_SUCCESS || cbData != sizeof(rcLogWindow))
		{
		// We were unable to the position of the window, so attempt to fit on the screen
		QRect rc = QApplication::desktop()->screenGeometry();
		rc.adjust(rc.right() / 2, 50, -50, -100);
		int y = rc.bottom() * 3 / 4;
		if (m_sNameMessageLog == c_sMessageLog)
			rc.setBottom(y - 50);	// Position the MessageLog at the top
		else
			rc.setTop(y);		// Position the ErrorLog at the bottom
		Assert(sizeof(rc) == sizeof(rcLogWindow));
		memcpy(OUT &rcLogWindow, IN &rc, sizeof(rcLogWindow));	// The implementation of QRect is the same as RECT
		}
	else
		{
		// Make sure the Chat Log fits within the screen
		WindowRectEnsurePartiallyVisibleWithinScreen(INOUT &rcLogWindow);
		}

	// Create the message log
	m_hwndMessageLog = ::CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW,
		pszwClassMessageLog,
		pszwWindowName,
		WS_SIZEBOX | WS_TILEDWINDOW,
		rcLogWindow.left, rcLogWindow.top, rcLogWindow.right - rcLogWindow.left, rcLogWindow.bottom - rcLogWindow.top,
		g_hwndMainWindow,
		NULL,
		g_hInstance,
		NULL);
	m_hwndRichEdit = MessageLog_HGetRichEdit(m_hwndMessageLog);
	if (!IsWindow(m_hwndRichEdit))
		{
		::OutputDebugStringA("Unable to create Rich Editor!");
		if (g_pwMainWindow != NULL)
			g_pwMainWindow->setWindowTitle("Unable to create Rich Editor!");
		}
	#else
	//	Create the chat debug window
	m_pwChatDebugger = new WChatDebugger(m_sNameMessageLog);
	g_pwMainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_pwChatDebugger);
	m_pwTextBrowser = m_pwChatDebugger->m_pwTextBrowserMessageLog;
	Assert(m_pwTextBrowser != NULL);
	#endif
	} // CreateLogWindow()

CMessageLog::CMessageLog(const QString & sNameMessageLog)
	{
	m_sNameMessageLog = sNameMessageLog;
	}

void
CMessageLog::Show()
	{
	#ifdef CONFIG_MESSAGE_LOG_WIN32
	if (m_hwndMessageLog == NULL)
		CreateLogWindow();
	::ShowWindow(m_hwndMessageLog, SW_SHOW);
	if (!g_fMessageLogsPaused)
		::SendMessage(m_hwndRichEdit, WM_VSCROLL, SB_BOTTOM, d_zNA);	// When the Message Log becomes visible, scroll at the bottom so the user may see the most recent messages.
	#else
	Report(m_pwChatDebugger != NULL && "The widget should have been created!");
	if (m_pwChatDebugger == NULL)
		CreateLogWindow();
	m_pwChatDebugger->show();
	#endif
	}

void
CMessageLog::Destroy()
	{
	#ifdef CONFIG_MESSAGE_LOG_WIN32
	::DestroyWindow(m_hwndMessageLog);
	//m_hwndMessageLog = NULL;
	#else
	#endif
	}

//	Empty the message log
void
CMessageLog::Empty()
	{
	#ifdef CONFIG_MESSAGE_LOG_WIN32
	if (m_hwndRichEdit != NULL)
		SendMessage(m_hwndRichEdit, WM_SETTEXT, d_zNA, (LPARAM)NULL);	// Set the text to an empty string
	#else
	if (m_pwTextBrowser != NULL)
		m_pwTextBrowser->clear();
	#endif
	}

void
CMessageLog::AppendTextU(QRGBX coxTextColor, PSZUC pszuTextAppend, PSZAC pszTextExtra)
	{
	if (g_fMessageLogsPaused)
		return;
#ifdef CONFIG_MESSAGE_LOG_WIN32
	if (g_hwndMainWindow == NULL)
		{
		// The main window has not been created yet, so this message is very early (typically an assertion failure within a constructor or something of that sort)
		//Assert(g_pwMainWindow == NULL);	// This pointer should also be NULL
		//Assert(m_hwndMessageLog == NULL);
		::OutputDebugStringA((PSZAC)pszuTextAppend);
		::OutputDebugStringA((PSZAC)pszTextExtra);
		return;
		}
	if (m_hwndMessageLog == NULL)
		CreateLogWindow();
	if (!IsWindow(m_hwndRichEdit))
		{
		::OutputDebugStringA("MessageLog: Rich Edit is unavailable\n");
		if (this != &g_oErrorLog)
			ErrorLog_AddNewMessage("MessageLog: Rich Edit is unavailable\n", pszuTextAppend);
		/*
		if (g_pwMainWindow != NULL)
			g_pwMainWindow->setWindowTitle(QString::fromUtf8((const char *)pszuTextAppend));	// Try to display something to the user using the caption as the "Message Log"
		*/
		return;
		}
	RichEdit_SelectionMoveToEnd(m_hwndRichEdit);
	int ichSel = 0;	// Index of the last selected character.
	::SendMessage(m_hwndRichEdit, EM_GETSEL, OUT (WPARAM)&ichSel, OUT (LPARAM)&ichSel);	// The start and end of the selection should be  the same, since we just called RichEdit_SelectionMoveToEnd().
	if (ichSel >= d_cchLogMax)
		{
		// The log is full, so flush some of the data
		::SendMessage(m_hwndRichEdit, EM_SETSEL, 0, d_cchLogMax / 4);	// Remove one fourth of the log
		::SendMessage(m_hwndRichEdit, EM_REPLACESEL, FALSE /* fCanUndo */, (LPARAM)L"[...]");
		RichEdit_SelectionMoveToEnd(m_hwndRichEdit);
		}

	// Set the character format to the insertion point
	CHARFORMAT cf;
	InitToGarbage(OUT &cf, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_BOLD;
	cf.crTextColor = COX_GetRGB(coxTextColor);
	cf.dwEffects = COX_IsBold(coxTextColor) ? CFE_BOLD : 0;
	Verify( ::SendMessage(m_hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, IN (LPARAM)&cf) );  // If the EM_SETCHARFORMAT fails, it may be because the method EditW_EnableColoring() was not called.

	SETTEXTEX st = { ST_SELECTION, CP_UTF8 };
	::SendMessage(m_hwndRichEdit, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)pszuTextAppend);
	if (pszTextExtra != NULL)
		::SendMessage(m_hwndRichEdit, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)pszTextExtra);
	::SendMessage(m_hwndRichEdit, WM_VSCROLL, SB_BOTTOM, d_zNA);	// Ensure the inserted text is visible by scrolling to the bottom
	::UpdateWindow(m_hwndRichEdit);	// Force the window to redraw immediately
#else
	Report(m_pwChatDebugger != NULL && "The widget should have been created!");
	if (m_pwChatDebugger == NULL)
		CreateLogWindow();
	Assert(m_pwChatDebugger != NULL);
	Assert(m_pwTextBrowser != NULL);
	if (m_pwTextBrowser != NULL)
		{
		QTextCursor oTextCursor = m_pwTextBrowser->textCursor();
		oTextCursor.movePosition(QTextCursor::End);
		m_pwTextBrowser->setTextCursor(oTextCursor);

		m_pwTextBrowser->setFontWeight(COX_IsBold(coxTextColor) ? QFont::Bold : QFont::Normal);
		m_pwTextBrowser->setTextColor(COX_GetQRGB(coxTextColor));
		m_pwTextBrowser->insertPlainText(QString::fromUtf8((char *)pszuTextAppend));
		if (pszTextExtra != NULL)
			m_pwTextBrowser->insertPlainText(QString::fromUtf8((char *)pszTextExtra));
		Widget_ScrollToEnd(m_pwTextBrowser);
		}
#endif
	} // AppendTextU()

void
CMessageLog::AppendTextFormat(QRGBX coxTextColor, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	AppendTextFormat_VL(coxTextColor, pszFmtTemplate, vlArgs);
	}

void
CMessageLog::AppendTextFormat_VL(QRGBX coxTextColor, PSZAC pszFmtTemplate, va_list vlArgs)
	{
	CStr str;
	str.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	AppendTextU(coxTextColor, str);
	}


//	Append the error if the log is created and visible
void
CMessageLog::AppendTextError(const SErrorLogEntry * pError)
	{
	Assert(pError != NULL);
	QString sDateTimeError = QDateTime::fromMSecsSinceEpoch(pError->tsError).toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
	AppendTextFormat(d_coBlack, "[$Q] ", &sDateTimeError);
	PSZUC pszmError = pError->szmError;
	AppendTextU(COX_MakeBold(d_coRedDark), pszmError, ":\n");
	AppendTextU(strcmpU(pszmError, c_szInternalErrorDetected) ? d_coRedDark : COX_MakeBold(d_coAssert), pszmError + CbAllocUtoU(pszmError), c_szNewLine);
	} // AppendTextError()

//	Initialize the MessageLog and ErrorLog
void
MessageLog_ModuleInitialize()
	{
	#ifndef CONFIG_MESSAGE_LOG_WIN32
	// If we are using the Qt widgets, we have to create them before restoring the state of the main window (so the widgets are positioned at the same place as the last session)
	g_oMessageLog.CreateWidget();
	g_oErrorLog.CreateWidget();
	#endif
	}

void
MessageLog_ModuleShutdown()
	{
	g_oMessageLog.Destroy();
	g_oErrorLog.Destroy();
	}

void
MessageLog_Show()
	{
	g_oMessageLog.Show();
	/*
	g_oMessageLog.AppendTextU(d_coBlack, (PSZUC)"For Debugging()\n");
	g_oMessageLog.AppendTextFormat(d_coGreenDark, "AppendTextFormat(sizeof(CHW)=$i bytes)\n", sizeof(CHW));
	g_oMessageLog.AppendTextFormat(d_coGreen, "^$s AppendTextFormat($s)\n", "hello");
	static int s_c = 0;
	s_c++;
	if (s_c == 1)
		{
		CStr str;
		str.BinAppendStringWithNullTerminator((PSZUC)"hello");
		g_oMessageLog.AppendTextFormat(d_coGoldenRod, "^$S AppendTextFormat($S)\n", &str);
		}
	if (s_c == 2)
		{
		QString s("hello");
		g_oMessageLog.AppendTextFormat(d_coOrange, "^$Q AppendTextFormat($Q)\n", &s);
		}
	if (s_c > 3)
		s_c = 0;
	*/
	}

void
MessageLog_AppendTextWithNewLine(QRGBX coxTextColor, PSZUC pszuTextAppend)
	{
	g_oMessageLog.AppendTextU(coxTextColor, pszuTextAppend, c_szNewLine);
	}

void
MessageLog_AppendTextFormatCo(QRGBX coxTextColor, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;

    va_start(OUT vlArgs, pszFmtTemplate);
	g_oMessageLog.AppendTextFormat_VL(coxTextColor, pszFmtTemplate, vlArgs);

}

void
MessageLog_AppendTextFormatSev(ESeverity eSeverity, PSZAC pszFmtTemplate, ...)
	{

    va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_oMessageLog.AppendTextFormat_VL(CoxFromSeverity(eSeverity), pszFmtTemplate, vlArgs);

    if (eSeverity >= eSeverityWarningToErrorLog)
		{
		#ifdef DEBUG
		MessageLog_Show();		// Any error automatically shows the Message Log
		#endif
		va_start(OUT vlArgs, pszFmtTemplate);
		CStr strError;
		ErrorLog_AddNewMessage((eSeverity != eSeverityErrorAssert) ? "Message Log Error" : "Message Log Critical Error", strError.Format_VL(pszFmtTemplate, vlArgs));
		}
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Array of strings representing recent errors.
//
//	Since most of the errors are handled by the GUI, there is no need to create an ErrorLog window/widget
//	unless explicitly requested by the user.  The ErrorLog differs from the MessageLog as only
//	errors are stored.  The MessageLog displays verbose information for debugging.
//
//	The ErrorLog is useful when doing technical support requiring the user to send to the developers the exact error.
//
//	IMPLEMENTATION NOTES
//	For efficiency, only the error context and error message are stored as an error.
//	The ErrorLog window is not created until explicitly requested by the user.
//

class CArrayPtrErrors : protected CArray
{
public:
	inline SErrorLogEntry ** PrgpGetErrorsStop(OUT SErrorLogEntry *** pppErrorStop) const { return (SErrorLogEntry **)PrgpvGetElementsStop((void ***)pppErrorStop); }
	void AddError_UI(const CBin * pbinError);
};

void
CArrayPtrErrors::AddError_UI(const CBin * pbinError)
	{
	int cbError = pbinError->CbGetData();
	SErrorLogEntry * paError = (SErrorLogEntry *)new CHU[sizeof(SErrorLogEntryHeaderOnly) + cbError];
	memcpy(OUT paError->szmError, IN pbinError->PvGetData(), cbError);
	paError->tsError = Timestamp_GetCurrentDateTime();
	if (Add(PA_CHILD paError) > 100)
		delete (SErrorLogEntry *)RemoveElementFirstPv();	// Make sure we keep only the last 100 errors
	g_oErrorLog.AppendTextError(IN paError);
	}
CArrayPtrErrors g_arraypErrors;

void
ErrorLog_Show()
	{
	g_oErrorLog.Show();
	g_oErrorLog.Empty();	// Empty any previous content of the Error Log

	// Populate the errors
	SErrorLogEntry ** ppErrorStop;
	SErrorLogEntry ** ppError = g_arraypErrors.PrgpGetErrorsStop(OUT &ppErrorStop);
	while (ppError != ppErrorStop)
		g_oErrorLog.AppendTextError(*ppError++);

	// Just for fun, display statistics regarding the number of assertions
	g_oErrorLog.AppendTextU(COX_MakeBold(d_coBlack), (PSZUC)"Summary of Errors:\n");
	g_oErrorLog.AppendTextFormat(g_cAssertionsFailed ? COX_MakeBold(d_coAssert) : d_coGreen, "Total number of internal errors: $L\n", g_cAssertionsFailed);
	#ifdef Q_OS_WIN
	extern UINT g_cTimerFailures;
	if (g_cTimerFailures > 0)
		g_oErrorLog.AppendTextFormat(d_coAssert, "Total number of timer failures: $I\n", g_cTimerFailures);
	#endif
	g_oErrorLog.AppendTextFormat(d_coGreen, "Total number of successful internal sanity checks: $L\n", g_cAssertionsTotalExecuted - g_cAssertionsFailed);
	}

//	Generic function to log an error
void
ErrorLog_AddNewMessage(PSZAC pszErrorContext, PSZUC pszErrorDetails)
	{
	Assert(pszErrorContext != NULL);
	if (pszErrorDetails == NULL)
		pszErrorDetails = c_szuEmpty;
	CBin binError;
	binError.BinAppendStringWithNullTerminator(pszErrorContext);
	binError.BinAppendStringWithNullTerminator((PSZAC)pszErrorDetails);
	g_arraypErrors.AddError_UI(IN &binError);
	}

//	Log an error from an XML node.  This kind of error is quite frequent,
//	as the chat is processing many XMPP stanzas.
void
ErrorLog_AddNewMessage(PSZAC pszErrorContext, const CXmlNode * pXmlNodeError)
	{
	Assert(pszErrorContext != NULL);
	CBin binError;
	binError.BinAppendStringWithNullTerminator(pszErrorContext);
	binError.BinAppendXmlNode(pXmlNodeError);
	binError.BinAppendNullTerminator();
	g_arraypErrors.AddError_UI(IN &binError);
	}

