//	Routines to convert a string to a numeric value and vice-versa.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


///////////////////////////////////////////////////////////
//	FStringToNumber()
//
//	Parse the source string pszSrc and extract its integer or float value.
//
//	Return TRUE if successful, otherwise return FALSE and set nErrCode to the error found.
//
//	INTERFACE NOTES:
//	Fields uFlags and pszSrc are not modified.
//
BOOL
FStringToNumber(INOUT SStringToNumber * pSTN)
	{
	Assert(pSTN != NULL);
	Assert(pSTN->pszuSrc != NULL);
	Assert(pSTN->pszuSrc == (PSZUC)pSTN->pszaSrc);
	pSTN->pchStop = pSTN->pszuSrc;
	pSTN->nErrCode = STN_errSuccess;	// No error yet
	pSTN->u.uData = 0;					// Set the output data to zero
	const UINT uFlags = pSTN->uFlags;
	UINT uBase = (uFlags & STN_mskfHexBase) ? 16 : 10;
	UINT cDigitParsed = 0;  // Number of digits parsed
	BOOL fIsNegative = FALSE;

	if ((uFlags & STN_mskfNoLeadingSpaces) == 0)
		{
		// Skip leading white spaces
		while (*pSTN->pchStop == ' ')
			pSTN->pchStop++;
		}

	if (uFlags & (STN_mskfGetFloat | STN_mskfGetDoubleFloat))
		{
		// The idea there is to try to determine if the float number
		// is valid.  This is because strtod() is too stupid to return
		// any error code.
		const CHU * pchStart = pSTN->pchStop;
		if (*pSTN->pchStop == '+' || *pSTN->pchStop == '-')
			pSTN->pchStop++;
		if (*pSTN->pchStop == '.')
			pSTN->pchStop++;
		if (*pSTN->pchStop == '\0')
			{
			// Premature end of string
			if (uFlags & STN_mskfMustHaveDigits)
				{
				pSTN->nErrCode = STN_errNoDigitsFound;
				return FALSE;
				}
			}
		else if (*pSTN->pchStop < '0' || *pSTN->pchStop > '9')
			{
			// Float value must start with a digit
			pSTN->nErrCode = STN_errIllegalDigit;
			return FALSE;
			}
		pSTN->u.dblData = strtod(IN (const char *)pchStart, OUT (char **)&pSTN->pchStop);

		if (uFlags & STN_mskfGetFloat)
			{
			const float fl = (float)pSTN->u.dblData;	// Need to use a temporary variable because they both use the same storage space
			pSTN->u.flData = fl;
			}
		goto CheckTail;
		} // if (get float number)

	// Check for a minus sign
	if (*pSTN->pchStop == '-')
		{
		if (uFlags & STN_mskfUnsignedInteger)
			{
			pSTN->nErrCode = STN_errMinusSignFound;
			return FALSE;
			}
		fIsNegative = TRUE;
		pSTN->pchStop++;
		}

	//  Skip leading zeroes
	while (*pSTN->pchStop == '0')
		{
		pSTN->pchStop++;
		cDigitParsed++;
		}
	// Look for the hexadecimal prefix (0x or 0X)
	if (*pSTN->pchStop == 'x' || *pSTN->pchStop == 'X')
		{
		if ((uFlags & STN_mskfAllowHexPrefix) == 0)
			{
			pSTN->nErrCode = STN_errIllegalDigit;
			return FALSE;
			}
		pSTN->pchStop++;
		cDigitParsed = 0;
		uBase = 16;
		} // if

	while (*pSTN->pchStop != '\0')
		{
		MAP_VALUE_BYTE mvbChar = Ch_GetMvb(*pSTN->pchStop);
		if (mvbChar >= (int)uBase)
			break;
		cDigitParsed++;
		UINT_P uDataT = (pSTN->u.uData * uBase) + mvbChar;
		/*
		const UINT chDigit = *pSTN->pchStop;
		if (!FIsHexDigit(chDigit))
			break;
		int iDigit = GetHexDigitValue(chDigit);
		if (iDigit >= (int)uBase)
			break;
		cDigitParsed++;
		UINT_P uDataT = (pSTN->u.uData * uBase) + iDigit;
		*/
		if (uDataT < pSTN->u.uData)
			{
			pSTN->nErrCode = STN_errIntegerOverflow;
			return FALSE;
			}
		pSTN->u.uData = uDataT;
		pSTN->pchStop++;
		} // while

	if (uFlags & STN_mskfAllowIntegerDecimalPoint)
		{
		Assert(uBase == 10);
		// Check if there is a decimal point
		if (*pSTN->pchStop == '.')
			{
			// Scan for the digits after the dot
			const int iDigit = *++pSTN->pchStop - '0';
			if (iDigit >= 5 && iDigit <= 9)
				pSTN->u.nData++;	// Round up
			// Skip the remaining digits
			while (Ch_FIsDigit((BYTE)*pSTN->pchStop))
				pSTN->pchStop++;
			}
		}

	if ((cDigitParsed == 0) && (uFlags & STN_mskfMustHaveDigits))
		{
		// Empty String found while not allowed
		Assert(pSTN->u.uData == 0);
		pSTN->nErrCode = STN_errNoDigitsFound;
		return FALSE;
		}
	if (fIsNegative)
		{
		pSTN->u.nData = -pSTN->u.nData;
		}

CheckTail:
	if ((uFlags & STN_mskfNoSkipTailingSpaces) == 0)
		{
		// Skip the white spaces at the end of the string
		while (*pSTN->pchStop == ' ')
			pSTN->pchStop++;
		}
	if (uFlags & STN_mskfAllowRandomTail)
		return TRUE;
	if (*pSTN->pchStop != '\0')
		{
		pSTN->nErrCode = STN_errIllegalDigit;
		return FALSE;
		}
	return TRUE;
	} // FStringToNumber()

