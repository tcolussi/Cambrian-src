///////////////////////////////////////////////////////////////////////////////////////////////////
//	CBin.h
//
//	Storage-efficient class to handle binary data.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CBIN_H
#define CBIN_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

/////////////////////////////////////////////////////////////////////
//	class CBin
//
//	Storage-efficient class to handle binary data.  Sometimes in the documentation, this object is called 'blob' which stands for 'Binary Large OBject'.
//	This class is also used to build and transform strings, as a string is a special case of binary data where the end of the string is denoted by a null-terminator.
//
//	To ensure code reliability and remove ambiguity when implementing the class CStr, every method transforming binary data will start with "Bin_".
//	Also, there are two category of methods: Init and Append.  Methods starting with the word Init do initialize the binary object with data provided by the parameter(s),
//	while methods starting with the word Append append to the binary object with the data provided by the parameter(s).
//
//	To make the code easier to write with the auto-complete of modern IDEs, the return types are at the end of the method rather than at the beginning,
//	so the methods are sorted alphabetically by the IDE and making it easier to find a method by functionality.
//	For instance, if a method returns a string, the method will be named GetDataPsz() rather than PszGetData() as it would be in the rest of the code.  After all, almost all the
//	methods for the CBin will return pointer to either a string, or a pointer to an array of bytes, or nothing, so the possible return types are quite limited.
//
//	Pbb		Return a BYTE * to the BEGINNING of the blob.
//	Pbe		Return a BYTE * to the END of the blob.
//	Psz()	The method returns a pointer to a string zero-terminator (this pointer may be NULL)
//	Sz()	The method returns a string zero-terminated (which means the pointer will never be NULL).  The Sz() is a shortcut of the decoration Psz_NZ().
//	Szv()	The method returns a string zero-terminated with a virtual null-terminator.  This applies only to the class CBin, as this class is the building block for the class CStr.
//
class CBin	// bin (or blob)
{
protected:
	struct SHeaderOnly
		{
		int cbAlloc;	// Amount of memory allocated
		int cbData;		// Amount of data in buffer
		};
	struct SHeaderWithData : public SHeaderOnly
		{
		BYTE rgbData[d_nLengthArrayDebug];		// Array of bytes (the true length of the array is determined by cbData)
		};
	SHeaderWithData * m_paData;	// Pointer to allocated buffer

public:
	inline CBin() { m_paData = NULL; }
	CBin(const CBin & bin);
	inline ~CBin() { delete m_paData; }
	BOOL FIsEmptyBinary() const;

	BYTE * PbbAllocateMemoryAndEmpty_YZ(int cbDataAlloc);
	BYTE * PbbAllocateMemoryAndSetSize_YZ(int cbData);
	BYTE * PbbAllocateMemoryToGrowBy_NZ(int cbDataGrowBy);
	BYTE * PbeAllocateExtraMemory(int cbAllocGrowBy);
	BYTE * PbeAllocateExtraMemoryAndSetSize(int cbDataGrowBy);
	BYTE * PbeAllocateExtraDataWithVirtualNullTerminator(int cbDataGrowBy);
	int CbGetData() const;
	int CbGetDataAllocated() const;
	int CbGetDataAllocatedOverhead() const;

	inline const void * PvGetDataUnsafe() const { return m_paData->rgbData; }	// It is the responsibility of the caller to make sure there is some data to the bin
	const void * PvGetData() const;
	BYTE * PbGetData();
	PSZUC PszuGetDataNZ() const;
	PSZAC PszaGetUtf8NZ() const { return (PSZAC)PszuGetDataNZ(); }		// Friendlier method for casting with QString::fromUtf8()

