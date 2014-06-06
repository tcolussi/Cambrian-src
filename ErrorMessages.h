///////////////////////////////////////////////////////////////////////////////////////////////////
//	ErrorMessages.h
//
//	Class capable to handle cascading error messages.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ERRORMESSAGES_H
#define ERRORMESSAGES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Class to store any error or warning.
//	The reason for storing the error in a class rather
//	than displaying it immediately to the user allows
//	the caller routine to re-format the error message.
//
//	This class is mostly used to parse network responses without
//	displaying all errors to the users.  Some low-level errors, such
//	as file not found, may not constitute an error, but part of the network protocol.
class CErrorMessage
{
protected:
	// The following fields are protected to prevent direct manipulation.
	// The process is setting an error code, and fetching the error description only when necessary/requested.
	// During that request, if the error description is empty, then the default error description is formatted from the error code.
	EError m_err;					// If this value is errSuccess then the other fields are undefined.
	ESeverity m_eSeverity;	// Severity of the error.  This is useful to determine the color of the error message.  Sometimes a message may be a warning rather than an error.
	CStr m_strErrorDescription;		// Description of the error.  If this field is empty, then the description is fetched from the error code.
	CStr m_strErrorDetails;			// Details of the error.  The detail gives additional information about the error.
	int m_nLine;						// Line number where the error occurred.  If this value is zero, then the error message is not related to a specific line number.  If this number is negative, then the line number has already been stored in the error message.

public:
	CErrorMessage();
	~CErrorMessage();
	void ClearError();
	BOOL FIsError() const { return (m_err != errSuccess); }
	EError EGetErrorCode() const { return m_err; }

	void SetErrorCode(EError err);
	void SetErrorCodeFormatted(EError err, ...);
	void SetErrorCodeFormatted_VL(EError err, PSZUC pszuFmtTemlate, va_list vlArgs);

	void SetLineError(int nLine) { m_nLine = nLine; }
	void SetLineErrorSilent(int nLine) { m_nLine = -nLine; }
	UINT GetLineError() const { return (m_nLine >= 0) ? m_nLine : -m_nLine; }

	/*
	VOID MakeSilentError() { m_err = APIERR_MakeSilentError(m_err); }

	APIERR ESetErrorHyperDataXmlNode(const CXmlNode * pXmlNodeError);
	APIERR ESetErrorHyperDataXmlServer(class CInternetServerXml * pInternetServer);
	APIERR ESetErrorHyperDataRequest(const class CInternetRequest * pRequest);

	VOID FormatErrorFromAPIERR(APIERR err, ...);
	VOID FormatErrorX(APIERR err, PSZAC pszFmtTemplate, ...);		// Display a generic error message followed by a description of the error
	VOID FormatErrorX_VL(APIERR err, PSZAC pszFmtTemplate, va_list vlArgs);

	VOID SetErrorAsWarning() { m_eSeverity = eSeverityWarning; }
*/
	PSZUC PszuGetErrorDescriptionReFormatted();
	BOOL FDisplayToMessageLog();
}; // CErrorMessage

void MessageLog_ModuleInitialize();

void MessageLog_Show();
void MessageLog_AppendTextWithNewLine(QRGBX coxTextColor, PSZUC pszuTextAppend);
void MessageLog_AppendTextFormatCo(QRGBX coxTextColor, PSZAC pszFmtTemplate, ...);
void MessageLog_AppendTextFormatSev(ESeverity eSeverity, PSZAC pszFmtTemplate, ...);

void ErrorLog_Show();
void ErrorLog_Hide();
void ErrorLog_AddNewMessage(PSZAC pszErrorContext, PSZUC pszErrorDetails);
void ErrorLog_AddNewMessage(PSZAC pszErrorContext, const CXmlNode * pXmlNodeError);

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (* PFn_StatusBarDisplayFunction)(ESeverity eSeverity, PSZUC pszuTextStatusBar);		//	Interface to display/route the messages destined to the status bar towards another window.
void StatusBar_SetDisplayFunction(PFn_StatusBarDisplayFunction pfnStatusBarDisplayFunction);
void StatusBar_RemoveDisplayFunction();
void StatusBar_DisplayFunctionDefault(ESeverity eSeverity, PSZUC pszuTextStatusBar);
void StatusBar_ClearText();
void StatusBar_SetTextU(PSZUC pszuText);
void StatusBar_SetTextErrorU(PSZUC pszuTextError);
void StatusBar_SetTextSevU(ESeverity eSeverity, PSZUC pszuText);
void StatusBar_SetTextFormat(PSZAC pszFmtTemplate, ...);
void StatusBar_SetTextFormatError(PSZAC pszFmtTemplate, ...);
void StatusBar_SetTextFormatSev(ESeverity eSeverity, PSZAC pszFmtTemplate, ...);
void StatusBar_SetTextFormatSev_VL(ESeverity eSeverity, PSZAC pszFmtTemplate, va_list vlArgs);

extern CStr g_strScratchBufferStatusBar;


//	The values of this enumeration must be in sync with QMessageBox::StandardButton
enum EAnswer
	{
	ezAnswerNone = QMessageBox::NoButton,
	eAnswerOk = QMessageBox::Ok,
	eAnswerSave = QMessageBox::Save,
	eAnswerSaveAll = QMessageBox::SaveAll,
	eAnswerOpen = QMessageBox::Open,
	eAnswerYes = QMessageBox::Yes,
	eAnswerYesToAll = QMessageBox::YesToAll,
	eAnswerNo = QMessageBox::No,
	eAnswerNoToAll = QMessageBox::NoToAll,
	eAnswerAbort = QMessageBox::Abort,
	eAnswerRetry = QMessageBox::Retry,
	eAnswerIgnore = QMessageBox::Ignore,
	eAnswerClose = QMessageBox::Close,
	eAnswerCancel = QMessageBox::Cancel,
	eAnswerDiscard = QMessageBox::Discard,
	eAnswerHelp = QMessageBox::Help,
	eAnswerApply = QMessageBox::Apply,
	eAnswerReset = QMessageBox::Reset
	};

EAnswer EMessageBoxQuestion(PSZAC pszFmtTemplate, ...);
EAnswer EMessageBoxInformation(PSZAC pszFmtTemplate, ...);
EAnswer EMessageBoxWarning(PSZAC pszFmtTemplate, ...);

#endif // ERRORMESSAGES_H