//	Convert a string to a number without caring about any error
INT_P
NStringToNumber_ZZR_ML(PSZUC pszString)
	{
	SStringToNumber stn;
	stn.uFlags = STN_mskzDecimalBase;
	stn.pszuSrc = pszString;
	if (FStringToNumber(INOUT &stn))
		return stn.u.nData;
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The value '$s' is not a valid number!\n", pszString);
	return 0;
	}

//	Parse a 64-bit integer
L64
LStringToNumber_ZZR_ML(PSZUC pszString)
	{
	BOOL fIsNegative = FALSE;
	if (*pszString == '-')
		{
		pszString++;
		fIsNegative = TRUE;
		}
	L64 lValue = 0;
	while (TRUE)
		{
		int ich = *pszString++ - '0';
		if (ich >= 0 && ich <= 9)
			lValue = lValue * 10 + ich;
		else
			break;
		}
	if (!fIsNegative)
		return lValue;
	return -lValue;
	}

UINT
UStringToNumberHexadecimal_ZZR_ML(PSZUC pszStringHexadecimal)
	{
	Assert(pszStringHexadecimal != NULL);
	SStringToNumber stn;
	stn.uFlags = STN_mskfHexBase;
	stn.pszuSrc = pszStringHexadecimal;
	if (FStringToNumber(INOUT &stn))
		return stn.u.uData;
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The value '$s' is not a valid hexadecimal number!\n", pszStringHexadecimal);
	return 0;
	}

struct SNumberLocalization
	{
	char chDecmialPoint;				// Character separating the decimal point (typicall the dot '.')
	char chThousandSeparator;
	char chNegative;
	char chUnused;
	};

const SNumberLocalization c_localizationEnglish = { '.', ',', '-', d_zNA };

/////////////////////////////////////////////////////////////////////
//	IntegerToString()
//
//	Convert a 32-bit integer into a null-terminating string with
//	optional zero-padding.
//
//	See also IntegerInsertThousandSeparatorsToNumber()
//
void
IntegerToString(
	CHU szBuffer[16],	// OUT: Output buffer integer string (should be greater than 16 characters)
	int nInteger,		// IN: 32-bit integer to convert to a string
	UINT uFlags)		// IN: Optional conversion flags
	{
	Assert(szBuffer != NULL);
//	InitToGarbage(OUT szBuffer, 16);

	CHU * psz = szBuffer;
	const UINT uBase = (uFlags & ITS_mskfHexadecimal) ? 16 : 10;
	const UINT chThousandSeparator = (uFlags & ITS_mskfThousandSeparator) ? ',' : '\0';
	int cchThousandSeparator = 3;

	if ((uFlags & (ITS_mskfUnsigned | ITS_mskfHexadecimal)) == 0 && nInteger < 0)
		{
		// Integer is negative
		*psz++ = '-';	// Add the minus sign
		nInteger = -nInteger;
		}
	CHU * pszStart = psz;
	while (TRUE)
		{
		*psz++ = c_rgchHexDigitsLowercase[(UINT)nInteger % uBase];
		nInteger /= uBase;
		if (nInteger <= 0)
			break;
		if (chThousandSeparator != '\0')
			{
			if (--cchThousandSeparator <= 0)
				{
				*psz++ = chThousandSeparator;
				cchThousandSeparator = 3;	// Reload the counter
				}
			}
		} // while

	for (int cchPad = (uFlags & ITS_mskmZeroPaddingMask) - (psz - szBuffer); cchPad > 0; cchPad--)
		{
		// Add extra zero-padding until desired length
		*psz++ = '0';
		}
	*psz-- = '\0';	// Append the null-terminator
	// We are done parsing, but the digits are in reverse order
	Assert(pszStart <= psz);
	for ( ; pszStart < psz; pszStart++, psz--)
		{
		// Swap the digits
		BYTE chT = *pszStart;
		*pszStart = *psz;
		*psz = chT;
		}
	} // IntegerToString()

