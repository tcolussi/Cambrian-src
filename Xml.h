/////////////////////////////////////////////////////////////////////
//	Xml.h
//
//	Classes to parse an XML stream/file.
//	The motivation for having this custom XML parser is flexbility and performance.
//	The QDomDocument is grossly inefficient both in terms of memory requirement and CPU cycles,
//	difficult to use and does not allow friendly inheritance since every QDomNode must be copied on the stack.
//
/////////////////////////////////////////////////////////////////////

#ifndef XML_H
#define XML_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CXmlTree;

/*
//	Class to speed up the sorting of XML nodes
class CXmlSortContext
{
public:
	CArrayEx m_array;
	CAllocator m_allocator;
	PFn_PSortKeyAllocateForElement m_pfnPSortKeyAllocateForElement;
	PFn_NCompareSortElements m_pfnSortCompare;
};

*/

/////////////////////////////////////////////////////////////////////
//	Class to store a single XML element or attribute
class CXmlNode		// pXmlNode
{
protected:
	enum // Flags for the m_lParam during the parsing
		{
		kmLineNumberMask		= 0x00FFFFFF,
		kfValueDirty			= 0x10000000	// A dirty flag which may useful to know which node have been modified.
		};
public:
	enum // Flags for uSerializeFlags, such as methods SerializeToUnicode() and EStreamNodesToUtf8().
		{
		STBF_kmIdentationMask	= 0x00FFFFFF,
		STBF_kfIndentTags		= 0x01000000,	// Indent the tags with tabs
		STBF_kfCRLF				= 0x02000000,	// Insert CRLF between elements
		STBF_kfNoHeaderXml		= 0x04000000,	// Do not write "<?xml version='1.0' encoding='utf-8'?>" at the beginning of the XML.  Sometimes the header is unnecessary, and other times it may cause an error if we are attempting to append multiple XML nodes into the same stream.
		STBF_kfNoClosingTag		= 0x08000000,	// Do not write the closing tag.  This is to save some storage space.  Although this feature is working, it is never used.
		STBF_kfDirtyNodesOnly	= 0x10000000,	// Serialize only the nodes marked as dirty (kfValueDirty)
		STBF_kfDebugLParam		= 0x80000000,	// Write the content of the m_lParam.  This is for debugging purpose only
		STBF_kmAssertValid		= 0x9700000F	// Typical bits allowed.  This is for debugging to avoid setting a wrong bit as uSerializeFlags
		};

public:
	PSZU m_pszuTagName;					// Tag name of the XML element or attribute (eg: "Name", "ID")
	PSZU m_pszuTagValue;				// Content of the tag value in UTF-8 format.  This pointer is never NULL for an attribute, however may be NULL for a self-closing element.
	LPARAM m_lParam;					// User-defined parameter.  This value is initialized to the line number by the parser
	PVOID m_pvParamExtra;				// Extra user-defined parameter for performance.  This value is not initialized, so it is the caller responsible to initialize if necessary.  Can be useful for sorting or things like.
	CXmlNode * m_pParent;				// The parent of this XML node, or NULL if the node is the root of the tree
	CXmlNode * m_pNextSibling;			// Next sibling node in the linked list
	CXmlNode * m_pAttributesList;		// Linked list of the attributes
	CXmlNode * m_pElementsList;			// Linked list of elements/children

public:
	CXmlTree * PFindRootNodeXmlTreeParent() const;
	CMemoryAccumulator * PFindMemoryAccumulatorOfXmlTreeParent() const;
	BOOL FCompareTagName(PSZAC pszuTagName) const;
	CXmlNode * PFindElementOrAttribute(PSZAC pszuNodePath) const;
	CXmlNode * PFindElement(PSZAC pszaElementName) const;
	CXmlNode * PFindElement(CHS chElementName) const;
	CXmlNode * PFindElementMatchingAttributeNameAndValue(PSZAC pszAttributeName, PSZAC pszAttributeValue) const;
	CXmlNode * PFindElementRemove(PSZAC pszaElementName);
	CXmlNode * PFindAttribute(PSZAC pszaAttributeName) const;
	CXmlNode * PFindAttribute(CHS chAttributeName) const;
	CXmlNode * PFindNextSibling() const;

