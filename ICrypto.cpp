//	Implementation of various crypto algorithms to encrypt and decrypt data.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

ICrypto::ICrypto()
	{
	InitToGarbage(OUT &m_pNext, sizeof(m_pNext));
	m_uFlags = 0;
	}

ICrypto::~ICrypto()
	{
	}

ICrypto *
ICrypto::S_PaAllocateCrypto_YZ(ECryptoClass eCryptoClass)
	{
	switch (eCryptoClass)
		{
	#ifdef COMPILE_WITH_ICRYPTO_AES_256_GSM
	case eCryptoClass_EVP_aes_256_gcm:
		return new CCryptoAes256gcm;
	#endif
	case eCryptoClass_OpenTransactions:
		return new CCryptoOpenTransactions;
	default:
		return NULL;
		}
	}

ICrypto *
ICrypto::S_PaAllocateCrypto_YZ(const CXmlNode * pXmlNodeCrypto)
	{
	Assert(pXmlNodeCrypto != NULL);
	Assert(pXmlNodeCrypto->m_pszuTagName != NULL);
	return S_PaAllocateCrypto_YZ(ECryptoClassFromPsz(pXmlNodeCrypto->m_pszuTagName));
	}


BOOL
CListCrypto::FContainsDuplicate(ICrypto * pCryptoCompare, int cInstancesMax)
	{
	Assert(pCryptoCompare != NULL);
	BOOL fDuplicateFound = FALSE;
	ECryptoClass eCryptoClass = pCryptoCompare->EGetCryptoClass();
	ICrypto * pCrypto = m_plistCrypto;
	while (pCrypto != NULL)
		{
		if (pCrypto->EGetCryptoClass() == eCryptoClass)
			{
			if (cInstancesMax-- <= 0)
				pCrypto->m_uFlags = ICrypto::F_kzNone;	// Don't serialize this value
			fDuplicateFound |= pCrypto->FCompareEqual(pCryptoCompare);
			}
		pCrypto = pCrypto->m_pNext;
		}
	return fDuplicateFound;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	The keys will always be serialized under the XML element 'K'
void
CListCrypto::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	if (pXmlExchanger->m_fSerializing)
		{
		if (m_plistCrypto == NULL)
			return;	// Nothing to serialize
		CBin * pbinTemp = pXmlExchanger->PGetBinTemporaryDuringSerializationInitAlreadyEncoded();	// Use the temporary buffer to serialize all the keys
		SerializeKeysToXml(IOUT pbinTemp, TRUE);
		(void)pXmlExchanger->XmlExchange_PAllocateElementFromCBinString(d_chXmlElementKeys, IN_MOD_TMP *pbinTemp);
		}
	else
		{
		const CXmlNode * pXmlKeys = pXmlExchanger->XmlExchange_PFindElement(d_chXmlElementKeys);
		if (pXmlKeys != NULL)
			UnserializeKeysFromXml(pXmlKeys->m_pElementsList, TRUE);
		}
	}

void
CListCrypto::SerializeKeysToXml(IOUT CBin * pbinXmlKeys, BOOL fSerializeToDisk) const
	{
	Assert(pbinXmlKeys != NULL);
	PSZAC pszFmtTemplate = "<$U";
	UINT uFlagsSerializeMask = ICrypto::F_kfDecrypt;
	if (fSerializeToDisk)
		{
		uFlagsSerializeMask = (ICrypto::F_kfEncrypt | ICrypto::F_kfDecrypt);
		pszFmtTemplate = "<$U" d_szXmlAttributeFlags_ux;
		}
	ICrypto * pCrypto = m_plistCrypto;
	while (pCrypto != NULL)
		{
		if (pCrypto->m_uFlags & uFlagsSerializeMask)
			{
			pbinXmlKeys->BinAppendText_VE(pszFmtTemplate, pCrypto->EGetCryptoClass(), pCrypto->m_uFlags);
			pCrypto->XmlSerialize(IOUT pbinXmlKeys);
			pbinXmlKeys->BinAppendXmlForSelfClosingElement();
			}
		pCrypto = pCrypto->m_pNext;
		}
	}

