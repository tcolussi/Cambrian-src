//	The code of this file was copied from .\Src\qtbase\src\3rdparty\sha1\sha1.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


// Test Vectors (from FIPS PUB 180-1)
//
//  SHA1("abc") =
//      A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
//
//  SHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") =
//      84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
//
//  SHA1(A million repetitions of "a") =
//      34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
//


// #define or #undef this, if you want the to wipe all
// temporary variables after processing
#define SHA1_WIPE_VARIABLES


struct Sha1State
{
	quint32 h0;
	quint32 h1;
	quint32 h2;
	quint32 h3;
	quint32 h4;

	quint64 messageSize;
	unsigned char buffer[64];
};


typedef union
{
	quint8  bytes[64];
	quint32 words[16];
} Sha1Chunk;

static inline quint32 rol32(quint32 value, unsigned int shift)
{
#ifdef Q_CC_MSVC
	return _rotl(value, shift);
#else
	return ((value << shift) | (value >> (32 - shift)));
#endif
}

static inline quint32 sha1Word(Sha1Chunk *chunk, const uint position)
{
	return (chunk->words[position & 0xf] = rol32(  chunk->words[(position+13) & 0xf]
												 ^ chunk->words[(position+ 8) & 0xf]
												 ^ chunk->words[(position+ 2) & 0xf]
												 ^ chunk->words[(position)    & 0xf], 1));
}

static inline void sha1Round0(Sha1Chunk *chunk, const uint position,
							  quint32 &v, quint32 &w, quint32 &x, quint32 &y, quint32 &z)
{
	z += ((( w & (x ^ y)) ^ y) + chunk->words[position] + 0x5A827999 + rol32(v, 5));
	w = rol32(w, 30);
}

static inline void sha1Round1(Sha1Chunk *chunk, const uint position,
							  quint32 &v, quint32 &w, quint32 &x, quint32 &y, quint32 &z)
{
	z += ((( w & (x ^ y)) ^ y) + sha1Word(chunk,position) + 0x5A827999 + rol32(v, 5));
	w = rol32(w, 30);
}

static inline void sha1Round2(Sha1Chunk *chunk, const uint position,
							  quint32 &v, quint32 &w, quint32 &x, quint32 &y, quint32 &z)
{
	z += (( w ^ x ^ y) + sha1Word(chunk, position) + 0x6ED9EBA1 + rol32(v, 5));
	w = rol32(w, 30);
}

static inline void sha1Round3(Sha1Chunk *chunk, const uint position,
							  quint32 &v, quint32 &w, quint32 &x, quint32 &y, quint32 &z)
{
	z += (((( w | x) & y) | (w & x)) + sha1Word(chunk, position) + 0x8F1BBCDC + rol32(v, 5));
	w = rol32(w, 30);
}

static inline void sha1Round4(Sha1Chunk *chunk, const uint position,
							  quint32 &v, quint32 &w, quint32 &x, quint32 &y, quint32 &z)
{
	z += ((w ^ x ^ y) + sha1Word(chunk, position) + 0xCA62C1D6 + rol32(v, 5));
	w = rol32(w, 30);
}