	//PSZUC PszuFindElementOrAttributeValue(PSZAC pszuTagName) const;

	VOID Clear() { InitToZeroes(OUT this, sizeof(*this)); }
	VOID SetAttributesListReverse(CXmlNode * pAttributesList);
	VOID AddAttributesListReverse(CXmlNode * pAttributesList);
	VOID ReverseLinkedListsElementsOnly();
	VOID ReverseLinkedListsElementsAndAttributes();
	VOID ReverseLinkedListAttributes();
	VOID MoveAttributeToHead(PSZAC pszaAttributeName);
	void RemoveEmptyElements();
	BOOL FIsEmptyElement() const;

	void SerializeToBinUtf8(IOUT CBin * pbinUtf8, UINT uSerializeFlags = 0) const;

	BOOL FFindAttributeAndCompareValue(PSZAC pszaAttributeName, PSZUC pszuAttributeValue) const;
	BOOL FFindElementAndCompareValue(PSZAC pszaNameElement, PSZUC pszuValueElementCompare) const;
	BOOL FFindElementAndCompareValue_true(PSZAC pszaNameElement) const;

	PSZUC PszuFindAttributeValue(CHS chAttributeName) const;
	PSZUC PszuFindAttributeValue(PSZAC pszaAttributeName) const;
	PSZUC PszuFindAttributeValue_NZ(CHS chAttributeName) const;
	PSZUC PszuFindAttributeValue_NZ(PSZAC pszaAttributeName) const;
	PSZUC PszuFindAttributeValue_ZZ(PSZAC pszaAttributeName) const;
	PSZUC PszuFindElementValue(PSZAC pszaElementName) const;
	PSZUC PszuFindElementValue_NZ(PSZAC pszaElementName) const;
	PSZUC PszuFindElementValue_ZZ(PSZAC pszaElementName) const;
	PSZUC PszuFindElementOrAttributeValue(PSZAC pszuNodePath) const;
	PSZUC PszuFindElementOrAttributeValue_NZ(PSZAC pszuNodePath) const;
	PSZUC PszuFindElementOrAttributeValue_ZZ(PSZAC pszuNodePath) const;
	L64   LFindAttributeValueDecimal_ZZR(CHS chAttributeName) const;
	L64   LFindAttributeValueDecimal_ZZR(PSZAC pszaAttributeName) const;
	UINT_P UFindAttributeValueDecimal_ZZR(PSZAC pszaAttributeName) const;
	UINT_P UFindAttributeValueDecimal_ZZR(CHS chAttributeName) const;
	UINT_P UFindAttributeValueHexadecimal_ZZR(PSZAC pszaAttributeName) const;
	UINT_P UFindAttributeValueHexadecimal_ZZR(CHS chAttributeName) const;
	INT_P NFindElementOrAttributeValueNumeric(PSZAC pszuNodePath) const;
	INT_P GetElementOrAttributeNumericValue(PSZAC pszuNodePath) const;
	INT_P GetElementOrAttributeNumericValue(PSZAC pszuNodePath, int iError) const;
	TIMESTAMP TsGetAttributeValueTimestamp_ML(CHS chAttributeName) const;
	void GetAttributeValueHashSha1_ML(PSZAC pszaAttributeName, OUT_ZZR SHashSha1 * pHashSha1) const;
	void GetAttributeValueDateTime_ML(PSZAC pszaAttributeName, OUT_ZZR QDateTime * pDateTime) const;
	void GetAttributeValueL64(PSZAC pszaAttributeName, OUT_ZZR L64 * plValue) const;
	void GetAttributeValueCStr(PSZAC pszaAttributeName, OUT CStr * pstrValue) const;
	void UpdateAttributeValueInt(CHS chAttributeName, OUT_F_UNCH int * pnValue) const;
	void UpdateAttributeValueUInt(CHS chAttributeName, OUT_F_UNCH UINT * puValue) const;
	void UpdateAttributeValueUIntHexadecimal(CHS chAttributeName, OUT_F_UNCH UINT * puValue) const;
	void UpdateAttributeValueL64(CHS chAttributeName, OUT_F_UNCH L64 * plValue) const;
	void UpdateAttributeValueTimestamp(CHS chAttributeName, OUT_F_UNCH TIMESTAMP * ptsValue) const;
	void UpdateAttributeValueCStr(CHS chAttributeName, OUT_F_UNCH CStr * pstrValue) const;
	void UpdateAttributeValueCBin(CHS chAttributeName, OUT_F_UNCH CBin * pbinValue) const;
	BOOL UpdateAttributeValueHashSha1(CHS chAttributeName, OUT_F_UNCH SHashSha1 * pHashSha1) const;

