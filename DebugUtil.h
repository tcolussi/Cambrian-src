///////////////////////////////////////////////////////////////////////////////////////////////////
//	Debugging macros
//

#ifndef DEBUGUTIL_H
#define DEBUGUTIL_H

#define d_nLengthArrayDebug		100	// This value is arbitrary chosen, however useful to have a peek of the content of an array under the debugger

#ifdef _DEBUG
	//#define DEBUG
#endif

#ifdef DEBUG
	#define DEBUG_WANT_ASSERT
	#define DEBUG_VALIDATE_HEAP
#endif

	#define d_chGarbageValue				0xE1		// This garbage value is the 'á', however it is useful because it is an odd number for a pointer and also produces invalid UTF-8 strings
	#define d_chwGarbageValue				0xE1E1
	#define d_dwGarbageValue				0xE1E1E1E1
	#define d_pszuGarbageValue				(PSZUC)d_dwGarbageValue

/////////////////////////////////////////////////////////////////////
//
// Handy macros
//
#ifndef DEBUG
	// Macros used in release build

	#define ENDORSE(f)				// Dummy macro to indicate a statement can be true (this is somewhat the opposite of the Assert() macro)
	#define UNUSED_PARAMETER(x)		// The parameter of the function/method is unused
	#define UNUSED_VARIABLE(x)		// The variable is unused (typically this is when a variable declared to cache information for debugging)
	#define DEBUG_CODE(x)
	#define NON_DEBUG_CODE(x)		x
	#define InitToGarbage(pv, cb)
	#define FIsGarbageInitialized(pv, cb)
	#define AssertIsGarbageInitialized(pv, cb)
	#define AssertIsNotGarbageInitialized(pv, cb)
	#define TraceGarbageAlloc(pvData, cbData)

	#define TRACE0(sz)
	#define TRACE1(sz, p1)
	#define TRACE2(sz, p1, p2)
	#define TRACE3(sz, p1, p2, p3)
	#define TRACE4(sz, p1, p2, p3, p4)
	#define TRACE5(sz, p1, p2, p3, p4, p5)
	#define TRACE6(sz, p1, p2, p3, p4, p5, p6)

	#define AssertValidUtf8(pszu)
	#define SleepDebug(nMilliseconds)

#else

	#define ENDORSE(f)					((void)(f))			// Dummy macro to indicate a statement can be true
	#define UNUSED_PARAMETER(x)			((void)(x))			// Macro to suppress the compiler warning
	#define UNUSED_VARIABLE(x)			((void)(x))
	#define DEBUG_CODE(x)				x
	#define NON_DEBUG_CODE(x)

	#define InitToGarbage(pv, cb)			memset(pv, d_chGarbageValue, cb)
	BOOL FIsGarbageInitialized(PCVOID pv, int cb);
	#define AssertIsGarbageInitialized(pv, cb)			Assert(FIsGarbageInitialized(pv, cb))
	#define AssertIsNotGarbageInitialized(pv, cb)		Assert(!FIsGarbageInitialized(pv, cb))
	void TraceGarbageAlloc(const void * pvData, int cbData);

	void _DebugTraceA(PSZAC pszFmtTemplate, ...);
	#define TRACE0(sz)							_DebugTraceA("$s", sz)
	#define TRACE1(sz, p1)						_DebugTraceA(sz, p1)
	#define TRACE2(sz, p1, p2)					_DebugTraceA(sz, p1, p2)
	#define TRACE3(sz, p1, p2, p3)				_DebugTraceA(sz, p1, p2, p3)
	#define TRACE4(sz, p1, p2, p3, p4)			_DebugTraceA(sz, p1, p2, p3, p4)
	#define TRACE5(sz, p1, p2, p3, p4, p5)		_DebugTraceA(sz, p1, p2, p3, p4, p5)
	#define TRACE6(sz, p1, p2, p3, p4, p5, p6)	_DebugTraceA(sz, p1, p2, p3, p4, p5, p6)

	void AssertValidUtf8(IN PSZUC pszuUtf8);

	#define SleepDebug(nMilliseconds)	::Sleep(nMilliseconds)

#endif // DEBUG

BOOL FIsZeroInit(const void * pv, int cb);

#ifdef DEBUG_WANT_ASSERT

	#define _DebugBreak()  asm("int $0x3")

	const char c_szInternalErrorDetected[] = "Internal Error Detected";
	#define d_coAssert		d_coRedHot		// Display an assertion in a red hot color

	extern L64 g_cAssertionsTotalExecuted;
	extern L64 g_cAssertionsFailed;

	BOOL _FAssertionFailed(PSZAC pszFile, int nLine, PSZAC pszExpression, BOOL fAssert);

	/////////////////////////////////////////////////////////////////////
	#define _Assert(f)			\
		do						\
		{						\
		g_cAssertionsTotalExecuted++;		\
		if (!(f) && _FAssertionFailed(__FILE__, __LINE__, #f, TRUE)) \
			_DebugBreak();	\
		} while (0)

	#define _Report(f)			\
		do						\
		{						\
		g_cAssertionsTotalExecuted++;		\
		if (!(f) && _FAssertionFailed(__FILE__, __LINE__, #f, FALSE)) \
			_DebugBreak();	\
		} while (0)

	#define Verify(f)		Assert(f)


	BOOL FIsDebugAllowanceReached(INOUT long * pcDebugAllowance, PSZAC pszaAllowanceName, long cDebugAllowanceDefault);
	VOID DebugAllowanceDisable(INOUT long * pcDebugAllowance);
	VOID DebugAllowanceEnable(INOUT long * pcDebugAllowance);
	UINT UCountBits(UINT uValue);
#else

	#define _Assert(f)
	#define _Report(f)
	#define Verify(f)		f

#endif	// ~DEBUG_WANT_ASSERT


/////////////////////////////////////////////////////////////////////
//	Assert(), macro
//
//	Represent a bug in the code.
//
#define Assert(f)		_Assert(f)


/////////////////////////////////////////////////////////////////////
//	Report(), macro
//
//	Report is an unusual situation.  This is somewhat similar to Assert but does not always represent a code bug.
//	eg: Unable to open a file.  The file should be there, and therefore should be opened, however in some rare cases, the file may fail have been deleted or locked.
//
#define Report(f)		_Report(f)

#define ReportBreakpoint(f)		Report(f)		// This is just a macro to force a breakpoint, typically when the code has never/rarely been executed and requires some special attention in case there are bugs


/////////////////////////////////////////////////////////////////////
//	Endorse(), macro
//
//	This macro is mostly used when validating parameters.
//	Some parameters are allowed to be NULL because they are optional
//	or simply because the interface uses the NULL case as a valid
//	input parameter.  In this case the Endorse() macro is used to
//	acknowledge the validity of such a parameter.
//
//	REMARKS
//	This macro is the opposite of Assert().
//
//	EXAMPLE
//	Endorse(p == NULL);	// Code acknowledge p == NULL to not be (or not cause) an error
//
#define Endorse(f)	ENDORSE(f)
#define EndorseFast(f)		// Macro to prevent the code to run inside the macro (sometimes the content of the Endorse() macro may be too slow)



#define ValidateHeapPointer(pv)

#endif // DEBUGUTIL_H