//	Append the keys at the beginning of the list in the same order as they appear in the XML.
void
CListCrypto::UnserializeKeysFromXml(const CXmlNode * pXmlNodeElementKey, BOOL fUnserializeFromDisk)
	{
	ICrypto ** ppaCrypto = &m_plistCrypto;
	while (pXmlNodeElementKey != NULL)
		{
		ICrypto * paCrypto = ICrypto::S_PaAllocateCrypto_YZ(pXmlNodeElementKey);
		if (paCrypto != NULL)
			{
			paCrypto->XmlUnserialize(pXmlNodeElementKey);
			if (fUnserializeFromDisk)
				{
				pXmlNodeElementKey->UpdateAttributeValueUIntHexadecimal(d_chXmlAttributeFlags, OUT_F_UNCH &paCrypto->m_uFlags);
				}
			else
				{
				// If we are not unserializing from disk, it means we are receiving new keys from the contact, and therefore we need to check for duplicates
				if (FContainsDuplicate(paCrypto, c_cInstancesMax))
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "Ignoring duplicate Crypto: ^N", pXmlNodeElementKey);
					delete paCrypto;
					goto Next;
					}
				paCrypto->m_uFlags |= ICrypto::F_kfEncrypt;	// This key will be used to encrypt the data to send to the contact
				}
			// Add the key to the 'end' of the list
			paCrypto->m_pNext = *ppaCrypto;
			*ppaCrypto = paCrypto;
			ppaCrypto = &paCrypto->m_pNext;	// Pointer to append the next crypto at the end of the linked list
			} // if
		Next:
		pXmlNodeElementKey = pXmlNodeElementKey->m_pNextSibling;
		} // while
	DisplayKeysToMessageLog();
	} // UnserializeKeysFromXml()