	inline UINT GetLineNumber() const { return (m_lParam & kmLineNumberMask); }
	VOID SetDirty();
	VOID SetDirtyAttributeID();
	VOID RemoveDirty() { m_lParam &= ~kfValueDirty; }
	VOID RemoveDirtyR();
	UINT UGetTotalDirtyNodes(const CXmlNode * pXmlNodeOriginal, PSZAC pszaAttributeNameSkip) const;
	BOOL FuIsDirty() const { return (m_lParam & kfValueDirty); }
	BOOL FuIsDirtyAttributeValue(PSZAC pszaAttributeName) const;
	BOOL FIsNodeComment() const;

	CXmlNode * PRemoveNode(PSZAC pszuNodePath);
	VOID RemoveFromParent();
	BOOL FIsNodeEmpty(PSZAC pszaAttributeID) const;
/*
	static VOID _SortNodes(INOUT CXmlNode ** ppNodeHead, INOUT CXmlSortContext * pXmlSortContext);
	VOID _SortNodesR(INOUT CXmlSortContext * pXmlSortContext);
	VOID SortElements(PVOID * pvSortContext);
	VOID SortAttributes(PVOID * pvSortContext);
	VOID SortSiblingsByLineNumber(INOUT CArrayEx * parrayDummy, INOUT CAllocator * pAllocatorDummy);
	VOID SortNodesByName();
	VOID SortNodesByAttribute(PSZAC pszAttributeName);
	VOID SortNodesByAttributeID();
*/
	void UpdateAttributeValueToStaticString(PSZAC pszaAttributeName, PSZAC pszAttributeValue);
	void UpdateAttributeValuesSwap(PSZAC pszaAttributeNameA, PSZAC pszaAttributeNameB);
	void UpdateElementValueWithFormattedString_VL_Gsb(PSZAC pszaElementName, PSZAC pszFmtTemplateValue, va_list vlArgs);

public:
	// Wrappers specific to the chat application
	CXmlNode * PFindElementQuery() const;
	CXmlNode * PFindElementIq() const;
	CXmlNode * PFindElementError() const;
	CXmlNode * PFindElementErrorStanza() const;
	CXmlNode * PFindElementData() const;
	CXmlNode * PFindElementText() const;
	CXmlNode * PFindElementSi() const;
	CXmlNode * PFindElementClose() const;
	CXmlNode * PFindElementMatchingAttributeValueXmlns(PSZAC pszAttributeValue) const;
	PSZUC PszFindElementValueText() const;
	PSZUC PszFindElementValueStanzaErrorText() const;
	PSZUC PszFindElementStanzaErrorDescription() const;
	PSZAC PszFindStanzaErrorDescription() const;
	PSZUC PszFindStanzaErrorElementName() const;

