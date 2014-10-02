///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class CStr
//
//	Class to store an UTF-8 string.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CSTR_H
#define CSTR_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CStr : public CBin
{
public:
	inline CStr() { }
	inline CStr(const CBin & binString) { InitFromCBin(IN binString); }
	inline CStr(const QString & sString) { InitFromStringQ(sString); }
	inline CStr(const QLineEdit & wLineEdit) { InitFromQLineEditTrimmed(wLineEdit); }
	inline CStr(const QPlainTextEdit & wTextEdit) { InitFromQTextEditTrimmed(wTextEdit); }
	inline CStr(const QTextEdit & wTextEdit) { InitFromQTextEditTrimmed(wTextEdit); }
	inline CStr(PSZUC pszuString) { BinInitFromStringWithNullTerminator((PSZAC)pszuString); }
	inline const CStr & operator = (const QString & sString) { InitFromStringQ(sString); return *this; }
	inline const CStr & operator = (const QLineEdit * pwLineEdit) { InitFromQLineEditTrimmed(*pwLineEdit); return *this; }
	inline const CStr & operator = (const QTextEdit * pwTextEdit) { InitFromQTextEditTrimmed(*pwTextEdit); return *this; }
	inline const CStr & operator = (const QByteArray & arrayb) { BinInitFromBinaryData(arrayb.constData(), arrayb.size() + 1); return *this; }
	inline const CStr & operator = (PSZUC pszuString) { BinInitFromStringWithNullTerminator((PSZAC)pszuString); return *this; }
	inline operator PSZUC() const { return PszuGetDataNZ(); }
	inline operator PSZAC() const { return (PSZAC)PszuGetDataNZ(); }		// Cast to PSZAC because the Qt framework often uses the following type: char *
	operator QString() const;
	QString ToQString() const;
	PSZUC PszuGetStringNullIfEmpty() const;
	BOOL FIsEmptyString() const;
	int CchGetLength() const;

	inline VOID InitFromStringU(PSZUC pszuString) { BinInitFromStringWithNullTerminator((PSZAC)pszuString); }
	VOID InitFromStringW(PSZWC pszwString);
	void InitFromStringQ(const QString & sString);
	void InitFromStringQTrimmed(const QString & sString);
	void InitFromQLineEditTrimmed(const QLineEdit & wLineEdit);
	void InitFromQTextEditTrimmed(const QPlainTextEdit & wTextEdit);
	void InitFromQTextEditTrimmed(const QTextEdit & wTextEdit);
	void InitFromCBin(const CBin & binString);
	VOID AppendTextW(PSZWC pszwString);
	VOID AppendTextU(PSZUC pszuString);
	VOID AppendSeparatorAndTextU(PSZAC pszaSeparator, PSZUC pszuAppend);
	VOID AppendSeparatorCommaAndTextU(PSZUC pszuAppend);
	PSZUC AppendFormat(PSZAC pszFmtTemplate, ...);
	PSZUC AppendFormat_VL(PSZAC pszFmtTemplate, va_list vlArgs);

	PSZUC Format(PSZAC pszFmtTemplate, ...);
	PSZUC Format_VL(PSZAC pszFmtTemplate, va_list vlArgs);

	void FormatErrorCode(EError err);
	void FormatErrorAtLineNumber(int nLineNumberError);

	BOOL FCompareStringsJIDs(PSZUC pszuJID) const;
	BOOL FCompareStringsNoCaseUntilCharacter(PSZUC pszuStringCompare, UINT chCompareUntil) const;
	BOOL FCompareStringsNoCaseCch(PSZUC pszuStringCompare, int cchStringCompare) const;
	BOOL FCompareStringsNoCase(PSZUC pszuString) const;
	BOOL FCompareStringsExactCase(PSZUC pszuString) const;
	BOOL FStringBeginsWith(PSZAC pszStringCompare) const;
	BOOL FStringContainsSubStringNoCase(PSZUC pszString) const;

	PSZR AppendTextUntilCharacterPszr(PSZUC pszuSource, UINT chCopyUntil);
	PSZR AppendTextUsernameFromJidPszr(PSZUC pszJID);
	void AppendServerNameToJidIfMissing(PSZUC pszServerName);
	PSZUC InitFromStringCopiedUntilCharacterSz(PSZUC pszBegin, UINT chCopyUntil);
	PSZUC InitFromStringCopiedUntilPch(PSZUC pszBegin, PCHUC pchCopyUntil);
	PSZUC InitFromBinaryToHexPsz(const CBin & bin);

	PSZUC FindSubStringPsz(PSZAC pszSubStringSearch) const;
	PSZUC InitFromTextBetweenPrefixAndPostfixPsz(PSZUC pszTextSource, PSZAC pszPrefix, PSZAC pszPostfix);
	PSZUC InitFromQueryStringPsz(PSZUC pszTextSource, PSZAC pszNameVariable);

	PSZUC InitFromValueOfCommaSeparatedStringPsz(PSZUC pszuCommaSeparatedString, PSZAC pszSearchToken);
	void TransformContentToMd5Hex();
	void TransformContentToMd5Hex(PSZAC pszFmtTemplate, ...);
	void TransformContentToLowercase();
	void TransformContentToLowercaseSearch();
	void TransformContentToLowercaseSort();
	void TrimLeadingAndTailingWhiteSpaces();
	PSZUC TrimTailingWhiteSpacesNZ();
	void StringTruncateAt(PSZU pchDataEnd);
	void StringTruncateAtCharacter(UINT chTruncate);
	void InitOnlyIfEmpty(const CStr & strString);
	void InitFromFingerprintBinary(const void * pvFingerprint, int cbFingerprint);
	void InitFromFingerprintBinary(const CBin & binFingerprint);
	void InitFromFingerprintBinary(const QByteArray & arraybFingerprint);
	void InitFromTextEncodedInBase85(PSZUC pszBase85);
	void InitFromTextEncodedInBase64(const CStr & strText);
	PSZUC InitFromBase64Sz(PSZUC pszBase64);
	void CopyStringToClipboard() const;
	void InitFromClipboard();
	void InitWithRandomPassword(const void * pvRandomValue);
	void InitWithRandomUsername(const CStr & strUsernameBase, UINT uRandomValue);

	BOOL FValidateServerName_MB() const;
	BOOL FValidateJID_MB() const;
	BOOL PathUrl_FIsValidHyperlinkNonCambrian() const;
	void PathFile_MakeFullPath(PSZUC pszPath, PSZUC pszFileName);
	void PathFile_StripFileName();
	PSZUC PathFile_PszGetFileNameOnly_NZ() const;

	EError FileReadE(const QString & sFileName);
	EError FileWriteE(const QString & sFileName, QIODevice::OpenModeFlag uFlagsExtra = QIODevice::NotOpen) const;

}; // CStr

