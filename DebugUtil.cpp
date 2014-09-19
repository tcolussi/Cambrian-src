///////////////////////////////////////////////////////////////////////////////////////////////////
//	DebugUtil.cpp
//	Debug utilities, mainly the code behind the Assert() macro.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
/*
#define Q_ASSERT(cond) if(!(cond))
{ QMessageBox::critical(NULL, QApplication::tr("Critical error"), QApplication::tr("Assertion '<em>%1</em>' has failed\n in file '<strong>%2</strong>', line %3.").arg($cond).arg(__FILE__).arg(__LINE__));
qFatal("Assertion failed: \"%s\" in file %s, line %d", $cond, __FILE__, __LINE__); }
*/

#ifdef DEBUG_WANT_ASSERT

L64 g_cAssertionsTotalExecuted;	// Total number of executed assertions.  This number is for statistical purpose, to see how many times the Assert()/Report() macro has been used.
L64 g_cAssertionsFailed;			// Number of failed assertions

BOOL g_fIgnoreAllAsserts;
CStr g_strAssertLast;	// Remember the content of the last assertion.  This is to prevent the same assertion to be displayed over and over again, as a long loop could have the same assertion failure several thousand times

#ifdef DEBUG
	#define DEBUG_ASSERT_NON_BLOCKING	// Comment this line to have a blocking Assert().  A blocking Assert() is useful to step into the debugger at the moment the assertion fails.
#else
	#define DEBUG_ASSERT_NON_BLOCKING	// On a release build, NEVER allow an assert to block
#endif

#if 1
#include "DialogAccountNew.h"

DDialogAssertionFailure * g_pawlistAssertionFailures;	// Linked list of all the assertion failures (this list is used so Cambrian does not display the same assertion failure twice)

DDialogAssertionFailure::DDialogAssertionFailure(const CStr & strMessageHtml, const CStr & strAssert) : DDialogOkCancelWithLayouts(c_szInternalErrorDetected, eMenuAction_ShowLogErrors)
	{
	m_pNext = g_pawlistAssertionFailures;
	g_pawlistAssertionFailures = this;
	m_strAssert = strAssert;
	m_cAsserts = 1;
	setMinimumWidth(500);
	#if 1
	WLabel * pwLabel = new WLabel;
	//pwLabel->setTextFormat(Qt::RichText);
	pwLabel->setText(strMessageHtml);
	pwLabel->setWordWrap(true);
	DialogBody_AddRowWidget_PA(pwLabel);
	#else
	DialogBody_AddRowWidget_PA(new WLabelSelectableWrap(strMessageHtml));	// I have no idea this code does not work, especially when an assertion contains the logical OR operator ("||")
	#endif

	WButtonTextWithIcon * pwButtonClose = new WButtonTextWithIcon("&Close|Dismiss this error message", eMenuAction_Close);
	WButtonTextWithIcon * pwButtonCopyToClipboard = new WButtonTextWithIcon("Copy|Copy the invitation link into the clipboard", eMenuAction_Copy);
	WButtonTextWithIcon * pwButtonViewErrorLog = new WButtonTextWithIcon("View Error Log...|Display the Error Log which displays detailed information about previous errors", eMenuAction_ShowLogErrors);
	m_poLayoutButtons->Layout_AddWidgetsAndResizeWidths_VEZA(pwButtonClose, pwButtonCopyToClipboard, pwButtonViewErrorLog, NULL);
	connect(pwButtonClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(pwButtonCopyToClipboard, SIGNAL(clicked()), this, SLOT(SL_CopyToClipboard()));
	connect(pwButtonViewErrorLog, SIGNAL(clicked()), this, SLOT(SL_ShowErrorLog()));
	show();
	}

DDialogAssertionFailure::~DDialogAssertionFailure()
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "DDialogAssertionFailure::~DDialogAssertionFailure(0x$p)\n$S\n", this, &m_strAssert);
	DDialogAssertionFailure * pwDialogPrevious = NULL;
	DDialogAssertionFailure * pwDialog = g_pawlistAssertionFailures;
	while (pwDialog != NULL)
		{
		if (pwDialog == this)
			{
			if (pwDialogPrevious == NULL)
				g_pawlistAssertionFailures = m_pNext;
			else
				pwDialogPrevious->m_pNext = m_pNext;
			return;
			}
		pwDialogPrevious = pwDialog;
		pwDialog = pwDialog->m_pNext;
		} // while
	Assert(FALSE && "Unreachable code");
	}

