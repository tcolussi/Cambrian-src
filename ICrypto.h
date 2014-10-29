//	ICrypto.h
//
//	Interface to support various crypto algorithms to encrypt and decrypt data.
#ifndef ICRYPTO_H
#define ICRYPTO_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <openssl/evp.h>

enum ECryptoClass
{
	eCryptoClass_zNone					= 0,						// No encryption
//	eCryptoClass_XOR					= _USZU3('X', 'O', 'R'),	// Used for debugging
	eCryptoClass_EVP_aes_256_gcm		= _USZU3('A', '2', 'g'),
	eCryptoClass_OpenTransactions		= _USZU2('O', 'T'),

	eCryptoClassMax
};

inline ECryptoClass ECryptoClassFromPsz(PSZUC pszCryptoClass) { return (ECryptoClass)UszuFromPsz(pszCryptoClass); }

enum ECryptoError
{
	eCryptoError_zSuccess	= 0,		// No error
	eCryptoError_Failure	= 1			// There was a failure to encrypt or decrypt the data

};

class ICrypto
{
public:
	ICrypto * m_pNext;
	enum EFlags
		{
		F_kzNone			= 0x0000,	// The key is no longer used and therefore should not be serialized
		F_kfEncrypt			= 0x0001,	// This key is used to encrypt the data to send to the contact
		F_kfDecrypt			= 0x0002,	// This key is used to decrypt data received from the remote contact
		};
	UINT m_uFlags;

public:
	ICrypto();
	virtual ~ICrypto();
	virtual ECryptoClass EGetCryptoClass() const = 0;
	virtual ECryptoError EEncrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact) = 0;
	virtual ECryptoError EDecrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact) = 0;
	virtual void XmlSerialize(IOUT CBin * pbinXmlAttributes) const = 0;
	virtual void XmlUnserialize(const CXmlNode * pXmlNodeElement) = 0;
	virtual BOOL FCompareEqual(const ICrypto * pCryptoCompare) const = 0;

public:
	static ICrypto * S_PaAllocateCrypto_YZ(ECryptoClass eCryptoClass);
	static ICrypto * S_PaAllocateCrypto_YZ(const CXmlNode * pXmlNodeCrypto);
}; // ICrypto


class CListCrypto	// This is somewhat a keyring
{
	enum { c_cInstancesMax = 3 };	// Allow a maximum of 3 instances of each class of ICrypto.  This value prevents the list to grow without limit.
public:
	ICrypto * m_plistCrypto;	// Linked list of crypto algorithms to encrypt and decrypt data.

public:
	CListCrypto() { m_plistCrypto = NULL; }
	BOOL FContainsDuplicate(ICrypto * pCryptoCompare, int cInstancesMax);
	void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
	void SerializeKeysToXml(IOUT CBin * pbinXmlKeys, BOOL fSerializeToDisk) const;
	void UnserializeKeysFromXml(const CXmlNode * pXmlNodeElementKey, BOOL fUnserializeFromDisk);
	void DisplayKeysToMessageLog() const;
};

#define d_szXmlElementKeys			"K"
#define d_chXmlElementKeys			'K'
#define d_chXmlAttributeFlags		'f'
#define d_szXmlAttributeFlags_ux	" f='$x'"
#define d_chXmlAttributeKey			'k'

#define BYTES_FROM_BITS(nBits)		((nBits) / 8)

class CCryptoAes256gcm : public ICrypto
{
protected:
	enum { c_cbBlockSize = 16 };	// The full block size of AES 256 is 16 bytes
	struct
		{
		BYTE rgbKey[BYTES_FROM_BITS(256)];
		BYTE rgbIV[EVP_MAX_IV_LENGTH];
		} m_data;
	EVP_CIPHER_CTX * m_haContext;

public:
	CCryptoAes256gcm();
	~CCryptoAes256gcm();
	void GenerateKey();
	virtual ECryptoClass EGetCryptoClass() const { return eCryptoClass_EVP_aes_256_gcm; }
	virtual ECryptoError EEncrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact);
	virtual ECryptoError EDecrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact);
	virtual void XmlSerialize(IOUT CBin * pbinXmlAttributes) const;
	virtual void XmlUnserialize(const CXmlNode * pXmlNodeElement);
	virtual BOOL FCompareEqual(const ICrypto * pCryptoCompare) const;
};

#define COMPILE_WITH_ICRYPTO_OPEN_TRANSACTIONS

#ifdef COMPILE_WITH_ICRYPTO_OPEN_TRANSACTIONS
#define d_szXmlAttributes_OT_strName_strKeyPublic		" n='^S' k='^S'"

class CCryptoOpenTransactions : public ICrypto
{
protected:
	CStr m_strNymID;							// OT nym
	CStr m_strKeyPublic;						// Public key of the contact (this key is used to encrypt messages)

public:
	CCryptoOpenTransactions();
	~CCryptoOpenTransactions();
	virtual ECryptoClass EGetCryptoClass() const { return eCryptoClass_OpenTransactions; }
	virtual ECryptoError EEncrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact);
	virtual ECryptoError EDecrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact);
	virtual void XmlSerialize(IOUT CBin * pbinXmlAttributes) const;
	virtual void XmlUnserialize(const CXmlNode * pXmlNodeElement);
	virtual BOOL FCompareEqual(const ICrypto * pCryptoCompare) const;
};
#endif // COMPILE_WITH_ICRYPTO_OPEN_TRANSACTIONS

#include <openssl/evp.h>

enum EEvpResult	// Enum value returned by most EVP_*() APIs
	{
	eEvp_zFailure	= 0,
	eEvp_Success	= 1
	};

#endif // ICRYPTO_H