	TIMESTAMP LFindAttributeValueIdTimestamp_ZZR() const;
	UINT UFindAttributeValueStanzaId_ZZR() const;
	UINT UFindAttributeValueId_ZZR() const;
	PSZUC PszFindAttributeValueId_NZ() const;
	PSZUC PszFindStanzaValueQueryXmlns_NZ() const;
	PSZUC PszFindAttributeValueXmlns_NZ() const;
	PSZUC PszFindAttributeValueType_NZ() const;
	PSZUC PszFindAttributeValueTypeNoError() const;
	PSZUC PszFindAttributeValueJid_NZ() const;
	PSZUC PszFindAttributeValueSid_NZ() const;
	PSZUC PszFindAttributeValueFrom_NZ() const;
	PSZUC PszFindAttributeValueTo_NZ() const;

	L64 LFindAttributeXcpOffset() const;

}; // CXmlNode


class CArrayPtrXmlNodes : public CArray
{
public:
	inline CXmlNode ** PrgpGetXmlNodesStop(OUT CXmlNode *** pppXmlNodeStop) const { return (CXmlNode **)PrgpvGetElementsStop((void ***)pppXmlNodeStop); }
	inline CXmlNode * PGetXmlNodeAt(int iNode) const { return (CXmlNode *)PvGetElementAt(iNode); }
};


/////////////////////////////////////////////////////////////////////
//	Class to store the entire XML tree by parsing a file into nodes.
//	The XML comments are removed from this file, unless the flag FXF_mskfKeepComments is set.
//
//	IMPLEMENTATION NOTES
//	The parsed XML tree is stored into a single root node.
//	This is why the object CXmlTree inherits CXmlNode because an entire XML file is in fact a special case of an XML node (the root node)
//
class CXmlTree : public CXmlNode	// pXmlTree
{
protected:
	enum
		{
		FXF_mskfKeepComments				= 0x000010000	// Do not discard the comments and XML directives.  This is useful for writing back the XML to file.
		};
	UINT m_uFlagsXmlFile;					// IN: Flags of the XML file
public:
	CMemoryAccumulator m_accumulatorNodes;	// Memory accumulator to store all the XML nodes, and sometimes the tag values
	CBin m_binXmlFileData;					// IN: Content of the XML file to parse (or to save to disk).  To improve performance, the parsing will modify this object by inserting null-terminators between elements and attributes.
	SFileSizeAndMd5 * m_pFileSizeAndMd5;	// INOUT: File size and MD5 of the XML.  Useful to determine if an XML has changed and must be saved to disk.

public:
	CXmlTree();
	~CXmlTree();
	VOID Destroy();
	void DestroyLargeBuffers();
	BOOL FIsEmpty() const;
	void SetFileDataCopy(const CBin & binXmlFileData);
	void SetFileDataCopy(const QByteArray & arraybXmlFileData);
	void SetFileDataCopy(PSZUC pszuXmlFileDataStart, PSZUC pszuXmlFileDataStop);

