#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

const CStr c_strEmpty;
const CString c_sEmpty;
const QByteArray c_arraybEmpty;

CStr::operator QString() const
	{
	if (m_paData != NULL)
		{
		const int cchUtf8 = m_paData->cbData - 1;
		if (cchUtf8 > 0)
			return QString::fromUtf8((const char *)m_paData->rgbData, cchUtf8);
		}
	return c_sEmpty;		// Return an empty QString
	}

//	Method to convert a CStr to a QString
QString
CStr::ToQString() const
	{
	if (m_paData != NULL)
		{
		const int cchUtf8 = m_paData->cbData - 1;
		if (cchUtf8 > 0)
			return QString::fromUtf8((const char *)m_paData->rgbData, cchUtf8);
		}
	return c_sEmpty;
	}

//	Return a NULL pointer if the content of the CStr is empty.
//	This is somewhat the opposite of PszuGetDataNZ() which never return NULL.
PSZUC
CStr::PszuGetStringNullIfEmpty() const
	{
	if (m_paData != NULL && m_paData->cbData > 0)
		{
		if (m_paData->rgbData[0] != '\0')
			return m_paData->rgbData;
		}
	return NULL;
	}

BOOL
CStr::FIsEmptyString() const
	{
	AssertValidStr(*this);
	return (m_paData == NULL || m_paData->cbData <= (int)sizeof(CHU));
	}

//	Return the length of the string without the null-terminator
int
CStr::CchGetLength() const
	{
	if (m_paData != NULL)
		{
		const int cbData = m_paData->cbData - sizeof(CHU);
		if (cbData >= 0)
			{
			Assert(m_paData->rgbData[cbData] == '\0' && "String must be always be null-terminated");
			return cbData;
			}
		}
	return 0;
	}

VOID
CStr::InitFromStringW(PSZWC pszwString)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	AppendTextW(pszwString);
	}

void
CStr::InitFromStringQ(const QString & sString)
	{
	InitFromStringW(sString.utf16());
	}

void
CStr::InitFromStringQTrimmed(const QString & sString)
	{
	InitFromStringQ(sString);
	TrimLeadingAndTailingWhiteSpaces();
	}

//	By default, always trim the white spaces when fetching the value from a QLineEdit.
//	Since most of the input fields, such as username, password, JID and so on, are required to have no spaces.  As a result,
//	it is safer to have them removed by default.
//	In the rare case leading and/or tailing white spaces are required, then we use another method.
void
CStr::InitFromQLineEditTrimmed(const QLineEdit & wLineEdit)
	{
	InitFromStringQTrimmed(wLineEdit.text());
	}

void
CStr::InitFromQTextEditTrimmed(const QPlainTextEdit & wTextEdit)
	{
	InitFromStringQTrimmed(wTextEdit.toPlainText());
	}
void
CStr::InitFromQTextEditTrimmed(const QTextEdit & wTextEdit)
	{
	InitFromStringQTrimmed(wTextEdit.toPlainText());
	}

void
CStr::InitFromCBin(const CBin & binString)
	{
	// Initialize the string from the content of the binary value
	if (m_paData != NULL)
		m_paData->cbData = 0;
	const SHeaderWithData * pDataSrc = binString.m_paData;
	if (pDataSrc == NULL)
		return;
	int cbData = pDataSrc->cbData;
	if (cbData > 0)
		{
		BYTE * pbDst = (BYTE *)PvSizeInit(cbData + 1);
		memcpy(OUT pbDst, IN pDataSrc->rgbData, cbData);
		pbDst[cbData] = '\0';
		Assert(m_paData->cbData == cbData + 1);
		Assert(CchGetLength() == cbData);
		}
	}

VOID
CStr::AppendTextW(PSZWC pszwString)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszwString));
	if (pszwString != NULL && pszwString[0] != '\0')
		{
		if (m_paData != NULL)
			{
			const int cbData = m_paData->cbData;
			if (cbData > 0)
				{
				Assert(m_paData->rgbData[m_paData->cbData - 1] == '\0' && "String must be always be null-terminated");
				m_paData->cbData = cbData - 1;	// Remove the null-terminator
				}
			}
		CHU * pchuDst = PbAllocateExtraMemory(CbAllocWtoU(pszwString));
		m_paData->cbData += 1 + EncodeUnicodeToUtf8(OUT pchuDst, IN pszwString);
		}
	} // AppendTextW()