static inline void sha1ProcessChunk(Sha1State *state, const unsigned char *buffer)
{
	// Copy state[] to working vars
	quint32 a = state->h0;
	quint32 b = state->h1;
	quint32 c = state->h2;
	quint32 d = state->h3;
	quint32 e = state->h4;

	quint8 chunkBuffer[64];
	memcpy(chunkBuffer, buffer, 64);

	Sha1Chunk *chunk = reinterpret_cast<Sha1Chunk*>(&chunkBuffer);

	for (int i = 0; i < 16; ++i)
		chunk->words[i] = qFromBigEndian(chunk->words[i]);

	sha1Round0(chunk,  0, a,b,c,d,e); sha1Round0(chunk,  1, e,a,b,c,d); sha1Round0(chunk,  2, d,e,a,b,c); sha1Round0(chunk,  3, c,d,e,a,b);
	sha1Round0(chunk,  4, b,c,d,e,a); sha1Round0(chunk,  5, a,b,c,d,e); sha1Round0(chunk,  6, e,a,b,c,d); sha1Round0(chunk,  7, d,e,a,b,c);
	sha1Round0(chunk,  8, c,d,e,a,b); sha1Round0(chunk,  9, b,c,d,e,a); sha1Round0(chunk, 10, a,b,c,d,e); sha1Round0(chunk, 11, e,a,b,c,d);
	sha1Round0(chunk, 12, d,e,a,b,c); sha1Round0(chunk, 13, c,d,e,a,b); sha1Round0(chunk, 14, b,c,d,e,a); sha1Round0(chunk, 15, a,b,c,d,e);
	sha1Round1(chunk, 16, e,a,b,c,d); sha1Round1(chunk, 17, d,e,a,b,c); sha1Round1(chunk, 18, c,d,e,a,b); sha1Round1(chunk, 19, b,c,d,e,a);
	sha1Round2(chunk, 20, a,b,c,d,e); sha1Round2(chunk, 21, e,a,b,c,d); sha1Round2(chunk, 22, d,e,a,b,c); sha1Round2(chunk, 23, c,d,e,a,b);
	sha1Round2(chunk, 24, b,c,d,e,a); sha1Round2(chunk, 25, a,b,c,d,e); sha1Round2(chunk, 26, e,a,b,c,d); sha1Round2(chunk, 27, d,e,a,b,c);
	sha1Round2(chunk, 28, c,d,e,a,b); sha1Round2(chunk, 29, b,c,d,e,a); sha1Round2(chunk, 30, a,b,c,d,e); sha1Round2(chunk, 31, e,a,b,c,d);
	sha1Round2(chunk, 32, d,e,a,b,c); sha1Round2(chunk, 33, c,d,e,a,b); sha1Round2(chunk, 34, b,c,d,e,a); sha1Round2(chunk, 35, a,b,c,d,e);
	sha1Round2(chunk, 36, e,a,b,c,d); sha1Round2(chunk, 37, d,e,a,b,c); sha1Round2(chunk, 38, c,d,e,a,b); sha1Round2(chunk, 39, b,c,d,e,a);
	sha1Round3(chunk, 40, a,b,c,d,e); sha1Round3(chunk, 41, e,a,b,c,d); sha1Round3(chunk, 42, d,e,a,b,c); sha1Round3(chunk, 43, c,d,e,a,b);
	sha1Round3(chunk, 44, b,c,d,e,a); sha1Round3(chunk, 45, a,b,c,d,e); sha1Round3(chunk, 46, e,a,b,c,d); sha1Round3(chunk, 47, d,e,a,b,c);
	sha1Round3(chunk, 48, c,d,e,a,b); sha1Round3(chunk, 49, b,c,d,e,a); sha1Round3(chunk, 50, a,b,c,d,e); sha1Round3(chunk, 51, e,a,b,c,d);
	sha1Round3(chunk, 52, d,e,a,b,c); sha1Round3(chunk, 53, c,d,e,a,b); sha1Round3(chunk, 54, b,c,d,e,a); sha1Round3(chunk, 55, a,b,c,d,e);
	sha1Round3(chunk, 56, e,a,b,c,d); sha1Round3(chunk, 57, d,e,a,b,c); sha1Round3(chunk, 58, c,d,e,a,b); sha1Round3(chunk, 59, b,c,d,e,a);
	sha1Round4(chunk, 60, a,b,c,d,e); sha1Round4(chunk, 61, e,a,b,c,d); sha1Round4(chunk, 62, d,e,a,b,c); sha1Round4(chunk, 63, c,d,e,a,b);
	sha1Round4(chunk, 64, b,c,d,e,a); sha1Round4(chunk, 65, a,b,c,d,e); sha1Round4(chunk, 66, e,a,b,c,d); sha1Round4(chunk, 67, d,e,a,b,c);
	sha1Round4(chunk, 68, c,d,e,a,b); sha1Round4(chunk, 69, b,c,d,e,a); sha1Round4(chunk, 70, a,b,c,d,e); sha1Round4(chunk, 71, e,a,b,c,d);
	sha1Round4(chunk, 72, d,e,a,b,c); sha1Round4(chunk, 73, c,d,e,a,b); sha1Round4(chunk, 74, b,c,d,e,a); sha1Round4(chunk, 75, a,b,c,d,e);
	sha1Round4(chunk, 76, e,a,b,c,d); sha1Round4(chunk, 77, d,e,a,b,c); sha1Round4(chunk, 78, c,d,e,a,b); sha1Round4(chunk, 79, b,c,d,e,a);

	// Add the working vars back into state
	state->h0 += a;
	state->h1 += b;
	state->h2 += c;
	state->h3 += d;
	state->h4 += e;

	// Wipe variables
#ifdef SHA1_WIPE_VARIABLES
	a = b = c = d = e = 0;
	memset(chunkBuffer, 0, 64);
#endif
}

