///////////////////////////////////////////////////////////////////////////////////////////////////
//	FilePathUtilities.cpp
//
//	Filename and filepath utility routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Remove the filename from a full path
void
CStr::PathFile_StripFileName()
	{
	if (m_paData == NULL)
		return;
	CHU * pchFileName = NULL;
	CHU * pch = m_paData->rgbData;
	while (TRUE)
		{
		switch (*pch++)
			{
		case '\0':
			if (pchFileName != NULL)
				{
				pchFileName[-1] = '\0';	// Insert a null-terminator at the place of the last path separator
				m_paData->cbData = pchFileName - m_paData->rgbData;
				Assert(m_paData->cbData <= m_paData->cbAlloc);
				}
			return;
		case '\\':
		case '/':
			pchFileName = pch;
			break;
			} // switch
		} // while
	Assert(FALSE && "Unreachable code");
	} // PathFile_StripFileName()


PSZUC
PszGetFileNameOnlyNZ(PSZUC pszFullPath)
	{
	Report(pszFullPath != NULL);
	if (pszFullPath != NULL)
		{
		PSZUC pszFileName = pszFullPath;
		const CHU * pch = pszFileName;
		while (TRUE)
			{
			switch (*pch++)
				{
			case '\0':
				Assert(pszFileName != NULL);
				if (pszFileName != NULL)
					return pszFileName;
				return pszFullPath;	// There is no path separator, so return the full path which is (should be) the file name
			case '\\':
			case '/':
				pszFileName = pch;
				break;
				} // switch
			} // while
		Assert(FALSE && "Unreachable code");
		} // if
	return c_szuEmpty;
	}

/////////////////////////////////////////////////////////////////////
//	Return TRUE if the file name is a relative path
//	Return FALSE if the file name is a full path or a UNC network path
BOOL
FIsFilePathRelative(IN PSZUC pszFileName)
	{
	Assert(pszFileName != NULL);
	Assert(pszFileName[0] != '\0');
	CHU ch = pszFileName[0];
	switch (ch)
		{
	case '.':	// Relative path
		return TRUE;
	case '\\':
	case '/':
		ch = pszFileName[1];
		if (ch == '\\' || ch == '/')
			return FALSE;
		} // switch
	// A relative path cannot have the colon ':' before the path separator '/' or '\\'
	while (TRUE)
		{
		ch = *pszFileName++;
		if (ch == '\0' || ch == '\\' || ch == '/')
			return TRUE;
		if (ch == ':')
			{
			// We have found a colon.  If there is no path separator, then it is still a relative path
			while (TRUE)
				{
				ch = *pszFileName++;
				if (ch == '\\' || ch == '/')
					return FALSE;
				if (ch == '\0')
					return TRUE;
				}
			} // if
		} // while
	} // FIsFilePathRelative()

//	These routines must be re-written to handle more comples cases
void
CStr::PathFile_MakeFullPath(PSZUC pszPath, PSZUC pszFileName)
	{
	Format("$s/$s", pszPath, pszFileName);
	}


/////////////////////////////////////////////////////////////////////
//	Return pointer to file name, skipping the base path (if any).
//	This routine never return NULL.
//
PSZUC
CStr::PathFile_PszGetFileNameOnly_NZ() const
	{
	return PszGetFileNameOnlyNZ(PszuGetDataNZ());
	}

EError
CStr::FileReadE(const QString & sFileName)
	{
	EError err = BinFileReadE(sFileName);
	BinAppendNullTerminator();
	return err;
	}

void
FileShowInExplorer(const CStr & strPathFile, BOOL fOpenFile)
	{
	CString sPathFile = strPathFile;
	if (!fOpenFile)
		{
		// We do not want to open the file, but just show it in Explorer (or in the GUI of the OS)
		#if defined(Q_OS_WIN)
			if (QFile::exists(sPathFile))
				{
				QStringList args;
				args << "/select," << QDir::toNativeSeparators(sPathFile);
				QProcess::startDetached("explorer", args);
				return;
				}
			MessageLog_AppendTextFormatSev(eSeverityWarning, "The file \"$Q\" does not exist, therefore showing only its folder\n", &sPathFile);
			goto StripFileName;
		#elif defined(Q_OS_MAC)
			QStringList args;
			args << "-e";
			args << "tell application \"Finder\"";
			args << "-e";
			args << "activate";
			args << "-e";
			args << "select POSIX file \"" + sPathFile + "\"";
			args << "-e";
			args << "end tell";
			QProcess::startDetached("osascript", args);
			return;
		#else
			// Since we do not know how to display a file in this platform, strip the filename and open its containing folder
			goto StripFileName;
		#endif
		}
	else if (!QFile::exists(sPathFile))
		{
		StripFileName:
		sPathFile.File_StripFileName();
		}
	MessageLog_AppendTextFormatSev(eSeverityNoise, "FileShowInExplorer(\"$Q\")\n", &sPathFile);
	#if defined(Q_OS_WIN)
	Assert(sizeof(ushort) == sizeof(wchar_t));
	SHELLEXECUTEINFOW seiW;
	InitToZeroes(OUT &seiW, sizeof(seiW));
	seiW.cbSize = sizeof(seiW);
	seiW.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
	seiW.nShow = SW_SHOWNORMAL;
	seiW.lpFile = (LPCWSTR)sPathFile.utf16();
	::ShellExecuteExW(INOUT &seiW);
	#else
	QDesktopServices::openUrl(QUrl::fromUserInput(sPathFile));
	#endif
	} // FileShowInExplorer()