	EError ELoadFile(const QString & sFileName);
	EError ELoadFileAndParseToXmlNodes(const QString & sFileName, INOUT_F_VALID CErrorMessage * pError);
	EError EParseFileDataToXmlNodes(INOUT_F_VALID CErrorMessage * pError, INOUT PSZU pszuXmlFileDataToParse = NULL);
	EError EParseFileDataToXmlNodesModify_ML(INOUT CBin * pbinXmlFileData);
	EError EParseFileDataToXmlNodesCopy_ML(IN const CBin & binXmlFileData);
	EError EParseFileDataToXmlNodesCopy_ML(IN const QByteArray & arraybXmlFileData);
	EError EParseFileDataToXmlNodes_ML();
	/*
	CXmlNode * PParseFileDataToXmlNodesAndValidateRootNodeUI(HWND hwndEditLog, PSZAC pszuNodeRoot);
	BOOL FParseFileDataIntoEditWindowU(IN PSZUC pszuFileData, HWND hwndEdit, UINT uSerializeFlags, HWND hwndChecksum);
	BOOL FParseFileDataIntoEditWindow(IN const CBin * pbinFileData, HWND hwndEdit, UINT uSerializeFlags, HWND hwndChecksum);
	VOID ParseXmlNodesWithoutError(PSZUC pszuXmlNodes, int cbXmlNodes);

	UINT CbGetEstimatedStorageForSerialization() const;
	*/
	VOID SerializeTreeToXml(UINT uSerializeFlagsExtra = 0) CONST_OUTPUT_BUFFER;
	EError EWriteToFileW(PSZWC pszwFileName) const;
/*
	inline VOID KeepComments() { m_uFlagsXmlFile |= FXF_mskfKeepComments; }
	inline VOID KeepCommentsAndConsecutiveWhiteSpaces() { m_uFlagsXmlFile |= (FXF_mskfKeepComments | FXF_mskfKeepConsecutiveWhiteSpaces); }
	inline VOID KeepConsecutiveWhiteSpaces() { m_uFlagsXmlFile |= FXF_mskfKeepConsecutiveWhiteSpaces; }
*/

	CXmlNode * PGetRootNodeValidate(PSZAC pszuNameRootNode) const;
	//CXmlNode * PGetRootNodeValidateUI(PSZAC pszuNameRootNode, HWND hwndEditLog) const;
	VOID RootNodeEnsureNotEmpty();

	VOID DuplicateNodes(const CXmlNode * pNodesSrc);
	VOID CloneNodesForDeletedValues(INOUT CXmlNode * pNodeDst, const CXmlNode * pNodesDeleted);
	VOID _CloneNodes(INOUT CXmlNode * pNodeDst, const CXmlNode * pNodesSrc);
	CXmlNode * _PAppendNodesAfter(INOUT CXmlNode * pNodeDst, const CXmlNode * pNodesSrc);

	CXmlNode * _PAllocateElementOrAttribute(PSZAC pszaElementOrAttributeName);
	CXmlNode * _PAllocateElement(CXmlNode * pNodeElementParent, PSZAC pszaElementName);
	CXmlNode * _PAllocateAttribute(CXmlNode * pNodeElementParent, PSZAC pszaAttributeName, PSZUC pszuAttributeValue);
	CXmlNode * _PAllocateAttributeWithExtraStorage(CXmlNode * pNodeElementParent, PSZAC pszaAttributeName, int cbAllocExtra);

private:
	VOID _InitBuffersForFileParsing();
	VOID _AppendNodeTagValue(INOUT CXmlNode * pNode, INOUT PSZU pszuTagValue, BOOL fCDATA = FALSE);

}; // CXmlTree


/////////////////////////////////////////////////////////////////////
//	typedef PFn_PaAllocateXmlObject()
//
//	Interface for a function capable of allocating an object inheriting the interface IXmlExchange.
//
//	The parameter pvContextAllocate is optional.  It is typically a pointer to the parent object or a structure
//	containing information to initialize the allocated object in its constructor.
//
typedef IXmlExchange * (* PFn_PaAllocateXmlObject)(PVOID pvContextAllocate);

typedef IXmlExchange * (* PFn_PaAllocateXmlObject2_YZ)(PVOID pvContextAllocate, const CXmlNode * pXmlNodeElement);

class CXmlExchanger : public CXmlTree
{
public:
	CMemoryAccumulator m_accumulatorText;	// Memory accumulator to store data when serializing.  This is typically for binary values and numbers which must be converted to a string before being assinged to m_pszuTagValue.
	CErrorMessage m_error;
	BOOL m_fSerializing;				// TRUE => Do the serialization (save the XML elements), FALSE => Unserialize (create XML elements from an XML stream/file)
	UINT m_uVersionFileFormat;			// What version of the XML format/schema is being unserialized
protected:
	CXmlNode * m_pXmlNodeSerialize;		// Current XML node to be serialized
	CArrayPtrXmlNodes m_arraypStack;	// Stack recording the history of XML nodes for serialization

public:
	CXmlExchanger(BOOL fSerialize = TRUE);