static inline void sha1InitState(Sha1State *state)
{
	state->h0 = 0x67452301;
	state->h1 = 0xEFCDAB89;
	state->h2 = 0x98BADCFE;
	state->h3 = 0x10325476;
	state->h4 = 0xC3D2E1F0;

	state->messageSize = 0;
}

static inline void sha1Update(Sha1State *state, const unsigned char *data, qint64 len)
{
	quint32 rest = static_cast<quint32>(state->messageSize & Q_UINT64_C(63));

	quint64 availableData = static_cast<quint64>(len) + static_cast<quint64>(rest);
	state->messageSize += len;

	if (availableData < Q_UINT64_C(64)) {
		memcpy(&state->buffer[rest], &data[0], len);

	} else {
		qint64 i = static_cast<qint64>(64 - rest);
		memcpy(&state->buffer[rest], &data[0], static_cast<qint32>(i));
		sha1ProcessChunk(state, state->buffer);

		qint64 lastI = len - ((len + rest) & Q_INT64_C(63));
		for( ; i < lastI; i += 64)
			sha1ProcessChunk(state, &data[i]);

		memcpy(&state->buffer[0], &data[i], len - i);
	}
}

static inline void sha1FinalizeState(Sha1State *state)
{
	quint64 messageSize = state->messageSize;
	unsigned char sizeInBits[8];
	qToBigEndian(messageSize << 3, sizeInBits);

	sha1Update(state, (const unsigned char *)"\200", 1);

	unsigned char zero[64];
	memset(zero, 0, 64);
	if (static_cast<int>(messageSize & 63) > 56 - 1) {
		sha1Update(state, zero, 64 - 1 - static_cast<int>(messageSize & 63));
		sha1Update(state, zero, 64 - 8);
	} else {
		sha1Update(state, zero, 64 - 1 - 8 - static_cast<int>(messageSize & 63));
	}

	sha1Update(state, sizeInBits, 8);
#ifdef SHA1_WIPE_VARIABLES
	memset(state->buffer, 0, 64);
	memset(zero, 0, 64);
	state->messageSize = 0;
#endif
}

