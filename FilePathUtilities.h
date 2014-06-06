#ifndef FILEPATHUTILITIES_H
#define FILEPATHUTILITIES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

PSZUC PszGetFileNameOnlyNZ(PSZUC pszFullPath);
BOOL FIsFilePathRelative(PSZUC pszFileName);

void FileShowInExplorer(const CStr & strPathFile, BOOL fOpenFile = FALSE);

#endif // FILEPATHUTILITIES_H