	void BinInitFromBinaryData(const void * pvData, int cbData);
	void BinInitFromBinaryDataWithVirtualNullTerminator(const void * pvData, int cbData);
	void BinInitFromBinaryDataWithExtraVirtualZeroes(const void * pvData, int cbData, int cbExtraVirtualZeroes);
	void BinInitFromCBin(const CBin * pbinSrc);
	void BinInitFromCBinWithVirtualNullTerminator(const CBin * pbinSrc);
	void BinInitFromCBinStolen(INOUT CBin * pbinSrc);
	void BinInitFromCBinSwapped(INOUT CBin * pbinSrc);
	void BinInitFromByteArray(const QByteArray & arraybData);
	void BinInitFromByte(BYTE bData);
	PSZU BinInitFromTextSzv_VE(PSZAC pszFmtTemplate, ...);
	PSZU BinInitFromTextSzv_VL(PSZAC pszFmtTemplate, va_list vlArgs);
	void BinInitFromText(PSZAC pszText);
	void BinInitFromStringWithNullTerminator(PSZAC pszData);
	void BinAppendStringWithNullTerminator(PSZAC pszString);
	void BinAppendText(PSZAC pszText);

	void BinAppendBinaryData(const void * pvData, int cbData);
	void BinAppendBinaryDataPvPv(const void * pvDataStart, const void * pvDataStop);
	void BinAppendCStr(const CStr & strSrc);
	void BinAppendCBin(const CBin & binSrc);
	void BinAppendCBinLowercaseAscii(const CBin & binSrc);
	void BinAppendCBinFromOffset(const CBin & binSrc, int ibDataBegin);
	void BinAppendByteArray(const QByteArray & arraybData);
	void BinAppendByte(UINT bData);
	void BinAppendBytes(UINT bData, int cCount);
	void BinAppendBytesEmpty(int cbDataEmpty);
	void BinAppendNullTerminator() { BinAppendByte('\0'); }
	PSZUC BinAppendNullTerminatorVirtualSzv() CONST_TEMPORARY_MODIFIED;
	PSZUC BinAppendNullTerminatorSz();
	PSZUC BinAppendNullTerminatorVirtualAndEmptySzv();
	void BinAppendCRLF() { BinAppendBinaryData(c_szCRLF, 2); }
	void BinAppendUInt16(UINT wData);
	void BinAppendUInt32(UINT uData);
	void BinAppendZeroBytesUntilAligned32();
	void BinAppendTextInteger(int nInteger, UINT uFlagsITS = 0);
	void BinAppendTextBytesKiB(L64 cbBytes);
	void BinAppendTextBytesKiBPercent(L64 cbBytesReceived, L64 cbBytesTotal);
	void BinAppendTextBytesKiBPercentProgress(L64 cbBytesReceived, L64 cbBytesTotal);
	void BinAppendTimestamp(TIMESTAMP ts);
	void BinAppendTimestampSpace(TIMESTAMP ts);
	PSZR BinAppendTextUntilCharacterPszr(PSZUC pszuSource, UINT chCopyUntil);
	void BinAppendTextFromCharacter(PSZUC pszuSource, UINT chCopyFrom);
	void BinUtf8AppendStringW(PSZWC pszwString);
	void BinUtf8AppendStringQ(const QString & sString);
	PSZU BinAppendHtmlHyperlinkFileSzv(UINT uTaskId, PSZUC pszFilename, BOOL fDisabled = FALSE);
	PSZU BinAppendHtmlButtonSzv(UINT uTaskId, UINT chTaskActionButton);
	PCHRO BinAppendHtmlHyperlinkPchro(PCHC pchTextBeforeHyperlink, PCHC pchHyperlinkBegin, EScheme eScheme);
	void BinAppendHtmlTextWithAutomaticHyperlinks(PSZUC pszText);
	void BinAppendHtmlListItems(const CArrayPsz & arraypszListItems);
	void BinAppendHtmlTextWithinTag(PSZAC pszTagHtml, PSZUC pszText);
	void BinAppendHtmlTextCch(PCHUC pchText, int cchuText);
	void BinAppendHtmlTextUntilPch(PCHUC pchTextBegin, PCHUC pchTextEnd);
	void BinAppendUrlPercentEncode(PSZUC pszUrl);
	void BinAppendXmlTextU(PSZUC pszText);
	void BinAppendXmlTextStr(const CStr & strText);
	void BinAppendXmlTextW(PSZWC pszwText);
	void BinAppendXmlAttributeUInt(PSZAC pszAttributeName, UINT uAttributeValue);
	void BinAppendXmlAttributeUInt(CHS chAttributeName, UINT uAttributeValue);
	void BinAppendXmlAttributeUIntHexadecimal(CHS chAttributeName, UINT uAttributeValueHexadecimal);
	void BinAppendXmlAttributeInt(CHS chAttributeName, int nAttributeValue);
	void BinAppendXmlAttributeL64(PSZAC pszAttributeName, L64 lAttributeValue);
	void BinAppendXmlAttributeL64(CHS chAttributeName, L64 lAttributeValue);
	void BinAppendXmlAttributeText(CHS chAttributeName, PSZUC pszAttributeValue);
	void BinAppendXmlAttributeText(PSZAC pszAttributeName, PSZUC pszAttributeValue);
	void BinAppendXmlAttributeTimestamp(CHS chAttributeName, const TIMESTAMP & tsAttributeValue);
	void BinAppendXmlAttributeCStr(CHS chAttributeName, const CStr & strAttributeValue);
	void BinAppendXmlAttributeCStr2(CHS chAttributeName, const CStr & strAttributeValue, PSZUC pszAttributeValuePriority = NULL);
	void BinAppendXmlAttributeCBin(CHS chAttributeName, const CBin & binAttributeValue);
	void BinAppendXmlAttributeBinaryPvCb(CHS chAttributeName, const void * pvData, int cbData);
	void BinAppendXmlAttributeOfContactIdentifier(CHS chAttributeName, const TContact * pContact);
	void BinAppendXmlElementText(PSZAC pszElementName, PSZUC pszElementValue);
	void BinAppendXmlElementText(PSZAC pszElementName, WEdit * pwEdit);
	void BinAppendXmlElementInt(PSZAC pszElementName, int nElementValue);
	void BinAppendXmlElementBinaryBase64(PSZAC pszElementName, const CBin & binElementValue);
	void BinAppendXmlForSelfClosingElement();
	void BinAppendXmlForSelfClosingElementQuote();
	void BinAppendXmlForSelfClosingElementQuoteTruncateAtOffset(const SOffsets * pOffsets);
	void BinAppendXmlClosingElement_TruncateIfEmpty(const SOffsets * pOffsets, CHS chXmlElementName);