static inline void sha1ToHash(Sha1State *state, unsigned char* buffer)
{
	qToBigEndian(state->h0, buffer);
	qToBigEndian(state->h1, buffer + 4);
	qToBigEndian(state->h2, buffer + 8);
	qToBigEndian(state->h3, buffer + 12);
	qToBigEndian(state->h4, buffer + 16);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void
HashSha1_CalculateFromBinary(OUT SHashSha1 * pHashSha1, IN const void * pvData, int cbData)
	{
	InitToGarbage(OUT pHashSha1, sizeof(*pHashSha1));
	Sha1State state;
	sha1InitState(OUT &state);
	sha1Update(INOUT &state, (const BYTE *)pvData, cbData);
	sha1FinalizeState(INOUT &state);
	sha1ToHash(IN &state, OUT (BYTE *)pHashSha1);
	}

void
HashSha1_CalculateFromCBin(OUT SHashSha1 * pHashSha1, IN const CBin & bin)
	{
	HashSha1_CalculateFromBinary(OUT pHashSha1, bin.PszuGetDataNZ(), bin.CbGetData());
	}

void
HashSha1_CalculateFromStringU(OUT SHashSha1 * pHashSha1, IN PSZUC pszuString)
	{
	HashSha1_CalculateFromBinary(OUT pHashSha1, pszuString, strlenU(pszuString));
	}

void
HashSha1_CalculateFromSaltedPassword(OUT SHashSha1 * pHashSha1, ESaltPrefix eSaltPrefix, IN PSZUC pszuPassword)
	{
	const int cchPassword = strlenU(pszuPassword);
	const int cbSaltedPassword = sizeof(UINT) + cchPassword;
	UINT * pSaltedPassword = (UINT *)alloc_stack(cbSaltedPassword);
	pSaltedPassword[0] = eSaltPrefix;
	memcpy(OUT pSaltedPassword + 1, IN pszuPassword, cchPassword);
	HashSha1_CalculateFromBinary(OUT pHashSha1, IN pSaltedPassword, cbSaltedPassword);
	}

BOOL
HashSha1_FInitFromStringBase85_ZZR_ML(OUT SHashSha1 * pHashSha1, IN PSZUC pszuStringBase85)
	{
	Assert(pHashSha1 != NULL);
	return Base85_FDecodeToBinary_ZZR_ML(IN pszuStringBase85, OUT_ZZR (BYTE *)pHashSha1, sizeof(SHashSha1));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Determine which hash hlgorithm is suitable given the size/length of the hash value.
EHashAlgorithm
EGetHashAlgorithmFromHashSize(int cbHashSize)
	{
	switch (cbHashSize)
		{
	case 0:	// Empty hash value have zero bytes
		return eHashAlgorithmNull;
	case sizeof(SHashMd5):
		return eHashAlgorithmMd5;
	case sizeof(SHashSha1):
		return eHashAlgorithmSha1;
	case sizeof(SHashSha256):
		return eHashAlgorithmSha256;
	case sizeof(SHashSha512):
		return eHashAlgorithmSha512;
		} // switch
	return eHashAlgorithmNil;
	} // EGetHashAlgorithmFromHashSize()

void
HashValue_CalculateFromBinary(OUT SHashValue * pHashValue, IN const void * pvData, int cbData, EHashAlgorithm eHashAlgorithm)
	{
	Assert(pHashValue != NULL);
	InitToGarbage(pHashValue, sizeof(*pHashValue));
	switch (eHashAlgorithm)
		{
	case eHashAlgorithmMd5:
		HashMd5_CalculateFromBinary(OUT (SHashMd5 *)pHashValue, pvData, cbData);
		return;
	case eHashAlgorithmSha1:
		HashSha1_CalculateFromBinary(OUT (SHashSha1 *)pHashValue, pvData, cbData);
		return;
	default:
		QCryptographicHash hash((QCryptographicHash::Algorithm)eHashAlgorithm);
		hash.addData((const char *)pvData, cbData);
		QByteArray arraybHash = hash.result();
		int cbHash = arraybHash.size();
		Assert(cbHash <= (int)sizeof(*pHashValue));
		if (cbHash > (int)sizeof(*pHashValue))
			cbHash = sizeof(*pHashValue);		// Just in case to prevent a buffer overflow
		memcpy(OUT pHashValue, IN arraybHash.data(), cbHash);
		} // switch
	} // HashValue_CalculateFromBinary()

//	Compute an encryption key from a password.
//	For security reasons, the application never stores a password but the key to decrypt data.
//	There may be many implementations to generate a key from a password, including adding a salt, or a combination of multiple hash algorithms, perhaps with repetition.
//	In the end, the entropy (randomness/strength) of the key depends on the length of the password.
void
HashKey256_CalculateKeyFromPassword(OUT SHashKey256 * pHashKey, IN PSZUC pszuPassword)
	{
	const QByteArray arraybHash = QCryptographicHash::hash(QByteArray((const char *)pszuPassword), QCryptographicHash::Sha256);
	Assert(arraybHash.size() == sizeof(*pHashKey));
	memcpy(OUT pHashKey, IN arraybHash.data(), sizeof(*pHashKey));
	}

//	This needs to be implemented.  At the moment, the encryption is just XOR with the key
void
CBin::DataEncryptAES256(const SHashKey256 * pKey)
	{
	Assert(pKey != NULL);

	}

void
CBin::DataDecryptAES256(const SHashKey256 * pKey)
	{
	Assert(pKey != NULL);

	}

BOOL
CBin::FCompareFingerprint(const SHashSha1 & hashSha1) const
	{
	SHashSha1 hash;
	HashSha1_CalculateFromCBin(OUT &hash, IN *this);
	return HashSha1_FCompareEqual(IN &hash, IN &hashSha1);
	}

BOOL
CBin::FCompareFingerprintWithCertificateBinaryData(const CSslCertificate & oCertificate) const
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData <= (int)sizeof(SHashSha512));
		EHashAlgorithm eHashAlgorithm = EGetHashAlgorithmFromHashSize(m_paData->cbData);
		if (eHashAlgorithm > 0)
			return FCompareBinary(oCertificate.GetHashValue(eHashAlgorithm));
		}
	return FALSE;
	}

