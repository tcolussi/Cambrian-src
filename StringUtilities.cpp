//	Generic routines to transform strings.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

const CHU c_mapbbLowercase[256] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 58, 59, 60, 61, 62, 63, 64,
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 91, 92, 93, 94, 95, 96,
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 123, 124, 125, 126, 127, 128,
	129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
	161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
	193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
	225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const CHU c_rgchHexDigitsLowercase[] = "0123456789abcdef";		// Use lowercase hexadecimal digits because most XMPP hash functions require lowercase hexadecimal strings

//	Valid characters when encoding in Base85.  This array is also used to encode in Base41 and Base16.
const char c_rgchBase85[]	 = "0123456789abcdefghijklmnopqrstuvwxyz+-_^!ABCDEFGHIJKLMNOPQRSTUVWXYZ?*%$#@;`~|[]{}()/\\"; // Unused characters: =, (the colon : represent 32 bits of zeroes, the dot . represent 16-bits)

//	To make c_mapbbBase85[] easier to read, the following macros are defined
#define _NT		d_mvbCharacterCategoryNullTerminator
#define _WS		d_mvbCharacterCategoryWhiteSpace
#define _PU		d_mvbCharacterCategoryPunctuation
#define _OR		d_mvbCharacterCategoryOutOfRange

//	Array to decode a Base85 into it original format.
//	This array is also used to decode Base41 and Base16 as well as determining if a character is a whitespace.
const BYTE c_mapbbBase85[256] =
{
	_NT, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _WS, _WS, _OR, _OR, _WS, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_WS,  40, _OR,  71,  70,  69, _OR, _OR,  81,  82,  68,  36, _OR,  37, _OR,  83,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9, _OR,  73, _OR, _OR, _OR,  67,
	 72,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  77,  84,  78,  39,  38,
	 74,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  79,  76,  80,  75, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR
};