void
CListCrypto::DisplayKeysToMessageLog() const
	{
	CBin binKeyData;
	ICrypto * pCrypto = m_plistCrypto;
	while (pCrypto != NULL)
		{
		binKeyData.Empty();
		binKeyData.BinAppendText_VE("\t 0x$p CryptoClass $U", pCrypto, pCrypto->EGetCryptoClass());
		UINT uFlags = pCrypto->m_uFlags;
		if (uFlags & ICrypto::F_kfEncrypt)
			binKeyData.BinAppendText(" F_kfEncrypt");
		if (uFlags & ICrypto::F_kfDecrypt)
			binKeyData.BinAppendText(" F_kfDecrypt");
		uFlags &= ~(ICrypto::F_kfEncrypt | ICrypto::F_kfDecrypt);
		if (uFlags != 0)
			binKeyData.BinAppendText_VE(" uFlags=0x$x", uFlags);
		binKeyData.BinAppendText(": ");
		pCrypto->XmlSerialize(IOUT &binKeyData);

		MessageLog_AppendTextWithNewLine(d_coPurple, binKeyData.BinAppendNullTerminatorSz());
		pCrypto = pCrypto->m_pNext;
		} // while
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef COMPILE_WITH_ICRYPTO_AES_256_GSM
CCryptoAes256gcm::CCryptoAes256gcm()
	{
	m_haContext = NULL;
	InitToGarbage(OUT &m_data, sizeof(m_data));
	}

CCryptoAes256gcm::~CCryptoAes256gcm()
	{
	if (m_haContext != NULL)
		EVP_CIPHER_CTX_free(PA_DELETING m_haContext);
	}

//	Generate a new key to be sent to the contact
void
CCryptoAes256gcm::GenerateKey()
	{
	m_uFlags |= F_kfDecrypt;	// This key will be used to decrypt data received by the remote contact
	InitToGarbage(OUT &m_data, sizeof(m_data));
	Assert(m_haContext == NULL);
	SHashSha1 hashSha1;	// Use a hash as the salt and the data to generate the key
	HashSha1_InitRandom(OUT &hashSha1);
	EVP_BytesToKey(EVP_aes_256_gcm(), EVP_sha1(), IN hashSha1.rgbData, IN hashSha1.rgbData, sizeof(hashSha1), 5, OUT m_data.rgbKey, OUT m_data.rgbIV);
	Assert(m_haContext == NULL);
	MessageLog_AppendTextFormatCo(d_coPurple, "Generating key AES 256 GSM: {p|}, IV={p|}\n", m_data.rgbKey, sizeof(m_data.rgbKey), m_data.rgbIV, sizeof(m_data.rgbIV));
	}

ECryptoError
CCryptoAes256gcm::EEncrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact)
	{
	Assert(pbin != NULL);
	Assert(pbin->CbGetData() > 0);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	if (m_haContext == NULL)
		m_haContext = EVP_CIPHER_CTX_new();
	Assert(m_haContext != NULL);

	if (eEvp_Success == EVP_EncryptInit_ex(m_haContext, EVP_aes_256_gcm(), NULL, IN m_data.rgbKey, IN m_data.rgbIV))
		{
		#ifdef DEBUG_
		int cbKey = EVP_CIPHER_CTX_key_length(m_haContext);
		Assert(cbKey == sizeof(m_data.rgbKey));
		int cbIV = EVP_CIPHER_CTX_iv_length(m_haContext);
		Assert(cbIV == sizeof(m_data.rgbIV));
		#endif
		int cbDataToEncrypt = pbin->CbGetData();
		int cbAlloc = cbDataToEncrypt + c_cbBlockSize;	// Allocate extra memory to include a full block size.  Since the block size is small, there is no need to allocate memory aligned exactly on a block size boundary.
		CBin binEncrypted;
		BYTE * prgbEncrypted = binEncrypted.PbbAllocateMemoryAndEmpty_YZ(cbAlloc);
		InitToGarbage(OUT prgbEncrypted, cbAlloc);
		int cbEncrypted;
		InitToGarbage(OUT &cbEncrypted, sizeof(cbEncrypted));
		if (eEvp_Success == EVP_EncryptUpdate(m_haContext, OUT prgbEncrypted, OUT &cbEncrypted, IN pbin->PbGetData(), cbDataToEncrypt))
			{
			int cbEncryptedExtra;
			InitToGarbage(OUT &cbEncryptedExtra, sizeof(cbEncryptedExtra));
			if (eEvp_Success == EVP_EncryptFinal_ex(m_haContext, IN prgbEncrypted + cbEncrypted, OUT &cbEncryptedExtra))
				{
				cbEncrypted += cbEncryptedExtra;
				//binEncrypted.DataTruncateAtOffset(cbEncrypted);
				pbin->BinInitFromBinaryData(IN prgbEncrypted, cbEncrypted);	// Copy the encrypted data back into the blob
				//MessageLog_AppendTextFormatCo(d_coRed, "Enctypted '$s' $I bytes into $I bytes: {pf} \n", pszText, cbText, cbEncrypted, rgbEncrypted, cbEncrypted);
				return eCryptoError_zSuccess;
				}
			}
		}
	return eCryptoError_Failure;
	}

ECryptoError
CCryptoAes256gcm::EDecrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact)
	{
	Assert(pbin != NULL);
	Assert(pbin->CbGetData() > 0);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	if (m_haContext == NULL)
		m_haContext = EVP_CIPHER_CTX_new();
	Assert(m_haContext != NULL);

	if (eEvp_Success == EVP_DecryptInit_ex(m_haContext, EVP_aes_256_gcm(), NULL, IN m_data.rgbKey, IN m_data.rgbIV))
		{
		#ifdef DEBUG_
		int cbKey = EVP_CIPHER_CTX_key_length(m_haContext);
		Assert(cbKey == sizeof(m_data.rgbKey));
		int cbIV = EVP_CIPHER_CTX_iv_length(m_haContext);
		Assert(cbIV == sizeof(m_data.rgbIV));
		#endif
		int cbDataToDecrypt = pbin->CbGetData();
		int cbAlloc = cbDataToDecrypt + c_cbBlockSize;	// Usually the decrypted block should be smaller, but just in case, add the block size
		CBin binDecrypted;
		BYTE * prgbDecrypted = binDecrypted.PbbAllocateMemoryAndEmpty_YZ(cbAlloc);
		InitToGarbage(OUT prgbDecrypted, cbAlloc);
		int cbDecrypted;
		InitToGarbage(OUT &cbDecrypted, sizeof(cbDecrypted));
		if (eEvp_Success == EVP_DecryptUpdate(m_haContext, OUT prgbDecrypted, OUT &cbDecrypted, IN pbin->PbGetData(), cbDataToDecrypt))
			{
			int cbDecryptedExtra;
			InitToGarbage(OUT &cbDecryptedExtra, sizeof(cbDecryptedExtra));
			if (eEvp_Success == EVP_DecryptFinal_ex(m_haContext, IN prgbDecrypted + cbDecrypted, OUT &cbDecryptedExtra))
				cbDecrypted += cbDecryptedExtra;
			pbin->BinInitFromBinaryData(IN prgbDecrypted, cbDecrypted);	// Copy the decrypted data back into the blob
			return eCryptoError_zSuccess;
			}
		}
	return eCryptoError_Failure;
	}