//	Append a UTF-8 string
VOID
CStr::AppendTextU(PSZUC pszuAppend)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszuAppend));
	if (pszuAppend == NULL)
		return;
	if (*pszuAppend == '\0')
		return;
	if (m_paData != NULL)
		{
		const int cbData = m_paData->cbData;
		if (cbData > 0)
			{
			Assert(m_paData->rgbData[m_paData->cbData - 1] == '\0' && "String must be always be null-terminated");
			m_paData->cbData = cbData - 1;	// Remove the null-terminator
			}
		}
	BinAppendStringWithNullTerminator((PSZAC)pszuAppend);
	}

VOID
CStr::AppendSeparatorAndTextU(PSZAC pszaSeparator, PSZUC pszuAppend)
	{
	Assert(pszaSeparator != NULL);
	Assert(*pszaSeparator != '\0');
	Assert(!FIsPointerAddressWithinBinaryObject(pszuAppend));
	if (pszuAppend == NULL || *pszuAppend == '\0')
		return;
	if (!FIsEmptyString())
		AppendTextU((PSZUC)pszaSeparator);
	AppendTextU(pszuAppend);
	}

VOID
CStr::AppendSeparatorCommaAndTextU(PSZUC pszuAppend)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszuAppend));
	AppendSeparatorAndTextU(c_szaCommaSeparator, pszuAppend);
	}

PSZUC
CStr::AppendFormat(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	return AppendFormat_VL(pszFmtTemplate, vlArgs);
	}

PSZUC
CStr::AppendFormat_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	if (m_paData != NULL)
		{
		const int cbData = m_paData->cbData;
		if (cbData > 0)
			{
			Assert(m_paData->rgbData[m_paData->cbData - 1] == '\0' && "String must be always be null-terminated");
			m_paData->cbData = cbData - 1;	// Remove the null-terminator
			}
		}
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	m_paData->cbData += sizeof(CHU);	// Include the null-terminator
	return (PSZUC)m_paData->rgbData;
	}

PSZUC
CStr::Format(PSZAC pszFmtTemplate, ...)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;	// Remove any previous string.
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	m_paData->cbData += sizeof(CHU);	// Include the null-terminator
	return (PSZUC)m_paData->rgbData;
	}

PSZUC
CStr::Format_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;	// Remove any previous string.
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	m_paData->cbData += sizeof(CHU);	// Include the null-terminator
	return (PSZUC)m_paData->rgbData;
	}

BOOL
CStr::FCompareStringsJIDs(PSZUC pszuJID) const
	{
	return FCompareStringsNoCaseUntilCharacter(pszuJID, '/');
	}

//	Useful to compare JIDs while skipping the resource
BOOL
CStr::FCompareStringsNoCaseUntilCharacter(PSZUC pszuStringCompare, UINT chCompareUntil) const
	{
	Assert(pszuStringCompare != NULL);
	Assert(pszuStringCompare[0] != '\0');
	if (m_paData != NULL)
		{
		PSZUC pszString = m_paData->rgbData;
		while (TRUE)
			{
			UINT ch1 = *pszString++;
			if (ch1 == '\0' || ch1 == chCompareUntil)
				break;
			if (ch1 >= 'A' && ch1 <= 'Z')
				ch1 += 32;	// Make lowercase
			UINT ch2 = *pszuStringCompare++;
			if (ch2 == '\0' || ch2 == chCompareUntil)
				break;
			if (ch2 >= 'A' && ch2 <= 'Z')
				ch2 += 32;	// Make lowercase
			if (ch1 != ch2)
				return FALSE;
			} // while
		return TRUE;
		}
	return FALSE;
	} // FCompareStringsNoCaseUntilCharacter()

