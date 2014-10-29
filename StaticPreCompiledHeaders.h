#ifndef PREPRECOMPILEDHEADERS_H
#define PREPRECOMPILEDHEADERS_H



#if defined(__WIN32__)
#include <winsock2.h>
#include <windows.h>
#endif

//#include <QtWidgets>
#include <QtNetwork>
#include <QTextBrowser>
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QMainWindow>

/*
#ifndef NDEBUG
	#define _DEBUG	// The GNU compiler automatically defines NDEBUG if this is not a debug build.
#endif
*/
#ifdef QT_DEBUG
	#define DEBUG
#endif
#define DEBUG_WANT_ASSERT	// We want Assert() in the release build

#include "DataTypesAndMacros.h"
#include "ErrorCodes.h"
#include "Sort.h"
//#include "StringsShared.h"



#endif // PREPRECOMPILEDHEADERS_H