const char c_rgchBase64[]	 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char c_rgchBase64Url[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";	// Useful to encode URls so none of its characters are % encoded.

//	Array to decode BOTH Base64 AND Base64Url.  It is the responsibility of the routine to check for characters '+' and '/' vs '-' and '_' when not allowed.
const BYTE c_mapbbBase64andBase64Url[256] =
{
	_NT, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _WS, _WS, _OR, _OR, _WS, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_WS, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,  62, _OR,  62, _OR,  63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, _OR, _OR, _OR, _OR,  63,
	_OR,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR,
	_OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR, _OR
};

//	Table to decode Base 16 (hexadecimal value) as well as determining if a character is a white space and/or a punctuation.
const BYTE c_mapbbHexDigits[256] =
{
	_NT, // 0x0   0
	_OR, // 0x1   1
	_OR, // 0x2   2
	_OR, // 0x3   3
	_OR, // 0x4   4
	_OR, // 0x5   5
	_OR, // 0x6   6
	_OR, // 0x7   7
	_OR, // 0x8   8
	_WS, // 0x9   9
	_WS, // 0xa   10
	_OR, // 0xb   11
	_OR, // 0xc   12
	_WS, // 0xd   13
	_OR, // 0xe   14
	_OR, // 0xf   15
	_OR, // 0x10   16
	_OR, // 0x11   17
	_OR, // 0x12   18
	_OR, // 0x13   19
	_OR, // 0x14   20
	_OR, // 0x15   21
	_OR, // 0x16   22
	_OR, // 0x17   23
	_OR, // 0x18   24
	_OR, // 0x19   25
	_OR, // 0x1a   26
	_OR, // 0x1b   27
	_OR, // 0x1c   28
	_OR, // 0x1d   29
	_OR, // 0x1e   30
	_OR, // 0x1f   31
	_WS, // 0x20   32
	_PU, // 0x21   33  !
	_PU, // 0x22   34  "
	_PU, // 0x23   35  #
	_PU, // 0x24   36  $
	_PU, // 0x25   37  %
	_PU, // 0x26   38  &
	_PU, // 0x27   39  '
	_PU, // 0x28   40  (
	_PU, // 0x29   41  )
	_PU, // 0x2a   42  *
	_PU, // 0x2b   43  +
	_PU, // 0x2c   44  ,
	_PU, // 0x2d   45  -
	_PU, // 0x2e   46  .
	_PU, // 0x2f   47  /
	  0, // 0x30   48  0
	  1, // 0x31   49  1
	  2, // 0x32   50  2
	  3, // 0x33   51  3
	  4, // 0x34   52  4
	  5, // 0x35   53  5
	  6, // 0x36   54  6
	  7, // 0x37   55  7
	  8, // 0x38   56  8
	  9, // 0x39   57  9
	_PU, // 0x3a   58  :
	_PU, // 0x3b   59  ;
	_PU, // 0x3c   60  <
	_PU, // 0x3d   61  =
	_PU, // 0x3e   62  >
	_PU, // 0x3f   63  ?
	_PU, // 0x40   64  @
	 10, // 0x41   65  A
	 11, // 0x42   66  B
	 12, // 0x43   67  C
	 13, // 0x44   68  D
	 14, // 0x45   69  E
	 15, // 0x46   70  F
	 16, // 0x47   71  G
	 17, // 0x48   72  H
	 18, // 0x49   73  I
	 19, // 0x4a   74  J
	 20, // 0x4b   75  K
	 21, // 0x4c   76  L
	 22, // 0x4d   77  M
	 23, // 0x4e   78  N
	 24, // 0x4f   79  O
	 25, // 0x50   80  P
	 26, // 0x51   81  Q
	 27, // 0x52   82  R
	 28, // 0x53   83  S
	 29, // 0x54   84  T
	 30, // 0x55   85  U
	 31, // 0x56   86  V
	 32, // 0x57   87  W
	 33, // 0x58   88  X
	 34, // 0x59   89  Y
	 35, // 0x5a   90  Z
	_PU, // 0x5b   91  [
	_PU, // 0x5c   92
	_PU, // 0x5d   93  ]
	_PU, // 0x5e   94  ^
	_PU, // 0x5f   95  _
	_PU, // 0x60   96  `
	 10, // 0x61   97  a
	 11, // 0x62   98  b
	 12, // 0x63   99  c
	 13, // 0x64   100  d
	 14, // 0x65   101  e
	 15, // 0x66   102  f
	 16, // 0x67   103  g
	 17, // 0x68   104  h
	 18, // 0x69   105  i
	 19, // 0x6a   106  j
	 20, // 0x6b   107  k
	 21, // 0x6c   108  l
	 22, // 0x6d   109  m
	 23, // 0x6e   110  n
	 24, // 0x6f   111  o
	 25, // 0x70   112  p
	 26, // 0x71   113  q
	 27, // 0x72   114  r
	 28, // 0x73   115  s
	 29, // 0x74   116  t
	 30, // 0x75   117  u
	 31, // 0x76   118  v
	 32, // 0x77   119  w
	 33, // 0x78   120  x
	 34, // 0x79   121  y
	 35, // 0x7a   122  z
	_PU, // 0x7b   123  {
	_PU, // 0x7c   124  |
	_PU, // 0x7d   125  }
	_PU, // 0x7e   126  ~
	_OR, // 0x7f   127
	_OR, // 0x80   128
	_OR, // 0x81   129
	_OR, // 0x82   130
	_OR, // 0x83   131
	_OR, // 0x84   132
	_OR, // 0x85   133
	_OR, // 0x86   134
	_OR, // 0x87   135
	_OR, // 0x88   136
	_OR, // 0x89   137
	_OR, // 0x8a   138
	_OR, // 0x8b   139
	_OR, // 0x8c   140
	_OR, // 0x8d   141
	_OR, // 0x8e   142
	_OR, // 0x8f   143
	_OR, // 0x90   144
	_OR, // 0x91   145
	_OR, // 0x92   146
	_OR, // 0x93   147
	_OR, // 0x94   148
	_OR, // 0x95   149
	_OR, // 0x96   150
	_OR, // 0x97   151
	_OR, // 0x98   152
	_OR, // 0x99   153
	_OR, // 0x9a   154
	_OR, // 0x9b   155
	_OR, // 0x9c   156
	_OR, // 0x9d   157
	_OR, // 0x9e   158
	_OR, // 0x9f   159
	_OR, // 0xa0   160
	_OR, // 0xa1   161
	_OR, // 0xa2   162
	_OR, // 0xa3   163
	_OR, // 0xa4   164
	_OR, // 0xa5   165
	_OR, // 0xa6   166
	_OR, // 0xa7   167
	_OR, // 0xa8   168
	_OR, // 0xa9   169
	_OR, // 0xaa   170
	_OR, // 0xab   171
	_OR, // 0xac   172
	_OR, // 0xad   173
	_OR, // 0xae   174
	_OR, // 0xaf   175
	_OR, // 0xb0   176
	_OR, // 0xb1   177
	_OR, // 0xb2   178
	_OR, // 0xb3   179
	_OR, // 0xb4   180
	_OR, // 0xb5   181
	_OR, // 0xb6   182
	_OR, // 0xb7   183
	_OR, // 0xb8   184
	_OR, // 0xb9   185
	_OR, // 0xba   186
	_OR, // 0xbb   187
	_OR, // 0xbc   188
	_OR, // 0xbd   189
	_OR, // 0xbe   190
	_OR, // 0xbf   191
	_OR, // 0xc0   192
	_OR, // 0xc1   193
	_OR, // 0xc2   194
	_OR, // 0xc3   195
	_OR, // 0xc4   196
	_OR, // 0xc5   197
	_OR, // 0xc6   198
	_OR, // 0xc7   199
	_OR, // 0xc8   200
	_OR, // 0xc9   201
	_OR, // 0xca   202
	_OR, // 0xcb   203
	_OR, // 0xcc   204
	_OR, // 0xcd   205
	_OR, // 0xce   206
	_OR, // 0xcf   207
	_OR, // 0xd0   208
	_OR, // 0xd1   209
	_OR, // 0xd2   210
	_OR, // 0xd3   211
	_OR, // 0xd4   212
	_OR, // 0xd5   213
	_OR, // 0xd6   214
	_OR, // 0xd7   215
	_OR, // 0xd8   216
	_OR, // 0xd9   217
	_OR, // 0xda   218
	_OR, // 0xdb   219
	_OR, // 0xdc   220
	_OR, // 0xdd   221
	_OR, // 0xde   222
	_OR, // 0xdf   223
	_OR, // 0xe0   224
	_OR, // 0xe1   225
	_OR, // 0xe2   226
	_OR, // 0xe3   227
	_OR, // 0xe4   228
	_OR, // 0xe5   229
	_OR, // 0xe6   230
	_OR, // 0xe7   231
	_OR, // 0xe8   232
	_OR, // 0xe9   233
	_OR, // 0xea   234
	_OR, // 0xeb   235
	_OR, // 0xec   236
	_OR, // 0xed   237
	_OR, // 0xee   238
	_OR, // 0xef   239
	_OR, // 0xf0   240
	_OR, // 0xf1   241
	_OR, // 0xf2   242
	_OR, // 0xf3   243
	_OR, // 0xf4   244
	_OR, // 0xf5   245
	_OR, // 0xf6   246
	_OR, // 0xf7   247
	_OR, // 0xf8   248
	_OR, // 0xf9   249
	_OR, // 0xfa   250
	_OR, // 0xfb   251
	_OR, // 0xfc   252
	_OR, // 0xfd   253
	_OR, // 0xfe   254
	_OR  // 0xff   255
};


PSZROU
PszroGetTextAfterCharacter(PSZUC pszText, UINT chCompare)
	{
	if (pszText != NULL)
		{
		while (TRUE)
			{
			const UINT chText = *pszText;
			if (chText == '\0')
				break;
			pszText++;
			if (chText == chCompare)
				break;
			} // while
		}
	return pszText;
	}

PSZROU
PszroGetServerNameFromJid(PSZUC pszJidOrEmail)
	{
	return PszroGetTextAfterCharacter(pszJidOrEmail, '@');
	}

//	Find the next ToolTip (if any)
//	Return the pointer to the ToolTip separator (if any)
PSZROA
PszroGetToolTipNext(PSZAC pszmTextAndToolTip)
	{
	if (pszmTextAndToolTip != NULL)
		{
		while (TRUE)
			{
			UINT ch = *pszmTextAndToolTip;
			if (ch == '\0' || ch == d_chSeparatorTextAndToolTip)
				break;
			pszmTextAndToolTip++;
			} // while
		}
	return pszmTextAndToolTip;
	}

BOOL
FTextContainsTooltip(PSZAC pszmTextAndToolTip)
	{
	if (pszmTextAndToolTip != NULL)
		{
		while (TRUE)
			{
			UINT ch = *pszmTextAndToolTip++;
			if (ch == '\0')
				break;
			if (ch == d_chSeparatorTextAndToolTip)
				return TRUE;
			}
		}
	return FALSE;
	}

//	Return NULL if the username is valid.
//	Return a pointer to illegal character for a username, according to XEP-0029 and RFC 6122.
//	Although the characters '?' and '*' are allowed in a JID, this function considers them as invalid.
//
//	This function stops validating the Username when reaching '@', assuming the remaining is the server name.
PCHERROR
PcheValidateUsername(PSZUC pszUsername)
	{
	Assert(pszUsername != NULL);
	while (TRUE)
		{
		CHS ch = *pszUsername;
		switch (ch)
			{
		default:
			if (ch > ' ')
				break;
			// Fall Through - The space character and other whitespaces are not allowed //
		case '\"':
		case '\'':
		case '&':
		case '/':
		case ':':
		case '<':
		case '>':
		case 0x7F: // (del)

		case '?':
		case '*':
			return pszUsername;
		case '@':	// Don't validate the remaining after the @
		case '\0':
			return NULL;
			} // switch
		pszUsername++;
		} // while
	return NULL;
	}

PCHERROR
PcheValidateServername(PSZUC pszServername)
	{
	Assert(pszServername != NULL);
	while (TRUE)
		{
		CHS ch = *pszServername;
		switch (ch)
			{
		default:
			if (ch > ' ')
				break;
			// Fall Through - The space character and other whitespaces are not allowed //
		case '@':
		case '\"':
		case '\'':
		case '&':
		case '/':
		case '<':
		case '>':
		case 0x7F: // (del)

		case '?':
		case '*':
			return pszServername;
		case '\0':
			return NULL;
			} // switch
		pszServername++;
		} // while
	return NULL;
	}

//	Return NULL if the JID is valid.
PCHERROR
PcheValidateJID(PSZUC pszJID)
	{
	Assert(pszJID != NULL);
	PCHERROR pchError = PcheValidateUsername(pszJID);
	if (pchError != NULL)
		return pchError;
	PSZRO pszroServerName = PszroGetServerNameFromJid(pszJID);
	if (*pszroServerName == '\0')
		return pszroServerName;	// There is no server name (which is an error), therefore return the end of the string
	return PcheValidateServername(pszroServerName);
	}


//	Same as FCompareStrings(), however both pszuString1 and pszuString2 may be NULL.
BOOL
FCompareStrings_YZ(PSZUC pszuString1, PSZUC pszuString2)
	{
	if (pszuString1 == NULL)
		return (pszuString2 == NULL || pszuString2[0] == '\0');
	if (pszuString2 == NULL)
		return (pszuString1[0] == '\0');
	while (TRUE)
		{
		const CHS ch1 = *pszuString1++;
		if (ch1 == *pszuString2++)
			{
			if (ch1 == '\0')
				return TRUE;
			continue;
			}
		return FALSE;
		} // while
	}

//	Return TRUE if two Unicode strings are equal.
//	The comparison is case sensitive.
BOOL
FCompareStringsW(PSZWC pszw1, PSZWC pszw2)
	{
	Assert(pszw1 != NULL);
	Assert(pszw2 != NULL);
	while (TRUE)
		{
		const CHWS chw1 = *pszw1++;
		if (chw1 == (CHWS)*pszw2++)
			{
			if (chw1 == '\0')
				return TRUE;
			continue;
			}
		return FALSE;
		} // while
	} // FCompareStringsW()


/////////////////////////////////////////////////////////////////////
//	Return pointer to the next string if pszStringCompare begins by pszStringReference
//	The comparison is not case sensitive, however the reference string must be in lowercase.
//
//	EXAMPLE
//	PSZUC psz = "/data=foo";
//	PszrCompareStringBeginNoCase(psz, "/data=");	// Return "foo"
//
PSZUC
PszrCompareStringBeginNoCase(PSZUC pszStringCompare, PSZAC pszStringReference)
	{
	Assert(pszStringCompare != NULL);
	Assert(pszStringReference != NULL);
	while (TRUE)
		{
		const CHS chStringReference = *pszStringReference++;
		if (chStringReference == '\0')
			return pszStringCompare;
		Assert((chStringReference < 'A' || chStringReference > 'Z') && "Reference string contains uppercase characters");
		CHS chStringCompare = Ch_GetCharLowercase(*pszStringCompare++);
		/*
		if (chStringCompare >= 'A' && chStringCompare <= 'Z')
			chStringCompare += 32;	// Make the character lowercase
		*/
		if (chStringCompare == chStringReference)
			continue;
		else
			return NULL;
		} // while
	} // PszrCompareStringBeginNoCase()


/////////////////////////////////////////////////////////////////////
//	Find a substring within a string.
//
//	Return the pointer to the remaining of the string (what is after pszSubString).
//	Return NULL if pszSubString is not found in pszString.
//
PSZUC
PszrStringContainsSubString(PSZUC pszString, PSZAC pszSubString)
	{
	Assert(pszString != NULL);
	Assert(pszSubString != NULL);
	Assert(pszSubString[0] != '\0');
	UINT chSubStringStart = *pszSubString;
	if (chSubStringStart == '\0')
		return NULL;	// The substring does not exist, this is an error but we return NULL to avoid crashes
	while (TRUE)
		{
		UINT chString = *pszString++;
		if (chString != chSubStringStart)
			{
			if (chString == '\0')
				return NULL;
			continue;
			}
		const CHU * pszStringResume = NULL;
		const CHU * pchT = (PSZUC)pszSubString + 1;
		while (TRUE)
			{
			UINT chT = *pchT++;
			if (chT == '\0')
				return pszString; // - strlenU(pszSubString);
			chString = *pszString++;
			if (chT != chString)
				{
				if (chString == chSubStringStart)
					{
					if (pszStringResume == NULL)
						pszStringResume = pszString - 1;
					}
				break;
				}
			}
		if (pszStringResume != NULL)
			pszString = pszStringResume;
		else if (chString == '\0')
			return NULL;
		} // while
	} // PszrStringContainsSubString()

PSZUC
PszrStringContainsSubStringLowerCase(PSZUC pszString, PSZAC pszSubString)
	{
	Assert(pszString != NULL);
	Assert(pszSubString != NULL);
	Assert(pszSubString[0] != '\0');
	UINT chSubStringStart = Ch_GetCharLowercase(*pszSubString);
	if (chSubStringStart == '\0')
		return NULL;	// The substring does not exist, this is an error but we return NULL to avoid crashes
	while (TRUE)
		{
		UINT chString = Ch_GetCharLowercase(*pszString++);
		if (chString != chSubStringStart)
			{
			if (chString == '\0')
				return NULL;
			continue;
			}
		const CHU * pszStringResume = NULL;
		const CHU * pchT = (PSZUC)pszSubString + 1;
		while (TRUE)
			{
			UINT chT = Ch_GetCharLowercase(*pchT++);
			if (chT == '\0')
				return pszString; //  - strlenU(pszSubString);
			chString = *pszString++;
			if (chT != chString)
				{
				if (chString == chSubStringStart)
					{
					if (pszStringResume == NULL)
						pszStringResume = pszString - 1;
					}
				break;
				}
			}
		if (pszStringResume != NULL)
			pszString = pszStringResume;
		else if (chString == '\0')
			return NULL;
		} // while
	} // PszrStringContainsSubStringLowerCase()

/////////////////////////////////////////////////////////////////////
//	Range of characters for surrogates.
//	A surrogate is an extra wide character to represent a unicode character.
//	Surrogates are similar to DBCS (Double-Byte Character Set)
//	REMARKS: The routine uses a bit mask with the bitwise operator AND to
//			 determine if a given character is a surrogate.  This can be done since
//			 the range of surrogate has been crafted to be an 'even' hexadecimal value.
#define d_chwUnicodeSurrogateLeadFirst	0xD800	// First lead character
#define d_chwUnicodeSurrogateLeadLast	0xDBFF	// Last lead character
#define d_chwUnicodeSurrogateFirst		0xDC00	// First surrogate character
#define d_chwUnicodeSurrogateLast		0xDFFF	// Last surrogate character
#define d_mskmUnicodeSurrogateMask		0xFC00	// Mask to determine a surrogate (lead or tail) character

/////////////////////////////////////////////////////////////////////
//	Return TRUE if the unicode is a lead surrogate character.
//	This routine is not used within this code, but is there for documentation
//	and as a helper for splitting unicode strings.
BOOL
FIsUnicodeCharSurrogateLead(UINT chw)
	{
	return (chw >= d_chwUnicodeSurrogateLeadFirst && chw <= d_chwUnicodeSurrogateLeadLast);
	}

//	Fetch the first unicode character from the string.
//	The returned character may require 32 bits if it is a surrogate.
//
//	Return the number of unicode characters stored in pszwFirstChar; the possible
//	values are 1 for a regular unicode (or ascii) character, and 2 for a surrogate.
int
GetUnicodeFirstChar(OUT CHW pszwFirstChar[2], IN PSZWC pszwUnicode)
	{
	Assert(pszwFirstChar != NULL);
	Assert(pszwUnicode != NULL);
	Endorse(*pszwUnicode == '\0');		// An empty string will return null first char

	UINT chwFirst = pszwUnicode[0];
	pszwFirstChar[0] = chwFirst;
	if (!FIsUnicodeCharSurrogateLead(chwFirst))
		return 1;
	// We got a surrogate
	pszwFirstChar[1] = pszwUnicode[1];
	return 2;
	}

UINT
GetUtf32FromSurrogates(UINT chwSurrogate1, UINT chwSurrogate2)
	{
	return ((chwSurrogate1 - d_chwUnicodeSurrogateLeadFirst) << 10) + (chwSurrogate2 - d_chwUnicodeSurrogateFirst + 0x00010000);
	}

PSZUC
PchroNextCharacterUtf8(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	UINT chu = *pszuString++;
	if (chu <= 0x7F)
		return (chu != '\0') ? pszuString : pszuString - 1;
	// We have a 'Unicode' character, so we have to continue until we have no more bits of pattern 10xxxxxx, either 11xxxxxx or 01xxxxxx
	while (TRUE)
		{
		chu = *pszuString++;
		if ((chu & 0xC0) == 0x80)
			continue;
		return pszuString - 1;
		}
	}

//	Calculate the number of bytes to copy a string (which is the length of the string pluss the null-terminator)
int
CbAllocUtoU(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	const CHU * pch = pszuString;
	while (*pch++ != '\0')
		;
	return (pch - pszuString);
	}

//	Calculate the number of bytes necessary to convert an UTF-8 string to Unicode.
int
CbAllocUtoW(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	const CHU * pch = pszuString;
	while (*pch++ != '\0')
		;
	return (pch - pszuString) * sizeof(CHW);
	}

//	Calculate the number of bytes necessary to convert a Unicode string to UTF-8.
//
//	IMPLEMENTATION NOTES
//	To make sure the returned is as accurate as possible, every Unicode character is analyzed
//	to determine how many UTF-8 characters are necessary.
//	Of course, the returned value may be a bit longer, as Unicode Surrogates require less space than what is returned by CbAllocWtoU().
int
CbAllocWtoU(PSZWC pszwString)
	{
	Assert(pszwString != NULL);
	int cbExtra = 0;
	const CHW * pchw = pszwString;
	while (TRUE)
		{
		const UINT chw = *pchw++;
		if (chw <= 0x7F)
			{
			if (chw == '\0')
				return (pchw - pszwString) + cbExtra;
			continue;
			}
		if (chw < 0x800)
			cbExtra++;
		else
			cbExtra += 2;
		} // while
	Assert(FALSE && "Unreachable code");
	}

PSZU
PaszAllocStringU(PSZUC pszString)
	{
	if (pszString == NULL)
		return NULL;
	int cch = strlenU(pszString);
	if (cch == 0)
		return NULL;
	cch++;
	CHU * paszString = new CHU[cch];
	memcpy(OUT paszString, IN pszString, cch);
	return paszString;
	}

PSZU
PaszAllocStringW(PSZWC pszwString)
	{
	if (pszwString == NULL || pszwString[0] == '\0')
		return NULL;
	int cbAlloc = CbAllocWtoU(pszwString);
	CHU * paszString = new CHU[cbAlloc];
	return PszuWtoU(OUT_OPTIONAL paszString, pszwString);
	}

PSZU
PaszAllocStringQ(const QString & sString)
	{
	return PaszAllocStringW(sString.utf16());
	}

//	Convert an UTF-8 string to Unicode
PSZW
PszwUtoW(OUT_OPTIONAL CHW pszwBuffer[], IN PSZUC pszuString)
	{
	Assert(pszwBuffer != NULL);
	Assert(pszuString != NULL);
	DecodeUtf8ToUnicode(OUT pszwBuffer, pszuString);
	return pszwBuffer;
	}

//	Convert a Unicode string into UTF-8
PSZU
PszuWtoU(OUT_OPTIONAL CHU pszuBuffer[], IN PSZWC pszwString)
	{
	Assert(pszuBuffer != NULL);
	Assert(pszwString != NULL);
	EncodeUnicodeToUtf8(OUT pszuBuffer, pszwString);
	return pszuBuffer;
	}

//	Convert a string into a 32-bit value.
//	Typically the string contains only one letter, however may have up to 3 letters.
//	If the string has more than 3 characters, then it is an error and the returned USZU is undefined.
//
//	Like the _USZU*(), this method is subject to the processor's architecture, therefore this routine must be modified for big-endian processors.
USZU
UszuFromPsz(PSZUC pszUszu)
	{
	Assert(pszUszu != NULL);
	#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)

	const BYTE bFirst = pszUszu[0];
	if (bFirst != '\0')
		{
		if (pszUszu[1] == '\0')
			return _USZU1(bFirst);
		if (pszUszu[2] == '\0')
			return _USZU2(bFirst, pszUszu[1]);
		return _USZU3(bFirst, pszUszu[1], pszUszu[2]);
		}
	return 0;

	#else

	#ifdef DEBUG_WANT_ASSERT
	PSZUC pszUszuOriginal = pszUszu;
	#endif
	Assert(FALSE && "This code was never tested!");
	USZU uszu = 0;
	while (TRUE)
		{
		CHS chu = *pszUszu++;
		if (chu != '\0')
			uszu = (uszu << 8) | chu;
		else
			break;
		}
	#ifdef DEBUG_WANT_ASSERT
	pszUszu = PszFromUSZU(uszu);
	Assert(FCompareStrings(pszUszu, pszUszuOriginal));
	#endif
	return uszu;

	#endif // #if...#else
	}

/////////////////////////////////////////////////////////////////////
//	Convert a Unicode string into UTF-8.
//
//	This routine is similar to strcpy() but encodes each Unicode character to UTF-8.
//	There is no error checking for destination buffer pszuUtf8; it is the responsability of the caller
//	to make sure the destination buffer has enough room for the entire encoded string.
//	Each wide character may take up to three UTF-8 bytes.
//
//	The routine check for illegal unicode sequence.  In such a case, the pointer ppchwError is set
//	to the first illegal unicode character, however the conversion continue until the end of the string.
//	The routine will skip the illegal character.  The caller can use **ppchwError to detect the
//	illegal unicode character and display a message to the user to report such error.
//
//	This method returns the number of UTF-8 characters (bytes) stored in buffer pszuUtf8.
//	The returned value does NOT include the null-terminator.
//
int
EncodeUnicodeToUtf8(OUT PSZU pszuUtf8, IN PSZWC pszwUnicode, OUT const CHW ** ppchwError)
	{
	Assert(pszuUtf8 != NULL);
	Assert(pszwUnicode != NULL);
	Assert((void *)pszuUtf8 != (void *)pszwUnicode);
	#ifdef Q_OS_WIN
	DEBUG_CODE( PSZWC pszwUnicodeDebug = pszwUnicode; )
	#endif

	if (ppchwError != NULL)
		*ppchwError = NULL;	// No error yet
	PSZU pszuUtf8Begin = pszuUtf8;
	while (TRUE)
		{
		CHWS chw = *pszwUnicode++;
		if (chw <= 0x7F)
			{
			// We got an Ansi character
			*pszuUtf8++ = (CHU)chw;
			if (chw == '\0')
				{
				#ifdef DEBUG
				#ifdef Q_OS_WIN
				// Verify if the decoding yields to the same result
				// A failure does not necessary means an error in the decoding; an invalid sequence of Unicode characters will produce a valid UTF-8 string, and therefore producing a different Unicode string.
				int cchuDebug = (pszuUtf8 - pszuUtf8Begin);
				int cbAlloc = CbAllocWtoU(pszwUnicodeDebug);
				Assert(cbAlloc >= cchuDebug);
				if (cchuDebug < 1024*1024)
					{
					PSZW paszwDebug = new CHW[cchuDebug];
					const CHU * pszuErrorDebug;
					DecodeUtf8ToUnicode(OUT paszwDebug, IN pszuUtf8Begin, OUT &pszuErrorDebug);
					Assert(pszuErrorDebug == NULL);
					Report(FCompareStringsW(IN paszwDebug, pszwUnicodeDebug) == TRUE && "Unicode string contains illegal sequence of characters");
					delete paszwDebug;
					}
				else
					{
					MessageLog_AppendTextFormatCo(d_coOrange, "EncodeUnicodeToUtf8() skipping verification because string is too large, having $I UTF-8 characters\n", cchuDebug);
					}
				#endif // Q_OS_WIN
				#endif // DEBUG
				return (pszuUtf8 - pszuUtf8Begin - 1);	// We are done, return the number of UTF-8 characters stored in buffer (excluding the null-terminator)
				}
			continue;
			}
		if (chw < 0x800)
			{
			// We got a simple Unicode character
			pszuUtf8[0] = (CHU)(0xC0 | (chw >> 6));
			pszuUtf8[1] = (CHU)(0x80 | (chw & 0x3F));
			pszuUtf8 += 2;
			continue;
			}
		// Check if there is a surrogate
		if ((chw & d_mskmUnicodeSurrogateMask) == d_chwUnicodeSurrogateLeadFirst)
			{
			Assert(chw >= d_chwUnicodeSurrogateLeadFirst && chw <= d_chwUnicodeSurrogateLeadLast);
			Assert(FIsUnicodeCharSurrogateLead(chw));
			CHWS chwSurrogate = *pszwUnicode;	// Get the second character, and convert to UTF-32/UCS-4 first
			if ((chwSurrogate & d_mskmUnicodeSurrogateMask) == d_chwUnicodeSurrogateFirst)
				{
				Assert(chwSurrogate >= d_chwUnicodeSurrogateFirst && chwSurrogate <= d_chwUnicodeSurrogateLast);
				UINT chwUtf32 = (((UINT)chw - d_chwUnicodeSurrogateLeadFirst) << 10) + (chwSurrogate - d_chwUnicodeSurrogateFirst + 0x00010000);
				Assert(GetUtf32FromSurrogates(chw, chwSurrogate) == chwUtf32);
				// Encode the double unicode character into four UTF-8 characters
				pszuUtf8[0] = (CHU)(0xF0 | (chwUtf32 >> 18));
				pszuUtf8[1] = (CHU)(0x80 | ((chwUtf32 >> 12) & 0x3F));
				pszuUtf8[2] = (CHU)(0x80 | ((chwUtf32 >> 6) & 0x3F));
				pszuUtf8[3] = (CHU)(0x80 | (chwUtf32 & 0x3F));
				pszuUtf8 += 4;
				pszwUnicode++;
				continue;
				}
			// We have an illegal surrogate
			if (ppchwError != NULL)
				{
				if (*ppchwError == NULL)
					*ppchwError = pszwUnicode;
				}
			else
				{
				Assert(FALSE && "Illegal Unicode character!");
				}
			goto InsertUnicodeCharacter;	// Instead of skipping the illegal Unicode character, just insert it in the UTF-8 string
			//continue;	// Skip the illegal unicode character
			}
		Assert(!(chw >= d_chwUnicodeSurrogateLeadFirst && chw <= d_chwUnicodeSurrogateLeadLast));
		if ((chw & d_mskmUnicodeSurrogateMask) != d_chwUnicodeSurrogateFirst)
			{
			// We have a normal unicode character
			Assert(chw < d_chwUnicodeSurrogateFirst || chw > d_chwUnicodeSurrogateLast);
			InsertUnicodeCharacter:
			pszuUtf8[0] = (CHU)(0xE0 | (chw >> 12));
			pszuUtf8[1] = (CHU)(0x80 | ((chw >> 6) & 0x3F));
			pszuUtf8[2] = (CHU)(0x80 | (chw & 0x3F));
			pszuUtf8 += 3;
			continue;
			}
		Assert(!(chw < d_chwUnicodeSurrogateFirst || chw > d_chwUnicodeSurrogateLast));
		// We got a surrogate character without a lead character
		Assert(chw >= d_chwUnicodeSurrogateFirst && chw <= d_chwUnicodeSurrogateLast);
		if (ppchwError != NULL)
			{
			if (*ppchwError == NULL)
				*ppchwError = pszwUnicode - 1;
			}
		else
			{
			Assert(FALSE && "Illegal Unicode character!");
			}
		goto InsertUnicodeCharacter;	// Instead of skipping the illegal Unicode character, just insert it in the UTF-8 string
		//continue;	// Skip the illegal unicode character
		} // while
	} // EncodeUnicodeToUtf8()


/////////////////////////////////////////////////////////////////////
//	Convert a UTF-8 string to Unicode.
//
//	This routine is similar to strcpy() but decodes UTF-8 bytes to Unicode.
//	There is no error checking for destination buffer pszwUnicode; it is the responsability of the caller
//	to make sure the destination buffer has enough room for the entire decoded string.
//	Each UTF-8 byte takes no more than one Unicode character, so it is safe to use a destination
//	unicode buffer the same length as the UTF-8 string.  Therefore allocate two bytes per UTF-8 character.
//
//	The routine check for illegal UTF-8 sequence.  In such a case, the pointer ppszuError is set
//	to the beginning of the first illegal UTF-8 sequence, however the decoding continues until the end of the string.
//	The routine interpret any illegal UTF-8 sequence as a sequence of regular ANSI characters.
//	The caller can use *ppszuError to detect the error and display a message to the user to report the illegal sequence.
//
//	This method returns length of the unicode string (that is, the number of unicode characters stored
//	in output buffer pszwUnicode, excluding the null-terminator)
//
int
DecodeUtf8ToUnicode(OUT PSZW pszwUnicode, IN PSZUC pszuUtf8, OUT PSZUC * ppchuError)
	{
	Assert(pszwUnicode != NULL);
	Assert(pszuUtf8 != NULL);
	Assert((void *)pszwUnicode != (void *)pszuUtf8);

	if (ppchuError != NULL)
		*ppchuError = NULL;	// No error yet
	PSZW pszwUnicodeBegin = pszwUnicode;
	while (TRUE)
		{
		UINT chu = *pszuUtf8++;
		if (chu <= 0x7F)
			{
			// We got a low-Ansi character
			*pszwUnicode++ = (CHW)chu;
			if (chu == '\0')
				break;
			continue;
			}
		if ((chu & 0xE0) < 0xC0)
			{
			// The lead character does not have the bit pattern 11xvvvvv
			if (ppchuError != NULL)
				{
				if (*ppchuError == NULL)
					*ppchuError = pszuUtf8 - 1;
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "DecodeUtf8ToUnicode() - Illegal UTF-8 character $i\n", chu);
				Assert(FALSE && "Illegal UTF-8 character!");
				}
			// Treat the illegal character as a regular Ansi character
			*pszwUnicode++ = (CHW)chu;
			continue;
			}
		// We have a sequence of multiple UTF-8 bytes
		UINT chu2 = *pszuUtf8++;	// Read the second character
		if ((chu2 & 0xC0) != 0x80)
			{
			// The second character does not have the bit pattern 10vvvvvv
			if (ppchuError != NULL)
				{
				if (*ppchuError == NULL)
					*ppchuError = pszuUtf8 - 2;
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Illegal UTF-8 character $i '$s'\n", chu2, pszuUtf8 - 2);
				Report(FALSE && "Illegal UTF-8 character!");
				}
			// Treat the illegal sequence as regular Ansi characters
			pszwUnicode[0] = (CHW)chu;
			pszwUnicode[1] = (CHW)chu2;
			pszwUnicode += 2;
			if (chu2 == '\0')
				break;
			continue;
			}
		if ((chu & 0xE0) == 0xC0)
			{
			// We have a sequence of exactly 2 bytes, as the the first character has the bit pattern 110vvvvv
			*pszwUnicode++ = (CHW)(((chu & 0x1F) << 6) | (chu2 & 0x3F));
			continue;
			}
		UINT chu3 = *pszuUtf8++;	// Read the third character
		if ((chu3 & 0xC0) != 0x80)
			{
			// The third character does not have the bit pattern 10vvvvvv
			if (ppchuError != NULL)
				{
				if (*ppchuError == NULL)
					*ppchuError = pszuUtf8 - 3;
				}
			else
				{
				Report(FALSE && "Illegal UTF-8 character!");
				}
			// Treat the illegal sequence as regular Ansi characters
			pszwUnicode[0] = (CHW)chu;
			pszwUnicode[1] = (CHW)chu2;
			pszwUnicode[2] = (CHW)chu3;
			pszwUnicode += 3;
			if (chu3 == '\0')
				break;
			continue;
			}
		if ((chu & 0xF0) == 0xE0)
			{
			// We have a sequence of exactly 3 bytes, as the first
			*pszwUnicode++ = (CHW)(((chu & 0x0F) << 12) | ((chu2 & 0x3F) << 6) | (chu3 & 0x3F));
			continue;
			}
		// We have a sequence of 4 bytes
		UINT chu4 = *pszuUtf8++;
		if ((chu4 & 0xC0) == 0x80 && (chu & 0xF8) == 0xF0)
			{
			UINT chwUtf32 = (((chu & 0x07) << 18) | ((chu2 & 0x3F) << 12) | ((chu3 & 0x3F) << 6) | (chu4 & 0x3F)) - 0x00010000;
			pszwUnicode[0] = (CHW)((chwUtf32 >> 10) + d_chwUnicodeSurrogateLeadFirst);
			pszwUnicode[1] = (CHW)((chwUtf32 & 0x03FF) + d_chwUnicodeSurrogateFirst);
			pszwUnicode += 2;
			continue;
			}
		// The fourth character does not have the bit pattern 10vvvvvv
		// or the first character does not have the bit pattern 11110vvv
		if (ppchuError != NULL)
			{
			if (*ppchuError == NULL)
				*ppchuError = pszuUtf8 - 4;
			}
		else
			{
			Report(FALSE && "Illegal UTF-8 character!");
			}
		// Treat the illegal sequence as regular Ansi characters
		pszwUnicode[0] = (CHW)chu;
		pszwUnicode[1] = (CHW)chu2;
		pszwUnicode[2] = (CHW)chu3;
		pszwUnicode[3] = (CHW)chu4;
		pszwUnicode += 4;
		if (chu4 == '\0')
			break;
		continue;
		} // while
	return (pszwUnicode - pszwUnicodeBegin - 1);
	} // DecodeUtf8ToUnicode()