BOOL
CBin::FCompareFingerprintWithCertificateBinaryData(const CBin & binCertificateBinaryData) const
	{
	if (m_paData != NULL && binCertificateBinaryData.m_paData != NULL)
		{
		const int cbHash = m_paData->cbData;
		Assert(cbHash <= (int)sizeof(SHashSha512));
		EHashAlgorithm eHashAlgorithm = EGetHashAlgorithmFromHashSize(cbHash);
		if (eHashAlgorithm > 0)
			{
			SHashValue hash;
			HashValue_CalculateFromBinary(OUT &hash, binCertificateBinaryData.m_paData->rgbData, binCertificateBinaryData.m_paData->cbData, eHashAlgorithm);
			return (memcmp(IN m_paData->rgbData, IN &hash, cbHash) == 0);
			}
		}
	return FALSE;
	}

void
HashSha1_InitEmpty(OUT SHashSha1 * pHashSha1)
	{
	Assert(sizeof(SHashSha1) == 20);	// 160 bits
	InitToZeroes(OUT pHashSha1, sizeof(*pHashSha1));
	}

struct _SRandom
	{
	GUID guid;
	qint64 msecSinceCurrentEpoch;
	UINT uPointMouseCoordinatesXxorY;
	};

// Try to include as much 'randomness' from the system using portable methods
class _CRandom
{
private:
	QUuid m_uuid;
	QPoint m_ptMouseCoordinates;
	qint64 m_msecSinceCurrentEpoch;
public:
	_CRandom() : m_uuid(QUuid::createUuid()), m_ptMouseCoordinates(QCursor::pos()), m_msecSinceCurrentEpoch(QDateTime::currentMSecsSinceEpoch()) { }
};


//	Create a 160-bit random value.
//	This function is quite slow, so use it only when necessary.
void
HashSha1_InitRandom(OUT SHashSha1 * pHashSha1)
	{
	_CRandom random;
	Assert(sizeof(random) == 32);	// This Assert() is not important and the content of class _CRandom may change, however it is always nice to have a confirmation of the size of the class
	HashSha1_CalculateFromBinary(OUT pHashSha1, IN &random, sizeof(random));	// Hash everything, so the random value does not disclose its source of randomness
	}

void
HashSha1_InitRandomIfEmpty(INOUT SHashSha1 * pHashSha1)
	{
	if (HashSha1_FIsEmpty(IN pHashSha1))
		HashSha1_InitRandom(OUT pHashSha1);
	}

BOOL
HashSha1_FIsEmpty(const SHashSha1 * pHashSha1)
	{
	Assert(pHashSha1 != NULL);
	return FIsZeroInit(pHashSha1, sizeof(*pHashSha1));
	//return (pHashSha1->lDataA == 0 && pHashSha1->lDataB == 0 && pHashSha1->uDataC == 0);
	}