//	Optimized method to compare CStr with with a portion of pszuStringCompare.
//	The motivation for this optimization is two strings having different lengths cannot be identical.
BOOL
CStr::FCompareStringsNoCaseCch(PSZUC pszuStringCompare, int cchCompare) const
	{
	Assert(pszuStringCompare != NULL);
	Assert(pszuStringCompare[0] != '\0');
	Assert(cchCompare > 0 && cchCompare <= (int)strlenU(pszuStringCompare));
	if (m_paData != NULL)
		{
		if (m_paData->cbData == cchCompare + 1)
			{
			// The length of the string is the same as what we wish to compare, so do the actual comparison
			PSZUC pszString = m_paData->rgbData;
			PSZUC pchStringStop = pszString + cchCompare;
			while (pszString != pchStringStop)
				{
				UINT ch1 = *pszString++;
				if (ch1 >= 'A' && ch1 <= 'Z')
					ch1 += 32;	// Make lowercase
				UINT ch2 = *pszuStringCompare++;
				if (ch2 >= 'A' && ch2 <= 'Z')
					ch2 += 32;	// Make lowercase
				if (ch1 != ch2)
					return FALSE;
				}
			return TRUE;
			}
		}
	return FALSE;
	} // FCompareStringsNoCaseCch()

BOOL
CStr::FCompareStringsNoCase(PSZUC pszuString) const
	{
	Assert(pszuString != NULL);
	if (m_paData != NULL)
		return ::FCompareStringsNoCase(m_paData->rgbData, pszuString);
	return (pszuString[0] == '\0');
	}

BOOL
CStr::FCompareStringsExactCase(PSZUC pszuString) const
	{
	Assert(pszuString != NULL);
	if (m_paData != NULL)
		return FCompareStrings(m_paData->rgbData, pszuString);
	return (pszuString[0] == '\0');
	}

BOOL
CStr::FStringBeginsWith(PSZAC pszStringCompare) const
	{
	Assert(pszStringCompare != NULL);
	Assert(pszStringCompare[0] != '\0');
	if (m_paData != NULL)
		return (PszrCompareStringBeginNoCase(m_paData->rgbData, pszStringCompare) != NULL);
	return FALSE;
	}

BOOL
CStr::FStringContainsSubStringNoCase(PSZUC pszString) const
	{
	Assert(pszString != NULL);
	Assert(pszString[0] != '\0');
	if (m_paData != NULL)
		return (PszrStringContainsSubStringLowerCase(m_paData->rgbData, (PSZAC)pszString) != NULL);
	return FALSE;
	}


//	See also BinAppendTextUntilCharacterPszr()
PSZR
CStr::AppendTextUntilCharacterPszr(PSZUC pszuSource, UINT chCopyUntil)
	{
	Assert(pszuSource != NULL);
	PSZUC pszSourceRemaining = BinAppendTextUntilCharacterPszr(pszuSource, chCopyUntil);
	BinAppendNullTerminator();
	return pszSourceRemaining;
	}

PSZR
CStr::AppendTextUsernameFromJidPszr(PSZUC pszJID)
	{
	return AppendTextUntilCharacterPszr(pszJID, '@');
	}

BOOL
FJidContainsServerName(PSZUC pszJID)
	{
	Assert(pszJID != NULL);
	return (PszroGetServerNameFromJid(pszJID)[0] != '\0');
	}

void
CStr::AppendServerNameToJidIfMissing(PSZUC pszServerName)
	{
	if (m_paData == NULL)
		return;
	if (!FJidContainsServerName(m_paData->rgbData))
		{
		// The JID is not valid, so append the server name
		AppendFormat("@$s", pszServerName);
		}
	}

PSZUC
CStr::InitFromStringCopiedUntilCharacterSz(PSZUC pszBegin, UINT chCopyUntil)
	{
	Assert(pszBegin != NULL);
	const CHU * pch = pszBegin;
	while (TRUE)
		{
		const UINT ch = *pch++;
		if (ch == '\0' || ch == chCopyUntil)
			break;
		} // while
	const int cch = pch - pszBegin;
	BinInitFromBinaryData(pszBegin, cch);
	m_paData->rgbData[cch - 1] = '\0';
	return m_paData->rgbData;
	} // InitFromStringCopiedUntilCharacterSz()

PSZUC
CStr::InitFromStringCopiedUntilPch(PSZUC pszBegin, PCHUC pchCopyUntil)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BinAppendBinaryDataPvPv(pszBegin, pchCopyUntil);
	return BinAppendNullTerminatorSz();
	}