/////////////////////////////////////////////////////////////////////
//	Convert a UTF-32 (also known as UCS-4) character into unicode string (UTF-16/UCS-2)
//	The returned unicode string is always null terminated (therefore the length of pszwUnicode must be at least 3 characters)
//
//	This routine may used to process the WM_UNICHAR message or to
//	convert HTML "&#D" or "&#xH" values into unicode string.
//			D is a decimal number, refers to the Unicode decimal character number D.
//			H is a hexadecimal number, refers to the Unicode hexadecimal character number H.
//
//	The return value is the number of unicode characters stored in pszwUnicode (1 or 2 unicode characters)
//
//	This routine does not check for errors.  The caller can analyze the
//	content of pszwUnicode to determine if there are surrogates out of
//	range and/or detect a sequence of illegal unicode characters.
int
EncodeUtf32toUnicode(UINT chwUtf32, OUT PSZW pszwUnicode)
	{
	Assert(pszwUnicode != NULL);
	if (chwUtf32 <= 0xFFFF)
		{
		// Normal unicode character
		pszwUnicode[0] = (CHW)chwUtf32;
		pszwUnicode[1] = '\0';
		return 1;
		}
	const UINT dw = chwUtf32 - 0x00010000;
	pszwUnicode[0] = (CHW)((dw >> 10) + d_chwUnicodeSurrogateLeadFirst);
	pszwUnicode[1] = (CHW)((dw & 0x03FF) + d_chwUnicodeSurrogateFirst);
	pszwUnicode[2] = '\0';
	return 2;
	} // EncodeUtf32toUnicode()


/////////////////////////////////////////////////////////////////////
//	Convert a UTF-32 to an UTF-8 string.
//
//	The returned UTF-8 string is always null-terminated, therefore pszuUtf8 must be at least 5 bytes.
//
//	Return the number of UTF-8 characters stored in pszuUtf8 (excluding the null-terminator).
//
int
EncodeUtf32toUtf8(UINT chwUtf32, OUT PSZU pszuUtf8)
	{
	if (chwUtf32 <= 0x7F)
		{
		pszuUtf8[0] = chwUtf32;
		pszuUtf8[1] = '\0';
		return (chwUtf32 > 0);
		}
	if (chwUtf32 < 0x800)
		{
		pszuUtf8[0] = (CHU)(0xC0 | (chwUtf32 >> 6));
		pszuUtf8[1] = (CHU)(0x80 | (chwUtf32 & 0x3F));
		pszuUtf8[2] = '\0';
		return 2;
		}
	// We have a large Unicode character with the possibility of a surrogate character, so use the big routine
	Assert(pszuUtf8 != NULL);
	CHW szwUnicode[3];
	EncodeUtf32toUnicode(IN chwUtf32, OUT szwUnicode);
	return EncodeUnicodeToUtf8(OUT pszuUtf8, IN szwUnicode);
	}


#ifdef DEBUG

void
AssertValidUtf8(IN PSZUC pszuUtf8)
	{
	if (pszuUtf8 == NULL)
		return;
	if (pszuUtf8[0] == d_chuXmlAlreadyEncoded)
		pszuUtf8++;	// Skip this character, as it will cause assertions.  The rest of the string should be a valid UTF-8 string.

	// The trick is to convert the string to Unicode and back to UTF-8.  If the round-tripping is correct, then the UTF-8 string is valid.
	const CHU * pchu = pszuUtf8;
	while (*pchu++ != '\0')
		;

	CHW * pszwTemp;
	CBin binTemp;
	int cchu = pchu - pszuUtf8;
	if (cchu < 1000*1000)
		{
		pszwTemp = (CHW *)alloc_stack(cchu * sizeof(CHW));
		}
	else
		{
		// The string is more than 1 MB, so allocate it on the heap
		pszwTemp = (CHW *)binTemp.PvSizeAlloc(cchu * sizeof(CHW));
		}

	const CHU * pszuError;
	int cchwUnicode = DecodeUtf8ToUnicode(OUT pszwTemp, IN pszuUtf8, OUT &pszuError);
	UNUSED_VARIABLE(cchwUnicode);
	UINT cbAllocWtoU = CbAllocWtoU(pszwTemp);
	Assert(cbAllocWtoU == strlenU(pszuUtf8) + 1);
	if (pszuError != NULL)
		{
		int cbString = strlenU(pszuUtf8);
		int ibOffset = pszuError - pszuUtf8;
		PSZWC pszwError = STR_UtoW(pszuError);
		UNUSED_VARIABLE(cbString);
		UNUSED_VARIABLE(ibOffset);
		UNUSED_VARIABLE(pszwError);
		//MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Corrupted string at offset $i: '$s'\n", ibOffset, pszuError);
		Assert(FALSE && "Invalid/corrupted UTF-8 string");
		return;
		}
	} // AssertValidUtf8()

#endif // DEBUG


void
CString::InitFromStringQ(const QString & s)
	{
	setUtf16(s.utf16(), s.size());	// Call the method setUtf16(), otherwise the constructor will recursively call the operator = which will cause a stack overflow
	}

void
CString::InitFromCBin(const CBin & bin)
	{
	*(QString *)this = bin.ToQString();
	}

void
CString::InitFromCStr(const CStr & str)
	{
	*(QString *)this = str.ToQString();
	//str.ToQString(OUT this);	// We need to use the method toQString(), otherwise the static method fromUtf8() will require the use of the = operator which will bust the stack
	}

void
CString::InitFromStringU(PSZUC pszuString)
	{
	*(QString *)this = QString::fromUtf8((const char *)pszuString);
	}

void
CString::Format(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	CStr str;
	InitFromStringU(str.Format_VL(pszFmtTemplate, vlArgs));
	}

void
CString::File_StripFileName()
	{
	CStr strFilePathOnly = *this;
	strFilePathOnly.PathFile_StripFileName();
	InitFromCStr(strFilePathOnly);
	}



//	Format an array of bytes into hexadecimal
void
Base16_StringFromBinary(OUT PSZU pszString, const BYTE prgbData[], int cbData)
	{
	Assert(pszString != NULL);
	Assert(prgbData != NULL);
	Assert(cbData >= 0);

	const BYTE * pbStop = prgbData + cbData;
	while (prgbData != pbStop)
		{
		UINT b = *prgbData++;
		pszString[0] = (BYTE)c_rgchHexDigitsLowercase[b >> 4];
		pszString[1] = (BYTE)c_rgchHexDigitsLowercase[b & 0x0F];
		pszString += 2;
		}
	*pszString = '\0';
	}

#define Base16_CchuAllocToFormatTextFromBinaryData(cbBinaryData)	((cbBinaryData) * 2 + (cbBinaryData) / 2 + sizeof(CHU))	// The function Base16_FormatTextFromBinaryData() add a separator every 4 hexadecimal character
#define d_cchBase16Format_Separator			2
#define d_chBase16Format_Separator_Space	' '
#define d_chBase16Format_Separator_Dash		'-'
#define d_chBase16Format_Separator_Colon	':'

//	Create a nice human-readable base-16 string with separators.
//	Return the length of pszTextBase16
UINT
Base16_FormatTextFromBinaryData(OUT PSZU pszTextBase16, const BYTE prgbBinaryData[], int cbBinaryData, UINT uFlagsSeparator)
	{
	Assert(pszTextBase16 != NULL);
	Assert(prgbBinaryData != NULL);
	Assert(cbBinaryData >= 0);
	CHU * pchTextBase16 = pszTextBase16;
	int cchUntilNextSeparator = d_cchBase16Format_Separator;
	const BYTE * pbStop = prgbBinaryData + cbBinaryData;
	while (prgbBinaryData != pbStop)
		{
		if (cchUntilNextSeparator-- == 0)
			{
			cchUntilNextSeparator = d_cchBase16Format_Separator - 1;
			*pchTextBase16++ = (BYTE)uFlagsSeparator;
			}
		UINT b = *prgbBinaryData++;
		pchTextBase16[0] = (BYTE)c_rgchHexDigitsLowercase[b >> 4];
		pchTextBase16[1] = (BYTE)c_rgchHexDigitsLowercase[b & 0x0F];
		pchTextBase16 += 2;
		}
	*pchTextBase16 = '\0';
	return (pchTextBase16 - pszTextBase16);
	}

//	Calculate how many bytes of binary data will be extracted from the Base-16 text.
UINT
Base16_CbTextToBinary(PSZAC pszTextBase16)
	{
	Assert(pszTextBase16 != NULL);
	int cchBase16 = 1;	// Always include an extra character in case the Base16 string has an odd number of character (which will help reporting a better error message to the user)
	while (TRUE)
		{
		UINT ch = *pszTextBase16++;
		if (Ch_FIsHexDigit(ch))
			cchBase16++;
		else if (ch == '\0')
			return (cchBase16 / 2);
		}
	}

//	This function convert text in Base16 to binary.  This function is the inverse of Base16_FormatTextFromBinaryData().
//
//	Return NULL if the function was successful and the binary data stored in prgbBinaryData[] is exactly cbBinaryData.
//	Return a pointer where the error occurred, either because of an invalid hexadecimal character, or the buffer prgbBinaryData[] is too short or too long to store the entire data from pszTextBase16.
PSZAC
Base16_PcheTextToBinary(IN PSZAC pszTextBase16, OUT_F_INV BYTE prgbBinaryData[], int cbBinaryData)
	{
	Assert(pszTextBase16 != NULL);
	Assert(prgbBinaryData != NULL);
	Assert(cbBinaryData >= 0);	// A zero value is not recommended, however will just return an empty string

	BYTE * pbBinaryDataStop = prgbBinaryData + cbBinaryData;
	while (prgbBinaryData != pbBinaryDataStop)
		{
		const BYTE bChar0 = pszTextBase16[0];
		MAP_VALUE_BYTE mvbChar0 = Ch_GetMvb(bChar0);
		if (mvbChar0 <= 0x0F)
			{
			MAP_VALUE_BYTE mvbChar1 = Ch_GetMvb(pszTextBase16[1]);
			if (mvbChar1 <= 0x0F)
				{
				*prgbBinaryData++ = (mvbChar0 << 4) | mvbChar1;
				pszTextBase16 += 2;
				continue;
				}
			return pszTextBase16 + 1;
			}
		// We got a character which is not an hexadecimal character
		switch (bChar0)
			{
		case d_chBase16Format_Separator_Space:
		case d_chBase16Format_Separator_Colon:
		case d_chBase16Format_Separator_Dash:
			pszTextBase16++;
			continue;	// Those characters are considered 'white spaces'
			}
		// Any other character, including the null-terminator is an invalid character
		return pszTextBase16;
		} // while
	if (*pszTextBase16 == '\0')
		return NULL;	// The text must terminate exactly where the binary buffer stops
	return pszTextBase16;
	} // Base16_PcheTextToBinary()
