//	Generic routines to transform strings.
#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

PSZROU PszroGetTextAfterCharacter(PSZUC pszText, UINT chCompare);
PSZROU PszroGetServerNameFromJid(PSZUC pszJidOrEmail);
PSZROA PszroGetToolTipNext(PSZAC pszmTextAndToolTip);
PCHERROR PcheValidateUsername(PSZUC pszUsername);
PCHERROR PcheValidateServername(PSZUC pszServername);
PCHERROR PcheValidateJID(PSZUC pszJID);
BOOL FJidContainsServerName(PSZUC pszJID);
PSZR PszrFindDomainExtension(PSZUC pszText);

enum EScheme
	{
	eSchemeUnknown = 0,
	eSchemeHttpImplicit = 0,
	eSchemeHttp,
	eSchemeHttps,
	};

EScheme EGetScheme(PSZUC pszUrl);


#define FCompareStringsBeginsWith(pszStringCompare, pszStringReference)	(PszrCompareStringBeginNoCase(pszStringCompare, pszStringReference) != NULL)

PSZR PszrCompareStringBeginNoCase(PSZUC pszStringCompare, PSZAC pszStringReference);
PSZR PszrStringContainsSubString(PSZUC pszString, PSZAC pszSubString);
PSZR PszrStringContainsSubStringLowerCase(PSZUC pszString, PSZAC pszSubString);

#define strlenA								strlen
#define strcmpA								strcmp
#define strcmpiA							strcmpi
#define strcpynA							lstrcpyn
#define strcpynU(pszuDst, pszuSrc, cbMax)	strcpynA((PSZA)(PSZU)pszuDst, (PSZAC)(PSZUC)pszuSrc, cbMax)
VOID strcpynUsafe(PSZU pszuDst, PSZUC pszuSrc, int cbMax);

#define strchrU(pszuStringSearch, chSearch)	strchr((PSZAC)(PSZUC)pszuStringSearch, chSearch)
#define strlenU(pszuString)					strlenA((PSZAC)(PSZUC)pszuString)
#define strcmpU(pszuString1, pszuString2)	strcmpA((PSZAC)(PSZUC)pszuString1, (PSZAC)(PSZUC)pszuString2)

#define FCompareStrings(pszuString1, pszuString2)	(strcmpA((PSZAC)(PSZUC)pszuString1, (PSZAC)(PSZUC)pszuString2) == 0)
BOOL FCompareStrings_YZ(PSZUC pszuString1, PSZUC pszuString2);
BOOL FCompareStringsW(PSZWC pszw1, PSZWC pszw2);

void TrimTailingWhiteSpacesNZ(INOUT PSZU pszuString);
void StringReplaceAnyCharacterByCharacter(INOUT PSZU pszuString, CHS chFind, CHS chReplace);

int CbAllocUtoU(PSZUC pszuString);
int CbAllocUtoW(PSZUC pszuString);
int CbAllocWtoU(PSZWC pszwString);
#define CbAllocCchwToU(cchw)		((cchw) * 3 + 1)	// A wide character may take up to 3 UTF-8 characters

#define alloc_stack							alloca				// Allocate memory on the stack
#define STR_UtoU(pszuString)				(PSZU)strcpy(OUT (PSZA)alloc_stack(CbAllocUtoU(pszuString)), (PSZA)pszuString)
#define STR_UtoW(pszuString)				PszwUtoW(OUT_OPTIONAL (CHW *)alloc_stack(CbAllocUtoW(pszuString)), IN pszuString)
PSZW PszwUtoW(OUT_OPTIONAL CHW pszwBuffer[], IN PSZUC pszuString);
PSZU PszuWtoU(OUT_OPTIONAL CHU pszuBuffer[], IN PSZWC pszwString);

PSZU PaszAllocStringU(PSZUC pszuString);
PSZU PaszAllocStringW(PSZWC pszwString);
PSZU PaszAllocStringQ(const QString & sString);