void
DDialogAssertionFailure::reject()
	{
	deleteLater();	// Delete the 'this' pointer
	}

void
DDialogAssertionFailure::IncreaseAssert()
	{
	CStr strCaption;
	Dialog_SetCaption(IN (PSZAC)strCaption.Format("$s ($I times)", c_szInternalErrorDetected, ++m_cAsserts));
	showNormal();
	raise();
	}

void
DDialogAssertionFailure::SL_CopyToClipboard()
	{
	m_strAssert.CopyStringToClipboard();
	}

void
DDialogAssertionFailure::SL_ShowErrorLog()
	{
	ErrorLog_Show();
	}

#endif // DEBUG_ASSERT_NON_BLOCKING

/////////////////////////////////////////////////////////////////////
//	Return TRUE to break into the debugger
BOOL
_FAssertionFailed(
	PSZAC pszFile,
	int nLine,
	PSZAC pszExpression,
	BOOL fAssert)	// Source is Assert() or Report()
	{
	g_cAssertionsFailed++;
	PSZAC pszAssert = fAssert ? "Assert" : "Report";
	qDebug("%s(%d): %s(%s)", pszFile, nLine, pszAssert, pszExpression);

	// Dump the content of the assertion to the Message Log and Error Log
	CStr strAssert;
	PSZUC pszuAssert = strAssert.Format("[$@] $s($s)\n\tFile $s, line $i.", pszAssert, pszExpression, pszFile, nLine);
	const BOOL fAssertionFailureSameAsPrevious = g_strAssertLast.FCompareBinary(strAssert);
	if (!fAssertionFailureSameAsPrevious)
		{
		g_strAssertLast = strAssert;
		MessageLog_AppendTextWithNewLine(fAssert ? COX_MakeBold(d_coAssert) : COX_MakeBold(d_coOrangeDark), pszuAssert);
		MessageLog_Show();
		ErrorLog_AddNewMessage(c_szInternalErrorDetected, pszuAssert);
		}
	CStr strMessageHtml;
	strMessageHtml.Format("<b>$s</b>(<b>^s</b>)<br/>File $s, line $i.<br/><br/>"
		"You have discovered a potential bug; <u>please</u> contact the developers of SocietyPro.",
		pszAssert, pszExpression, pszFile, nLine);
	//qDebug(strMessageHtml);

	#ifdef DEBUG_ASSERT_NON_BLOCKING
	// Before displaying a new window with the error, search if the assertion is already there
	strAssert.Format("$s($s)\n\tFile $s, line $i.", pszAssert, pszExpression, pszFile, nLine);	// Reformat the assertion without the timestamp
	DDialogAssertionFailure * pwDialog = g_pawlistAssertionFailures;
	while (pwDialog != NULL)
		{
		if (pwDialog->m_strAssert.FCompareBinary(strAssert))
			{
			pwDialog->IncreaseAssert();
			return FALSE;
			}
		pwDialog = pwDialog->m_pNext;
		}
	(void)new DDialogAssertionFailure(strMessageHtml, strAssert);
	return FALSE;	// Non-blocking assertion never break into the debugger
	#endif

	if (g_fIgnoreAllAsserts)
		return FALSE;
	g_fIgnoreAllAsserts = TRUE;	// Prevent recursive entries

	strMessageHtml.AppendTextU((PSZUC)"<br/><br/>You may hold down the <b>Shift</b> key while clicking on the button <b>Ignore Error</b> to prevent future display of this error message.");
	// Show the assertion failure to the user with a Message Box (this code will block the thread until the user click on one of the button)
	int iButton = QMessageBox::critical(g_pwMainWindow, c_szInternalErrorDetected, strMessageHtml, "  Copy Error to Clipboard  ", "  Break Into Debugger  ", "  Ignore Error  ", 2, 2);
	g_fIgnoreAllAsserts = FALSE;

	switch (iButton)
		{
	case 0:
		strAssert.CopyStringToClipboard();
		break;
	case 1:
		return TRUE;	// Returning TRUE breaks into the debugger
		} // switch

	if (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier)	// if (::GetAsyncKeyState(VK_SHIFT) < 0)
		{
		// Holding the shift key will ignore any further messages
		g_fIgnoreAllAsserts = TRUE;
		}
	return FALSE;
	} // _FAssertionFailed()