/*
//	Convert Base16 text into binary and display a message box to the user if the Base16 contains an invalid character.
BOOL
Base16_FTextToBinaryMB(IN PSZAC pszTextBase16, OUT_F_INV BYTE prgbBinaryData[], int cbBinaryData)
	{
	PSZAC pcheInvalid = Base16_PcheTextToBinary(IN (PSZAC)pszTextBase16, OUT prgbBinaryData, cbBinaryData);
	if (pcheInvalid == NULL)
		return TRUE;

	return FALSE;
	}
*/

void
CBin::BinAppendTextFormattedFromFingerprintBinary(const void * pvFingerprintBinary, int cbFingerprintBinary)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pvFingerprintBinary));
	Assert(cbFingerprintBinary >= 0);
	PSZU pszHex = PbAllocateExtraMemory(Base16_CchuAllocToFormatTextFromBinaryData(cbFingerprintBinary));
	m_paData->cbData += Base16_FormatTextFromBinaryData(OUT pszHex, (const BYTE *)pvFingerprintBinary, cbFingerprintBinary, d_chBase16Format_Separator_Space);
	Assert(strlenU(pszHex) < Base16_CchuAllocToFormatTextFromBinaryData(cbFingerprintBinary));
	}

//	Convert a fingerptint the user entered into binary.
//	Return FALSE if an invalid fingerprint was entered.
//	Return TRUE if a valid or empty fingerprint was entered.
//	If this method fails, the content of the bin is unchanged.
BOOL
CBin::BinAppendBinaryFingerprintF_MB(IN PSZUC pszTextFingerprint) OUT_F_UNCH
	{
	Assert(pszTextFingerprint != NULL);
	Assert(!FIsPointerAddressWithinBinaryObject(pszTextFingerprint));
	const int cbBinaryFingerprint = Base16_CbTextToBinary((PSZAC)pszTextFingerprint);
	BYTE * pbBinaryFingerprint = PbAllocateExtraDataWithVirtualNullTerminator(cbBinaryFingerprint);
	PSZUC pcheInvalid = (PSZUC)Base16_PcheTextToBinary(IN (PSZAC)pszTextFingerprint, OUT pbBinaryFingerprint, cbBinaryFingerprint);
	if (pcheInvalid == NULL)
		{
		// No error occurred while converting from Base16 to Binary, so check if the length of the binary is one of the known fingerprint
		if (EGetHashAlgorithmFromHashSize(cbBinaryFingerprint) >= eHashAlgorithmNull)
			return TRUE;
		(void)EMessageBoxWarning("The fingerprint <b>$s</b> does not match any MD5, SHA-1, SHA-256 or SHA-512 format.", pszTextFingerprint);
		}
	m_paData->cbData = pbBinaryFingerprint - m_paData->rgbData;	// Truncate the data as before
	if (pcheInvalid != NULL)
		(void)EMessageBoxWarning("The fingerprint <b>$s</b> constains an invalid character <b>{s1}</b> at position $i.", pszTextFingerprint, pcheInvalid, pcheInvalid - pszTextFingerprint + 1);
	return FALSE;
	} // BinAppendBinaryFingerprintF_MB()

BOOL
CBin::BinAppendBinaryFingerprintF_MB(const QLineEdit * pwEdit)
	{
	Assert(pwEdit != NULL);
	CStr strFingerprint = *pwEdit;
	return BinAppendBinaryFingerprintF_MB(IN strFingerprint);
	}

void
CStr::InitFromFingerprintBinary(const void * pvFingerprint, int cbFingerprint)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BinAppendTextFormattedFromFingerprintBinary(pvFingerprint, cbFingerprint);
	Assert(m_paData != NULL);
	Assert(m_paData->rgbData[m_paData->cbData] == '\0');
	m_paData->cbData++;	// Add the null-terminator
	}

void
CStr::InitFromFingerprintBinary(const CBin & binFingerprint)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	SHeaderWithData * pDataFingerprint = binFingerprint.m_paData;
	if (pDataFingerprint != NULL)
		InitFromFingerprintBinary(pDataFingerprint->rgbData, pDataFingerprint->cbData);
	}

void
CStr::InitFromFingerprintBinary(const QByteArray & arraybFingerprint)
	{
	InitFromFingerprintBinary(arraybFingerprint.constData(), arraybFingerprint.size());
	}

//	This method is the complement to Base16_StringFromBinary().
//	Return FALSE if an error occurred or if the pszString is too long or too short
BOOL
Base16_FStringToBinary(IN PSZAC pszString, OUT_F_INV BYTE prgbData[], int cbData)
	{
	Assert(pszString != NULL);
	Assert(prgbData != NULL);
	Assert(cbData >= 0);	// A zero value is not recommended, however will just return an empty string

	BYTE * pbDataStop = prgbData + cbData;
	while (prgbData != pbDataStop)
		{
		MAP_VALUE_BYTE mvbChar0 = Ch_GetMvb(pszString[0]);
		if (mvbChar0 <= 0x0F)
			{
			MAP_VALUE_BYTE mvbChar1 = Ch_GetMvb(pszString[1]);
			if (mvbChar1 <= 0x0F)
				{
				*prgbData++ = (mvbChar0 << 4) | mvbChar1;
				pszString += 2;
				continue;
				}
			}
		// Invalid digits
		return FALSE;
		} // while
	return (*pszString == '\0');	// String must terminate here
	} // Base16_FStringToBinary()

BOOL
CBin::BinAppendBinaryFromTextBase16F(PSZUC pszTextBase16)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszTextBase16));
	int cbBinary = Base16_CbTextToBinary((PSZAC)pszTextBase16);
	BYTE * pbBinary = PbAllocateExtraDataWithVirtualNullTerminator(cbBinary);
	return Base16_FStringToBinary(IN (PSZAC)pszTextBase16, OUT pbBinary, cbBinary);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
void
Guid_Empty(OUT GUID * pGuid)
	{
	InitToZeroes(OUT pGuid, sizeof(GUID));
	}

BOOL
Guid_FIsEmpty(const GUID * pGuid)
	{
	Assert(pGuid != NULL);
	return ((((L64 *)pGuid)[0] == 0) && (((L64 *)pGuid)[1] == 0));
	}

void
Guid_Generate(OUT GUID * pGuid)
	{
	QUuid uuid = QUuid::createUuid();
	Assert(sizeof(uuid) == sizeof(GUID));
	memcpy(OUT pGuid, IN &uuid, sizeof(GUID));
	}

void
Guid_GenerateIfEmpty(INOUT GUID * pGuid)
	{
	if (Guid_FIsEmpty(IN pGuid))
		Guid_Generate(OUT pGuid);
	}

BOOL
Guid_FInitFromStringHex(OUT GUID * pGuid, IN PSZAC pszString)
	{
	Assert(pGuid != NULL);
	Assert(pszString != NULL);
	if (Base16_FStringToBinary(IN pszString, OUT_F_INV (BYTE *)pGuid, sizeof(GUID)))
		return TRUE;
	// Return an empty GUID
	Guid_Empty(OUT pGuid);
	return FALSE;
	}

//	Return the number of bytes necessary to allocate for encoding binary data into Base85.
//	This function includes for the null terminator and an extra byte.  To
UINT
Base85_CbEncodeAlloc(int cbDataBinary)
	{
	return (cbDataBinary + cbDataBinary / 4) + 1 + 1;
	}

//	Encode 4 bytes of binary data into 5 bytes of Base85 characters.
//	Return the length of the Base85-encoded string, excluding the null-terminator
UINT
Base85_CchEncodeToText(OUT PSZU pszStringBase85, const BYTE prgbDataBinary[], int cbDataBinary)
	{
	Assert(pszStringBase85 != NULL);
	Assert(prgbDataBinary != NULL || cbDataBinary == 0);
	PSZU pszStringBase85Begin = pszStringBase85;
	const int cbDataBinaryAligned32 = (cbDataBinary & ~3);
	const BYTE * pbDataStopAligned32 = prgbDataBinary + cbDataBinaryAligned32;
	const UINT * puBinary = (const UINT *)prgbDataBinary;
	while (puBinary != (const UINT *)pbDataStopAligned32)
		{
		UINT uBinary = *puBinary++;
		PSZU pszStringBase85Stop = pszStringBase85 + 5;
		while (pszStringBase85 != pszStringBase85Stop)
			{
			*pszStringBase85++ = c_rgchBase85[uBinary % 85];
			uBinary /= 85;
			}
		}
	if (cbDataBinary & 3)
		{
		UINT uBinary = 0;
		while (pbDataStopAligned32 != prgbDataBinary + cbDataBinary)
			uBinary = (uBinary << 8) | *pbDataStopAligned32++;
		while (uBinary > 0)
			{
			*pszStringBase85++ = c_rgchBase85[uBinary % 85];
			uBinary /= 85;
			}
		}
	*pszStringBase85 = '\0';
	Assert((pszStringBase85 - pszStringBase85Begin) < (int)Base85_CbEncodeAlloc(cbDataBinary));
	return (pszStringBase85 - pszStringBase85Begin);
	} // Base85_CchEncodeToText()

//	Return how many bytes are needed to be allocated to decode a Base85 string.
//	The returned value is NOT the exact length of decoded string, however how much memory
//	should be allocated for parameter prgbDataBinarySzv[] in function Base85_CbDecodeToBinary().
//
//	Optionally this function return the pointer where the decoding will stop.
//	In a successful scenario, the pointer will be the null-terminator of pszStringBase85;
UINT
Base85_CbDecodeAlloc(PSZUC pszStringBase85, OUT PCHRO * ppchroStop)
	{
	int cchMapValues = 2;	// Include for an extra character and the null-terminator
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase85[(BYTE)*pszStringBase85++];
		if (mvbMapValue < 85)
			cchMapValues++;
		else if (mvbMapValue != d_mvbCharacterCategoryWhiteSpace)
			break;
		} // while
	if (ppchroStop != NULL)
		*ppchroStop = pszStringBase85 - 1;
	return cchMapValues * 4 / 5;
	}

//	Return TRUE if the Base85 string can be decoded into a binary buffer.
//	For simplicity, this function is optimized for fixed-length binary data which is 32-bit aligned, such as a GUID or SHA1
BOOL
Base85_FCanDecodeToBinary(PSZUC pszStringBase85, int cbDataBinary)
	{
	int cchMapValues = 0;
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase85[(BYTE)*pszStringBase85++];
		if (mvbMapValue < 85)
			cchMapValues++;
		else if (mvbMapValue != d_mvbCharacterCategoryWhiteSpace)
			break;
		} // while
	return (cchMapValues * 4 == cbDataBinary * 5);
	}


//	Return the number of bytes stored in prgbDataBinarySzv[].
//	This function does NOT include any null-terminator, so it is the responsibility of the caller to add it if necessary.
UINT
Base85_CbDecodeToBinary(IN PSZUC pszStringBase85, OUT_F_INV BYTE prgbDataBinary[])
	{
	Assert(pszStringBase85 != NULL);
	Assert(prgbDataBinary != NULL);
	Assert(sizeof(c_mapbbBase85) == 256);
	#ifdef DEBUG_WANT_ASSERT
	const int cbDecodeAllocDebug = Base85_CbDecodeAlloc(pszStringBase85);
	#endif

	UINT * pdwBinary = (UINT *)prgbDataBinary;
	UINT uBinary = 0;
	UINT uMultiplicator = 1;
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase85[(BYTE)*pszStringBase85++];
		if (mvbMapValue < 85)
			{
			uBinary += mvbMapValue * uMultiplicator;
			if (uMultiplicator != 85*85*85*85)
				uMultiplicator *= 85;
			else
				{
				// We have read 5 characters from the Base85 string, so it is time to store them into the binary buffer
				uMultiplicator = 1;
				*pdwBinary++ = uBinary;
				uBinary = 0;
				}
			}
		else if (mvbMapValue != d_mvbCharacterCategoryWhiteSpace)
			break;
		} // while
	BYTE * pbBinary = (BYTE *)pdwBinary;
	if (uBinary > 0)
		{
		// Add the remaining binary data
		const BYTE b0 = uBinary;
		const BYTE b1 = uBinary >> 8;
		UINT uBinaryHiWord = uBinary >> 16;
		if (uBinaryHiWord == 0)
			{
			if (b1 == 0)
				{
				*pbBinary++ = b0;
				}
			else
				{
				pbBinary[0] = b1;
				pbBinary[1] = b0;
				pbBinary += 2;
				}
			}
		else
			{
			if (uBinaryHiWord <= 0xFF)
				{
				pbBinary[0] = uBinaryHiWord;
				pbBinary[1] = b1;
				pbBinary[2] = b0;
				pbBinary += 3;
				}
			else
				{
				*pdwBinary = uBinary;
				pbBinary += 4;
				}
			} // if...else
		} // if
	Assert(pbBinary - prgbDataBinary < cbDecodeAllocDebug);
	return pbBinary - prgbDataBinary;
	} // Base85_CbDecodeToBinary()

BOOL
Base85_FDecodeToBinary_ML(IN PSZUC pszStringBase85, OUT_F_INV BYTE prgbDataBinary[], int cbDataBinary)
	{
	Assert(pszStringBase85 != NULL);
	Assert(prgbDataBinary != NULL);
	Assert(cbDataBinary > 0);
	if (Base85_FCanDecodeToBinary(pszStringBase85, cbDataBinary))
		{
		if (Base85_CbDecodeToBinary(IN pszStringBase85, OUT_F_INV prgbDataBinary) == (UINT)cbDataBinary)
			return TRUE;
		}
	if (*pszStringBase85 != '\0')
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The Base85 string '$s' cannot be decoded into a buffer of $i bytes\n", pszStringBase85, cbDataBinary);
	return FALSE;
	}

BOOL
Base85_FDecodeToBinary_ZZR_ML(IN PSZUC pszStringBase85, OUT_ZZR BYTE prgbDataBinary[], int cbDataBinary)
	{
	if (pszStringBase85 != NULL)
		{
		if (Base85_FDecodeToBinary_ML(IN pszStringBase85, OUT_F_INV prgbDataBinary, cbDataBinary))
			return TRUE;
		}
	InitToZeroes(OUT prgbDataBinary, cbDataBinary);	// Change from OUT_F_INV to OUT_ZZR
	return FALSE;
	}

#define d_tsYear2114		3153600000000
#define d_tsMax7Digits		32057708828124	// This timestamp represent the largest date which can fit in a 7 digit Base85 value "\\\\\\\".  Such a date is Nov 13, 2985 which is 971 years from now (2014).  Until then, we will assume all dates fit within 7 digits, so a 32 bit pointer serialized in hexadecimal is not mistaken with a date.  Of course, in 971 years, this value has to be change, however such max value will give us plenty of time for testing a 7 digit timestamp.


static TIMESTAMP s_tsInvalidLast;	// For debugging purpose
void
Timestamp_DebugValidate_ML(TIMESTAMP ts)
	{
	if (ts < d_tsYear2114)
		return;	// All timestamps prior year 2114 are considered valid
	if (ts == d_tsMax7Digits)
		return;	// This is an exception for debugging purpose
	if (ts == s_tsInvalidLast)
		return;
	s_tsInvalidLast = ts;	// Prevent a stack overflow because the routine formatting the error will also validate the timestamp
	QString sDateTime = QDateTime::fromMSecsSinceEpoch(ts).toString();
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The timestamp '$t' ($l) is out of range with value of $Q\n", ts, ts, &sDateTime);
	}

//	Encode a timestamp to a string.
//	Return the number of characters stored in pszTimestamp, excluding the null-terminator.
//	For optimization, the timestamps are encoded in Base85.
int
Timestamp_CchToString(TIMESTAMP ts, OUT CHU pszTimestamp[16])
	{
	Timestamp_DebugValidate_ML(ts);
	Assert(pszTimestamp != NULL);
	CHU * pchTimestamp = pszTimestamp;
	while (TRUE)
		{
		*pchTimestamp++ = c_rgchBase85[ts % 85];
		ts /= 85;
		if (ts <= 0)
			break;
		}
	const int cchTimestamp = pchTimestamp - pszTimestamp;
	*pchTimestamp-- = '\0';	// Append the null-terminator
	// We are done parsing, but the digits are in reverse order
	Assert(pszTimestamp <= pchTimestamp);
	for ( ; pszTimestamp < pchTimestamp; pszTimestamp++, pchTimestamp--)
		{
		// Swap the digits
		CHS chT = *pszTimestamp;
		*pszTimestamp = *pchTimestamp;
		*pchTimestamp = chT;
		}
	return cchTimestamp;
	} // Timestamp_CchToString()