PCHRO PchroNextCharacterUtf8(PSZUC pszuString);
int EncodeUnicodeToUtf8(OUT PSZU pszuUtf8, IN PSZWC pszwUnicode, OUT const CHW ** ppchwError = NULL);
int DecodeUtf8ToUnicode(OUT PSZW pszwUnicode, IN PSZUC pszuUtf8, OUT PSZUC * ppchuError = NULL);

int EncodeUtf32toUnicode(UINT chwUtf32, OUT PSZW pszwUnicode);
int EncodeUtf32toUtf8(UINT chwUtf32, OUT PSZU pszuUtf8);

#define CP_UNICODE			1200	// Code Page for the Unicode character set

extern const CHU c_mapbbLowercase[256];

extern const CHU c_rgchHexDigitsLowercase[];
typedef BYTE MAP_VALUE_BYTE;	// (mvb) Byte containing a Map Value
extern const MAP_VALUE_BYTE c_mapbbHexDigits[256];

//	Map Values sored as a Byte (mvb)
#define d_mvbCharacterCategoryNullTerminator	0xFF
#define d_mvbCharacterCategoryWhiteSpace		0xFE	// The character is a white space (' ', '\t', '\r', '\n')
#define d_mvbCharacterCategoryPunctuation		0xFD
#define d_mvbCharacterCategoryOutOfRange		0xFC	// The value is useless

#define Ch_GetMvb(ch)										(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch])
#define Ch_FIsDigit(ch)										(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] <= 9)
#define Ch_FIsHexDigit(ch)									(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] <= 0x0F)
#define Ch_FIsAlphaNumeric(ch)								(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] <= 35)
#define Ch_FIsWhiteSpace(ch)								(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] == d_mvbCharacterCategoryWhiteSpace)
#define Ch_FIsWhiteSpaceOrNullTerminator(ch)				(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] >= d_mvbCharacterCategoryWhiteSpace)
#define Ch_FIsPunctuationOrWhiteSpaceOrNullTerminator(ch)	(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] >= d_mvbCharacterCategoryPunctuation)
#define Ch_FIsPunctuation(ch)								(c_mapbbHexDigits[(MAP_VALUE_BYTE)ch] == d_mvbCharacterCategoryPunctuation)
#define Ch_GetCharLowercase(ch)								(c_mapbbLowercase[(MAP_VALUE_BYTE)ch])
#define Ch_ToUppercase(ch)									((ch) - 0x20)	// Convert a character from lowercase to uppercase (this assumes the character is already within 'a' and 'z')
#define Ch_ToLowercase(ch)									((ch) + 0x20)	// Convert a character from uppercase to lowercase (this assumes the character is already within 'A' and 'Z')
#define Ch_ToOtherCase(ch)									((ch) ^ 0x20)	// Convert a character from one case to the other (this assumes the character is already within 'A' and 'Z', or 'a' and 'z')

void Base16_StringFromBinary(OUT PSZU pszString, const BYTE prgbData[], int cbData);	//	Format an array of bytes into hexadecimal
BOOL Base16_FStringToBinary(IN PSZAC pszString, OUT_F_INV BYTE prgbData[], int cbData);
UINT Base16_FormatTextFromBinaryData(OUT PSZU pszTextBase16, const BYTE prgbBinaryData[], int cbBinaryData, UINT uFlagsSeparator);
UINT Base16_CbTextToBinary(PSZAC pszTextBase16);
PSZAC Base16_PcheTextToBinary(IN PSZAC pszTextBase16, OUT_F_INV BYTE prgbBinaryData[], int cbBinaryData);

PSZU Base41_EncodeToText(OUT PSZU pszStringBase41, const BYTE prgbDataBinary[], int cbDataBinary);

UINT Base64_CchEncoded(int cbDataBinary);
UINT Base64_CchEncodeToText(OUT PSZU pszStringBase64, const BYTE prgbDataBinary[], int cbDataBinary);

