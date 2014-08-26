//	Routines to convert a string to a numeric value and vice-versa.
#ifndef STRINGNUMERIC_H
#define STRINGNUMERIC_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


///////////////////////////////////////
struct SStringToNumber		// stn
	{
	UINT uFlags;				// IN: Flags to parse the string
	union
		{
		PSZUC pszuSrc;			// IN: Source string in UTF-8
		PSZAC pszaSrc;			// IN: Source string in Ansi
		};
	const CHU * pchStop;	// OUT: Pointer to where the parsing stopped
	int nErrCode;				// OUT: Error code
	union
		{
		INT_P nData;			// OUT: Signed integer value
		UINT_P uData;			// OUT: Unsigned integer value
		L64 llData;				// OUT: 64-bit integer value
		float flData;			// OUT: Float value
		double dblData;			// OUT: Double float value
		} u;
	};


///////////////////////////////////////
//	Flags for FStringToNumber()
#define STN_mskzDecimalBase					0x0000	// Use decimal base (default)
#define STN_mskzSkipLeadingSpaces			0x0000	// Skip white spaces at the beginning of the string
#define STN_mskzSkipTailingSpaces			0x0000	// Skip white spaces at the end of the string

#define STN_mskfHexBase						0x0001	// Use hexadecimal base as default
#define STN_mskfAllowHexPrefix				0x0002	// Look for a 0x prefix and select the appropriate base
#define STN_mskfAllowIntegerDecimalPoint	0x0004	// Allow the integer to have a decimal point (like a float value rounded to the nearest integer).  For instance, "2.4" will be be parsed as 2 and "2.5" will be parsed as 3.
#define STN_mskfAllowRandomTail				0x0008	// Stop parsing as soon as a non-digit is found without returning an error.  This is used to parse multiple numbers in a string

#define STN_mskfGetLongLong					0x0010	// NYI: Get a 64-bit integer
#define STN_mskfGetFloat					0x0020	// Get a float
#define STN_mskfGetDoubleFloat				0x0040	// Get a double float

#define STN_mskfNoLeadingSpaces				0x1000	// Interpret white spaces at beginning of string as error
#define STN_mskfNoSkipTailingSpaces			0x2000	// Do not skip any tailing spaces.  This is useful for tight parsing
#define STN_mskfMustHaveDigits				0x4000	// Interpret an empty string as an error instead of the value zero
#define STN_mskfUnsignedInteger				0x8000	// Interpret the minus sign as an error

///////////////////////////////////////
//	Error codes for for FStringToNumber()
#define STN_errSuccess           0  // No error
#define STN_errIntegerOverflow   1  // Integer too large
#define STN_errIllegalDigit      2  // String contains an illegal digit
#define STN_errNoDigitsFound     3  // No digits found (typically an empty string)
#define STN_errMinusSignFound	 4  // The number was negative


BOOL FStringToNumber(INOUT SStringToNumber * pSTN);
INT_P NStringToNumber_ZZR_ML(PSZUC pszString);
L64 LStringToNumber_ZZR_ML(PSZUC pszString);
UINT UStringToNumberHexadecimal_ZZR_ML(PSZUC pszStringHexadecimal);

///////////////////////////////////////
//	Flags for IntegerToString() and Integer64ToString()
#define ITS_mskzSigned				0x00000000	// Integer is signed (default)
#define ITS_mskfUnsigned			0x01000000	// Integer is unsigned
#define ITS_mskfHexadecimal			0x02000000	// Convert integer to hex string (without 0x prefix)
#define ITS_mskfThousandSeparator	0x04000000	// Write a comma for the separator
#define ITS_mskmZeroPaddingMask		0x0000FFFF	// Pad the string with zeroes until desired length

void IntegerToString(OUT CHU pszuString[16], int nInteger, UINT uFlags = 0);
void Integer64ToString(OUT CHU pszuString[32], L64 lInteger, UINT uFlags = 0);