//	Routine to parse a timestamp and return the pointer where the parsing stopped
PCHRO
Timestamp_PchFromString(OUT_UNTIL_STOP TIMESTAMP * pts, PSZUC pszTimestamp)
	{
	Assert(pts != NULL);
	Assert(pszTimestamp != NULL);
	TIMESTAMP ts = 0;	// Use a variable on the stack for performance
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase85[(BYTE)*pszTimestamp++];
		if (mvbMapValue < 85)
			ts = ts * 85 + mvbMapValue;
		else
			{
			Timestamp_DebugValidate_ML(ts);
			*pts = ts;
			return pszTimestamp - 1;	// Return the pointer where the parsing stopped (if successful, this points to a null-terminator)
			}
		} // while
	} // Timestamp_PchFromString()

//	Same as above, except an error is displayed to the Message Log
TIMESTAMP
Timestamp_FromString_ML(PSZUC pszTimestamp)
	{
	#ifdef DEBUG_WANT_ASSERT
	TIMESTAMP tsDebug;
	Timestamp_PchFromString(OUT_UNTIL_STOP &tsDebug, pszTimestamp);	// Use the original routine to make sure the returned result is the same
	#endif
	PSZUC pszTimestampBegin = pszTimestamp;

	TIMESTAMP ts = 0;
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase85[(BYTE)*pszTimestamp++];
		if (mvbMapValue < 85)
			ts = ts * 85 + mvbMapValue;
		else
			{
			Assert(tsDebug == ts);
			Timestamp_DebugValidate_ML(ts);
			if (pszTimestamp[-1] != '\0')
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Invalid Timestamp $s\n", pszTimestampBegin);
			return ts;	// Return what we have so far
			}
		}
	}

//	Similar routine, however no error is reported, and zero is returned if there is any kind of error and/or overflow.
TIMESTAMP
Timestamp_FromString_ZZR(PSZUC pszTimestamp)
	{
	PSZUC pszTimestampBegin = pszTimestamp;
	TIMESTAMP ts = 0;
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase85[(BYTE)*pszTimestamp++];
		if (mvbMapValue < 85)
			ts = ts * 85 + mvbMapValue;
		else
			{
			// To be valid, the timestamp should be fully parsed, and no more than 7 characters
			if (pszTimestamp[-1] == '\0' && (pszTimestamp - pszTimestampBegin) <= 8)
				{
				Timestamp_DebugValidate_ML(ts);	// The timestamp should be valid.  If not, just report it to the Message Log
				return ts;
				}
			//TRACE1("The timestamp '$s' does not meet the criteria of a valid timestamp", pszTimestampBegin);
			return 0;
			}
		}
	} // Timestamp_FromString_ZZR()

//	Encode a timestamp in Base64Url.  The motivation for this routine is having a uniform URL encoding, as encoding in Base85 will lead to % encoding of the URL, thus hiding potential bugs when parsing.
//	The implementation of this routine was a copy & paste from Timestamp_CchToString() and a replace of 85 by 64.
int
Timestamp_CchEncodeToBase64Url(TIMESTAMP ts, OUT CHU pszTimestamp[16])
	{
	Timestamp_DebugValidate_ML(ts);
	Assert(pszTimestamp != NULL);
	CHU * pchTimestamp = pszTimestamp;
	while (TRUE)
		{
		*pchTimestamp++ = c_rgchBase64Url[ts % 64];
		ts /= 64;
		if (ts <= 0)
			break;
		}
	const int cchTimestamp = pchTimestamp - pszTimestamp;
	*pchTimestamp-- = '\0';	// Append the null-terminator
	// We are done parsing, but the digits are in reverse order
	Assert(pszTimestamp <= pchTimestamp);
	for ( ; pszTimestamp < pchTimestamp; pszTimestamp++, pchTimestamp--)
		{
		// Swap the digits
		CHS chT = *pszTimestamp;
		*pszTimestamp = *pchTimestamp;
		*pchTimestamp = chT;
		}
	return cchTimestamp;
	}

PCHRO
Timestamp_PchDecodeFromBase64Url(OUT_ZZR TIMESTAMP * pts, PSZUC pszTimestamp)
	{
	Assert(pts != NULL);
	Assert(pszTimestamp != NULL);
	TIMESTAMP ts = 0;	// Use a variable on the stack for performance
	while (TRUE)
		{
		const BYTE mvbMapValue = c_mapbbBase64andBase64Url[(BYTE)*pszTimestamp++];
		if (mvbMapValue < 64)
			ts = ts * 64 + mvbMapValue;
		else
			{
			*pts = ts;
			return pszTimestamp - 1;	// Return the pointer of the last character parsed
			}
		} // while
	}



int
TimestampDelta_CchToString(TIMESTAMP_DELTA dts, OUT CHU pszTimestampDelta[32])
	{
	InitToGarbage(OUT pszTimestampDelta, 32);
	CHU * pchTimestampDelta = pszTimestampDelta;
	if (dts < 0)
		{
		dts = -dts;
		*pchTimestampDelta++ = '-';
		}
	if (dts < d_ts_cMinutes)
		{
		// Display the timestamp in seconds
		pchTimestampDelta = PchNumberToString64_Decimal0or1(OUT pchTimestampDelta, dts, d_ts_cSeconds);
		*pchTimestampDelta++ = 's';
		}
	else if (dts < d_ts_cHours)
		{
		// Display the timestamp in minutes
		pchTimestampDelta = PchNumberToString64_Decimal0or1(OUT pchTimestampDelta, dts, d_ts_cMinutes);
		*pchTimestampDelta++ = 'm';
		}
	else if (dts < d_ts_cDays)
		{
		// Display the timestamp in hours
		pchTimestampDelta = PchNumberToString64_Decimal0or1(OUT pchTimestampDelta, dts, d_ts_cHours);
		*pchTimestampDelta++ = 'h';
		}
	else
		{
		pchTimestampDelta = PchNumberToString64_Decimal0or1(OUT pchTimestampDelta, dts, d_ts_cDays);
		*pchTimestampDelta++ = 'd';
		}
	*pchTimestampDelta = '\0';
	return (pchTimestampDelta - pszTimestampDelta);
	}

int
TimestampDelta_CchToStringLongText(TIMESTAMP_DELTA dts, OUT CHU pszTimestampDelta[64])
	{
	InitToGarbage(OUT pszTimestampDelta, 64);
	PSZAC pszLongText = c_szaEmpty;
	int cchTimestampDelta = TimestampDelta_CchToString(dts, OUT pszTimestampDelta);
	CHU * pchTimestampDelta = pszTimestampDelta + cchTimestampDelta - 1;
	switch (*pchTimestampDelta)
		{
	case 's':
		pszLongText = "seconds";
		break;
	case 'm':
		pszLongText = "minutes";
		break;
	case 'h':
		pszLongText = "hours";
		break;
	case 'd':
		pszLongText = "days";
		break;
		}
	Assert(pszLongText[0] != '\0');
	*pchTimestampDelta++ = ' ';
	while (TRUE)
		{
		CHS ch = *pszLongText++;
		*pchTimestampDelta++ = ch;
		if (ch == '\0')
			break;
		}
	return pchTimestampDelta - pszTimestampDelta - 1;
	} // TimestampDelta_CchToStringLongText()