#ifdef DEBUG_WANT_ASSERT
	void AssertValidStr(const CStr & str);
#else
	#define AssertValidStr(str)
#endif

//	Wrapper to the QString class.
//	This wrapper is useful to convert UTF-8 string to QString.
//	We need this wrapper because the QString constructor uses QLatin1String which result in garbage alphabet for non-latin characters.
class CString : public QString
{
public:
	inline CString() { }
	inline CString(const QString & s) { InitFromStringQ(s); }
	inline CString(const CBin & bin) { InitFromCBin(bin); }
	inline CString(const CStr & str) { InitFromCStr(str); }
	inline CString(PSZUC pszuString) { InitFromStringU(pszuString); }
	inline const CString & operator = (const QString & s) { InitFromStringQ(s); return *this; }
	inline const CString & operator = (const CStr & str) { InitFromCStr(str); return *this; }
	inline const CString & operator = (PSZUC pszuString) { InitFromStringU(pszuString); return *this; }

	void InitFromStringQ(const QString & s);
	void InitFromCBin(const CBin & bin);
	void InitFromCStr(const CStr & str);
	void InitFromStringU(PSZUC pszuString);
	void Format(PSZAC pszFmtTemplate, ...);
	void TrimWhiteSpacesAtEnd();
	void File_StripFileName();
}; // CString

#endif // CSTR_H