CHU * PchNumberToString32(OUT CHU prgchString[24], int nNumber);
CHU * PchNumberToString64(OUT CHU prgchString[24], L64 lNumber);
CHU * PchNumberToString64_Decimal0or1(OUT CHU prgchString[24], L64 lNumber, UINT uDivideBy);

struct SNumberToStringFormat	// (format)
	{
	UINT uDivideBy;
	int cDecimalMinimum;		// Use -1 for no decimal point, and 0 for just a decimal point without any digit.
	int cDecimalMaximum;
	PSZAC pszFmtTemplatePositive0;		// "0 BTC", "$0 USD"
	PSZAC pszFmtTemplateNegative0;		// Optional
	};
CHU * PchNumberToStringOnly(OUT CHU prgchString[32], L64 lNumber, const SNumberToStringFormat * pFormat);
CHU * PchNumberToStringFmtTemplate(OUT CHU prgchString[64], L64 lNumber, const SNumberToStringFormat * pFormat);

#define d_cSatoshisPerBitcoin		100000000
CHU * PchNumberToStringFormat_BTC(OUT CHU prgchStringBitcoin[64], AMOUNT amtSatoshis);
CHU * PchNumberToStringFormat_mBTC(OUT CHU prgchStringBitcoin[64], AMOUNT amtSatoshis);

#define d_chEncodingAmountBitcoin		'B'
#define d_chEncodingAmountMilliBitcoin	'm'
#define d_chEncodingAmountDollar		'$'
#define d_chEncodingAmountPFormat		'*'	// Pointer to SNumberToStringFormat

int Amount_CchFormat(OUT CHU pszAmount[64], AMOUNT amt, CHS chEncodingAmount);
QString Amount_SFormat(AMOUNT amt, CHS chEncodingAmount);
QString Amount_SFormat_BTC(AMOUNT amt);
QString Amount_SFormat_mBTC(AMOUNT amt);

///////////////////////////////////////
TIMESTAMP Timestamp_GetCurrentDateTime();
int Timestamp_CchToString(TIMESTAMP ts, OUT CHU pszTimestamp[16]);
PCHRO Timestamp_PchFromString(OUT_UNTIL_STOP TIMESTAMP * pts, PSZUC pszTimestamp);
PCHRO Timestamp_PchFromStringSkipWhiteSpaces(OUT_UNTIL_STOP TIMESTAMP * pts, PSZUC pszTimestamp);
BOOL Timestamp_FGetNextTimestamp(OUT_F_UNCH TIMESTAMP * pts, INOUT PSZUC * ppszmTimestamps);
TIMESTAMP Timestamp_FromString_ML(PSZUC pszTimestamp);
TIMESTAMP Timestamp_FromStringW_ML(const QString & sTimestamp);
TIMESTAMP Timestamp_FromString_ZZR(PSZUC pszTimestamp);
int Timestamp_CchEncodeToBase64Url(TIMESTAMP ts, OUT CHU pszTimestamp[16]);
PCHRO Timestamp_PchDecodeFromBase64Url(OUT_ZZR TIMESTAMP * pts, PSZUC pszTimestamp);
QString Timestamp_ToStringBase85(TIMESTAMP ts);
QDateTime Timestamp_ToQDateTime(TIMESTAMP ts);

//	Routines to compare timestamps for sorting purpose

#if (QT_POINTER_SIZE == 8)
inline NCompareResult NCompareSortTimestamps(TIMESTAMP tsA, TIMESTAMP tsB) { return (tsA - tsB); }	// If the integer size is 64 bit, then we can do a subtraction
#else
inline NCompareResult NCompareSortTimestamps(TIMESTAMP tsA, TIMESTAMP tsB)
	{
	if (tsA < tsB)
		return -1;
	if (tsA > tsB)
		return +1;
	return 0;
	}
#endif

int TimestampDelta_CchToString(TIMESTAMP_DELTA dts, OUT CHU pszTimestampDelta[32]);
#endif // STRINGNUMERIC_H