	EError ELoadFileAndParseToXmlNodes(const QString & sFileName);

	void StackPushNode(CXmlNode * pXmlNodeSerialize);
	CXmlNode ** PpStackPushNodes(CXmlNode * pXmlNodeSerialize);
	void StackPopNodes();
	void StackPopNode();
	PSZUC XmlExchange_PszGetValue(PSZAC pszuTagName) const;

	BOOL XmlExchangeNodeRootF(PSZAC pszuNameRootNode);
	void XmlExchangeElementBegin(PSZAC pszuElementName);
	void XmlExchangeElementEnd(PSZAC pszuElementName);
	void XmlExchangeBinary(PSZAC pszuTagName, INOUT_ZZR BYTE prgbDataBinary[], int cbDataBinary);
	void XmlExchangeBin(PSZAC pszuTagName, INOUT CBin * pbinValue);
	void XmlExchangeStrBase85(PSZAC pszuTagName, INOUT CStr * pstrValue);
	void XmlExchangeStr(PSZAC pszuTagName, INOUT CStr * pstrValue);
	void XmlExchangeStrConditional(PSZAC pszuTagName, INOUT CStr * pstrValue, BOOL fuConditionToSerialize);
	void XmlExchangeInt(PSZAC pszuTagName, INOUT_F_UNCH_S int * pnValue);
	void XmlExchangeUInt(PSZAC pszuTagName, INOUT_F_UNCH_S UINT * puValue);
	void XmlExchangeUIntHex(PSZAC pszuTagName, INOUT_F_UNCH_S UINT * puValueHex);
	void XmlExchangeUIntHexFlagsMasked(PSZAC pszuTagNameFlags, INOUT UINT * puFlags, UINT uFlagsSerializeMask);
	void XmlExchangeUShort(PSZAC pszuTagName, INOUT_F_UNCH_S quint16 * pusValue);
	void XmlExchangeGuid(PSZAC pszuTagNameGuid, INOUT_F_UNCH_S GUID * pGuid);
	void XmlExchangeSha1(PSZAC pszuTagNameSha1, INOUT_F_UNCH_S SHashSha1 * pHashSha1);
	void XmlExchangeTimestamp(PSZAC pszuTagNameTimestamp, INOUT_F_UNCH_S TIMESTAMP * ptsValue);
	void XmlExchangeDateTime(PSZAC pszuTagNameDateTime, INOUT_F_UNCH_S QDateTime * pDateTime);
	void XmlExchangeObjects(CHS chTagNameObjects, INOUT_F_UNCH_S CArrayPtrXmlSerializable * parraypaObjects, PFn_PaAllocateXmlObject pfnPaAllocatorObject, PVOID pvContextAllocate = d_zNA);
	void XmlExchangeObjects(PSZAC pszuTagNameObjects, PSZAC pszuTagNameObject, INOUT_F_UNCH_S CArrayPtrXmlSerializable * parraypaObjects, PFn_PaAllocateXmlObject pfnPaAllocatorObject, PVOID pvContextAllocate = d_zNA);
	void XmlExchangeObjects2(CHS chTagNameObjects, INOUT_F_UNCH_S CArrayPtrXmlSerializable * parraypaObjects, PFn_PaAllocateXmlObject2_YZ pfnPaAllocatorObject2_YZ, PVOID pvContextAllocate = d_zNA);
	void XmlExchangePointer(CHS chTagNamePointer, INOUT_F_UNCH_S IXmlExchangeObjectID ** ppObject, const CArrayPtrXmlSerializableObjects * parraypObjectsLookup);
	void XmlExchangePointers(PSZAC pszuTagNamePointers, INOUT_F_UNCH_S CArrayPtrXmlSerializableObjects * parraypObjects, const CArrayPtrXmlSerializableObjects & arraypObjectsLookup);
	void XmlExchangeWriteAttribute(PSZAC pszNameAttribute, PSZUC pszAttributeValue);
	void XmlExchangeWriteAttributeUSZU(PSZAC pszNameAttribute, USZU uszuAttributeValue);
	void XmlExchangeWriteAttributeRtiSz(PSZAC pszNameAttribute, RTI_ENUM rtiAttributeValue);

protected:
	void AllocateAttributePsz(PSZAC pszaAttributeName, PSZUC pszValue);
	void AllocateAttributeValueCopyPsz(PSZAC pszaAttributeName, PSZUC pszValue);
	void AllocateAttributeValueInteger(PSZAC pszaAttributeName, int nValue);
	void AllocateAttributeValueGuid(PSZAC pszaAttributeName, const GUID * pGuid);
	void AllocateAttributeValueStringQ(PSZAC pszaAttributeName, const QString & sString);
	void AllocateAttributeValueCBinString(PSZAC pszaAttributeName, IN_MOD_TMP CBin & binString);
}; // CXmlExchanger

