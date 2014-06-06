///////////////////////////////////////////////////////////////////////////////////////////////////
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
	if (!g_strAssertLast.FCompareBinary(strAssert))
		{
		g_strAssertLast = strAssert;
		MessageLog_AppendTextWithNewLine(COX_MakeBold(d_coAssert), pszuAssert);
		MessageLog_Show();
		ErrorLog_AddNewMessage(c_szInternalErrorDetected, pszuAssert);
		}

	if (g_fIgnoreAllAsserts)
		return FALSE;
	g_fIgnoreAllAsserts = TRUE;	// Prevent recursive entries
	// Show the error to the user
	CStr strMessageBox;
	strMessageBox.Format("<b>$s</b>(<b>^s</b>)<br/>File $s, line $i.<br/><br/>"
		"You have discovered a potential bug; <u>please</u> contact the developers of Cambrian.<br/><br/>"
		"You may hold down the <b>Shift</b> key while clicking on the button <b>Ignore Error</b> to prevent future display of this error message.",
		pszAssert, pszExpression, pszFile, nLine);
	int iButton = QMessageBox::critical(g_pwMainWindow, c_szInternalErrorDetected, strMessageBox, "  Copy Error to Clipboard  ", "  Break Into Debugger  ", "  Ignore Error  ", 2, 2);
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
			"To improve the performance of Cambrian in debug mode, redundant error checking for '$U' has been disabled (after completing successfully %D verifications).\n",
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