//	Encode 4 bytes of binary data into 6 bytes of base41 characters.
//	This function is a simplified version of Base85_CchEncodeToText() because the binary data must be 32-bit aligned.
//	Also there is no 'decoding' of Base41, as this function is to create unique filenames from a unique hash value.
//	The motivation of using Base41 instead of Base16 is reducing the length of the filenames as well as ensuring all filesnames have lowercase characters (under some file systems the filenames are case sensitive).
//	Also, the array c_rgchBase85[] was carefully crafted to have its 41st characters to not include any reserved characters such *?:/\<>.|%
PSZU
Base41_EncodeToText(OUT PSZU pszStringBase41, const BYTE prgbDataBinary[], int cbDataBinary)
	{
	Assert(IS_ALIGNED_32(cbDataBinary));
	const UINT * puBinary = (const UINT *)prgbDataBinary;
	const UINT * puBinaryStop = (const UINT *)(prgbDataBinary + cbDataBinary);
	while (puBinary != puBinaryStop)
		{
		UINT uBinary = *puBinary++;
		PSZU pszStringBase41Stop = pszStringBase41 + 6;
		while (pszStringBase41 != pszStringBase41Stop)
			{
			*pszStringBase41++ = c_rgchBase85[uBinary % 41];
			uBinary /= 41;
			}
		}
	*pszStringBase41 = '\0';
	return pszStringBase41;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Return how many base64 characters would be encoded given a number of bytes of binary data.
//	This function is very useful to pre-allocate memory before encoding binary data into base64
//	The return value does NOT include the null-terminator, so the caller is responsible of allocating an extra byte before calling the function Base64_CchEncodeToText().
UINT
Base64_CchEncoded(int cbDataBinary)
	{
	return ((cbDataBinary + 2) / 3 * 4);
	}

//	Encode binary data into base64
//	Return the number of characters (excluding the null-terminator) stored in pszStringBase64.
UINT
Base64_CchEncodeToText(OUT PSZU pszStringBase64, const BYTE prgbData[], int cbData)
	{
	PSZU pszStringBase64Begin = pszStringBase64;
	int cTriples = cbData / 3;
	int cbTriples = cTriples * 3;
	int cbRemain = cbData - cbTriples;
	const BYTE * pbDataStop = prgbData + cbTriples;
	while (prgbData != pbDataStop)
		{
		UINT ch1 = prgbData[0];
		UINT ch2 = prgbData[1];
		UINT ch3 = prgbData[2];
		prgbData += 3;

		pszStringBase64[0] = c_rgchBase64[(ch1 >> 2) & 0x3F];
		pszStringBase64[1] = c_rgchBase64[(((ch1 &  3) << 4) + (ch2 >> 4)) & 0x3F];
		pszStringBase64[2] = c_rgchBase64[(((ch2 & 15) << 2) + (ch3 >> 6)) & 0x3F];
		pszStringBase64[3] = c_rgchBase64[ch3 & 0x3F];
		pszStringBase64 += 4;
		} // while
	if (cbRemain > 0)
		{
		Assert(cbRemain == 1 || cbRemain == 2);
		UINT ch1 = prgbData[0];
		UINT ch2 = (cbRemain == 1) ? 0 : prgbData[1];
		pszStringBase64[0] = c_rgchBase64[(ch1 >> 2) & 0x3F];
		pszStringBase64[1] = c_rgchBase64[(((ch1 & 3) << 4) + (ch2 >> 4)) & 0x3F];
		pszStringBase64[2] = (cbRemain == 1) ? '=' : c_rgchBase64[((ch2 & 15) << 2) & 0x3F];
		pszStringBase64[3] = '=';
		pszStringBase64 += 4;
		}
	*pszStringBase64 = '\0';
	Assert((UINT)(pszStringBase64 - pszStringBase64Begin) == Base64_CchEncoded(cbData));
	return (pszStringBase64 - pszStringBase64Begin);
	} // Base64_CchEncodeToText()

/*	Those methods are no longer used
//	Calculate the MD5 from the content of the string
void
CStr::TransformContentToMd5Hex()
	{
	QByteArray arraybData(PszaGetUtf8NZ());
	QByteArray arraybHash = QCryptographicHash::hash(arraybData, QCryptographicHash::Md5);
	*this = arraybHash.toHex();
	}

void
CStr::TransformContentToMd5Hex(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	Format_VL(pszFmtTemplate, vlArgs);
	TransformContentToMd5Hex();
	}
*/

void
CBin::BinInitFromCalculatingHashSha1(const CBin & binData)
	{
	SHashSha1 * pHashSha1 = (SHashSha1 *)PvSizeInit(sizeof(SHashSha1));
	HashSha1_CalculateFromBinary(OUT pHashSha1, IN binData.PvGetData(), binData.CbGetData());
	Assert(CbGetData() == sizeof(SHashSha1));
	}

void
CBin::BinHashToMd5()
	{
	SHashMd5 hashMd5;
	HashMd5_CalculateFromCBin(OUT &hashMd5, *this);
	BinInitFromBinaryData(IN &hashMd5, sizeof(hashMd5));
	}

//	PSZAC pszaChallengeDecoded = "realm="jabber.cambrian.org",nonce="bd9250a14d354f5c89ccc4fdd50120ab",qop="auth",charset=utf-8,algorithm=md5-sess";
//	PSZUC pszuNonce = strNonce.InitFromValueOfCommaSeparatedStringPsz(IN pszaChallengeDecoded, "nonce");	// Return "bd9250a14d354f5c89ccc4fdd50120ab" (without the quotes)
PSZUC
CStr::InitFromValueOfCommaSeparatedStringPsz(PSZUC pszuCommaSeparatedString, PSZAC pszSearchToken)
	{
	Assert(pszSearchToken != NULL);
	Assert(pszSearchToken[0] != '\0');
	Assert(pszSearchToken[0] != ' ');	// Spaces should not be part of the token

	if (m_paData != NULL)
		m_paData->cbData = 0;
	if (pszuCommaSeparatedString != NULL)
		{
		const UINT chSearchTokenFirst = (BYTE)*pszSearchToken++;
		if (chSearchTokenFirst != '\0')
			{
			while (TRUE)
				{
				const UINT chString = *pszuCommaSeparatedString++;
				if (Ch_FIsWhiteSpace(chString))
					continue;	// Skip any leading whitespaces
				if (chString == chSearchTokenFirst)
					{
					// The first character is matching, so check if all other characters are matching
					PSZUC pchSearchToken = (PSZUC)pszSearchToken;
					while (TRUE)
						{
						const UINT ch = *pchSearchToken++;
						if (ch == '\0')
							{
							// We found a match
							if (*pszuCommaSeparatedString++ != '=')
								return NULL;		// The syntax is the token must be followed by a value (example: "username=joesmith")
							// Now, extract the value
							const UINT chValue = *pszuCommaSeparatedString;
							if (chValue == '\"' || chValue == '\'')
								{
								// We have a quoted string, so find the other matching
								PSZUC pchMatchingQuote = ++pszuCommaSeparatedString;
								while (TRUE)
									{
									const UINT chMatchingQuote = *pchMatchingQuote++;
									if (chMatchingQuote == chValue)
										{
										BinInitFromBinaryData(IN pszuCommaSeparatedString, pchMatchingQuote - pszuCommaSeparatedString - 1);
										break;
										}
									if (chMatchingQuote == '\0')
										{
										Assert(FALSE && "Unexpected end of string while looking for matching closing quote");
										return NULL;
										}
									}
								} // if (quote)
							else
								{
								// Search until the comma
								PSZUC pchComma = pszuCommaSeparatedString;
								while (TRUE)
									{
									const UINT chComma = *pchComma++;
									if (chComma == ',' || chComma == '\0')
										{
										BinInitFromBinaryData(IN pszuCommaSeparatedString, pchComma - pszuCommaSeparatedString - 1);
										break;
										}
									} // while
								}
							BinAppendNullTerminator();
							return m_paData->rgbData;
							}
						if (ch != *pszuCommaSeparatedString++)
							break;
						} // while
					} // if

				// Skip the remaining of the string
				while (TRUE)
					{
					const UINT chString = *pszuCommaSeparatedString++;
					if (chString == '\0')
						return NULL;
					if (chString == '=')
						{
						// Skip the value
						UINT chValue = *pszuCommaSeparatedString++;
						if (chValue == '\"' || chValue == '\'')
							{
							while (TRUE)
								{
								const UINT ch = *pszuCommaSeparatedString++;
								if (ch == chValue)
									{
									// We have found our closing quote
									while (Ch_FIsWhiteSpace(*pszuCommaSeparatedString))
										pszuCommaSeparatedString++;
									if (*pszuCommaSeparatedString == ',')
										pszuCommaSeparatedString++;	// Skip the comma
									break;
									}
								if (ch == '\0')
									{
									Assert(FALSE && "Unexpected end of string while looking for matching closing quote");
									return NULL;
									}
								} // while
							}
						else
							{
							// Find the next comma
							while (TRUE)
								{
								if (chValue == ',')
									break;
								if (chValue == '\0')
									return NULL;
								chValue = *pszuCommaSeparatedString++;
								} // while
							} // if...else
						break;
						} // if
					} // if
				} // while
			} //if
		} // if
	return NULL;
	} // InitFromValueOfCommaSeparatedStringPsz()

//	Make sure the content of the string contains lowercase characters.
//	This method does preserves accents. For instance, the name "Hélène" would become "hélène".
void
CStr::TransformContentToLowercase()
	{
	if (m_paData == NULL)
		return;
	CString s = *this;
	InitFromStringQ(s.toLower());
	}

//	Transform the content of the string into lowercase for searching purpose.
//	When searching, accents are stripped out.  For instance, the name "Hélène" would become "helene".
void
CStr::TransformContentToLowercaseSearch()
	{
	TransformContentToLowercase();
	}

//	Transform the content of the string into lowercase for sorting purpose.
//	When sorting strings, the accents may remain, however with a different priority.
//	Depending on the locale, the same letter may have a different position in the sorting dictionary.
//	This method is a one-way trip: the content of the string is a null-terminated string, however may not contain any of its orignal letters
//	as each character of the original string may be expanded into several bytes representing the sorting indices.
void
CStr::TransformContentToLowercaseSort()
	{
	TransformContentToLowercase();
	}


//	Return the number of bytes (including the null-terminator) of the truncated string
int
CbTrimTailingWhiteSpaces(INOUT PSZU pszuString)
	{
	Assert(pszuString != NULL);

	// Find the end of the string
	CHU * pch = pszuString;
	while (*pch++ != '\0')
		;
	pch -= 2;
	while (pch > pszuString)
		{
		switch (*pch)
			{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			*pch-- = '\0';
			continue;
		default:
			goto Done;
			}
		} // while
	Done:
	return (pch - pszuString + 2);
	} // CbTrimTailingWhiteSpaces()

//	Replace all instances of a character by another character.
//	This method may be useful to remove unsafe characters from the unicode string.
void
StringReplaceAnyCharacterByCharacter(INOUT PSZU pszuString, CHS chFind, CHS chReplace)
	{
	Assert(pszuString != NULL);
	while (TRUE)
		{
		CHS ch = *pszuString++;
		if (ch == '\0')
			break;
		if (ch == chFind)
			pszuString[-1] = chReplace;
		}
	}


PSZUC
CStr::TrimTailingWhiteSpacesNZ()
	{
	if (m_paData != NULL && m_paData->cbData > 0)
		{
		m_paData->cbData = CbTrimTailingWhiteSpaces(INOUT m_paData->rgbData);
		AssertValidStr(*this);
		return m_paData->rgbData;
		}
	return c_szuEmpty;
	}

void
CStr::TrimLeadingAndTailingWhiteSpaces()
	{
	// Need to implement trimming the leading white spaces
	TrimTailingWhiteSpacesNZ();
	AssertValidStr(*this);
	}

void
CStr::StringTruncateAt(PSZU pchDataEnd)
	{
	Assert(FIsPointerAddressWithinBinaryObject(pchDataEnd));
	Assert(m_paData != NULL);
	*pchDataEnd = '\0';
	m_paData->cbData = pchDataEnd - m_paData->rgbData + 1;
	Assert(m_paData->cbData > 0 && m_paData->cbData <= m_paData->cbAlloc);
	}

void
CStr::StringTruncateAtCharacter(UINT chTruncate)
	{
	if (m_paData != NULL)
		{
		CHU * pch = m_paData->rgbData;
		CHU * pchStop = pch + m_paData->cbData;
		while (pch != pchStop)
			{
			if (*pch == chTruncate)
				{
				StringTruncateAt(pch);
				return;
				}
			pch++;
			}
		}
	}

//	Replace the content of the string only if empty
void
CStr::InitOnlyIfEmpty(const CStr & strString)
	{
	if (PszuGetStringNullIfEmpty() == NULL)
		BinInitFromCBin(IN &strString);
	}

//	Create a mapping for every letter of the alphabet, since most Top-Level Domains are within this range
const PSZAC c_rgpszmTLDs[26] =
	{
	"cdefgilmnoqrstuwz",			// TLDs starting with 'a': ac, ad, ae, af, ag, ai, al, am, an, ao, aq, ar, as, at, au, aw, az
	"abdefghijmnorstvwyz",			// TLDs starting with 'b': ba, bb, bd, be, bf, bg, bh, bi, bj, bm, bn, bo, br, bs, bt, bv, bw, by, bz
	"acfghiklmnorsuvxyz,om,oop",	// TLDs starting with 'c': ca, cc, cf, cg, ch, ci, ck, cl, cm, cn, co, cr, cs, cu, cv, cx cy, cz, com, coop
	"ejkmoz",						// TLDs starting with 'd': de, dj, dk, dm, do, dz
	"ceghrst,du",					// TLDs starting with 'e': ec, ee, eg, eh, er, es, et, edu
	"ijkmorx",						// TLDs starting with 'f': fi, fj, fk, fm, fo, fr, fx
	"abdefhilmnpqrstuwy,ov",		// TLDs starting with 'g': ga, gb, gd, ge, gf, gh, gi, gl, gm, gn, gp, gq, gr, gs, gt, gu, gw, gy, gov
	"kmnrtu",						// TLDs starting with 'h': hk, hm, hn, hr, ht, hu
	"delnoqrst,nfo",				// TLDs starting with 'i': id, ie, il, in, io, iq, ir, is, it, info
	"mop",							// TLDs starting with 'j': jm, jo, jp
	"eghimnprwyz",					// TLDs starting with 'k': ke, kg, kh, ki, km, kn, kp, kr, kw, ky, kz
	"abcikrstuvy",					// TLDs starting with 'l': la, lb, lc, li, lk, lr, ls, lt, lu, lv, ly
	"acdghklmnopqrstuvwxyz,il",		// TLDs starting with 'm': ma, mc, md, mg, mh, mk, ml, mm, mn, mo, mp, mq, mr, ms, mt, mu, mv, mw, mx, my, mz, mil
	"acefgiloprtuz,et",				// TLDs starting with 'n': na, nc, ne, nf, ng, ni, nl, no, np, nr, nt, nu, nz, net
	"m,rg",							// TLDs starting with 'o': om, org
	"aefghklmnrtwy",				// TLDs starting with 'p': pa, pe, pf, pg, ph, pk, pl, pm, pn, pr, pt, pw, py
	"a",							// TLDs starting with 'q': qa
	"eouw",							// TLDs starting with 'r': re, ro, ru, rw
	"abcdeghijklmnortuvyz",			// TLDs starting with 's': sa, sb, sc, sd, se, sg, sh, si, sj, sk, sl, sm, sn, so, sr, st, su, sv, sy, sz
	"cdfghjkmnoprtvwz",				// TLDs starting with 't': tc, td, tf, tg, th, tj, tk, tm, tn, to, tp, tr, tt, tv, tw, tz
	"agkmsyz",						// TLDs starting with 'u': ua, ug, uk, um, us, uy, uz
	"aceginu",						// TLDs starting with 'v': va, vc, ve, vg, vi, vn, vu
	"fs",							// TLDs starting with 'w': wf, ws
	c_szaEmpty,						// TLDs starting with 'x':
	"etu",							// TLDs starting with 'y': ye, yt, yu
	"amrw"							// TLDs starting with 'z': za, zm, zr, zw
	}; // c_rgpszmTLDs[]

//	Return NULL if the text does NOT contain Top-Level Domain name.
//	Return pointer to the following character after the Top-Level Domain name.
PSZR
PszrFindDomainExtension(PSZUC pszText)
	{
	Assert(pszText != NULL);
	CHS chTLD = Ch_GetCharLowercase(pszText[0]);
	int iTLD = chTLD - 'a';
	if (iTLD >= 0 && iTLD < (int)LENGTH(c_rgpszmTLDs))
		{
		PSZAC pszTLDs = c_rgpszmTLDs[iTLD];
		// Search for a matching TLD
		chTLD = Ch_GetCharLowercase(pszText[1]);
		if (chTLD < 'a' || chTLD > 'z')
			return NULL;
		PSZUC pszText2 = pszText + 2;
		if (Ch_FIsPunctuationOrWhiteSpaceOrNullTerminator(pszText2[0]))
			{
			// We have a two-letter TLD, which is typically a country code
			while (TRUE)
				{
				CHS ch = *pszTLDs++;
				if (ch == chTLD)
					return pszText2;
				if (ch == ',' || ch == '\0')
					return NULL;
				} // while
			}
		// We have a longer TLD, which may be a .com, .org, .info or something else
		while (TRUE)
			{
			CHS ch = *pszTLDs++;
			if (ch == ',')
				break;		// Skip until we find the longer TLDs (after the two-letter TLDs)
			if (ch == '\0')
				return NULL;
			} // while
		while (TRUE)
			{
			CHS ch = *pszTLDs++;
			Assert(ch != ',');
			if (ch == chTLD)
				{
				// Compare the rest of the string
				PSZUC pszTextCompare = pszText2;
				while (TRUE)
					{
					ch = *pszTLDs++;
					if ((ch == ',' || ch == '\0') && Ch_FIsPunctuationOrWhiteSpaceOrNullTerminator(*pszTextCompare))
						return pszTextCompare;
					if (ch != *pszTextCompare++)
						break;
					} // while
				} // if
			// Otherwise skip to the next TLD
			while (TRUE)
				{
				ch = *pszTLDs++;
				if (ch == ',')
					break;
				if (ch == '\0')
					return NULL;
				} //while
			} // while
		} // if
	return NULL;
	} // PszrFindDomainExtension()

EScheme
EGetScheme(PSZUC pszUrl)
	{
	Assert(pszUrl != NULL);
	EScheme eScheme = eSchemeUnknown;
	CHS ch = Ch_GetCharLowercase(pszUrl[0]);
	if (ch == 'h')
		{
		if (Ch_GetCharLowercase(pszUrl[1] == 't') && Ch_GetCharLowercase(pszUrl[2] == 't') && Ch_GetCharLowercase(pszUrl[3] == 'p'))
			{
			pszUrl += 4;
			if (Ch_GetCharLowercase(pszUrl[0]) == 's' && pszUrl[1] == ':')
				return eSchemeHttps;
			eScheme = eSchemeHttp;
			}
		}
	if (*pszUrl == ':')
		return eScheme;
	return eSchemeUnknown;
	}


#ifdef DEBUG
void
_CompareUnicodeUTF8(IN PSZWC pszwUnicode, IN const char * pszuUtf8)
	{
	CHW szwUnicode[100];
	const CHW * pszwUnicodeError;
	int cchUnicode;
	CHU szuUtf8[100];
	const CHU * pszuUtf8Error;
	int cchUtf8;

	InitToGarbage(OUT szuUtf8, sizeof(szuUtf8));
	cchUtf8 = EncodeUnicodeToUtf8(OUT szuUtf8, IN pszwUnicode, OUT &pszwUnicodeError);
	Assert(pszwUnicodeError == NULL);	// No error
	Assert(strcmp((char *)szuUtf8, pszuUtf8) == 0);

	InitToGarbage(OUT szwUnicode, sizeof(szwUnicode));
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN (PSZUC)pszuUtf8, OUT &pszuUtf8Error);
	Assert(pszuUtf8Error == NULL);
	Assert(memcmp(szwUnicode, pszwUnicode, (cchUnicode + 1) * sizeof(CHW)) == 0);

	// Make the round trip by converting the converted U
	CHU szuUtf8RoundTrip[100];
	int cchUtf8RoundTrip = EncodeUnicodeToUtf8(OUT szuUtf8RoundTrip, IN szwUnicode, OUT &pszwUnicodeError);
	Assert(pszwUnicodeError == NULL);
	Assert(cchUtf8RoundTrip == cchUtf8);
	Assert(strcmp((char *)szuUtf8RoundTrip, (char *)szuUtf8) == 0);
	}