UINT Base85_CbEncodeAlloc(int cbDataBinary);
UINT Base85_CchEncodeToText(OUT PSZU pszStringBase85, const BYTE prgbDataBinary[], int cbDataBinary);
UINT Base85_CbDecodeAlloc(PSZUC pszStringBase85, OUT PCHRO * ppchroStop = NULL);
BOOL Base85_FCanDecodeToBinary(PSZUC pszStringBase85, int cbDataBinary);
UINT Base85_CbDecodeToBinary(IN PSZUC pszStringBase85, OUT_F_INV BYTE prgbDataBinary[]);
BOOL Base85_FDecodeToBinary_ML(IN PSZUC pszStringBase85, OUT_F_INV BYTE prgbDataBinary[], int cbDataBinary);
BOOL Base85_FDecodeToBinary_ZZR_ML(IN PSZUC pszStringBase85, OUT_ZZR BYTE prgbDataBinary[], int cbDataBinary);


#ifndef GUID_DEFINED
struct GUID { BYTE rgbData[16]; };	// A GUID has 16 bytes
#endif

void Guid_InitEmpty(OUT GUID * pGuid);
BOOL Guid_FIsEmpty(const GUID * pGuid);
void Guid_Generate(OUT GUID * pGuid);
void Guid_GenerateIfEmpty(INOUT GUID * pGuid);
BOOL Guid_FInitFromStringHex(OUT GUID * pGuid, IN PSZAC pszString);
inline void Guid_ToStringHex(OUT CHU pszStringHex[16*2+1], IN const GUID * pGuid)	{ Base16_StringFromBinary(OUT pszStringHex, (const BYTE *)pGuid, sizeof(GUID)); }	// The buffer pszString must have at least 33 bytes


///////////////////////////////////////
//	The salt prefix enables the user to reuse the same password for a configuration, a wallet and a password vault,
//	yet the function HashSha1_CalculateFromSaltedPassword() will produce a different hash, hence a different filename.
enum ESaltPrefix
	{
	eSaltPrefix_Config			= UINT32_FROM_CHARS('~', 'C', 'O', '|'),
	eSaltPrefix_Wallet			= UINT32_FROM_CHARS('~', 'W', 'A', '|'),
	eSaltPrefix_PasswordVault	= UINT32_FROM_CHARS('~', 'P', 'V', '|')
	};

struct SHashMd5 { BYTE rgbData[16]; };	// The MD5 hash value contains 128 bits

inline void HashMd5_InitEmpty(OUT SHashMd5 * pHashMd5)								{ Guid_InitEmpty(OUT (GUID *)pHashMd5); }
inline BOOL HashMd5_FIsEmpty(IN const SHashMd5 * pHashMd5)							{ return Guid_FIsEmpty(IN (const GUID *)pHashMd5); }
void HashMd5_CalculateFromBinary(OUT SHashMd5 * pHashMd5, IN const void * pvData, int cbData);
void HashMd5_CalculateFromCBin(OUT SHashMd5 * pHashMd5, IN const CBin & bin);
void HashMd5_CalculateFromCStr(OUT SHashMd5 * pHashMd5, IN const CStr & str);
void HashMd5_CalculateFromStringU(OUT SHashMd5 * pHashMd5, IN PSZUC pszuString);
void HashMd5_CalculateFromStringFormatted_Gsb(OUT SHashMd5 * pHashMd5, IN PSZAC pszFmtTemplate, ...);
BOOL HashMd5_FAssignHashValue(INOUT_F_UNCH SHashMd5 * pHashMd5Destination, IN const SHashMd5 * pHashMd5Source);
inline BOOL HashMd5_FInitFromStringHex(OUT SHashMd5 * pHashMd5, IN const CHU pszStringHex[16*2+1])		{ return Guid_FInitFromStringHex(OUT (GUID *)pHashMd5, (PSZAC)pszStringHex); }
BOOL HashMd5_FInitFromStringBase64(OUT SHashMd5 * pHashMd5);
inline void HashMd5_ToStringHex(OUT CHU pszStringHex[16*2+1], IN const SHashMd5 * pHashMd5)			{ Base16_StringFromBinary(OUT pszStringHex, (const BYTE *)pHashMd5, sizeof(SHashMd5)); }
void HashMd5_ToStringBase64(OUT CHU pszStringBase64[], IN const SHashMd5 * pHashMd5);