//	Routine very similar to IntegerToString() however for large (64-bit) numbers.
void
Integer64ToString(
	OUT CHU pszuString[],
	L64 lInteger,
	UINT uFlags)
	{
	Assert(pszuString != NULL);
	const UINT chThousandSeparator = (uFlags & ITS_mskfThousandSeparator) ? ',' : '\0';
	int cchThousandSeparator = 3;

	CHU * psz = pszuString;
	if (lInteger < 0)
		{
		// Integer is negative
		*psz++ = '-';	// Add the minus sign
		lInteger = -lInteger;
		}

	CHU * pszStart = psz;
	while (TRUE)
		{
		*psz++ = '0' + (int)(lInteger % 10);
		lInteger /= 10;
		if (lInteger <= 0)
			break;
		if (chThousandSeparator != '\0')
			{
			if (--cchThousandSeparator <= 0)
				{
				*psz++ = chThousandSeparator;
				cchThousandSeparator = 3;	// Reload the counter
				}
			}
		} // while

	*psz-- = '\0';	// Append the null-terminator
	// We are done parsing, but the digits are in reverse order
	Assert(pszStart <= psz);
	for ( ; pszStart < psz; pszStart++, psz--)
		{
		// Swap the digits
		BYTE chT = *pszStart;
		*pszStart = *psz;
		*psz = chT;
		}
	} // Integer64ToString()


CHU *
PchNumberToString32(OUT CHU prgchString[24], int nNumber)
	{
	// At the moment, just use the PchNumberToString64() since it gives the same result
	return PchNumberToString64(OUT prgchString, nNumber);
	}

//	Convert a 64-bit integer to a string.  The string is NULL null-terminated.
CHU *
PchNumberToString64(OUT CHU prgchString[24], L64 lNumber)
	{
	Assert(prgchString != NULL);
	InitToGarbage(OUT prgchString, 24);

	if (lNumber < 0)
		{
		// Integer is negative
		*prgchString++ = '-';	// Add the minus sign
		lNumber = -lNumber;
		}
	CHU * pchString = prgchString;
	while (TRUE)
		{
		*pchString++ = '0' + (int)(lNumber % 10);
		lNumber /= 10;
		if (lNumber <= 0)
			break;
		} // while
	Assert(prgchString < pchString);
	for (CHU * pch = pchString - 1; prgchString < pch; prgchString++, pch--)
		{
		// Swap the digits
		CHS ch = *prgchString;
		*prgchString = *pch;
		*pch = ch;
		}
	return pchString;
	} // PchNumberToString64()

//	Format a number with an optional one decimal.
//	The decimal will appear if the number is less than 10, so there is a precision of two digits.
CHU *
PchNumberToString64_Decimal0or1(OUT CHU prgchString[24], L64 lNumber, UINT uDivideBy)
	{
	Assert(prgchString != NULL);
	InitToGarbage(OUT prgchString, 24);
	Assert(lNumber >= 0);
	Assert(uDivideBy > 0);
	lNumber += uDivideBy / 20;	// Since we are using integer division, add half of it so we 'round' the value
	L64 lInteger = lNumber / uDivideBy;
	if (lInteger <= 9)
		{
		prgchString[0] = '0' + (CHU)lInteger;
		L64 lDecimal = (lNumber * 10 / uDivideBy) - lInteger * 10;
		//MessageLog_AppendTextFormatCo(d_coGray, "$f => lInteger=$l, lDecimal=$l\n", (double)lNumber / lDivideBy, lInteger, lDecimal);
		Assert(lDecimal >= 0);
		if (lDecimal <= 0)
			return prgchString + 1;
		prgchString[1] = c_localizationEnglish.chDecmialPoint;
		prgchString[2] = '0' + (CHU)lDecimal;
		return prgchString + 3;
		}
	// If the value is larger or equal to 10, then do not display any decimal
	return PchNumberToString64(OUT prgchString, lInteger);
	}