PSZUC
CStr::FindSubStringPsz(PSZAC pszSubStringSearch) const
	{
	Assert(pszSubStringSearch != NULL);
	if (m_paData != NULL)
		return PszrStringContainsSubString(m_paData->rgbData, pszSubStringSearch);
	return NULL;
	}

PSZUC
CStr::InitFromTextBetweenPrefixAndPostfixPsz(PSZUC pszTextSource, PSZAC pszPrefix, PSZAC pszPostfix)
	{
	Assert(pszTextSource != NULL);
	Assert(pszPrefix != NULL);
	Assert(pszPrefix[0] != '\0');
	Assert(pszPostfix != NULL);
	Assert(pszPostfix[0] != '\0');
	if (m_paData != NULL)
		m_paData->cbData = 0;
	PSZUC pszBegin = PszrStringContainsSubString(pszTextSource, pszPrefix);
	if (pszBegin != NULL)
		{
		PSZUC pszEnd = PszrStringContainsSubString(pszBegin, pszPostfix);
		if (pszEnd != NULL)
			{
			BinAppendBinaryDataPvPv(pszBegin, pszEnd - strlen(pszPostfix));
			return BinAppendNullTerminatorSz();
			}
		}
	return NULL;
	}

PSZUC
CStr::InitFromQueryStringPsz(PSZUC pszTextSource, PSZAC pszNameVariable)
	{
	Assert(pszTextSource != NULL);
	Assert(pszNameVariable != NULL);
	Assert(pszNameVariable[0] != '\0');
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BOOL fQueryStringFound = FALSE;
	while (TRUE)
		{
		CHS ch = *pszTextSource++;
		if (ch == '\0')
			break;	// We reached the end of the string without a QueryString, so return NULL.
		if (ch == '&' && fQueryStringFound)
			goto CompareQueryString;
		if (ch == '?')
			{
			fQueryStringFound = TRUE;
			CompareQueryString:
			PSZUC pszQueryStringValue = PszrCompareStringBeginNoCase(pszTextSource, pszNameVariable);
			if (pszQueryStringValue != NULL && *pszQueryStringValue == '=')
				{
				// We have found our QueryString value, now search until the end which is either the end of the string, a white space, or the ampersand (&)
				const CHU * pchEnd = pszQueryStringValue;
				while (TRUE)
					{
					ch = *++pchEnd;
					switch (ch)
						{
					case '\0':
					case '\t':
					case '\r':
					case '\n':
					case ' ':
					case '&':	// The ampersand separates QueryString values, and therefore indicates the end of the current QueryString value
						// Now it it time to decode (remove the escaping from) the QueryString
						PSZU pszQueryStringDecoded = (PSZU)PvSizeAlloc(pchEnd - pszQueryStringValue);
						while (TRUE)
							{
							ch = *++pszQueryStringValue;
							if (pszQueryStringValue >= pchEnd)
								break;
							Assert(ch != '\0');
							if (ch == '+')
								ch = ' ';
							else if (ch == '%')
								{
								MAP_VALUE_BYTE mvbChar0 = Ch_GetMvb(pszQueryStringValue[1]);
								if (mvbChar0 <= 0x0F)
									{
									MAP_VALUE_BYTE mvbChar1 = Ch_GetMvb(pszQueryStringValue[2]);
									if (mvbChar1 <= 0x0F)
										{
										ch = (mvbChar0 << 4) | mvbChar1;
										pszQueryStringValue += 2;
										}
									}
								}
							/*
							if (ch == '\0')
								break;
							*/
							*pszQueryStringDecoded++ = ch;
							} // while
						*pszQueryStringDecoded = '\0';
						m_paData->cbData = pszQueryStringDecoded - m_paData->rgbData + 1;
						Assert(m_paData->cbData <= m_paData->cbAlloc);
						return m_paData->rgbData;
						} // switch
					} // while
				} // if
			} // if (?)
		} // while
	return NULL;
	} // InitFromQueryStringPsz()

#ifdef DEBUG_WANT_ASSERT
void
AssertValidStr(const CStr & str)
	{
	int cchStrlen = strlenU(str);
	int cchString = str.CchGetLength();
	Assert(cchStrlen == cchString);
	}
#endif