void
TEST_UnicodeUTF()
	{
	CHW szwUnicode[16];
	const CHW * pszwUnicodeError;
	int cchUnicode;
	CHU szuUtf8[16];
	const CHU * pszuUtf8Error;
	int cchUtf8;

	for (cchUnicode = 0; cchUnicode <= 0xFFFF; cchUnicode++)
		{
		szwUnicode[0] = cchUnicode;
		szwUnicode[1] = '\0';

		InitToGarbage(OUT szuUtf8, sizeof(szuUtf8));
		EncodeUnicodeToUtf8(OUT szuUtf8, IN szwUnicode, OUT &pszwUnicodeError);
		Assert(strlenU(szuUtf8) <= 4);
		if (FIsUnicodeCharSurrogateLead(cchUnicode))
			{
			Assert(pszwUnicodeError != NULL);
			Assert(*pszwUnicodeError == '\0');	// The next character is a null-terminator
			}
		else if (cchUnicode >= d_chwUnicodeSurrogateFirst && cchUnicode <= d_chwUnicodeSurrogateLast)
			{
			Assert(pszwUnicodeError != NULL);
			Assert(*pszwUnicodeError == cchUnicode);
			}
		else
			{
			Assert(pszwUnicodeError == NULL);
			}

		InitToGarbage(szwUnicode, sizeof(szwUnicode));
		DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8);
		Assert(szwUnicode[0] == cchUnicode);
		Assert(cchUnicode == 0 || szwUnicode[1] == '\0');

		CHU szuTemp[8];
		InitToGarbage(OUT szuTemp, sizeof(szuTemp));
		EncodeUtf32toUtf8(cchUnicode, OUT szuTemp);
		Assert(strcmpU(szuTemp, szuUtf8) == 0);
		} // for

	// Build a unicode string with illegal surrogates
	szwUnicode[0] = 'a';
	szwUnicode[1] = d_chwUnicodeSurrogateLeadFirst;
		szwUnicode[2] = 'b';
	szwUnicode[3] = 'c';
	szwUnicode[4] = 0x0000;
	cchUtf8 = EncodeUnicodeToUtf8(OUT szuUtf8, IN szwUnicode, OUT &pszwUnicodeError);
	Assert(pszwUnicodeError != NULL);	// The error must point to the illegal surrogate
	Assert(*pszwUnicodeError == 'b');
	Assert(szuUtf8[0] == 'a');
	Assert(szuUtf8[1] == 0xED);		// The routine encodes the lead-surrogate as a normal Unicode character,  however having pszwUnicodeError pointing tho the faulty character
	Assert(szuUtf8[2] == 0xA0);
	Assert(szuUtf8[3] == 0x80);
	Assert(szuUtf8[4] == 'b');
	Assert(szuUtf8[5] == 'c');
	Assert(szuUtf8[6] == '\0');		// The routine must ignore the whole lead-surrogate pair

	// Build a unicode string with a surrogate without a lead
	szwUnicode[0] = 'a';
	szwUnicode[1] = d_chwUnicodeSurrogateFirst;
	szwUnicode[2] = 'b';
	szwUnicode[3] = d_chwUnicodeSurrogateLast;
	szwUnicode[4] = 0x0000;
	cchUtf8 = EncodeUnicodeToUtf8(OUT szuUtf8, IN szwUnicode, OUT &pszwUnicodeError);
	Assert(pszwUnicodeError != NULL);	// The error must point to the illegal surrogate
	Assert(*pszwUnicodeError == d_chwUnicodeSurrogateFirst);
	Assert(szuUtf8[0] == 'a');
	Assert(szuUtf8[1] == 0xED);		// The routine encodes the surrogate as a normal Unicode character, however having pszwUnicodeError pointing tho the faulty character
	Assert(szuUtf8[2] == 0xB0);
	Assert(szuUtf8[3] == 0x80);
	Assert(szuUtf8[4] == 'b');
	Assert(szuUtf8[5] == 0xED);
	Assert(szuUtf8[6] == 0xBF);
	Assert(szuUtf8[7] == 0xBF);
	Assert(szuUtf8[8] == '\0');		// The routine must ignore the surrogate character

	// Build a UTF-8 string with illegal sequences
	szuUtf8[0] = 0xFF;	// This is an illegal UTF-8 character, since any lead must be one of the following pattern: 110vvvvv, 1110vvvv, 11110vvv
	szuUtf8[1] = 0x20;
	szuUtf8[2] = 0x00;
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(pszuUtf8Error != NULL);			// There must be an error
	Assert(*pszuUtf8Error == (BYTE)0xFF);	// The error must point to the beginning of the illegal sequence
	Assert(memcmp(szwUnicode, L"\xFF ", (cchUnicode + 1) * sizeof(CHW)) == 0);


	szuUtf8[0] = 0xF0;	// Mark the beginning of a sequence of 4 bytes
	// This sequence will be illegal because the second byte 0x20 does not start with pattern 10vvvvvv
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(pszuUtf8Error != NULL);			// There must be an error
	Assert(*pszuUtf8Error == (BYTE)0xF0);	// The error must point to the beginning of the illegal sequence
	Assert(memcmp(szwUnicode, L"\xF0 ", (cchUnicode + 1) * sizeof(CHW)) == 0);

	szuUtf8[1] = 0x9F;	// Make the second byte have the pattern 10011111
	// The error will occur because the sequence must be 4 bytes, and there is only 2 bytes
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(pszuUtf8Error != NULL);			// There must be an error
	Assert(*pszuUtf8Error == (BYTE)0xF0);	// The error must point to the beginning of the illegal sequence
	Assert(szwUnicode[0] == 0x00F0);
	Assert(szwUnicode[1] == 0x009F);
	Assert(szwUnicode[2] == 0x0000);

	szuUtf8[2] = 0x80;
	szuUtf8[3] = 0x00;
	// The error will occur because the sequence must be 4 bytes, and there is only 3 bytes
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(cchUnicode == 3);
	Assert(pszuUtf8Error != NULL);			// There must be an error
	Assert(*pszuUtf8Error == (BYTE)0xF0);	// The error must point to the beginning of the illegal sequence
	Assert(szwUnicode[0] == 0x00F0);
	Assert(szwUnicode[1] == 0x009F);
	Assert(szwUnicode[2] == 0x0080);
	Assert(szwUnicode[3] == 0x0000);

	// This time, there should be no error
	szuUtf8[3] = 0x80 | 0x3F;
	szuUtf8[4] = 0x00;
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(cchUnicode == 2);	// The four UTF-8 bytes have been combined into a single unicode symbol represented by two wide characters
	Assert(pszuUtf8Error == NULL);

	szuUtf8[0] = 0xF1;	// Another valid sequence
	szuUtf8[4] = 'W';
	szuUtf8[5] = 0x00;
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(cchUnicode == 3);	// Same as above, but one more character to the string
	Assert(pszuUtf8Error == NULL);
	Assert(szwUnicode[2] == 'W');
	Assert(szwUnicode[3] == 0x0000);

	// Now, create an illegal sequence with a lead character having bit pattern 11111000
	szuUtf8[0] = 0xF8;
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(cchUnicode == 5);	// The whole UTF-8 sequence is interpreted as Ansi characters
	Assert(pszuUtf8Error != NULL);
	Assert(*pszuUtf8Error == 0xF8);
	Assert(szwUnicode[0] == 0x00F8);
	Assert(szwUnicode[1] == 0x009F);
	Assert(szwUnicode[2] == 0x0080);
	Assert(szwUnicode[3] == 0x00BF);
	Assert(szwUnicode[4] == 'W');
	Assert(szwUnicode[5] == 0x0000);

	// Create an illegal lead character having a bit pattern 10000010.  The lead character must have pattern 11xxxvvv
	szuUtf8[0] = 0x82;
	cchUnicode = DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(cchUnicode == 5);	// The whole UTF-8 sequence is interpreted as Ansi characters
	Assert(pszuUtf8Error != NULL);
	Assert(*pszuUtf8Error == (BYTE)0x82);
	Assert(szwUnicode[0] == 0x0082);
	Assert(szwUnicode[1] == 0x009F);
	Assert(szwUnicode[2] == 0x0080);
	Assert(szwUnicode[3] == 0x00BF);
	Assert(szwUnicode[4] == 'W');
	Assert(szwUnicode[5] == 0x0000);

	// More tests from http://www.unicode.org/versions/Unicode4.0.0/ch03.pdf (page 23)
	//	Table 3-3. Examples of Unicode Encoding Forms
	//  Code Point		UTF-32		UTF-16		UTF-8
	//	U+004D			0000004D	004D		4D
	//	U+0430			00000430	0430		D0 B0
	//	U+4E8C			00004E8C	4E8C		E4 BA 8C
	//	U+10302			00010302	D800 DF02	F0 90 8C 82  (surrograte)


	szwUnicode[0] = 0x0430;
	szwUnicode[1] = 0x0000;
	cchUtf8 = EncodeUnicodeToUtf8(OUT szuUtf8, IN szwUnicode, OUT &pszwUnicodeError);
	Assert(cchUtf8 == 2);
	Assert(pszwUnicodeError == NULL);
	Assert(szuUtf8[0] == 0xD0);
	Assert(szuUtf8[1] == 0xB0);
	Assert(szuUtf8[2] == 0x00);
	InitToGarbage(OUT szwUnicode, sizeof(szwUnicode));
	DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(szwUnicode[0] == 0x0430);
	Assert(szwUnicode[1] == 0x0000);

	szwUnicode[0] = 0x4E8C;
	szwUnicode[1] = 0x0000;
	cchUtf8 = EncodeUnicodeToUtf8(OUT szuUtf8, IN szwUnicode, OUT &pszwUnicodeError);
	Assert(cchUtf8 == 3);
	Assert(pszwUnicodeError == NULL);
	Assert(szuUtf8[0] == 0xE4);
	Assert(szuUtf8[1] == 0xBA);
	Assert(szuUtf8[2] == 0x8C);
	Assert(szuUtf8[3] == 0x00);
	InitToGarbage(OUT szwUnicode, sizeof(szwUnicode));
	DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(szwUnicode[0] == 0x4E8C);
	Assert(szwUnicode[1] == 0x0000);


	szwUnicode[0] = 0xD800;
	szwUnicode[1] = 0xDF02;
	szwUnicode[2] = 0x0000;
	cchUtf8 = EncodeUnicodeToUtf8(OUT szuUtf8, IN szwUnicode, OUT &pszwUnicodeError);
	Assert(cchUtf8 == 4);
	Assert(pszwUnicodeError == NULL);
	Assert(szuUtf8[0] == 0xF0);
	Assert(szuUtf8[1] == 0x90);
	Assert(szuUtf8[2] == 0x8C);
	Assert(szuUtf8[3] == 0x82);
	Assert(szuUtf8[4] == 0x00);
	InitToGarbage(OUT szwUnicode, sizeof(szwUnicode));
	DecodeUtf8ToUnicode(OUT szwUnicode, IN szuUtf8, OUT &pszuUtf8Error);
	Assert(szwUnicode[0] == 0xD800);
	Assert(szwUnicode[1] == 0xDF02);
	Assert(szwUnicode[2] == 0x0000);

	InitToGarbage(OUT szwUnicode, sizeof(szwUnicode));
	EncodeUtf32toUnicode(0x00010302, OUT szwUnicode);
	Assert(szwUnicode[0] == 0xD800);
	Assert(szwUnicode[1] == 0xDF02);
	Assert(szwUnicode[2] == 0x0000);

	InitToGarbage(OUT szuUtf8, sizeof(szuUtf8));
	EncodeUtf32toUtf8(0x00010302, OUT szuUtf8);
	Assert(szuUtf8[0] == 0xF0);
	Assert(szuUtf8[1] == 0x90);
	Assert(szuUtf8[2] == 0x8C);
	Assert(szuUtf8[3] == 0x82);
	Assert(szuUtf8[4] == 0x00);
	} // TEST_UnicodeUTF()


void
TEST_Md5(PSZAC pszaText, PSZAC pszaHashCode)
	{
	SHashMd5 hashMd5;
	InitToGarbage(OUT &hashMd5, sizeof(hashMd5));
	HashMd5_CalculateFromStringU(OUT &hashMd5, (PSZUC)pszaText);

	CHU szBuffer[200];
	InitToGarbage(OUT szBuffer, sizeof(szBuffer));
	HashMd5_ToStringHex(OUT szBuffer, IN &hashMd5);
	Assert(strcmpU(szBuffer, pszaHashCode) == 0 && "The md5 does not match the expected value");

	SHashMd5 hashMd5Reversed;
	InitToGarbage(OUT &hashMd5Reversed, sizeof(hashMd5Reversed));
	BOOL fSuccess = HashMd5_FInitFromStringHex(OUT &hashMd5Reversed, IN szBuffer);
	Assert(fSuccess);
	Assert(memcmp(&hashMd5Reversed, &hashMd5, sizeof(hashMd5)) == 0);
	}

void
TEST_Sha1(PSZAC pszaText, PSZAC pszaHashCode)
	{
	SHashSha1 hashSha1;
	InitToGarbage(OUT &hashSha1, sizeof(hashSha1));
	HashSha1_CalculateFromStringU(OUT &hashSha1, (PSZUC)pszaText);

	CHU szBuffer[200];
	InitToGarbage(OUT szBuffer, sizeof(szBuffer));
	HashSha1_ToStringHex(OUT szBuffer, IN &hashSha1);
	Assert(strcmpU(szBuffer, pszaHashCode) == 0 && "The SHA-1 does not match the expected value");
	}

void
Test_EncodeDecodeTimestampBase64(TIMESTAMP ts)
	{
	CHU szTimestamp[16];
	unsigned cchTimestamp = Timestamp_CchEncodeToBase64Url(IN ts, OUT szTimestamp);
	Assert(strlenU(szTimestamp) == cchTimestamp);
	TIMESTAMP tsDecoded;
	PCHRO pchRemain = Timestamp_PchDecodeFromBase64Url(OUT &tsDecoded, IN szTimestamp);
	Assert(*pchRemain == '\0');
	Assert(tsDecoded == ts);
	}

void
TEST_Base64()
	{
	/*
	CStr str;
	str.BinInitFromStringWithoutNullTerminator("const BYTE c_mapbbBase64[256] =\n{");
	CHU szString[4] = { ' ', d_zNA, d_zNA, d_zNA };
	int ch = 0;
	while (ch <= 255)
		{
		PSZAC pszValue = NULL;
		switch (ch)
			{
		case '\0':
			pszValue = "_NT";
			break;
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			pszValue = "_WS";
			break;
		default:
			const char * pch = strchr(c_rgchBase64, ch);
			if (pch == NULL)
				pszValue = "_OR";
			else
				{
				const int ich = pch - c_rgchBase64;
				Assert(ich >= 0 && ich < 64);
				if (ich < 10)
					{
					szString[1] = ' ';
					szString[2] = '0' + ich;
					}
				else
					{
					szString[1] = '0' + ich / 10;
					szString[2] = '0' + ich % 10;
					}
				pszValue = (char *)szString;
				}
			} // switch
		str.BinAppendTextSzv_VE("$s$s", (ch++ % 32) ? ", " : ",\n\t", pszValue);
		} // while
	str.BinAppendStringWithNullTerminator("\n};");
	MessageLog_AppendTextFormatCo(d_coBlack, "$S\n", &str);
	*/
	Test_EncodeDecodeTimestampBase64(0);
	Test_EncodeDecodeTimestampBase64(1);
	Test_EncodeDecodeTimestampBase64(1234567890);
	}

void
TEST_Base85Encode(PSZAC pszText)
	{
	CHU szEncoded[100];
	Base85_CchEncodeToText(OUT szEncoded, (const BYTE *)pszText, strlen(pszText));
//	Base64_CchEncodeToText(szEncoded, (const BYTE *)pszText, strlen(pszText));

	CHU szDecoded[100];
	int cchDecoded = Base85_CbDecodeToBinary(IN szEncoded, OUT szDecoded);
	szDecoded[cchDecoded] = '\0';
//	MessageLog_AppendTextFormatCo(d_coBlack, "Base85_CchEncodeToText:\n\t$s \n\t$s \n\t$s\n", pszText, szEncoded, szDecoded);
	if (!FCompareStrings(szDecoded, pszText))
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Base85 Encoding/Decoding Error!\n");
	}

void
TEST_Base85()
	{
	// Validate the c_rgchBase85[] to make sure there are no duplicates and no reserved characters
	const char c_rgchBase85Reserved[] = "<>&'\" =, :.";	// The following characters are reserved to make sure there is no need for encoding in XML/HTML and/or other common tasks

	for (int ich = 0; ich < 85; ich++)
		{
		char ch = c_rgchBase85[ich];
		//MessageLog_AppendTextFormatCo(d_coBlack, "ch[$i] = $b\n", ich, ch);
		Assert(ch != '\0');
		const char * pch = strchr(c_rgchBase85, ch);
		Assert(pch == c_rgchBase85 + ich);	// Search if there is any duplicate
		pch = strchr(c_rgchBase85Reserved, ch);
		Assert(pch == NULL);	// Base85 should not use any reserved character
		}

	/*
	// Generate the source code to decode a Base85 string.
	CStr str;
	str.BinInitFromStringWithoutNullTerminator("const BYTE c_mapbbBase85[256] =\n{");
	CHU szString[4] = { ' ', d_zNA, d_zNA, d_zNA };
	int ch = 0;
	while (ch <= 255)
		{
		PSZAC pszValue = NULL;
		switch (ch)
			{
		case '\0':
			pszValue = "_NT";
			break;
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			pszValue = "_WS";
			break;
		default:
			const char * pch = strchr(c_rgchBase85, ch);
			if (pch == NULL)
				pszValue = "_OR";
			else
				{
				const int ich = pch - c_rgchBase85;
				Assert(ich >= 0 && ich < 85);
				if (ich < 10)
					{
					szString[1] = ' ';
					szString[2] = '0' + ich;
					}
				else
					{
					szString[1] = '0' + ich / 10;
					szString[2] = '0' + ich % 10;
					}
				pszValue = (char *)szString;
				}
			} // switch
		str.BinAppendTextSzv_VE("$s$s", (ch++ % 32) ? ", " : ",\n\t", pszValue);
		} // while
	str.BinAppendStringWithNullTerminator("\n};");
	MessageLog_AppendTextFormatCo(d_coBlack, "$S\n", &str);
	*/

	TEST_Base85Encode("");
	TEST_Base85Encode("A");
	TEST_Base85Encode("a");
	TEST_Base85Encode("ab");
	TEST_Base85Encode("abc");
	TEST_Base85Encode("abcd");
	TEST_Base85Encode("abcde");
	TEST_Base85Encode("abcdef");
	TEST_Base85Encode("abcdefg");
	/*
	RECT rc = { 10, 0, 100, 100 };
	CHU szRectBase85[100];
	Base85_CchEncodeToText(szRectBase85, (const BYTE *)&rc, sizeof(rc));
//	MessageLog_AppendTextFormatCo(d_coBlack, "Rect Base85 Encoded: $s\n", szRectBase85);
//	MessageLog_AppendTextFormatCo(d_coBlack, "sizeof(sizeof(QDate)) = $i\n", sizeof(QDate));
	*/
	} // TEST_Base85()

void
TEST_GenerateTableAsciiFlags()
	{
	return;
	// Generate the source code to decode a hexadecimal values / Base16 string.  This table also contains flags to detect white spaces and punctuation.
	CStr strTable;
	strTable.BinInitFromStringWithoutNullTerminator("const BYTE c_mapbbHexDigits[256] =\n{");
	int ch = 0;
	while (ch <= 255)
		{
		CStr strValue;
		switch (ch)
			{
		case '\0':
			strValue.Format("_NT");
			break;
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			strValue.Format("_WS");
			break;
		default:
			if (ch >= '0' && ch <= '9')
				strValue.Format("  $i", ch - '0');
			else if (ch >= 'a' && ch <= 'z')
				strValue.Format(" $i", ch - 'a' + 10);
			else if (ch >= 'A' && ch <= 'Z')
				strValue.Format(" $i", ch - 'A' + 10);
			else if (ch > ' ' && ch <= '~')
				strValue.Format("_PU");
			else
				strValue.Format("_OR");
			} // switch

		int chPrintable = (ch >= ' ' && ch <= '~') ? ch : ' ';
		strTable.BinAppendTextSzv_VE("\n\t$S, // 0x$x   $i  $b", &strValue, ch, ch, chPrintable);
		ch++;
		} // while
	strTable.BinAppendStringWithNullTerminator("\n};");
	MessageLog_AppendTextFormatCo(d_coBlack, "$S\n", &strTable);
	} // TEST_GenerateTableAsciiFlags()

void
TEST_PchNumberToString64(L64 lNumber, UINT uDivideBy, PSZAC pszResultExpected)
	{
	CHU rgchResult[100];
	CHU * pchNext = PchNumberToString64_Decimal0or1(OUT rgchResult, lNumber, uDivideBy);
	*pchNext = '\0';
	if (!FCompareStrings(rgchResult, pszResultExpected))
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "PchNumberToString64($l, $i) returned '$s' while expecting value '$s'\n", lNumber, uDivideBy, rgchResult, pszResultExpected);
		}
	}

void
TEST_PchNumberToStringFormat_BTC(L64 amtSatoshis, PSZAC pszResultExpected)
	{
	CHU rgchResult[100];
	CHU * pchNext = PchNumberToStringFormat_BTC(OUT rgchResult, amtSatoshis);
	*pchNext = '\0';
	if (!FCompareStrings(rgchResult, pszResultExpected))
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "TEST_PchNumberToStringFormat_BTC($l) returned '$s' while expecting value '$s'\n", amtSatoshis, rgchResult, pszResultExpected);
		}
	}