CHU *
PchNumberToStringOnly(OUT CHU prgchString[32], L64 lNumber, const SNumberToStringFormat * pFormat)
	{
	InitToGarbage(OUT prgchString, 32);
	Assert(lNumber >= 0);
	Assert(pFormat != NULL);
	Assert(pFormat->cDecimalMinimum <= pFormat->cDecimalMaximum);
	Assert(pFormat->cDecimalMinimum >= 0);
	Assert(pFormat->cDecimalMaximum < 10);
	Assert(pFormat->uDivideBy > 0);
	UINT uDivideBy = pFormat->uDivideBy;
	L64 lInteger = lNumber / uDivideBy;
	UINT uDecimal = (lNumber % uDivideBy);
	// First, format the decimal part, so we may know if we need to round-up
	CHU rgchDecimal[20];
	InitToGarbage(OUT rgchDecimal, sizeof(rgchDecimal));
	CHU * pchDecimal = rgchDecimal;
	while (TRUE)
		{
		uDivideBy /= 10;
		if (uDivideBy <= 0)
			break;
		*pchDecimal++ = '0' + (uDecimal % 10);
		uDecimal /= 10;
		} // while
	*pchDecimal = '\0';	// Insert a null-terminator so we can determine if we need to round-up
	int cchDecimal = pchDecimal - rgchDecimal;	// How many decimal characters are available
	// As usual, reverse the array, so we can easily determine how many digits to keep
	CHU * pchSwapBegin = rgchDecimal;
	for (CHU * pchSwapEnd = pchDecimal - 1; pchSwapBegin < pchSwapEnd; pchSwapBegin++, pchSwapEnd--)
		{
		CHS ch = *pchSwapBegin;
		*pchSwapBegin = *pchSwapEnd;
		*pchSwapEnd = ch;
		}
	#if 0
	MessageLog_AppendTextFormatCo(d_coGreen,
		"\n\n$l / $u = lInteger=$l, lDecimal=$l  (cDecimalMin=$i, cDecimalMax=$i)\n"
		"rgchDecimal[$i] = $s\n", lNumber, pFormat->uDivideBy, lInteger, (lNumber % pFormat->uDivideBy), pFormat->cDecimalMinimum, pFormat->cDecimalMaximum, cchDecimal, rgchDecimal);
	#endif
	// We need to determine how many decimal digits to keep
	if (cchDecimal > pFormat->cDecimalMaximum)
		{
		// Keep only the maximum allowed number of decimal
		cchDecimal = pFormat->cDecimalMaximum;
		// TODO: Need to round up (if necessary)
		}

	// Format the 'integer' part
	pchSwapBegin = prgchString;
	int cchThousandSeparator = 3;
	while (TRUE)
		{
		*prgchString++ = '0' + (int)(lInteger % 10);
		lInteger /= 10;
		if (lInteger <= 0)
			break;
		if (--cchThousandSeparator <= 0)
			{
			cchThousandSeparator = 3;	// Reload the counter
			*prgchString++ = c_localizationEnglish.chThousandSeparator;
			}
		} // while
	for (CHU * pchSwapEnd = prgchString - 1; pchSwapBegin < pchSwapEnd; pchSwapBegin++, pchSwapEnd--)
		{
		CHS ch = *pchSwapBegin;
		*pchSwapBegin = *pchSwapEnd;
		*pchSwapEnd = ch;
		}
	// Now, include the decimal part, if any
	const int cchDecimalMinimum = pFormat->cDecimalMinimum;
	while (cchDecimal < cchDecimalMinimum)
		{
		// Pad with zeroes
		rgchDecimal[cchDecimal++] = '0';
		}
	while (cchDecimal > cchDecimalMinimum)
		{
		// Trim the tailing zeroes (if any)
		if (rgchDecimal[cchDecimal-1] != '0')
			break;
		cchDecimal--;
		}
	if (cchDecimal > 0)
		{
		*prgchString++ = c_localizationEnglish.chDecmialPoint;
		CHU * pchDecimalStart = rgchDecimal;
		CHU * pchDecimalStop = rgchDecimal + cchDecimal;
		while (pchDecimalStart < pchDecimalStop)
			*prgchString++ = *pchDecimalStart++;
		}
	return prgchString;
	} // PchNumberToStringOnly()