void
CCryptoAes256gcm::XmlSerialize(IOUT CBin * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeBinaryPvCb(d_chXmlAttributeKey, IN &m_data, sizeof(m_data));
	}

void
CCryptoAes256gcm::XmlUnserialize(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueBinary(d_chXmlAttributeKey, OUT_F_UNCH &m_data, sizeof(m_data));
	}

BOOL
CCryptoAes256gcm::FCompareEqual(const ICrypto * pCryptoCompare) const
	{
	return (0 == memcmp(IN &m_data, &((CCryptoAes256gcm *)pCryptoCompare)->m_data, sizeof(m_data)));
	}
#endif // COMPILE_WITH_ICRYPTO_AES_256_GSM

///////////////////////////////////////////////////////////////////////////////////////////////////
CCryptoOpenTransactions::CCryptoOpenTransactions()
	{

	}

CCryptoOpenTransactions::~CCryptoOpenTransactions()
	{

	}

ECryptoError
CCryptoOpenTransactions::EEncrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact)
	{
	QString receiverNymId = pContact->m_strNymID;
	QString signerNymId = pContact->PGetProfile()->m_strNymID;
	QString qDataStanza = pbin->ToQString();
	//strEncryptedText = pOTX->signAndEncrypt(signerNymId,receiverNymId,qDataStanza).toStdString();
	return eCryptoError_zSuccess;
	}

ECryptoError
CCryptoOpenTransactions::EDecrypt(INOUT_F_UNCH_S CBin * pbin, TContact * pContact)
	{
	QString receiverNymId = pContact->m_strNymID;
	QString signerNymId = pContact->PGetProfile()->m_strNymID;
	QString qDataStanza = pbin->ToQString();
	//bool decryptSuccessful=pOTX->decryptAndVerify(signerNymId,receiverNymId,qDataDecodedSignedEncrypted,decryptedText);
	return eCryptoError_zSuccess;
	}

void
CCryptoOpenTransactions::XmlSerialize(IOUT CBin * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendText_VE(d_szXmlAttributes_OT_strName_strKeyPublic, &m_strNymID, &m_strKeyPublic);
	}

void
CCryptoOpenTransactions::XmlUnserialize(const CXmlNode * pXmlNodeElement)
	{
	m_strNymID = pXmlNodeElement->PszuFindAttributeValue('n');
	m_strKeyPublic = pXmlNodeElement->PszuFindAttributeValue('k');
	}

BOOL
CCryptoOpenTransactions::FCompareEqual(const ICrypto * pCryptoCompare) const
	{
	return m_strNymID.FCompareStringsExactCase(((CCryptoOpenTransactions *)pCryptoCompare)->m_strNymID);	// If the two Nym IDs are identical, then the objects are considered equal
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
ICrypto *
TContact::PGetCrytoForEncrypting_YZ() CONST_MCC
	{
	ICrypto * pCrypto = m_listaCrypto.m_plistCrypto;
	while (pCrypto != NULL)
		{
		if (pCrypto->m_uFlags & ICrypto::F_kfEncrypt)
			return pCrypto;
		pCrypto = pCrypto->m_pNext;
		}
	return NULL;
	}