	void BinAppendXmlNode(const CXmlNode * pXmlNode);
	void BinAppendXmlNodeNoWhiteSpaces(const CXmlNode * pXmlNode);
	void BinAppendDataEncoded(const void * pvData, int cbData, UINT chEncoding);
	void BinAppendTextOffsetsInit_VE(OUT SOffsets * pOffsets, PSZAC pszFmtTemplate, ...);
	void BinAppendTextOffsetsInitXmlElement(OUT SOffsets * pOffsets, CHS chXmlElementName);
	void BinAppendTextOffsetsTruncateIfEmpty_VE(IN const SOffsets * pOffsets, PSZAC pszFmtTemplate, ...);
	void BinAppendText_VE(PSZAC pszFmtTemplate, ...);
	PSZU BinAppendTextSzv_VL(PSZAC pszFmtTemplate, va_list vlArgs);
	void BinAppendStringBase16FromBinaryData(const void * pvBinaryData, int cbBinaryData);
	void BinAppendStringBase16FromBinaryData(const CBin & bin);
	void BinAppendTextFormattedFromFingerprintBinary(const void * pvFingerprintBinary, int cbFingerprintBinary);
	BOOL BinAppendBinaryFingerprintF_MB(PSZUC pszTextFingerprint) OUT_F_UNCH;
	BOOL BinAppendBinaryFromTextBase16F(PSZUC pszTextBase16);
	void BinAppendStringBase41FromBinaryData(const void * pvDataBinary, int cbDataBinary);
	void BinAppendStringBase64FromBinaryData(const void * pvDataBinary, int cbDataBinary);
	void BinAppendStringBase64FromBinaryData(const QByteArray & arraybData);
	void BinAppendStringBase64FromBinaryData(const CBin * pbinData);
	void BinAppendStringBase64FromStringU(PSZUC pszuString);
	void BinAppendStringBase85FromBinaryData(const void * pvDataBinary, int cbDataBinary);
	void BinAppendStringBase85FromBinaryData(const CBin * pbinData);

