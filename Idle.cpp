//	Functions to detect how long the user has been idle / away from the computer
//
//	These functions are heavily platform dependent, and as a result defined in this file.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#ifdef Q_OS_WIN
UINT
UGetIdleTimeInMinutes_Win32()
	{
	LASTINPUTINFO lastinputinfo;
	lastinputinfo.cbSize = sizeof(LASTINPUTINFO);
	::GetLastInputInfo(OUT &lastinputinfo);
	int cMinutes = (::GetTickCount() - lastinputinfo.dwTime) / (1000 * 60);
	Report(cMinutes >= 0);
	if (cMinutes >= 0)
		return cMinutes;
	return 0;
	}
PFn_UGetIdleTimeInMinutes g_pfnUGetIdleTimeInMinutes = UGetIdleTimeInMinutes_Win32;
#define _pfnUGetIdleTimeInMinutes
#endif


#ifndef _pfnUGetIdleTimeInMinutes
QPoint g_ptMouseCoordinatesLast;

//	Generic function to determine the idle (in minutes)
//	This function uses the position of mouse cursor to determine idleness.  If the mouse cursor
//	has not moved for a while, the function assumes the user is idle.
UINT
UGetIdleTimeInMinutes_Generic()
	{
	QPoint ptMouseCoordinatesPrev = g_ptMouseCoordinatesLast;
	g_ptMouseCoordinatesLast = QCursor::pos();
	if (g_ptMouseCoordinatesLast != ptMouseCoordinatesPrev)
		{
		// The mouse position moved, so assume there is user activity and reset the idle timer
		g_cMinutesIdleKeyboardOrMouse = 0;
		}
	return g_cMinutesIdleKeyboardOrMouse;
	}
PFn_UGetIdleTimeInMinutes g_pfnUGetIdleTimeInMinutes = UGetIdleTimeInMinutes_Generic;
#endif


/*
inline int64 GetPerformanceCounter()
{
	int64 nCounter = 0;
#if defined(WIN32) || defined(WIN64)
	QueryPerformanceCounter((LARGE_INTEGER*)&nCounter);
#else
	timeval t;
	gettimeofday(&t, NULL);
	nCounter = t.tv_sec * 1000000 + t.tv_usec;
#endif
	return nCounter;
}
*/