/////////////////////////////////////////////////////////////////////
void
_DebugTraceA(PSZAC pszFmtTemplate, ...)
	{
	if (pszFmtTemplate == NULL)
		{
		_DebugBreak();
		return;
		}
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	CStr str;
	str.Format_VL(pszFmtTemplate, vlArgs);
	qDebug() << CString(str);
	}


BOOL
FIsDebugAllowanceReached(INOUT long * pcDebugAllowance, PSZAC pszaAllowanceName, long cDebugAllowanceDefault)
	{
	Assert(pcDebugAllowance != NULL);
	Assert(pszaAllowanceName != NULL);
	Assert(cDebugAllowanceDefault > 0);

	const int cAllowance = *pcDebugAllowance - 1;
	if (cAllowance < 0)
		return TRUE;
	if (cAllowance == 0)
		{
		MessageLog_AppendTextFormatCo(d_coGreen,
			"To improve the performance of SocietyPro in debug mode, redundant error checking for '$U' has been disabled (after completing successfully %D verifications).\n",
			pszaAllowanceName, cDebugAllowanceDefault);
		MessageLog_Show();
		}
	*pcDebugAllowance = cAllowance;
	return FALSE;
	}

VOID
DebugAllowanceDisable(INOUT long * pcDebugAllowance)
	{
	Assert(pcDebugAllowance != NULL);
	Assert(*pcDebugAllowance > 0);
	*pcDebugAllowance = -*pcDebugAllowance;
	}

VOID
DebugAllowanceEnable(INOUT long * pcDebugAllowance)
	{
	Assert(pcDebugAllowance != NULL);
	Assert(*pcDebugAllowance < 0);
	*pcDebugAllowance = -*pcDebugAllowance;
	}

//	Count the number of bits in the 32-bit integer.
UINT
UCountBits(UINT uValue)
	{
	int cBits = 0;
	while (TRUE)
		{
		if (uValue & 1)
			cBits++;
		uValue >>= 1;	// Divide by 2
		if (uValue == 0)
			break;
		}
	return cBits;
	}

#endif // DEBUG_WANT_ASSERT

/////////////////////////////////////////////////////////////////////
//	Return TRUE if a block of memory is all initialized with zeroes
BOOL
FIsZeroInit(const void * pv, int cb)
	{
	for (const BYTE * pb = (BYTE *)pv; cb-- > 0; pb++)
		{
		if (*pb != 0)
			return FALSE;
		}
	return TRUE;
	}

#ifdef DEBUG

//	Return TRUE if all characters are garbage.
//	Return FALSE if the buffer contains one or more non-garbage character.
BOOL
FIsGarbageInitialized(PCVOID pv, int cb)
	{
	for (const BYTE * pb = (const BYTE *)pv; cb-- > 0; pb++)
		{
		if (*pb != d_chGarbageValue)
			return FALSE;
		}
	return TRUE;
	}

#endif // DEBUG