#define PPX		(IXmlExchangeObjectID **)	// The purpose of this macro is suppressing the compiler error: warning: invalid conversion from 'ITreeItem**' to 'IXmlExchangeObjectID**' [-fpermissive]

#define d_uVersionFileFormat0			0	// The very first file format where the filenames of the chat history was stored using a random GUID and binary data was stored in Base64.  This value is implicit, as there is no attribute "Version='0'"
#define d_szVersionFileFormatCurrent	"1"	// Current file format: All binary data is stored in Base85 and the filenames are determined from a SHA-1 of the JIDs of the account and the contact.
#define d_chXmlAttributeVersion			'v'

/*
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
class CHashElementXmlNode : public CHashElement
{
public:
	PSZUC m_pszuNodeValue;		// This is the hash key
	CXmlNode * m_pNodeXML;
};

class CHashTableXmlNodes : public CHashTable
{
public:
	CHashTableXmlNodes();
	VOID BuildHashTableOfIDsFromNodeParent(const CXmlNode * pXmlNodeParent);
	VOID BuildHashTableOfIDsFromFirstElement(const CXmlNode * pXmlNodeFirstElement, PSZAC pszaAttributeNameID);
	BOOL BuildHashTableOfElementNamesOrAttributeIDs(const CXmlNode * pXmlNodeFirstElement, PSZAC pszaAttributeID);

	VOID AddHashElementXmlNode(PSZAC pszaNodeValue, CXmlNode * pNodeXML);
	CHashElementXmlNode * PFindHashElementXmlNode(PSZAC pszaNodeValue) const { return (CHashElementXmlNode *)PFindHashElement(pszaNodeValue); }
	CHashElementXmlNode * PFindHashElementAndRemoveXmlNode(PSZAC pszaNodeValue) { return (CHashElementXmlNode *)PFindHashElementAndRemove(pszaNodeValue); }

public:
	static PSZUC PszuGetHashKey(CHashElementXmlNode * pHashElement) { return pHashElement->m_pszuNodeValue; }	// This routine must be compatible with interface PFn_PszGetHashKey()
};

#define d_coLogHeaderLightGrey		RGB(128, 128, 128)		// Color to draw the header before dumping a log
*/

PSZUC PszuFindValidXmlElements(PSZUC pszuDataBufferToScan, OUT_F_INV PSZUC * ppszuDataBufferXmlStop);

// The following 0xFF prefix indicates the string is already encoded in XML and therefore there is no need to call.
// The character 0xFF is safe because it is illegal to use in an UTF-8 string, and therefore there is no danger of a conflict.
// This character is used ONLY at the beginning of a string.
#define d_szuXmlAlreadyEncoded	"\xFF"
#define d_chuXmlAlreadyEncoded	0xFF


#endif // XML_H