struct SHashSha1 { BYTE rgbData[20]; };	// The SHA-1 hash contains 160 bits

void HashSha1_InitEmpty(OUT SHashSha1 * pHashSha1);
void HashSha1_InitRandom(OUT SHashSha1 * pHashSha1);
void HashSha1_InitRandomIfEmpty(INOUT SHashSha1 * pHashSha1);
BOOL HashSha1_FIsEmpty(const SHashSha1 * pHashSha1);
void HashSha1_CalculateFromBinary(OUT SHashSha1 * pHashSha1, IN const void * pvData, int cbData);
void HashSha1_CalculateFromCBin(OUT SHashSha1 * pHashSha1, IN const CBin & bin);
void HashSha1_CalculateFromStringU(OUT SHashSha1 * pHashSha1, IN PSZUC pszuString);
void HashSha1_CalculateFromSaltedPassword(OUT SHashSha1 * pHashSha1, ESaltPrefix eSaltPrefix, IN PSZUC pszuPassword);
inline void HashSha1_ToStringHex(OUT CHU pszStringHex[20*2 + 1], IN const SHashSha1 * pHashSha1)			{ Base16_StringFromBinary(OUT pszStringHex, (const BYTE *)pHashSha1, sizeof(SHashSha1)); }
inline BOOL HashSha1_FCompareEqual(const SHashSha1 * pHashSha1A, const SHashSha1 * pHashSha1B)				{ return (0 == memcmp(pHashSha1A, pHashSha1B, sizeof(SHashSha1))); }
//inline void HashSha1_ToStringBase64(OUT CHU pszStringHex[42], IN const SHashSha1 * pHashSha1)			{ Base64_CchStringFromBinary(OUT pszStringHex, (const BYTE *)pHashSha1, sizeof(SHashSha1)); }
BOOL HashSha1_FInitFromStringBase85_ZZR_ML(OUT_ZZR SHashSha1 * pHashSha1, IN PSZUC pszuStringBase85);

struct SHashSha256 { BYTE rgbData[32]; };	// The SHA-256 hash contains 256 bits
struct SHashSha512 { BYTE rgbData[64]; };	// The SHA-256 hash contains 512 bits

enum EHashAlgorithm	// This enum must be in sync with QCryptographicHash::Algorithm
	{
	eHashAlgorithmNil = -1, // Invalid hash algorithim
	eHashAlgorithmNull = 0,	// Empty hash value returned by EGetHashAlgorithmFromHashSize()
	eHashAlgorithmMd5 = QCryptographicHash::Md5, // == 1
	eHashAlgorithmSha1 = QCryptographicHash::Sha1, // == 2
	eHashAlgorithmSha256 = QCryptographicHash::Sha256, // == 4
	eHashAlgorithmSha512 = QCryptographicHash::Sha512 // == 6
	};
EHashAlgorithm EGetHashAlgorithmFromHashSize(int cbHashSize);

struct SHashValue { BYTE rgbData[sizeof(SHashSha512)]; };	// Generic structure capable to hold the largest hash value
void HashValue_CalculateFromBinary(OUT SHashValue * pHashValue, IN const void * pvData, int cbData, EHashAlgorithm eHashAlgorithm);

struct SHashKey256 { BYTE rgbData[32];	};	//	Structure holding a key of 256 bits.  This structure is to encrypt and decrypt data.
void HashKey256_CalculateKeyFromPassword(OUT SHashKey256 * pHashKey, IN PSZUC pszuPassword);

#endif // STRINGUTILITIES_H