CHU *
PchNumberToStringFmtTemplate(OUT CHU prgchString[64], L64 lNumber, const SNumberToStringFormat * pFormat)
	{
	Assert(pFormat != NULL);
	Assert(pFormat->pszFmtTemplatePositive0 != NULL);
	PSZAC pszFmtTemplatePositive0 = pFormat->pszFmtTemplatePositive0;
	if (lNumber >= 0)
		{
		while (TRUE)
			{
			CHS ch = *pszFmtTemplatePositive0++;
			if (ch == '0')
				prgchString = PchNumberToStringOnly(OUT prgchString, lNumber, pFormat);
			else if (ch != '\0')
				*prgchString++ = ch;
			else
				break;
			}
		}
	else
		{
		PSZAC pszFmtTemplateNegative0 = pFormat->pszFmtTemplatePositive0;
		if (pszFmtTemplateNegative0 == NULL)
			pszFmtTemplateNegative0 = pszFmtTemplatePositive0;
		while (TRUE)
			{
			CHS ch = *pszFmtTemplateNegative0++;
			if (ch == '0')
				{
				if (pszFmtTemplateNegative0 != pszFmtTemplatePositive0)
					*prgchString++ = c_localizationEnglish.chNegative;
				prgchString = PchNumberToStringOnly(OUT prgchString, -lNumber, pFormat);
				}
			else if (ch != '\0')
				*prgchString++ = ch;
			else
				break;
			}
		}
	return prgchString;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
const SNumberToStringFormat c_format_Null	= { 1, 0, 0, NULL, NULL };
const SNumberToStringFormat c_format_BTC	= { d_cSatoshisPerBitcoin, 4, 8, "0 BTC", NULL };
const SNumberToStringFormat c_format_mBTC	= { d_cSatoshisPerBitcoin / 1000, 1, 5, "0 mBTC", "(0 mBTC)" };
const SNumberToStringFormat c_format_USD	= { 100, 2, 2, "$0 USD", "($0 USD)" };

CHU *
PchNumberToStringFormat_BTC(OUT CHU prgchStringBitcoin[64], AMOUNT amtSatoshis)
	{
	return PchNumberToStringFmtTemplate(OUT prgchStringBitcoin, amtSatoshis, IN &c_format_BTC);
	}

CHU *
PchNumberToStringFormat_mBTC(OUT CHU prgchStringBitcoin[64], AMOUNT amtSatoshis)
	{
	return PchNumberToStringFmtTemplate(OUT prgchStringBitcoin, amtSatoshis, IN &c_format_mBTC);
	}

CHU *
PchNumberToStringFormat_USD(OUT CHU prgchStringDollar[64], AMOUNT amtCents)
	{
	return PchNumberToStringFmtTemplate(OUT prgchStringDollar, amtCents, IN &c_format_USD);
	}

int
Amount_CchFormat(OUT CHU pszAmount[64], AMOUNT amt, CHS chEncodingAmount)
	{
	const SNumberToStringFormat * pFormat;
	switch (chEncodingAmount)
		{
	case d_chEncodingAmountBitcoin:
		pFormat = &c_format_BTC;
		break;
	case d_chEncodingAmountMilliBitcoin:
		pFormat = &c_format_mBTC;
		break;
	default:
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Amount_CchFormat($l) - unknown format '$b'\n", amt, chEncodingAmount);
		pFormat = &c_format_Null;
		}
	CHU * pch = PchNumberToStringFmtTemplate(OUT pszAmount, amt, IN pFormat);
	*pch = '\0';
	return (pch - pszAmount);
	}

QString
Amount_SFormat(AMOUNT amt, CHS chEncodingAmount)
	{
	CHU szAmount[64];
	int cchAmount = Amount_CchFormat(OUT szAmount, amt, chEncodingAmount);
	return QString::fromUtf8((const char *)szAmount, cchAmount);
	}
QString
Amount_SFormat_BTC(AMOUNT amt)
	{
	return Amount_SFormat(amt, d_chEncodingAmountBitcoin);
	}

QString
Amount_SFormat_mBTC(AMOUNT amt)
	{
	return Amount_SFormat(amt, d_chEncodingAmountMilliBitcoin);
	}