	void  BinAppendBinaryDataFromBase64(const QString & sBase64);
	void  BinAppendBinaryDataFromBase64(const QByteArray & arraybDataBase64);
	PSZUC BinAppendBinaryDataFromBase64Szv(PSZUC pszuBase64);
	int   BinAppendBinaryDataFromBase85SCb_ML(PSZUC pszuBase85);
	PSZUC BinAppendBinaryDataFromBase85Szv_ML(PSZUC pszuBase85);
	PSZUC BinAppendTextVirtualSzv_VL(OUT int * pcbDataFormatted, IN PSZAC pszFmtTemplate, va_list vlArgs) CONST_TEMPORARY_MODIFIED;
	PSZUC PszAppendVirtualXmlAttributes(PSZAC pszFmtTemplateAttributes, ...) CONST_TEMPORARY_MODIFIED;
	void BinEnsureContentHasNoNullTerminatorAndIsTerminatedWithVirtualNullTerminator();
	BOOL FContainsXmlNode() const;

	BOOL FIsPointerAddressWithinBinaryObject(const void * pvData) const;

	int CbGetDataAfterOffset(int ibData, int cbDataMax, OUT int * pcbDataRemaining) const;
	PVOID PvGetDataAtOffset(int ibData) const;
	PVOID DataTruncateAtOffsetPv(UINT cbDataKeep);
	void DataTruncateAtOffset(UINT cbDataKeep);
	void DataTruncateAtPointer(const void * pvDataEnd);
	void DataRemoveUntilPointer(const void * pvDataStart);
	void RemoveData(int ibData, int cbData);
	void RemoveBytesFromEnd(int cbData);
	void InsertData(int ibData, const void * pvData, int cbData);
	void InsertDataAtBeginning(const void * pvData, int cbData);
	void InsertByteAtBeginning(BYTE bData);
	int  IbFindData(const void * pvData, int cbData) const;
	int  IbFindDataAfterString(PSZAC pszString) const;
	void SubstituteData(int ibRepl, int cbRepl, const void* pvSubst, int cbSubst);

	BOOL FCompareBinary(const CBin & binCompare) const;
	BOOL FCompareBinary(const QByteArray & arraybCompare) const;
	BOOL FCompareFingerprint(const SHashSha1 & hashSha1) const;
	BOOL FCompareFingerprintWithCertificateBinaryData(const CSslCertificate & oCertificate) const;
	BOOL FCompareFingerprintWithCertificateBinaryData(const CBin & binCertificateBinaryData) const;
	BOOL BinAppendBinaryFingerprintF_MB(const QLineEdit * pwEdit);
	void operator = (const CBin & binSrc);
	inline const CBin & operator = (const QByteArray & arrayb) { BinInitFromBinaryData(arrayb.constData(), arrayb.size()); return *this; }
	operator QByteArray() const;
	QByteArray ToQByteArrayDeepCopy() const;
	QByteArray ToQByteArrayShared() const;
	QString ToQString() const;
	void Empty();
	void FreeExtra();
	void FreeBuffer();
	void GarbageInitializedUnusedBytes() const;

	PCHU PchFindCharacter(UINT ch) const;

	EError BinFileReadE(const QString & sFileName);
	EError BinFileWriteE(const QString & sFileName, QIODevice::OpenModeFlag uFlagsExtra = QIODevice::NotOpen) const;

	SBlobPvCb GetBlobAfterOffset(int ibData) const;
	SBlobPvCb GetBlobAfterOffsetForXmppStanza(const CXmlNode * pXmlNodeStanza) const;
	void DataEncryptAES256(const SHashKey256 * pKey);
	void DataDecryptAES256(const SHashKey256 * pKey);

	void CalculateHashMd5FromFormattedString_VL(OUT SHashMd5 * pHashMd5, PSZAC pszFmtTemplate, va_list vlArgs) CONST_TEMPORARY_MODIFIED;
	BOOL FAssignFileSizeAndMd5(INOUT_F_UNCH SFileSizeAndMd5 * pFileSizeAndMd5) const;
	void BinInitFromCalculatingHashSha1(const CBin & binData);
	void BinHashToMd5();

private:
	static SHeaderWithData * S_PaAllocateBytes(int cbDataAlloc);
	friend class CStr;	// To access CBin::m_paData
}; // CBin

#endif // CBIN_H