void
TEST_PchNumberToStringFormat_mBTC(L64 amtSatoshis, PSZAC pszResultExpected)
	{
	CHU rgchResult[100];
	CHU * pchNext = PchNumberToStringFormat_mBTC(OUT rgchResult, amtSatoshis);
	*pchNext = '\0';
	if (!FCompareStrings(rgchResult, pszResultExpected))
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "PchNumberToStringFormat_mBTC($l) returned '$s' while expecting value '$s'\n", amtSatoshis, rgchResult, pszResultExpected);
		}
	}
CHU * PchNumberToStringFormat_USD(OUT CHU prgchStringDollar[64], AMOUNT amtCents);

void
TEST_PchNumberToStringFormat_USD(L64 amtCents, PSZAC pszResultExpected)
	{
	CHU rgchResult[100];
	CHU * pchNext = PchNumberToStringFormat_USD(OUT rgchResult, amtCents);
	*pchNext = '\0';
	if (!FCompareStrings(rgchResult, pszResultExpected))
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "TEST_PchNumberToStringFormat_USD($l) returned '$s' while expecting value '$s'\n", amtCents, rgchResult, pszResultExpected);
		}
	}

void
TEST_GenerateTableLowersase()
	{
	// Generate the source code to have a macro to quickly convert from uppercase to lowercase
	CStr strTable;
	strTable.BinInitFromStringWithoutNullTerminator("const CHU c_mapbbLowercase[256] =\n{");
	int ch = 0;
	while (ch <= 255)
		{
		int chLowercase = ch;
		if (ch >= 'A' && ch <= 'Z')
			chLowercase = Ch_ToLowercase(ch);
		if (Ch_FIsAlphaNumeric(chLowercase))
			strTable.BinAppendTextSzv_VE("'$b'", chLowercase);
		else
			strTable.BinAppendTextSzv_VE("$i", chLowercase);
		strTable.BinAppendTextSzv_VE((ch++ % 32) ? ", " : ",\n\t");
		} // while
	strTable.BinAppendStringWithNullTerminator("\n};");
	MessageLog_AppendTextFormatCo(d_coBlack, "$S\n", &strTable);
	} // TEST_GenerateTableLowersase()

BOOL
TEST_FIsValidTLD(PSZAC pszTLD, PSZAC pszRemain = NULL)
	{
	PSZR pszrRemain = PszrFindDomainExtension((PSZUC)pszTLD);
	if (pszrRemain != NULL)
		{
		return (FCompareStrings_YZ(pszrRemain, (PSZUC)pszRemain));
		}
	return FALSE;
	}

void
TEST_AutoHyperlink(PSZAC pszText, PSZAC pszTextWithHyperlink)
	{
	CStr str;
	str.BinAppendHtmlTextWithAutomaticHyperlinks((PSZUC)pszText);
	PSZUC pszResult = str.BinAppendNullTerminatorSz();
	if (!FCompareStrings(pszTextWithHyperlink, pszResult))
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "AutoHyperlink Failure:\n\tSource\t\t= '$s'\n\tExpecting\t= '$s'\n\tResult\t\t\t= '$s'\n", pszText, pszTextWithHyperlink, pszResult);
		}
	}

void
TEST_StringRoutines()
	{
	//TEST_UnicodeUTF();

	PSZUC pszuCommaSeparatedString = (PSZUC)" username=joe, realm='cambrian.org', nonce='OA6MG9tEQGm2hh',cnonce='OA6MHXh6VqTrRk' , nc=00000001";
	CStr str;
	str.InitFromValueOfCommaSeparatedStringPsz(pszuCommaSeparatedString, "username");
	Assert(str.FCompareStringsNoCase((PSZUC)"joe"));
	str.InitFromValueOfCommaSeparatedStringPsz(pszuCommaSeparatedString, "realm");
	Assert(str.FCompareStringsNoCase((PSZUC)"cambrian.org"));
	str.InitFromValueOfCommaSeparatedStringPsz(pszuCommaSeparatedString, "nc");
	Assert(str.FCompareStringsNoCase((PSZUC)"00000001"));

	L64 cblFileSize = 1234567890;
	PSZUC pszAttributes = str.PszAppendVirtualXmlAttributes("id^u,file^s,size^l", 10, "test.txt", cblFileSize);
	Assert(FCompareStrings(pszAttributes, " id='10' file='test.txt' size='1234567890'"));

	PSZUC pszStringCompare = (PSZUC)"/Data=foo";
	PSZUC pszResult = PszrCompareStringBeginNoCase(pszStringCompare, "/data=");
	Assert(FCompareStrings(pszResult, "foo"));

	pszResult = PszrStringContainsSubString(pszStringCompare, "=");
	Assert(FCompareStrings(pszResult, "foo"));
	pszResult = PszrStringContainsSubString(pszStringCompare, "data");	// The comparison is case sensitive
	Assert(pszResult == NULL);
	pszResult = PszrStringContainsSubStringLowerCase(pszStringCompare, "=");
	Assert(FCompareStrings(pszResult, "foo"));
	pszResult = PszrStringContainsSubStringLowerCase(pszStringCompare, "Data");
	Assert(FCompareStrings(pszResult, "=foo"));
	pszResult = PszrStringContainsSubStringLowerCase(pszStringCompare, "data");
	Assert(FCompareStrings(pszResult, "=foo"));
	pszResult = PszrStringContainsSubStringLowerCase(pszStringCompare, "dATA");
	Assert(FCompareStrings(pszResult, "=foo"));

	str.InitFromStringU((PSZUC)"joe");
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"j", '\0'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"jo", '\0'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe", '\0'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe", '@'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@example.com", '@'));
	str.InitFromStringU((PSZUC)"joe@example.com/resource");
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe", '@'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@", '@'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@example.com", '@'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@example.com", '/'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@example.com", '\0'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@example.com/resource", '/'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"joe@example.com/otherresource", '/'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"Joe@Example.com/otherresource", '/'));
	Assert(str.FCompareStringsNoCaseUntilCharacter((PSZUC)"Joe@Example.com/otherresource", '.'));
	Assert(!str.FCompareStringsNoCaseUntilCharacter((PSZUC)"Joe@Example.com/otherresource", '\0'));	// The strings differ because the character until is different

	Assert(sizeof(SHashMd5) == 16);	// 128 bits
	TEST_Md5("", "d41d8cd98f00b204e9800998ecf8427e");	// Reverse: 7e 42 f8 ec 98 09 80 e9 04 b2 00 8f d9 8c 1d d4 => "7E42F8EC980980E904B2008FD98C1DD4"
	TEST_Md5("a", "0cc175b9c0f1b6a831c399e269772661");
	TEST_Md5("abc", "900150983cd24fb0d6963f7d28e17f72");
	TEST_Md5("The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6");
	TEST_Md5("The quick brown fox jumps over the lazy cog", "1055d3e698d289f2af8663725127bd4b");

	Assert(sizeof(SHashSha1) == 20);	// 160 bits
	TEST_Sha1("The quick brown fox jumps over the lazy dog", "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");
	TEST_Sha1("The quick brown fox jumps over the lazy cog", "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3");

	TEST_Base85();

	TEST_Base64();

	Assert(LStringToNumber_ZZR_ML((PSZUC)"-29999999") == -29999999);

	TEST_PchNumberToString64(0, 10, "0");
	TEST_PchNumberToString64(0, 1000, "0");
	TEST_PchNumberToString64(12, 10, "1.2");
	TEST_PchNumberToString64(1200, 1000, "1.2");
	TEST_PchNumberToString64(5100, 1000, "5.1");
	TEST_PchNumberToString64(50, 10, "5");
	TEST_PchNumberToString64(500, 100, "5");
	TEST_PchNumberToString64(150, 10, "15");
	TEST_PchNumberToString64(1500, 10, "150");
	TEST_PchNumberToString64(1500, 100, "15");
	TEST_PchNumberToString64(123, 1, "123");
	TEST_PchNumberToString64(1234, 1, "1234");
	TEST_PchNumberToString64(123000, 1000, "123");
	TEST_PchNumberToString64(1234000, 1000, "1234");
	TEST_PchNumberToString64(123456, 1000, "123");
	TEST_PchNumberToString64(1234567, 1000, "1234");
	TEST_PchNumberToString64(651, 1000, "0.7");
	TEST_PchNumberToString64(639, 1000, "0.6");
	TEST_PchNumberToString64(639, 100, "6.4");

	TEST_PchNumberToStringFormat_USD(1, "$0.01 USD");
	TEST_PchNumberToStringFormat_USD(12, "$0.12 USD");
	TEST_PchNumberToStringFormat_USD(123, "$1.23 USD");
	TEST_PchNumberToStringFormat_USD(120, "$1.20 USD");
	TEST_PchNumberToStringFormat_USD(100, "$1.00 USD");
	TEST_PchNumberToStringFormat_USD(1000*100, "$1,000.00 USD");
	TEST_PchNumberToStringFormat_USD(    1234500,      "$12,345.00 USD");
	TEST_PchNumberToStringFormat_USD(	12345600,     "$123,456.00 USD");
	TEST_PchNumberToStringFormat_USD(  123456700,   "$1,234,567.00 USD");
	TEST_PchNumberToStringFormat_USD( 1234567800,  "$12,345,678.00 USD");
	TEST_PchNumberToStringFormat_USD(12345678900, "$123,456,789.00 USD");
	TEST_PchNumberToStringFormat_USD(123456789012345, "$1,234,567,890,123.45 USD");

	TEST_PchNumberToStringFormat_mBTC(1234567, "12.34567 mBTC");
	TEST_PchNumberToStringFormat_mBTC(1200000, "12.0 mBTC");
	TEST_PchNumberToStringFormat_mBTC(1000000, "10.0 mBTC");
	TEST_PchNumberToStringFormat_mBTC(d_cSatoshisPerBitcoin, "1,000.0 mBTC");
	TEST_PchNumberToStringFormat_BTC(d_cSatoshisPerBitcoin, "1.0000 BTC");	// Minimum of 3 digits
	TEST_PchNumberToStringFormat_BTC(1, "0.00000001 BTC");
	TEST_PchNumberToStringFormat_BTC( 29999999,  "0.29999999 BTC");
	TEST_PchNumberToStringFormat_BTC(-29999999, "-0.29999999 BTC");
	TEST_PchNumberToStringFormat_mBTC( 29999999,  "299.99999 mBTC");
	TEST_PchNumberToStringFormat_mBTC(-29999999, "-299.99999 mBTC");

	Assert(Timestamp_FromString_ZZR((PSZUC)"\\\\\\\\\\\\\\") == d_tsMax7Digits);	// This timestamp is 'valid' because it has 7 digits ("\\\\\\\")
	Assert(Timestamp_FromString_ZZR((PSZUC)"\\\\\\\\\\\\\\1") == 0);	// This timestamp is not valid because it is past year 2985.

	TEST_GenerateTableAsciiFlags();
	Assert(Ch_FIsDigit('0'));
	Assert(Ch_FIsDigit('9'));
	Assert(!Ch_FIsDigit('a'));
	Assert(Ch_FIsHexDigit('a'));
	Assert(Ch_FIsHexDigit('F'));
	Assert(!Ch_FIsHexDigit('G'));
	Assert(Ch_FIsAlphaNumeric('G'));
	Assert(Ch_FIsAlphaNumeric('z'));
	Assert(Ch_FIsAlphaNumeric('0'));
	Assert(!Ch_FIsAlphaNumeric('?'));

	//TEST_GenerateTableLowersase();
	Assert(TEST_FIsValidTLD("ca"));
	Assert(!TEST_FIsValidTLD("xa"));
	Assert(TEST_FIsValidTLD("com"));
	Assert(!TEST_FIsValidTLD("comm"));
	Assert(TEST_FIsValidTLD("com!", "!"));
	Assert(TEST_FIsValidTLD("info"));
	Assert(TEST_FIsValidTLD("coop"));

	TEST_AutoHyperlink(".org", ".org");	// Do not encode a domain extension without a domain name

	TEST_AutoHyperlink("cambrian.org", "<a href='http://cambrian.org'>cambrian.org</a>");
	TEST_AutoHyperlink("http:||cambrian.org", "<a href='http:||cambrian.org'>http:||cambrian.org</a>");
	TEST_AutoHyperlink("https://cambrian.org", "<a href='https://cambrian.org'>https://cambrian.org</a>");

	TEST_AutoHyperlink(" https://cambrian.org", " <a href='https://cambrian.org'>https://cambrian.org</a>");
	TEST_AutoHyperlink("cambrian.org/test", "<a href='http://cambrian.org/test'>cambrian.org/test</a>");
	TEST_AutoHyperlink("cambrian.org/test/", "<a href='http://cambrian.org/test/'>cambrian.org/test/</a>");
	TEST_AutoHyperlink("cambrian.org.", "<a href='http://cambrian.org'>cambrian.org</a>.");
	TEST_AutoHyperlink(".cambrian.org", ".<a href='http://cambrian.org'>cambrian.org</a>");

	TEST_AutoHyperlink(".cambrian.org.", ".<a href='http://cambrian.org'>cambrian.org</a>.");
	TEST_AutoHyperlink("cambrian..org", "cambrian..org");		// A domain name cannot end with a dot, therefore cannot have consecutive dots
	TEST_AutoHyperlink("cambrian...org", "cambrian...org");		// Or triple dots

	TEST_AutoHyperlink("(cambrian.org)", "(<a href='http://cambrian.org'>cambrian.org</a>)");
	TEST_AutoHyperlink("<cambrian.org>", "&lt;<a href='http://cambrian.org'>cambrian.org</a>&gt;");

	TEST_AutoHyperlink("(cambrian.gt)", "(<a href='http://cambrian.gt'>cambrian.gt</a>)");

	TEST_AutoHyperlink("try cambrian.org.", "try <a href='http://cambrian.org'>cambrian.org</a>.");
	TEST_AutoHyperlink("try cambrian.org, or www.cambrian.org.", "try <a href='http://cambrian.org'>cambrian.org</a>, or <a href='http://www.cambrian.org'>www.cambrian.org</a>.");

	TEST_AutoHyperlink("try <b>www.cambrian.org</b>", "try &lt;b&gt;<a href='http://www.cambrian.org'>www.cambrian.org</a>&lt;/b&gt;");
	TEST_AutoHyperlink("try 'www.cambrian.org'", "try &#39;<a href='http://www.cambrian.org'>www.cambrian.org</a>&#39;");
	TEST_AutoHyperlink("try 'www.cambrian.org/test'", "try &#39;<a href='http://www.cambrian.org/test'>www.cambrian.org/test</a>&#39;");

	TEST_AutoHyperlink("info@cambrian.org", "info@cambrian.org");	// Don't encode emails
	TEST_AutoHyperlink("cambrian.org?email=info@cambrian.org", "<a href='http://cambrian.org?email=info@cambrian.org'>cambrian.org?email=info@cambrian.org</a>");

	TEST_AutoHyperlink("xmlns='http://jabber.org/features/iq-register'/>", "xmlns=&#39;<a href='http://jabber.org/features/iq-register'>http://jabber.org/features/iq-register</a>&#39;/&gt;");
	TEST_AutoHyperlink("xmlns=\"http://jabber.org/features/iq-register\"/>", "xmlns=&quot;<a href='http://jabber.org/features/iq-register'>http://jabber.org/features/iq-register</a>&quot;/&gt;");


	USZU uszu = UszuFromPsz((PSZUC)"");
	Assert(uszu == 0);
	uszu = UszuFromPsz((PSZUC)"a");
	Assert(uszu == _USZU1('a'));
	uszu = UszuFromPsz((PSZUC)"ab");
	Assert(uszu == _USZU2('a', 'b'));
	Assert(FCompareStrings(PszFromUSZU(uszu), "ab"));
	uszu = UszuFromPsz((PSZUC)"abc");
	Assert(uszu == _USZU3('a', 'b', 'c'));
	Assert(FCompareStrings(PszFromUSZU(uszu), "abc"));

	int cchBuffer;
	CHU szuBuffer[10];

	strcpy(OUT (char *)szuBuffer, "abc");
	cchBuffer = CbTrimTailingWhiteSpaces(INOUT szuBuffer);
	Assert(cchBuffer == 4);

	strcpy(OUT (char *)szuBuffer, "abc ");
	cchBuffer = CbTrimTailingWhiteSpaces(INOUT szuBuffer);
	Assert(cchBuffer == 4);

	} // TEST_StringRoutines()

#endif // DEBUG
