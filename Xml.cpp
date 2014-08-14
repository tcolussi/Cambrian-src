///////////////////////////////////////////////////////////////////////////////////////////////////
//	Xml.cpp
//
//	Classes to parse an XML stream/file.
//	The motivation for having a custom XML parser is flexbility and performance.
//	The QDomDocument is grossly inefficient both in terms of memory requirement and CPU cycles.
//	I estimate the QDomDocument takes about 1000 times more CPU cycles than my XML parser, and about 20 times more memory,
//	which means loading an XML document taking 5ms and requiring 1 MB results in 5 seconds of wait time and taking 20 MB of RAM with QDomDocument.
//
//	Also, QDomDocument is difficult to use and does not allow friendly inheritance since every QDomNode must be copied on the stack.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


BOOL
FCompareXmlElementNamesWithinBuffer(PSZUC pszuElementOpening, PSZUC pszuElementClosing)
	{
	while (TRUE)
		{
		const UINT chElementOpening = *pszuElementOpening++;
		if (chElementOpening == '>' || Ch_FIsWhiteSpace(chElementOpening))
			{
			return (*pszuElementClosing == '>');
			}
		if (chElementOpening != *pszuElementClosing++)
			return FALSE;
		} // while
	}

//	Scan a string to see if there are any valid XML nodes to extract.
//	This function does not handle nested XML nodes having the same name, such as: "<x>abc<x>def</x></x>".
//
//	Return a pointer to the beginning of the element, and the end of the last element.
//	Return NULL if the buffer to scan does not contain a valid XML element.
//
//	IMPLEMENTATION NOTES
//	This function does NOT handle XML comments <!--  -->
PSZUC
PszuFindValidXmlElements(PSZUC pszuDataBufferToScan, OUT_F_INV PSZUC * ppszuDataBufferXmlStop)
	{
	Assert(pszuDataBufferToScan != NULL);
	Assert(ppszuDataBufferXmlStop != NULL);

	PSZUC pszuElementOpened;	// Current opened element
	const CHU * pchTemp = pszuDataBufferToScan;
	// Search for the first opening angle < which should mean the beginning of an XML element
	while (TRUE)
		{
		const UINT ch = *pchTemp++;
		if (ch == '<')
			{
			const UINT chNext = *pchTemp;
			if (chNext == '/')
				continue;		// We have a closing element such as </stream:stream>
			pszuElementOpened = pchTemp;
			if (chNext == '?')
				{
				PSZUC pszuXmlStart = pchTemp - 1;
				// We got an XML declaration <?xml version="1.0" encoding="UTF-8" ?>
				// As a result, return the entire string as a valid XML.  This is a bit of a hack because pszuDataBufferToScan may not contain a valid XML, however given the closing element </stream:stream> is already missing
				// there is no harm returning the entire pszuDataBufferToScan.  The CXmlTree is capable to parse an XML with with missing closing element </stream:stream>.
				while (*pchTemp != '\0')
					pchTemp++;		// Go to the end of the string
				*ppszuDataBufferXmlStop = pchTemp;
				return pszuXmlStart;
				}
			break;	// We found our XML element
			} // if
		if (ch == '\0')
			return NULL;	// We have reached the end of the string without any < which means there are no XML node in it
		} // while
	PSZUC pszuDataBufferXmlNodeFirst = NULL;	// First valid XML node

	PSZUC pszuElementCompare = pchTemp;
	UINT chElementCompare = *pchTemp++;	// Fetch the first character for optimization
	while (TRUE)
		{
		const UINT ch = *pchTemp++;
		if (ch == '\0')
			return pszuDataBufferXmlNodeFirst;		// We reached the end of the string.  At this point, it is possible pszuDataBufferXmlNodeFirst is not NULL because we have found a previous XML node, however it may also be NULL if we never found a valid XML node.
		if (ch == '/')
			{
			if (pchTemp[0] == '>')
				{
				// We have a self-closing element such as "<abc d='e'/>"
				if (pszuElementOpened == pszuElementCompare)
					{
					// The closing element is the same as the one we want to compare, so we have found a match
					pchTemp++;	// Include the closing angle >
					goto ValidXmlNode;
					}
				// Otherwise, just skip it
				}
			continue;
			}
		if (ch != '<')
			{
			if (ch == '>')
				pszuElementOpened = NULL;	// We no longer have an opened element
			continue;
			}
		if (pchTemp[0] == '/')
			{
			// We have a closing element
			if (pchTemp[1] != chElementCompare)
				continue;
			if (FCompareXmlElementNamesWithinBuffer(pszuElementCompare, pchTemp + 1))
				{
				// Skip until the closing angle >
				while (TRUE)
					{
					const UINT chEnd = *pchTemp++;
					if (chEnd == '\0')
						return pszuDataBufferXmlNodeFirst;	// Premature end of string (so return anything we got so far
					if (chEnd == '>')
						break;
					}
				// We have found a valid XML node, so record its position
				ValidXmlNode:
				*ppszuDataBufferXmlStop = pchTemp;
				if (pszuDataBufferXmlNodeFirst == NULL)
					pszuDataBufferXmlNodeFirst = pszuElementCompare - 1;
				// Keep scanning in case we find other XML nodes
				while (TRUE)
					{
					const UINT ch = *pchTemp++;
					if (ch == '<')
						{
						pszuElementOpened = pchTemp;
						break;
						}
					if (ch == '\0')
						return pszuDataBufferXmlNodeFirst;	// We reached the end of the string without any other XML element, so return what we got so far
					}
				pszuElementCompare = pchTemp;
				chElementCompare = *pchTemp++;	// Fetch the first character for optimization
				}
			} // if
		else if (ch == chElementCompare)
			{
			// We have an opening element
			Assert(FALSE && "NYI");
			}
		} // while
	Assert(FALSE && "Unreachable code!");
	} // PszuFindValidXmlElements()

#ifdef DEBUG

//	Return the offsets of the Xml nodes
int
IbFindValidXmlNodes(PSZAC pszDataBufferToScan, OUT int * pibDataBufferXmlStop)
	{
	PSZUC pszuDataBufferXmlStop;
	PSZUC pchTemp = PszuFindValidXmlElements(IN (PSZUC)pszDataBufferToScan, OUT_F_INV &pszuDataBufferXmlStop);
	if (pchTemp != NULL)
		{
		// Compute the offsets
		PSZUC pszDataBufferToScanEnd = pszuDataBufferXmlStop;
		while (*pszDataBufferToScanEnd != '\0')
			pszDataBufferToScanEnd++;
		*pibDataBufferXmlStop = pszuDataBufferXmlStop - pszDataBufferToScanEnd;
		return pchTemp - (PSZUC)pszDataBufferToScan;
		}
	*pibDataBufferXmlStop = d_zNA;	// For debugging purpose
	return -1;
	}

void
TEST_PszuFindValidXmlNodes()
	{
	int ibStart;
	int ibStop;

	ibStart = IbFindValidXmlNodes("<abc a='b'>xyz</abc>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("  <abc a='b'>xyz</abc> ", OUT &ibStop);
	Assert(ibStart == 2);
	Assert(ibStop == -1);

	ibStart = IbFindValidXmlNodes(" <x>abc</x> ", OUT &ibStop);
	Assert(ibStart == 1);
	Assert(ibStop == -1);

	ibStart = IbFindValidXmlNodes(" <x>abc</x><y>def</y> ", OUT &ibStop);
	Assert(ibStart == 1);
	Assert(ibStop == -1);

	ibStart = IbFindValidXmlNodes(" <x>abc</x><y>de", OUT &ibStop);
	Assert(ibStart == 1);
	Assert(ibStop == -5);	// Ignore the last 5 characters "<y>de"

	ibStart = IbFindValidXmlNodes("<x>abc</x><y>d</y", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == -7);	// Ignore the last 7 characters "<y>d</y"

	ibStart = IbFindValidXmlNodes("abc<x>def</x><", OUT &ibStop);
	Assert(ibStart == 3);	// Ignore "abc"
	Assert(ibStop == -1);	// Ignore the last character

	ibStart = IbFindValidXmlNodes("<x>abc", OUT &ibStop);
	Assert(ibStart == -1);	// There is no valid XML node
	Assert(ibStop == d_zNA);

	ibStart = IbFindValidXmlNodes("<x", OUT &ibStop);
	Assert(ibStart == -1);	// There is no valid XML node
	Assert(ibStop == d_zNA);

	ibStart = IbFindValidXmlNodes("abc", OUT &ibStop);
	Assert(ibStart == -1);	// There is no valid XML node
	Assert(ibStop == d_zNA);

	ibStart = IbFindValidXmlNodes("<?xml version='1.0'' encoding='UTF-8' ?><a>b</a>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<? ?><a>b</a>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<i><c s='s'/><t>abc</t></i>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<i><c s='s'/><t>abc</t></i> <i><c s='s'/><t>abc</t></i>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<i><c s='s'/><c s='s'/></i>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<i><c s='s'/><c s='s'/></i><c s='s'/>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<i d='1' t='r'/>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	ibStart = IbFindValidXmlNodes("<a/><b/><c/> ", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == -1);

	ibStart = IbFindValidXmlNodes("</stream><a/><b/><c/> ", OUT &ibStop);
	Assert(ibStart == 9);	//Skip the "</stream>"
	Assert(ibStop == -1);

	ibStart = IbFindValidXmlNodes("<presence/>", OUT &ibStop);
	Assert(ibStart == 0);
	Assert(ibStop == 0);

	} // TEST_PszuFindValidXmlNodes()

#endif // DEBUG

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Convert the XML entities to plain text.
//
//	The input buffer is the same as the output buffer for performance optimization: if the string has no XML entities, there is no copying of memory.
//	Since XML entities are larger than their plain text equivalent, the length of the decoded XML is always equal or smaller than the input.
//	To encode XML entities, use CBin::BinAppendXmlTextU().
//
//	This routine returns the length (excluding the null-terminator) of the decoded string.
int
XmlDecode(INOUT PSZU pszuXml)
	{
	Assert(pszuXml != NULL);
	CHS ch;
	const CHU * pchuSrc = pszuXml;
	while (TRUE)
		{
		ch = *pchuSrc++;
		if (ch == '\0')
			return (pchuSrc - pszuXml - 1);	// We have reached the end of the string without any XML entity (this is probably most of the cases
		if (ch == '&')
			break;	// We have an XML eneity, so we need to do some processing
		} // while
	CHU * pchuDst = (CHU *)pchuSrc - 1;
	while (TRUE)
		{
		// We have a new XML entity
		// The code may be a bit difficult to read because it is optimized for performance.
		ch = *pchuSrc++;
		switch (ch)
			{
		case 'l':	// &lt;
			if (pchuSrc[0] == 't' && pchuSrc[1] == ';')
				{
				pchuSrc += 2;
				*pchuDst++ = '<';
				goto CopyUntilNextXmlEntity;
				}
			break;
		case 'g':	// &gt;
			if (pchuSrc[0] == 't' && pchuSrc[1] == ';')
				{
				pchuSrc += 2;
				*pchuDst++ = '>';
				goto CopyUntilNextXmlEntity;
				}
			break;
		case 'a':	// &amp; or &apos;
			if (pchuSrc[0] == 'm' && pchuSrc[1] == 'p' && pchuSrc[2] == ';')
				{
				pchuSrc += 3;
				*pchuDst++ = '&';
				goto CopyUntilNextXmlEntity;
				}
			if (pchuSrc[0] == 'p' && pchuSrc[1] == 'o' && pchuSrc[2] == 's' && pchuSrc[3] == ';')
				{
				pchuSrc += 4;
				*pchuDst++ = '\'';
				goto CopyUntilNextXmlEntity;
				}
			break;
		case 'q':	// &quot;
			if (pchuSrc[0] == 'u' && pchuSrc[1] == 'o' && pchuSrc[2] == 't' && pchuSrc[3] == ';')
				{
				pchuSrc += 4;
				*pchuDst++ = '\"';
				goto CopyUntilNextXmlEntity;
				}
			break;			
		case '\0':
			// We have an end of string right after the '&'.  This is is an error because we are expecting an entity
			Report(FALSE && "Missing XML entity!");
			pchuDst[0] = '&';
			pchuDst[1] = '\0';
			return (pchuDst - pszuXml + 1);
		case '#':
			// Can be decimal or hexadecimal value
			SStringToNumber stn;
			InitToGarbage(OUT &stn, sizeof(stn));
			stn.uFlags = STN_mskfNoLeadingSpaces | STN_mskfNoSkipTailingSpaces | STN_mskfAllowRandomTail | STN_mskfAllowHexPrefix | STN_mskfMustHaveDigits | STN_mskfUnsignedInteger;
			stn.pszuSrc = pchuSrc;
			if (FStringToNumber(INOUT &stn) && *stn.pchStop == ';')
				{
				Assert(stn.nErrCode == STN_errSuccess);
				pchuSrc = stn.pchStop + 1;
				if (stn.u.uData <= 0x7F)
					{
					if (stn.u.uData != '\0')
						*pchuDst++ = (BYTE)stn.u.uData;
					}
				else
					{
					pchuDst += EncodeUtf32toUtf8(IN stn.u.uData, OUT pchuDst);
					}
				goto CopyUntilNextXmlEntity;
				} // if
			} // switch

		// If we reach this line of code, we have an error in the encoding of the XML data.  The best solution is leaving the "entity" as is in the XML data.
		Report(FALSE && "Invalid XML entity!");
		pchuDst[0] = '&';
		pchuDst[1] = ch;
		pchuDst += 2;

		CopyUntilNextXmlEntity:
		while (TRUE)
			{
			ch = *pchuSrc++;
			if (ch == '&')
				break;	// We have a new XML entity
			*pchuDst++ = ch;
			if (ch == '\0')
				return (pchuDst - pszuXml - 1);
			} // while
		} // while
	Assert(FALSE && "Unreachable code");
	} // XmlDecode()

#ifdef DEBUG
void
TEST_XmlDecode(PSZAC pszaXml, PSZAC pszaPlainText)
	{
	CHU szuTemp[100];	// Use a temporary buffer to store the XML string
	InitToGarbage(OUT szuTemp, sizeof(szuTemp));
	strcpy(OUT (char *)szuTemp, pszaXml);	// Create an invalid XML entity
	UINT cch = XmlDecode(INOUT szuTemp);
	Assert(strlenU(szuTemp) == cch);
	Assert(strcmpU(szuTemp, pszaPlainText) == 0);
	}

void
TEST_XmlDecode()
	{
	TEST_XmlDecode("&lt;&apos;HE&amp;LLO&quot;&gt;", "<'HE&LLO\">");
	TEST_XmlDecode("&#9; abc &#x9;", "\t abc \t");

	// Invalid XML entities
	//TEST_XmlDecode("ERR&OR", "ERR&OR");
	//TEST_XmlDecode("ERR&OR&", "ERR&OR&");
	}

void
TEST_XmlEncode(PSZAC pszaPlainText, PSZAC pszaXmlEncoded)
	{
	CBin bin;
	bin.BinAppendXmlTextU((PSZUC)pszaPlainText);
	PSZUC pszuXmlEncoded = bin.BinAppendNullTerminatorSz();
	Assert(strcmpU(pszuXmlEncoded, pszaXmlEncoded) == 0);
	}

void
TEST_XmlEncode()
	{
	TEST_XmlEncode("it's me", "it&#39;s me");
	TEST_XmlEncode("it'", "it&#39;");
	TEST_XmlEncode("'it'", "&#39;it&#39;");
	TEST_XmlEncode("<hello>", "&lt;hello&gt;");		// There is no need to encode the closing angle >
	TEST_XmlEncode("\t<hello>\t", "\t&lt;hello&gt;\t");
//	TEST_XmlEncode("\u0010Q\u0011", "\u0010Q\u0011");	 (This line does not compile on Mac)
	}

void
TEST_XmlEncodingAndDecoding()
	{
	TEST_XmlEncode();
	TEST_XmlDecode();
	TEST_PszuFindValidXmlNodes();
	}

#endif // DEBUG

//	Set the linked list of attributes.
//	This is a method because each attribute does not have a parent yet
//	Also, this method reverses the linked list

VOID
CXmlNode::SetAttributesListReverse(CXmlNode * pAttributesList)
	{
	Endorse(pAttributesList == NULL);
	m_pAttributesList = NULL;
	while (pAttributesList != NULL)
		{
		CXmlNode * pNext = pAttributesList->m_pNextSibling;
		pAttributesList->m_pParent = this;
		pAttributesList->m_pNextSibling = m_pAttributesList;
		m_pAttributesList = pAttributesList;
		pAttributesList = pNext;
		}
	}

//	Add the attributes at the end of the list
VOID
CXmlNode::AddAttributesListReverse(CXmlNode * pAttributesList)
	{
	Assert(pAttributesList != NULL);

	if (m_pAttributesList == NULL)
		{
		SetAttributesListReverse(pAttributesList);
		}

	// Find the last attribute
	CXmlNode * pLast = m_pAttributesList;
	while (TRUE)
		{
		CXmlNode * pNext = pLast->m_pNextSibling;
		if (pNext == NULL)
			break;
		pLast = pNext;
		} // while
	Assert(pLast != NULL);

	// Now, append the new attributes in reverse order
	while (pAttributesList != NULL)
		{
		CXmlNode * pNext = pAttributesList->m_pNextSibling;
		pAttributesList->m_pParent = this;
		pLast->m_pNextSibling = pAttributesList;
		pAttributesList->m_pNextSibling = pNext;
		pAttributesList = pNext;
		}

	} // AddAttributesListReverse()

//	Reverse all elements of the node and sub-nodes.
//	This method reverses only the elements because method SetAttributesList() takes care of the attributes.
VOID
CXmlNode::ReverseLinkedListsElementsOnly()
	{
	if (m_pElementsList != NULL)
		{
		// Always re-insert at the beginning
		CXmlNode * p = m_pElementsList;
		m_pElementsList = NULL;
		while (p != NULL)
			{
			p->ReverseLinkedListsElementsOnly();
			CXmlNode * pNext = p->m_pNextSibling;
			p->m_pNextSibling = m_pElementsList;
			m_pElementsList = p;
			p = pNext;
			}
		}
	} // ReverseLinkedLists()

//	Reverse the linked list for the elements and attributes.
//	This method is useful because in many cases, new nodes (elements and attributes) are usually inserted
//	at the beginning of the list, and therefore must be reversed when the operation is over.
VOID
CXmlNode::ReverseLinkedListsElementsAndAttributes()
	{
	if (m_pAttributesList != NULL)
		ReverseLinkedListAttributes();

	// Reverse the elements
	if (m_pElementsList != NULL)
		{
		// Always re-insert at the beginning
		CXmlNode * p = m_pElementsList;
		m_pElementsList = NULL;
		while (p != NULL)
			{
			p->ReverseLinkedListsElementsAndAttributes();
			CXmlNode * pNext = p->m_pNextSibling;
			p->m_pNextSibling = m_pElementsList;
			m_pElementsList = p;
			p = pNext;
			}
		}
	} // ReverseLinkedListsElementsAndAttributes()

//	Reverse the order of the attributes
VOID
CXmlNode::ReverseLinkedListAttributes()
	{
	Assert(m_pAttributesList != NULL);
	// Always re-insert at the beginning
	CXmlNode * p = m_pAttributesList;
	m_pAttributesList = NULL;
	while (p != NULL)
		{
		CXmlNode * pNext = p->m_pNextSibling;
		p->m_pNextSibling = m_pAttributesList;
		m_pAttributesList = p;
		p = pNext;
		}
	} // ReverseLinkedListAttributes()


//	Recursive
void
CXmlNode::RemoveEmptyElements()
	{
	CXmlNode * pXmlElementPrev = NULL;
	CXmlNode * pXmlElement = m_pElementsList;
	while (pXmlElement != NULL)
		{
		CXmlNode * pXmlElementNext = pXmlElement->m_pNextSibling;
		pXmlElement->RemoveEmptyElements();
		if (pXmlElement->FIsEmptyElement())
			{
			//MessageLog_AppendTextFormatCo(d_coRed, "XML element 0x$p <$s/> is empty\n", pXmlElement, pXmlElement->m_pszuTagName);
			if (pXmlElementPrev == NULL)
				m_pElementsList = pXmlElementNext;
			else
				pXmlElementPrev->m_pNextSibling = pXmlElementNext;
			}
		else
			pXmlElementPrev = pXmlElement;
		pXmlElement = pXmlElementNext;
		} // while
	}

BOOL
CXmlNode::FIsEmptyElement() const
	{
	return (m_pElementsList == NULL && m_pAttributesList == NULL && (m_pszuTagValue == NULL || m_pszuTagValue[0] == '\0'));
	}

//	Use a specific string to designate an XML comment.  The string can be almost anything, as long as its storage is constant because we are comparing pointers and not the data.
//#define	c_szCommentXML		c_szZero
const char c_szCommentXML[] = "!";

BOOL
CXmlNode::FIsNodeComment() const
	{
	return (m_pszuTagName == (PSZU)c_szCommentXML);
	}
/*
//	Return pointer of the node removed (if any)
CXmlNode *
CXmlNode::PRemoveNode(PSZAC pszuNodePath)
	{
	CXmlNode * pNodeRemove = PFindElementOrAttribute(pszuNodePath);
	if (pNodeRemove != NULL)
		pNodeRemove->RemoveFromParent();
	return pNodeRemove;
	} // PRemoveNode()
*/

void
CXmlNode::RemoveFromParent()
	{
	Assert(m_pParent->m_pElementsList != NULL);
	CXmlNode * pXmlElementPrev = NULL;
	CXmlNode * pXmlElement = m_pParent->m_pElementsList;
	while (pXmlElement != NULL)
		{
		if (pXmlElement == this)
			{
			if (pXmlElementPrev == NULL)
				m_pParent->m_pElementsList = m_pNextSibling;
			else
				pXmlElementPrev->m_pNextSibling = pXmlElement->m_pNextSibling;
			return;
			}
		pXmlElementPrev = pXmlElement;
		pXmlElement = pXmlElement->m_pNextSibling;
		}
	} // RemoveFromParent()

//	Serialize the XML nodes into a binary buffer containing UTF-8 data.
//	The binary buffer is NOT appended with neither a virtual null-terminator or null-terminator.
void
CXmlNode::SerializeToBinUtf8(IOUT CBin * pbinUtf8, UINT uSerializeFlags) const
	{
	Assert(pbinUtf8 != NULL);
	Assert((uSerializeFlags & ~STBF_kmAssertValid) == 0);
	Assert(m_pszuTagName != NULL);
	Assert(!FIsNodeComment());

	if (m_pszuTagName != NULL && *m_pszuTagName != '\0')
		{
		const int cIdentation = (uSerializeFlags & STBF_kmIdentationMask) - 1;
		if (uSerializeFlags & STBF_kfIndentTags)
			pbinUtf8->BinAppendBytes('\t', cIdentation);
		pbinUtf8->BinAppendByte('<');
		pbinUtf8->BinAppendText((PSZAC)m_pszuTagName);
		CXmlNode * pAttribute = m_pAttributesList;
		while (pAttribute != NULL)
			{
			Assert(pAttribute->m_pAttributesList == NULL);
			Assert(pAttribute->m_pElementsList == NULL);
			Assert(pAttribute->m_pParent == this);
			//Assert(pAttribute->m_pszuTagValue != NULL);
			pbinUtf8->BinAppendByte(' ');
			pbinUtf8->BinAppendText((PSZAC)pAttribute->m_pszuTagName);
			pbinUtf8->BinAppendUInt16(UINT16_FROM_CHARS('=', '\"'));	// Append two bytes "=\""
			pbinUtf8->BinAppendXmlTextU(pAttribute->m_pszuTagValue);
			pbinUtf8->BinAppendByte('\"');
			pAttribute = pAttribute->m_pNextSibling;
			} // while
		if (m_pElementsList != NULL)
			{
			pbinUtf8->BinAppendByte('>');
			pbinUtf8->BinAppendXmlTextU(m_pszuTagValue);
			if (uSerializeFlags & STBF_kfCRLF)
				pbinUtf8->BinAppendCRLF();
			CXmlNode * pElement = m_pElementsList;
			while (pElement != NULL)
				{
				Assert(pElement->m_pParent == this);
				pElement->SerializeToBinUtf8(IOUT pbinUtf8, uSerializeFlags + 1);
				pElement = pElement->m_pNextSibling;
				}
			if (uSerializeFlags & STBF_kfIndentTags)
				pbinUtf8->BinAppendBytes('\t', cIdentation);
			}
		else
			{
			if (m_pszuTagValue == NULL || m_pszuTagValue[0] == '\0')
				{
				// We have tag without any value, so we can write a self-closing tag
				pbinUtf8->BinAppendUInt16(UINT16_FROM_CHARS('/', '>'));	// Append "/>"
				if (uSerializeFlags & STBF_kfCRLF)
					pbinUtf8->BinAppendCRLF();
				return;
				}
			else
				{
				Assert(*m_pszuTagValue != '\0');	// Empty tags should not be there
				pbinUtf8->BinAppendByte('>');
				pbinUtf8->BinAppendXmlTextU(m_pszuTagValue);
				}
			} // if...else
		pbinUtf8->BinAppendUInt16(UINT16_FROM_CHARS('<', '/'));		// Append two bytes for "</"
		pbinUtf8->BinAppendText((PSZAC)m_pszuTagName);
		pbinUtf8->BinAppendByte('>');

		#ifdef DEBUG
		if (uSerializeFlags & STBF_kfDebugLParam)
			pbinUtf8->BinAppendTextSzv_VE("\t\t <!-- lParam=0x$p ($i) -->", m_lParam, m_lParam);
		#endif
		if (uSerializeFlags & STBF_kfCRLF)
			pbinUtf8->BinAppendCRLF();
		}
	else
		{
		// We got an empty tag name, this is typically the 'root' element or an XML error.
		// As a result just skip the empty node and serialize its child elements (if any)
		CXmlNode * pElement = m_pElementsList;
		while (pElement != NULL)
			{
			pElement->SerializeToBinUtf8(IOUT pbinUtf8, uSerializeFlags);
			pElement = pElement->m_pNextSibling;
			}
		} // if...else
	} // SerializeToBinUtf8()

BOOL
CXmlNode::FCompareTagName(PSZAC pszuTagName) const
	{
	Assert(pszuTagName != NULL);
	Assert(*pszuTagName != '\0');
	Assert(m_pszuTagName != NULL);
	#ifdef DEBUG
	PSZAC pszuTagNameDebug = pszuTagName;
	UNUSED_VARIABLE(pszuTagNameDebug);
	#endif
	// Compare the  with the current node
	const CHU * pchuTagNameThis = m_pszuTagName;
	while (TRUE)
		{
		UINT chuTagName = *pszuTagName++;
		if (chuTagName != *pchuTagNameThis++)
			return FALSE;
		if (chuTagName == '\0')
			return TRUE;
		} // while
	} // FCompareTagName()


/*
//	The search is not case sensitive
CXmlNode *
CXmlNode::PFindElementOrAttributeByName(PSZUC pszuTagName) const
	{
	CXmlNode * p = m_pElementsList;
	while (p != NULL)
		{
		if (p->FCompareTagName(pszuTagName))
			return p;
		p = p->m_pNextSibling;
		}
	p = m_pAttributesList;
	while (p != NULL)
		{
		Assert(p->m_pAttributesList == NULL);
		Assert(p->m_pElementsList == NULL);
		Assert(p->m_pParent == this);
		if (p->FCompareTagName(pszuTagName))
			return p;
		p = p->m_pNextSibling;
		}
	return NULL;
	} // PFindElementOrAttributeByName()
*/

//	Find the element of attribute matching the path.
//	The comparison is not case sensitive
CXmlNode *
CXmlNode::PFindElementOrAttribute(PSZAC pszuNodePath) const
	{
	Assert(pszuNodePath != NULL);
	Endorse(*pszuNodePath == '\0');		// Empty node path will obviously return NULL, but will not crash
	Assert(*pszuNodePath != '\0');		// Just checking!
	Assert(m_pszuTagName != NULL);

	// As a small optimization, we compute the total number of dots.  This will indicate how deep to search
	// within the elements before comparing the tag name for an attribute or an element.
	const CHU * pchuNodePath = (PSZUC)pszuNodePath;
	int cDots = 0;
	while (TRUE)
		{
		UINT chuNodePath = *pchuNodePath++;
		if (chuNodePath == '.')
			cDots++;
		else if (chuNodePath == '\0')
			break;
		} // while

	// Search the elements
	const CXmlNode * pThis = m_pElementsList;
	while (pThis != NULL)
		{
		// Compare the path with the current node
		pchuNodePath = (PSZUC)pszuNodePath;
		const CHU * pchuTagName = pThis->m_pszuTagName;
		Assert(pchuTagName != NULL);
		Assert(*pchuTagName != '\0');
		while (TRUE)
			{
			CHS chuNodePath = *pchuNodePath++;
			CHS chuTagName = *pchuTagName++;
			Assert(chuTagName != '.');
			if (chuTagName == '\0')
				{
				// We have reached the end of the tag, so search the child elements/attributes for a matching name
				if (chuNodePath == '\0')
					{
					// We have found a match within the elements
					return const_cast<CXmlNode *>(pThis);
					}
				else if (chuNodePath == '.')
					{
					// This is a special case where we are searching consecutive nodes, separated by the dot
					if (--cDots > 0)
						{
						// We have some more dots to go, so search for the element.  We don't need to search for the attributes since the attributes are not allowed to have child nodes
						pThis = pThis->m_pElementsList;
						if (pThis == NULL)
							{
							Assert(FALSE && "NYT");
							return NULL;
							}
						pszuNodePath = (PSZAC)pchuNodePath;	// Move the pointer to the next sub path
						break;	// Go to the first loop
						}
					else
						{
						// There are no more dots, so search for the elements or the attributes
						CXmlNode * p = pThis->m_pElementsList;
						while (p != NULL)
							{
							if (p->FCompareTagName((PSZAC)pchuNodePath))
								return p;
							p = p->m_pNextSibling;
							}
						p = pThis->m_pAttributesList;
						while (p != NULL)
							{
							if (p->FCompareTagName((PSZAC)pchuNodePath))
								return p;
							p = p->m_pNextSibling;
							}
						// There is no element and no attribute matching the name
						//Assert(FALSE && "NYT");
						return NULL;
						} // if...else
					}
				else
					{
					goto NextNode;
					} // if...else
				}
			else if (chuTagName != chuNodePath)
				{
				NextNode:
				CXmlNode * pNext = pThis->m_pNextSibling;
				if (pNext == NULL)
					goto SearchAttributes;
				pThis = pNext;
				break;	// Go to the first loop
				}
			} // while
		} // while

	// We have exhausted the elements, so search the attributes
SearchAttributes:
	CXmlNode * p = m_pAttributesList;
	while (p != NULL)
		{
		if (p->FCompareTagName(pszuNodePath))
			return p;
		p = p->m_pNextSibling;
		}
	return NULL;
	} // PFindElementOrAttribute()

CXmlNode *
CXmlNode::PFindAttribute(PSZAC pszaAttributeName) const
	{
	CXmlNode * p = m_pAttributesList;
	while (p != NULL)
		{
		if (p->FCompareTagName(pszaAttributeName))
			return p;
		p = p->m_pNextSibling;
		}
	return NULL;
	}

//	Find an attribue matching a one-character name.
//	In many cases, the data is serialized to XML using a one-character attribute name.
CXmlNode *
CXmlNode::PFindAttribute(CHS chAttributeName) const
	{
	CXmlNode * p = m_pAttributesList;
	while (p != NULL)
		{
		if (p->m_pszuTagName[0] == chAttributeName && p->m_pszuTagName[1] == '\0')
			return p;
		p = p->m_pNextSibling;
		}
	return NULL;
	}

CXmlNode *
CXmlNode::PFindElement(PSZAC pszaElementName) const
	{
	Assert(pszaElementName != NULL);
	Assert(pszaElementName[0] != '\0');
	CXmlNode * p = m_pElementsList;
	while (p != NULL)
		{
		if (p->FCompareTagName(pszaElementName))
			return p;
		p = p->m_pNextSibling;
		}
	return NULL;
	} // PFindElement()

CXmlNode *
CXmlNode::PFindElement(CHS chElementName) const
	{
	CXmlNode * p = m_pElementsList;
	while (p != NULL)
		{
		if (p->m_pszuTagName[0] == chElementName && p->m_pszuTagName[1] == '\0')
			return p;
		p = p->m_pNextSibling;
		}
	return NULL;
	}

CXmlNode *
CXmlNode::PFindElementMatchingAttributeNameAndValue(PSZAC pszAttributeName, PSZAC pszAttributeValue) const
	{
	CXmlNode * pElement = m_pElementsList;
	while (pElement != NULL)
		{
		CXmlNode * pAttribute = pElement->PFindAttribute(pszAttributeName);
		if (pAttribute != NULL)
			{
			Assert(pAttribute->m_pszuTagValue != NULL);
			if (FCompareStrings(pAttribute->m_pszuTagValue, pszAttributeValue))
				return pElement;
			}
		pElement = pElement->m_pNextSibling;
		}
	return NULL;
	}

//	Find the element and return pointer to it if any, otherwise return NULL.
//	Remove the returned element from the XML tree.
CXmlNode *
CXmlNode::PFindElementRemove(PSZAC pszaElementName)
	{
	Assert(pszaElementName != NULL);
	Assert(pszaElementName[0] != '\0');
	if (m_pElementsList == NULL)
		return NULL;
	CXmlNode * pElement = m_pElementsList;
	if (pElement->FCompareTagName(pszaElementName))
		{
		// The first element is our match
		m_pElementsList = pElement->m_pNextSibling;
		return pElement;
		}
	// Loop through all the elements
	while (TRUE)
		{
		CXmlNode * pElementNext = pElement->m_pNextSibling;
		if (pElementNext == NULL)
			return NULL;
		if (pElementNext->FCompareTagName(pszaElementName))
			{
			pElement->m_pNextSibling = pElementNext->m_pNextSibling;
			return pElementNext;
			}
		pElement = pElementNext;
		} // while
	Assert(FALSE && "Unreachable code");
	} // PFindElementRemove()

//	Find the next sibling element having the same name as the current node.
//	This method is used to loop to get a complete list of all sibling elements
CXmlNode *
CXmlNode::PFindNextSibling() const
	{
	CXmlNode * pSibling = m_pNextSibling;
	while (pSibling != NULL)
		{
		if (pSibling->FCompareTagName((PSZAC)m_pszuTagName))
			return pSibling;
		pSibling = pSibling->m_pNextSibling;
		}
	return NULL;
	}

//	All comparison is case sensitive.
BOOL
CXmlNode::FFindAttributeAndCompareValue(PSZAC pszaAttributeName, PSZUC pszuAttributeValue) const
	{
	Assert(pszaAttributeName != NULL);
	Assert(*pszaAttributeName != '\0');
	Assert(pszuAttributeValue!= NULL);

	CXmlNode * pAttribute = m_pAttributesList;
	while (pAttribute != NULL)
		{
		Assert(pAttribute->m_pszuTagName != NULL);
		Assert(pAttribute->m_pszuTagValue != NULL);
		if (strcmp((PSZAC)pAttribute->m_pszuTagName, pszaAttributeName) == 0)
			{
			return (strcmp((PSZAC)pAttribute->m_pszuTagValue, (PSZAC)pszuAttributeValue) == 0);
			}
		pAttribute = pAttribute->m_pNextSibling;
		}
	return FALSE;
	}

//	The comparison is case sensitive
BOOL
CXmlNode::FFindElementAndCompareValue(PSZAC pszaNameElement, PSZUC pszuValueElementCompare) const
	{
	Assert(pszaNameElement != NULL);
	Assert(pszaNameElement[0] != '\0');
	Assert(pszuValueElementCompare != NULL);

	CXmlNode * pElement = m_pElementsList;
	while (pElement != NULL)
		{
		Assert(pElement->m_pszuTagName != NULL);
		Report(pElement->m_pszuTagValue != NULL && "This is typically a self-closing element with no value - it is not an error, but those empty elements should be removed");
		if (strcmp((PSZAC)pElement->m_pszuTagName, pszaNameElement) == 0)
			{
			return (strcmp((PSZAC)pElement->m_pszuTagValue, (PSZAC)pszuValueElementCompare) == 0);
			}
		pElement = pElement->m_pNextSibling;
		}
	return FALSE;
	}

BOOL
CXmlNode::FFindElementAndCompareValue_true(PSZAC pszaNameElement) const
	{
	return FFindElementAndCompareValue(pszaNameElement, (PSZUC)"true");
	}

PSZUC
CXmlNode::PszuFindAttributeValue(CHS chAttributeName) const
	{
	CXmlNode * pAttribute = m_pAttributesList;
	while (pAttribute != NULL)
		{
		if (pAttribute->m_pszuTagName[0] == chAttributeName && pAttribute->m_pszuTagName[1] == '\0')
			return pAttribute->m_pszuTagValue;
		pAttribute = pAttribute->m_pNextSibling;
		}
	return NULL;
	}

//	Return NULL if the attribute does not exist.
//	Return an empty string if the attribute is defined but contains no text.
//
PSZUC
CXmlNode::PszuFindAttributeValue(PSZAC pszaAttributeName) const
	{
	Assert(pszaAttributeName != NULL);
	Assert(*pszaAttributeName != '\0');

	CXmlNode * pAttribute = m_pAttributesList;
	while (pAttribute != NULL)
		{
		Assert(pAttribute->m_pszuTagValue != NULL);
		if (pAttribute->FCompareTagName(pszaAttributeName))
			return pAttribute->m_pszuTagValue;
		pAttribute = pAttribute->m_pNextSibling;
		}
	return NULL;
	}

//	Same as PszuFindAttributeValue(), except never return NULL
PSZUC
CXmlNode::PszuFindAttributeValue_NZ(CHS chAttributeName) const
	{
	CXmlNode * pAttribute = m_pAttributesList;
	while (pAttribute != NULL)
		{
		Assert(pAttribute->m_pszuTagValue != NULL);
		if (pAttribute->m_pszuTagName[0] == chAttributeName && pAttribute->m_pszuTagName[1] == '\0')
			return pAttribute->m_pszuTagValue;
		pAttribute = pAttribute->m_pNextSibling;
		}
	return c_szuEmpty;
	}

//	Never return a NULL pointer
PSZUC
CXmlNode::PszuFindAttributeValue_NZ(PSZAC pszaAttributeName) const
	{
	PSZUC pszuAttributeValue = PszuFindAttributeValue(pszaAttributeName);
	if (pszuAttributeValue != NULL)
		return pszuAttributeValue;
	return c_szuEmpty;
	}

//	Return NULL if the attribute value is empty.
PSZUC
CXmlNode::PszuFindAttributeValue_ZZ(PSZAC pszaAttributeName) const
	{
	PSZUC pszuAttributeValue = PszuFindAttributeValue(pszaAttributeName);
	if (pszuAttributeValue != NULL)
		{
		if (pszuAttributeValue[0] == '\0')
			return NULL;
		}
	return pszuAttributeValue;
	}

BOOL
CXmlNode::FuIsDirtyAttributeValue(PSZAC pszaAttributeName) const
	{
	Assert(pszaAttributeName != NULL);
	Assert(*pszaAttributeName != '\0');

	CXmlNode * pAttribute = m_pAttributesList;
	while (pAttribute != NULL)
		{
		Assert(pAttribute->m_pszuTagValue != NULL);
		if (pAttribute->FCompareTagName(pszaAttributeName))
			return (pAttribute->m_lParam & kfValueDirty);
		pAttribute = pAttribute->m_pNextSibling;
		}
	return FALSE;
	}

PSZUC
CXmlNode::PszuFindElementValue(PSZAC pszaElementName) const
	{
	Assert(pszaElementName != NULL);
	CXmlNode * pNode = PFindElement(pszaElementName);
	if (pNode != NULL)
		{
		Endorse(pNode->m_pszuTagValue == NULL);	// This is the case of a self-closing element
		return pNode->m_pszuTagValue;
		}
	return NULL;
	}

PSZUC
CXmlNode::PszuFindElementValue_NZ(PSZAC pszaElementName) const
	{
	Assert(pszaElementName != NULL);
	CXmlNode * pNode = PFindElement(pszaElementName);
	if (pNode != NULL)
		{
		if (pNode->m_pszuTagValue != NULL)
			{
			Endorse(pNode->m_pszuTagValue[0] == '\0');	// An empty string is allowed
			return pNode->m_pszuTagValue;
			}
		}
	return c_szuEmpty;
	}

PSZUC
CXmlNode::PszuFindElementValue_ZZ(PSZAC pszaElementName) const
	{
	Assert(pszaElementName != NULL);
	CXmlNode * pNode = PFindElement(pszaElementName);
	if (pNode != NULL)
		{
		Endorse(pNode->m_pszuTagValue == NULL);	// This is the case of a self-closing element
		PSZUC pszuTagValue = pNode->m_pszuTagValue;
		if (pszuTagValue != NULL && pszuTagValue[0] != '\0')
			return pszuTagValue;
		}
	return NULL;
	}

//	Return NULL if the element or attribute does not exist.
//	Return an empty string if the attribute is defined, but contains no text.
PSZUC
CXmlNode::PszuFindElementOrAttributeValue(PSZAC pszuNodePath) const
	{
	Assert(pszuNodePath != NULL);
	CXmlNode * pNode = PFindElementOrAttribute(pszuNodePath);
	if (pNode != NULL)
		{
		// Assert(pNode->m_pszuTagValue != NULL);		// If pNode->m_pszuTagValue == NULL, it means it is a self-closing element such as <Name />
		Endorse(pNode->m_pszuTagValue[0] == '\0');		// An empty string is allowed
		return pNode->m_pszuTagValue;
		}
	return NULL;
	}

PSZUC
CXmlNode::PszuFindElementOrAttributeValue_NZ(PSZAC pszuNodePath) const
	{
	Assert(pszuNodePath != NULL);
	CXmlNode * pNode = PFindElementOrAttribute(pszuNodePath);
	if (pNode != NULL)
		{
		Assert(pNode->m_pszuTagValue != NULL);
		Endorse(*pNode->m_pszuTagValue != '\0');	// An empty string is allowed
		return pNode->m_pszuTagValue;
		}
	return c_szuEmpty;
	}

PSZUC
CXmlNode::PszuFindElementOrAttributeValue_ZZ(PSZAC pszuNodePath) const
	{
	Assert(pszuNodePath != NULL);
	CXmlNode * pNode = PFindElementOrAttribute(pszuNodePath);
	if (pNode != NULL)
		{
		PSZUC pszuTagValue = pNode->m_pszuTagValue;
		//Assert(pszuTagValue != NULL);
		if (pszuTagValue != NULL && *pszuTagValue != '\0')
			return pszuTagValue;
		}
	return NULL;
	}

L64
CXmlNode::LFindAttributeValueDecimal_ZZR(CHS chAttributeName) const
	{
	return LStringToNumber_ZZR_ML(PszuFindAttributeValue_NZ(chAttributeName));
	}
L64
CXmlNode::LFindAttributeValueDecimal_ZZR(PSZAC pszaAttributeName) const
	{
	return LStringToNumber_ZZR_ML(PszuFindAttributeValue_NZ(pszaAttributeName));
	}

UINT_P
CXmlNode::UFindAttributeValueDecimal_ZZR(PSZAC pszaAttributeName) const
	{
	return NStringToNumber_ZZR_ML(PszuFindAttributeValue_NZ(pszaAttributeName));
	}

UINT_P
CXmlNode::UFindAttributeValueDecimal_ZZR(CHS chAttributeName) const
	{
	return NStringToNumber_ZZR_ML(PszuFindAttributeValue_NZ(chAttributeName));
	}

//	Useful to unserialize an identifier or a pointer.
//	Return 0 if there is no attribute or if the hexadecimal value is invalid.
UINT_P
CXmlNode::UFindAttributeValueHexadecimal_ZZR(PSZAC pszaAttributeName) const
	{
	CXmlNode * pAttribute = m_pAttributesList;
	while (pAttribute != NULL)
		{
		Assert(pAttribute->m_pszuTagValue != NULL);
		if (pAttribute->FCompareTagName(pszaAttributeName))
			{
			PSZUC pszuTagValue = pAttribute->m_pszuTagValue;
			Assert(pszuTagValue != NULL);
			if (pszuTagValue != NULL && pszuTagValue[0] != '\0')
				{
				SStringToNumber stn;
				stn.uFlags = STN_mskfHexBase;
				stn.pszuSrc = pszuTagValue;
				if (FStringToNumber(INOUT &stn))
					return stn.u.uData;
				// An error occurred while parsing the hexadecimal value
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The XML attribute '$s' having value '$s' is not a valid hexadecimal value!\n", pszaAttributeName, pszuTagValue);
				}
			break;
			} // if
		pAttribute = pAttribute->m_pNextSibling;
		}
	return 0;
	} // UFindAttributeValueHexadecimal_ZZR()

UINT_P
CXmlNode::UFindAttributeValueHexadecimal_ZZR(CHS chAttributeName) const
	{
	const CHA szuAttributeName[2] = { (CHA)chAttributeName, '\0' };
	return UFindAttributeValueHexadecimal_ZZR(szuAttributeName);
	}

//	Wrapper to return the numeric value of an element or attribute.
//	Return 0 if the node could not be found or contains empty text.
//	Return -1 if an error occurred during the parsing such as invalid digits.
//
//	It is possible the value the element or attribute has a value of 0 or -1 which do not represent an error.
//	To find exactly what's doing on, the caller must manually parse.
INT_P
CXmlNode::NFindElementOrAttributeValueNumeric(PSZAC pszuNodePath) const
	{
	Assert(pszuNodePath != NULL);
	CXmlNode * pNode = PFindElementOrAttribute(pszuNodePath);
	if (pNode != NULL)
		{
		PSZUC pszuTagValue = pNode->m_pszuTagValue;
		Assert(pszuTagValue != NULL);
		if (pszuTagValue != NULL && pszuTagValue[0] != '\0')
			{
			SStringToNumber stn;
			stn.uFlags = STN_mskfAllowHexPrefix;
			stn.pszuSrc = pszuTagValue;
			if (FStringToNumber(INOUT &stn))
				return stn.u.nData;
			// An error occurred while parsing
			return -1;
			}
		}
	return 0;
	} // NFindElementOrAttributeValueNumeric()

TIMESTAMP
CXmlNode::TsGetAttributeValueTimestamp_ML(CHS chAttributeName) const
	{
	return Timestamp_FromString_ML(PszuFindAttributeValue_NZ(chAttributeName));
	}

void
CXmlNode::GetAttributeValueHashSha1_ML(PSZAC pszaAttributeName, OUT_ZZR SHashSha1 * pHashSha1) const
	{
	(void)Base85_FDecodeToBinary_ZZR_ML(IN PszuFindAttributeValue_NZ(pszaAttributeName), OUT_ZZR (BYTE *)pHashSha1, sizeof(*pHashSha1));
	}

void
CXmlNode::GetAttributeValueDateTime_ML(PSZAC pszaAttributeName, OUT_ZZR QDateTime * pDateTime) const
	{
	Assert(pszaAttributeName != NULL);
	Assert(pDateTime != NULL);
	CXmlNode * pNodeDateTime = PFindAttribute(pszaAttributeName);
	if (pNodeDateTime == NULL)
		return;
	PSZUC pszDateTime = pNodeDateTime->m_pszuTagValue;
	*pDateTime = QDateTime::fromString((PSZAC)pszDateTime, Qt::ISODate);
	if (!pDateTime->isValid())
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Invalid QDateTime $s\n", pszDateTime);
	}

void
CXmlNode::GetAttributeValueL64(PSZAC pszaAttributeName, OUT_ZZR L64 * plValue) const
	{
	*plValue = LStringToNumber_ZZR_ML(PszuFindAttributeValue_NZ(pszaAttributeName));
	}

void
CXmlNode::GetAttributeValueCStr(PSZAC pszaAttributeName, OUT CStr * pstrValue) const
	{
	pstrValue->BinInitFromStringWithNullTerminator((PSZAC)PszuFindAttributeValue(pszaAttributeName));
	}

void
CXmlNode::UpdateAttributeValueInt(CHS chAttributeName, OUT_F_UNCH int * pnValue) const
	{
	UpdateAttributeValueUInt(chAttributeName, (UINT *)pnValue);
	}

void
CXmlNode::UpdateAttributeValueUInt(CHS chAttributeName, OUT_F_UNCH UINT * puValue) const
	{
	Assert(puValue != NULL);
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute == NULL)
		return;
	*puValue = NStringToNumber_ZZR_ML(pNodeAttribute->m_pszuTagValue);
	}
void
CXmlNode::UpdateAttributeValueUIntHexadecimal(CHS chAttributeName, OUT_F_UNCH UINT * puValue) const
	{
	Assert(puValue != NULL);
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute == NULL)
		return;
	*puValue = UStringToNumberHexadecimal_ZZR_ML(pNodeAttribute->m_pszuTagValue);
	}
void
CXmlNode::UpdateAttributeValueL64(CHS chAttributeName, OUT_F_UNCH L64 * plValue) const
	{
	Assert(plValue != NULL);
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute == NULL)
		return;
	*plValue = LStringToNumber_ZZR_ML(pNodeAttribute->m_pszuTagValue);
	}
void
CXmlNode::UpdateAttributeValueTimestamp(CHS chAttributeName, OUT_F_UNCH TIMESTAMP * ptsValue) const
	{
	Assert(ptsValue != NULL);
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute == NULL)
		return;
	*ptsValue = Timestamp_FromString_ML(pNodeAttribute->m_pszuTagValue);
	}
void
CXmlNode::UpdateAttributeValueCStr(CHS chAttributeName, OUT_F_UNCH CStr * pstrValue) const
	{
	Assert(pstrValue != NULL);
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute == NULL)
		return;
	pstrValue->BinInitFromStringWithNullTerminator((PSZAC)pNodeAttribute->m_pszuTagValue);
	}

void
CXmlNode::UpdateAttributeValueCBin(CHS chAttributeName, OUT_F_UNCH CBin * pbinValue) const
	{
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute != NULL)
		{
		pbinValue->Empty();
		pbinValue->BinAppendBinaryDataFromBase85Szv_ML(IN pNodeAttribute->m_pszuTagValue);
		}
	}

//	Return TRUE if a valid hash value was extracted
BOOL
CXmlNode::UpdateAttributeValueHashSha1(CHS chAttributeName, OUT_F_UNCH SHashSha1 * pHashSha1) const
	{
	CXmlNode * pNodeAttribute = PFindAttribute(chAttributeName);
	if (pNodeAttribute != NULL)
		return Base85_FDecodeToBinary_ZZR_ML(IN pNodeAttribute->m_pszuTagValue, OUT_ZZR (BYTE *)pHashSha1, sizeof(*pHashSha1));
	return FALSE;
	}



/*
//	Same as above, however return iError if unable to find the element or the value
int
CXmlNode::GetElementOrAttributeNumericValue(PSZAC pszuNodePath, int iError) const
	{
	Assert(pszuNodePath != NULL);
	CXmlNode * pNode = PFindElementOrAttribute(pszuNodePath);
	if (pNode != NULL)
		{
		PSZUC pszuTagValue = pNode->m_pszuTagValue;
		Assert(pszuTagValue != NULL);
		if (pszuTagValue != NULL && *pszuTagValue != '\0')
			{
			SStringToNumber stn;
			stn.uFlags = STN_mskfAllowHexPrefix;
			stn.pszSrc = (PSZAC)pszuTagValue;
			if (FStringToNumber(INOUT &stn))
				return stn.nData;
			}
		}
	return iError;
	} // GetElementOrAttributeNumericValue()

//	Mark the node as dirty.
//
//	IMPLEMENTATION NOTES
//	In order for the parent node to know a child node is dirty, we will mark all the parents as dirty.
VOID
CXmlNode::SetDirty()
	{
	CXmlNode * pNode = this;
	while (pNode != NULL)
		{
		pNode->m_lParam |= kfValueDirty;
		pNode = pNode->m_pParent;
		}
	}


//	Clear all the dirty flags.
//	To improve performance, not all nodes are visited; only those parent nodes having the dirty bit set.
VOID
CXmlNode::RemoveDirtyR()
	{
	m_lParam &= ~kfValueDirty;
	CXmlNode * pNode = m_pAttributesList;
	while (pNode != NULL)
		{
		pNode->m_lParam &= ~kfValueDirty;
		pNode = pNode->m_pNextSibling;
		} // while
	// Walk the list of elements
	pNode = m_pElementsList;
	while (pNode != NULL)
		{
		if (pNode->m_lParam & kfValueDirty)
			pNode->RemoveDirtyR();				// If the parent node is not dirty, then we do not care about the dirty children
		pNode = pNode->m_pNextSibling;
		}
	} // RemoveDirtyR()

//	_SortNodes(), static
VOID
CXmlNode::_SortNodes(CXmlNode ** ppNodeHead, CXmlSortContext * pXmlSortContext)
	{
	Assert(ppNodeHead != NULL);
	Assert(pXmlSortContext != NULL);
	Assert(pXmlSortContext->m_pfnPSortKeyAllocateForElement != NULL);
	Assert(pXmlSortContext->m_pfnSortCompare != NULL);

	// Calculate the number of nodes to sort
	int cNodes = 0;
	CXmlNode * pNode = *ppNodeHead;
	while (pNode != NULL)
		{
		cNodes++;
		pNode = pNode->m_pNextSibling;
		}
	if (cNodes <= 1)
		return;	// The list is too short to be sorted
	SORTKEY ** ppSortKey = (SORTKEY **)pXmlSortContext->m_array.SetSizeUnsafe(cNodes);
	PFn_PSortKeyAllocateForElement pfnPSortKeyAllocateForElement = pXmlSortContext->m_pfnPSortKeyAllocateForElement;
	CAllocator * pAllocator = &pXmlSortContext->m_allocator;
	pAllocator->RecycleBuffer();
	pNode = *ppNodeHead;
	Assert(pNode != NULL);
	while (TRUE)
		{
		SORTKEY * pSortKey = pfnPSortKeyAllocateForElement(IN pAllocator, pNode);
		pSortKey->pvElement = pNode;
		*ppSortKey++ = (pSortKey + 1);
		pNode = pNode->m_pNextSibling;
		if (pNode == NULL)
			break;
		} // while

	// Do the sorting
	pXmlSortContext->m_array.Sort(pXmlSortContext->m_pfnSortCompare);

	// Restore the pointers from the array
	CXmlNode * pNodeHead = NULL;
	SORTKEY ** ppSortKeyStop;
	ppSortKey = (SORTKEY **)pXmlSortContext->m_array.GetDataStop(OUT (void ***)&ppSortKeyStop);
	while (ppSortKey != ppSortKeyStop)
		{
		SORTKEY * pSortKey = *--ppSortKeyStop;
		CXmlNode * pNode = (CXmlNode *)(pSortKey - 1)->pvElement;
		pNode->m_pNextSibling = pNodeHead;
		pNodeHead = pNode;
		}
	*ppNodeHead = pNodeHead;
	} // _SortNodes()

VOID
CXmlNode::_SortNodesR(INOUT CXmlSortContext * pXmlSortContext)
	{
	#ifdef DEBUG_
	PSZUC pszuID = PszuFindAttributeValue(c_szID);
	ChatDebugger_AppendTextFormatCo(coRed, "CXmlNode::_SortNodesR() - Sorting <$U ID='$U'>$U</$U>\n", m_pszuTagName, pszuID, m_pszuTagValue, m_pszuTagName);
	#endif
	if (m_pAttributesList != NULL)
		_SortNodes(INOUT &m_pAttributesList, pXmlSortContext);
	if (m_pElementsList != NULL)
		{
		_SortNodes(INOUT &m_pElementsList, pXmlSortContext);
		CXmlNode * pNode = m_pElementsList;
		while (TRUE)
			{
			pNode->_SortNodesR(pXmlSortContext);
			pNode = pNode->m_pNextSibling;
			if (pNode == NULL)
				break;
			}
		} // if
	} // _SortNodesR()
*/
/*
SORTKEY *
PSortKeyAllocateForXmlNodeName(INOUT class CAllocator * pAllocator, CXmlNode * pXmlNode)
	{
	Assert(pAllocator != NULL);
	Assert(pXmlNode != NULL);
	PSZUC pszuName = pXmlNode->m_pszuTagName;
	Assert(pszuName != NULL);
	const CHU * pchuSrc = pszuName;
	while (*pchuSrc++ != '\0')
		;
	SORTKEY * pSortKey = (SORTKEY *)pAllocator->PvAllocateData(sizeof(SORTKEY) + (pchuSrc - pszuName));
	// Build the sort key in lowercase
	CHU * pchuDst = (CHU *)(pSortKey + 1);
	while (TRUE)
		{
		UINT chu = *pszuName++;
		if (chu >= 'A' && chu <= 'Z')
			chu += 32;	// Make lowercase
		*pchuDst++ = chu;
		if (chu == '\0')
			break;
		} // while
	return pSortKey;
	} // PSortKeyAllocateForXmlNodeName()

VOID
CXmlNode::SortNodesByName()
	{
	CXmlSortContext xmlSortContext;
	xmlSortContext.m_pfnPSortKeyAllocateForElement = (PFn_PSortKeyAllocateForElement)PSortKeyAllocateForXmlNodeName;
	xmlSortContext.m_pfnSortCompare = (PFn_NCompareSortElements)NCompareSortElementsByBytesZ;
	_SortNodesR(INOUT &m_pElementsList, INOUT &xmlSortContext);
	}

VOID
CXmlNode::SortNodesByAttribute(PSZAC pszAttributeName)
	{

	}
*/
#if 0
SORTKEY *
PSortKeyAllocateForXmlNodeByAttributeID(INOUT class CAllocator * pAllocator, CXmlNode * pXmlNode)
	{
	Assert(pAllocator != NULL);
	Assert(pXmlNode != NULL);
	PSZUC pszuName = pXmlNode->PszuFindAttributeValue(c_szID);
	if (pszuName == NULL || pszuName[0] == '\0')
		pszuName = pXmlNode->m_pszuTagName;	// The node has no attribute, so use its element name instead

	Assert(pszuName != NULL);
	// We want to have the ID always as the first attribute, so we will change its sort key to something else
	if (pszuName[0] == 'I' && pszuName[1] == 'D')
		{
		Assert(pszuName[2] == '\0');
		pszuName = (PSZUC)c_szZero;
		}
	return pAllocator->PSortKeyAllocateForStringA((PSZAC)pszuName);
	/*
	const CHU * pchuSrc = pszuName;
	while (*pchuSrc++ != '\0')
		;
	SORTKEY * pSortKey = (SORTKEY *)pAllocator->PvAllocateData(sizeof(SORTKEY) + (pchuSrc - pszuName));
	// Build the sort key in lowercase
	CHU * pchuDst = (CHU *)(pSortKey + 1);
	while (TRUE)
		{
		UINT chu = *pszuName++;
		if (chu >= 'A' && chu <= 'Z')
			chu += 32;	// Make lowercase
		*pchuDst++ = chu;
		if (chu == '\0')
			break;
		} // while
	return pSortKey;
	*/
	} // PSortKeyAllocateForXmlNodeByAttributeID()

VOID
CXmlNode::SortNodesByAttributeID()
	{
	CXmlSortContext xmlSortContext;
	xmlSortContext.m_pfnPSortKeyAllocateForElement = (PFn_PSortKeyAllocateForElement)PSortKeyAllocateForXmlNodeByAttributeID;
	xmlSortContext.m_pfnSortCompare = (PFn_NCompareSortElements)NCompareSortElementsByBytesZ;
	_SortNodesR(INOUT &xmlSortContext);
	}

#endif
/*
VOID
CXmlNode::SortSiblingsByLineNumber(INOUT CArrayEx * parrayDummy, INOUT CAllocator * pAllocatorDummy)
	{
	Assert(parrayDummy != NULL);
	Assert(pAllocatorDummy != NULL);
	parrayDummy->RemoveAll();
	pAllocatorDummy->RecycleBuffer();

	CXmlNode * pXmlNode = this;
	while (TRUE)
		{
		SORTKEYINTEGER * pSortKey = pAllocatorDummy->PvAllocateSortKeyInteger(pXmlNode->GetLineNumber());
		parrayDummy->Add(IN &pSortKey->nValue);
		pSortKey->pvElement = pXmlNode;
		pXmlNode = pXmlNode->m_pNextSibling;
		if (pXmlNode == NULL)
			break;
		} // while

	int SortCompareIntegerPointers(const int * pnElement1, const int * pnElement2, LPARAM lParamCompareSort);
	parrayDummy->Sort((PFn_NCompareSortElements)SortCompareIntegerPointers);

	// Restore the sorting

	} // SortSiblingsByLineNumber()
*/

void
CXmlNode::UpdateAttributeValueToStaticString(PSZAC pszaAttributeName, PSZAC pszAttributeValue)
	{
	CXmlNode * pXmlNodeAttribute = PFindAttribute(pszaAttributeName);
	Report(pXmlNodeAttribute != NULL);
	if (pXmlNodeAttribute != NULL)
		pXmlNodeAttribute->m_pszuTagValue = (PSZU)pszAttributeValue;	// No need to allocate the attribute value, since it is assumed to be static
	}

void
CXmlNode::UpdateAttributeValuesSwap(PSZAC pszaAttributeNameA, PSZAC pszaAttributeNameB)
	{
	PSZU pszuAttributeValueA = NULL;
	PSZU pszuAttributeValueB = NULL;
	CXmlNode * pXmlNodeAttributeA = PFindAttribute(pszaAttributeNameA);
	Report(pXmlNodeAttributeA != NULL);
	CXmlNode * pXmlNodeAttributeB = PFindAttribute(pszaAttributeNameB);
	Report(pXmlNodeAttributeB != NULL);
	if (pXmlNodeAttributeB != NULL)
		pszuAttributeValueB = pXmlNodeAttributeB->m_pszuTagValue;
	if (pXmlNodeAttributeA != NULL)
		{
		pszuAttributeValueA = pXmlNodeAttributeA->m_pszuTagValue;
		pXmlNodeAttributeA->m_pszuTagValue = pszuAttributeValueB;
		}
	if (pXmlNodeAttributeB != NULL)
		pXmlNodeAttributeB->m_pszuTagValue = pszuAttributeValueA;
	}

void
CXmlNode::UpdateElementValueWithFormattedString_VL_Gsb(PSZAC pszaElementName, PSZAC pszFmtTemplateValue, va_list vlArgs)
	{
	CXmlNode * pXmlNodeElement = PFindElement(pszaElementName);
	Report(pXmlNodeElement != NULL);
	if (pXmlNodeElement != NULL)
		{
		// We need to allocate the value for the element
		pXmlNodeElement->m_pszuTagValue = PFindMemoryAccumulatorOfXmlTreeParent()->PszuAllocateCopyStringU(IN g_strScratchBufferStatusBar.Format_VL(pszFmtTemplateValue, vlArgs)); // Use the scratch buffer to format the string (this is safe as long as we are not using the status bar at the same time)
		//PFindMemoryAccumulatorOfParentXmlTree()->PszuAllocateContentOfCStr(IN &g_strScratchBufferStatusBar);
		}
	}


PSZUC
CXmlNode::PszFindAttributeValueId_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_id);
	}
/*
UINT
CXmlNode::UFindAttributeValueId_ZZR() const
	{
	return UFindAttributeValueDecimal_ZZR(c_sza_id);
	}
*/
TIMESTAMP
CXmlNode::LFindAttributeValueIdTimestamp_ZZR() const
	{
	return Timestamp_FromString_ZZR(PszuFindAttributeValue_NZ(c_sza_id));
	}
/*
UINT
CXmlNode::UFindAttributeValueStanzaId_ZZR() const
	{
	return UFindAttributeValueHexadecimal_ZZR(c_sza_id);
	}
*/

CXmlNode * CXmlNode::PFindElementQuery() const 	{ 	return PFindElement(c_sza_query); 	}
CXmlNode * CXmlNode::PFindElementIq() const { return PFindElement(c_sza_iq); }
CXmlNode * CXmlNode::PFindElementError() const { return PFindElement(c_sza_error); }
CXmlNode * CXmlNode::PFindElementData() const {	return PFindElement(c_sza_data); }
CXmlNode * CXmlNode::PFindElementText() const {	return PFindElement(c_sza_text); }
CXmlNode * CXmlNode::PFindElementSi() const { return PFindElement(c_sza_si); }
CXmlNode * CXmlNode::PFindElementClose() const { return PFindElement(c_sza_close); }

CXmlNode *
CXmlNode::PFindElementMatchingAttributeValueXmlns(PSZAC pszAttributeValue) const
	{
	return PFindElementMatchingAttributeNameAndValue(c_sza_xmlns, pszAttributeValue);
	}

PSZUC
CXmlNode::PszFindElementValueText() const
	{
	CXmlNode * pXmlElementText = PFindElementText();
	if (pXmlElementText != NULL)
		return pXmlElementText->m_pszuTagValue;
	return NULL;
	}

PSZUC
CXmlNode::PszFindStanzaValueQueryXmlns_NZ() const
	{
	CXmlNode * pXmlNodeQuery = PFindElementQuery();
	if (pXmlNodeQuery != NULL)
		return pXmlNodeQuery->PszFindAttributeValueXmlns_NZ();
	return c_szuEmpty;
	}

PSZUC
CXmlNode::PszFindAttributeValueXmlns_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_xmlns);
	}

PSZUC
CXmlNode::PszFindAttributeValueType_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_type);
	}

//	Return NULL if the attribute value is "error", otherwise return the value of the attribe, or c_szuEmpty if the attribute value was empty or non-existent
PSZUC
CXmlNode::PszFindAttributeValueTypeNoError() const
	{
	PSZUC pszType = PszFindAttributeValueType_NZ();
	if (FCompareStrings(pszType, c_sza_error))
		return NULL;
	return pszType;
	}
PSZUC
CXmlNode::PszFindAttributeValueJid_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_jid);	// Jabber Identifier
	}
PSZUC
CXmlNode::PszFindAttributeValueSid_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_sid);	// Session Identifier
	}
PSZUC
CXmlNode::PszFindAttributeValueFrom_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_from);
	}
PSZUC
CXmlNode::PszFindAttributeValueTo_NZ() const
	{
	return PszuFindAttributeValue_NZ(c_sza_to);
	}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
CXmlTree::CXmlTree()
	{
	Clear();	// Clear the root node
	Assert(m_pszuTagName == NULL);
	m_pszuTagName = (PSZU)c_szuEmpty;		// For performance reasons, prevent the tag name to be NULL
	m_uFlagsXmlFile = 0;
	m_pFileSizeAndMd5 = NULL;
	}

CXmlTree::~CXmlTree()
	{
	#ifdef DEBUG
	Destroy();	// Use the method Destroy() to force the values to ge initialized to a garbage value
	#endif
	}

VOID
CXmlTree::Destroy()
	{
	Clear();	// Clear the root node
	Assert(m_pszuTagName == NULL);
	m_pszuTagName = (PSZU)c_szuEmpty;		// For performance reasons, prevent the tag name to be NULL

//	m_binXmlFileData.Empty();				// Flush the file data
	m_accumulatorNodes.RecycleBuffer();		// Recycle the allocator
//	Assert(!FIsFileDataInitialized());
	}

//	Return TRUE if the content of the XML file is empty.
//	An empty XML file is a file without a single element node.
BOOL
CXmlTree::FIsEmpty() const
	{
	return (m_pElementsList == NULL);
	}
/*
VOID
CXmlTree::SetFileDataFromBackup(INOUT CBin * pbinFileDataBackup)
	{
	SetFileDataStealCopy(INOUT pbinFileDataBackup);

	// Make sure we set all the pointers to NULL to prevent them from pointing to garbage content
	Clear();
	m_pszuTagName = (PSZU)c_szuEmpty;
	}

VOID
CXmlTree::SetFileDataStealCopy(INOUT CBin * pbinFileData)
	{
	Assert(pbinFileData != NULL);
	if (pbinFileData != &m_binXmlFileData)
		m_binXmlFileData.BinInitFromCBinStolen(INOUT pbinFileData);
	}
*/

//	Set the file data to parse.  It is necessary to copy the content of the original
//	binary buffer because the parser will destroy the data stored in m_binXmlFileData.
//	If the caller wants to keep a copy of the original XML before parsing, then the caller must call this method.
VOID
CXmlTree::SetFileDataCopy(const CBin & binXmlFileData)
	{
	Assert(&binXmlFileData != NULL);
	Assert(&binXmlFileData != &m_binXmlFileData);
	m_binXmlFileData.BinInitFromCBinWithVirtualNullTerminator(IN &binXmlFileData);
	}

void
CXmlTree::SetFileDataCopy(const QByteArray & arraybXmlFileData)
	{
	Assert(&arraybXmlFileData != NULL);
	m_binXmlFileData.BinInitFromBinaryData(IN arraybXmlFileData.data(), arraybXmlFileData.size() + 1);
	}

void
CXmlTree::SetFileDataCopy(PSZUC pszuXmlFileDataStart, PSZUC pszuXmlFileDataStop)
	{
	m_binXmlFileData.BinInitFromBinaryDataWithVirtualNullTerminator(IN pszuXmlFileDataStart, pszuXmlFileDataStop - pszuXmlFileDataStart);
	}


EError
CXmlTree::ELoadFile(const QString & sFileName)
	{
	return m_binXmlFileData.BinFileReadE(sFileName);
	}

EError
CXmlTree::ELoadFileAndParseToXmlNodes(const QString & sFileName, INOUT_F_VALID CErrorMessage * pError)
	{
	Assert(pError != NULL);
	EError err = ELoadFile(sFileName);
	if (err != errSuccess)
		return err;
	if (m_pFileSizeAndMd5 != NULL)
		{
		// Remember the size of the file and its MD5 so we may skip a disk hit if the content has not changed when saving the XML content to disk
		(void)m_binXmlFileData.FAssignFileSizeAndMd5(INOUT_F_UNCH m_pFileSizeAndMd5);
		}
	return EParseFileDataToXmlNodes(INOUT_F_VALID pError);
	}

VOID
CXmlTree::_InitBuffersForFileParsing()
	{
	Clear();
	m_pszuTagName = (PSZU)c_szuEmpty;		// Prevent the tag name to be NULL, but can be empty
	m_accumulatorNodes.RecycleBuffer();
	m_accumulatorNodes.SetAllocSize(m_binXmlFileData.CbGetData());	// For performance reasons, pre-allocate some memory in the accumulator.  From statistical experience, the number of bytes required by the accumulator is roughly the same as the size of the XML file
	}


//	EParseFileDataToXmlNodes()
//
//	Core routine to parse an XML content into XML nodes.
//
//	Return ERROR_SUCCESS if the content of the XML file was parsed successfully.
//	Return ERROR_PARSE_* if there has been a serious error preventing the XML file to be parsed.
//	Any warning is stored in pError and returned as ERROR_SUCCESS.
//
//	PERFORMANCE NOTES
//	This method calls PszuNormalizeToUtf8AndReturnCodePage() which attempts to convert Unicode to UTF-8.  In the future, a flag could be passed to skip the normalization if the data is already known to be in the UTF-8 format.
EError
CXmlTree::EParseFileDataToXmlNodes(INOUT_F_VALID CErrorMessage * pError, INOUT PSZU pszuFileDataToParse)
	{
	Assert(pError != NULL);
	Endorse(pszuFileDataToParse == NULL);	// Use the content from m_binXmlFileData
	_InitBuffersForFileParsing();			// Clear any previously parsed file (if any)

	// The first thing is to determine the encoding of the file
	UINT chu;
	CHU * pchuData = pszuFileDataToParse;
	if (pchuData == NULL)
		{
		pchuData = (PSZU)m_binXmlFileData.PszuGetDataNZ();
		Assert(pchuData != NULL);
		while (Ch_FIsWhiteSpace(*pchuData))
			pchuData++;
		chu = *pchuData;
		if (chu == '<' && pchuData[1] == '?')
			{
			if (PszrCompareStringBeginNoCase(pchuData + 2, c_sza_xml) != NULL)
				{
				pchuData += 5;	// Skip the "<?xml"
				while (TRUE)
					{
					chu = *pchuData++;
					if (chu == '>')
						break;
					else if (chu == '\0')
						{
						pchuData--;	// Rewind the pointer to trigger an error in the next statement
						break;
						}
					} // while
				} // if
			} // if
		} // if

	// At this point, make sure we have a valid UTF-8 string
	AssertValidUtf8(IN pchuData);

	// Check if there are any '\n' or '\r' and normalize the strings to use '\n' as the end of line
	pszuFileDataToParse = pchuData;
	while (TRUE)
		{
		switch (*pszuFileDataToParse++)
			{
		case '\0':
			if (pszuFileDataToParse - pchuData <= 1)
				return errSuccess;	// The XML file is empty, therefore the 'parsing' is successful
		case '\n':
			goto BeginParsing;
		case '\r':
			// We got a '\r' so check if it is followed by a '\n'
			if (*pszuFileDataToParse == '\n')
				goto BeginParsing;	// We have a CRLF pair, so we are good
			// We have a single '\r' without the following '\n', so we assume the whole file uses '\r' as a line separator.
			// Therefore replace all '\r' by '\n'
			*(pszuFileDataToParse - 1) = '\n';
			while (TRUE)
				{
				switch (*pszuFileDataToParse)
					{
				case '\0':
					goto BeginParsing;
				case '\r':
					*pszuFileDataToParse = '\n';
					} // switch
				pszuFileDataToParse++;
				} // while
			} // switch
		} // while
	BeginParsing:

	// Now we got everything UTF-8, so begin the parsing of text into nodes
	int nLineNumber = 1;				// Current line number
	int nLineTag = 0;					// Line number where the tag begins.  The tag may span multiple lines if there are many attributes
	CXmlNode * pNode = this;
	PSZU pszuTagValue = NULL;		// The tag value may spawn several tags
	pszuFileDataToParse = pchuData;	// Remember the "beginning" of the data to parse.  This is important when we want to keep a comment and need to backtrack the white spaces.

	while (TRUE)
		{
		chu = *pchuData++;
		if (chu == '<')
			{
			// We got an XML tag
			*(pchuData - 1) = '\0';	// Remove the opening bracket to create a null-terminated string for the previous tag value
			if (pszuTagValue != NULL)
				{
				AssertValidUtf8(IN pszuTagValue);
				_AppendNodeTagValue(INOUT pNode, INOUT pszuTagValue);
				pszuTagValue = NULL;
				}
			nLineTag = nLineNumber;
			PSZU pszuTagNameBegin = pchuData;
			// Find the closing of the tag.  Typically this is done by searching for the > character but it may not be the case
			chu = *pchuData;
			if (chu == '!')
				{
				// Search for a <![CDATA[ tag, which is the exception of the comment
				chu = pchuData[1];
				if (chu == '[')
					{
					PSZU pszuTagValueCDATA = (PSZU)PszrCompareStringBeginNoCase(pchuData + 2, "CDATA[");
					if (pszuTagValueCDATA != NULL)
						{
						pchuData = pszuTagValueCDATA;
						// We have a [CDATA[ tag, so search for its termination tag ]]>
						while (TRUE)
							{
							chu = *pchuData++;
							if (chu == '\n')
								{
								nLineNumber++;
								continue;
								}
							// This loop is executed until we find the closing "]]>"
							if (chu == '\0')
								{
								pError->SetErrorCodeFormatted(errParseXmlPrematureEofForCDATA_i, nLineTag); // Premature end of file while looking for termination of <![CDATA[ from line $i
								pError->SetLineErrorSilent(nLineTag);
								return errParseXmlPrematureEofForCDATA_i;
								}
							if (chu == ']')
								{
								if (pchuData[0] == ']' && pchuData[1] == '>')
									{
									// We got our closing of the <![CDATA[...]]>
									*(pchuData - 1) = '\0';	// Remove the ]]>
									pchuData += 2;
									AssertValidUtf8(IN pszuTagValueCDATA);
									_AppendNodeTagValue(INOUT pNode, INOUT pszuTagValueCDATA, TRUE);
									break;
									}
								}
							} // while
						continue;	// Process the next character from the XML source
						} // if (<![CDATA[...]]>)
					}
				else if (chu == '-' && pchuData[2] == '-')
					{
					// We got an XML comment
					pchuData += 3;		// Skip the <!--
					PSZU pszuCommentBegin = pchuData;
					while (TRUE)
						{
						chu = *pchuData++;
						if (chu == '\n')
							{
							nLineNumber++;
							continue;
							}
						if (chu == '\0')
							goto PrematureEndOfFile;
						if (chu == '-' && pchuData[0] == '-' && pchuData[1] == '>')
							{
							if (m_uFlagsXmlFile & FXF_mskfKeepComments)
								{
								// Terminate the string for the comment, and remove any consecutive white spaces
								CHU * pchuEnd = pchuData - 1;
								while (TRUE)
									{
									*pchuEnd-- = '\0';	// Truncate the string
									if (!Ch_FIsWhiteSpace(*pchuEnd))
										break;
									}
								while (Ch_FIsWhiteSpace(*pszuCommentBegin))
									pszuCommentBegin++;

								// To keep the comment, create a node containing the comment
								CXmlNode * pNodeComment = (CXmlNode *)m_accumulatorNodes.PvAllocateData(sizeof(CXmlNode));
								Assert(IS_ALIGNED_32(pNodeComment));
								pNodeComment->m_pszuTagName = (PSZU)c_szCommentXML;
								pNodeComment->m_pszuTagValue = pszuCommentBegin;
								pNodeComment->m_lParam = nLineTag;
								pNodeComment->m_pParent = pNode;
								pNodeComment->m_pNextSibling = pNode->m_pElementsList;
								pNodeComment->m_pElementsList = NULL;
								pNodeComment->m_pAttributesList = NULL;
								pNode->m_pElementsList = pNodeComment;
								} // if (keep comments)
							pchuData += 2;	// Skip the closing comment -->
							break;
							} // if (end of comment)
						} // while
					continue;	// Process the next character from the XML source
					} // if...else (<!--...-->)

				// We got a regular XML comment and/or directive starting with <! so search for the closing >
				while (TRUE)
					{
					chu = *pchuData++;
					if (chu == '\n')
						{
						nLineNumber++;
						continue;
						}
					if (chu == '\0')
						goto PrematureEndOfFile;
					if (chu == '>')
						break;
					} // while
				continue;
				}
			else if (chu == '?')
				{
				// We got an XML directive which is almost identical to a comment
				while (TRUE)
					{
					chu = *pchuData++;
					if (chu == '\n')
						{
						nLineNumber++;
						continue;
						}
					if (chu == '\0')
						goto PrematureEndOfFile;
					if (chu == '?' && pchuData[0] == '>')
						{
						pchuData++;
						break;
						}
					} // while
				continue;
				}
			else if (chu == '/')
				{
				// We have a closing tag
				while (TRUE)
					{
					chu = *pchuData;
					if (chu == '\0')
						goto PrematureEndOfFile;
					if (chu == '>')
						{
						*pchuData++ = '\0';
						break;
						}
					pchuData++;
					} // while

				// Verify if it matches the current tag
				Assert(pNode != NULL);
				PSZUC pszuTagNameOpening = pNode->m_pszuTagName;
				if (pszuTagNameOpening == NULL || strcmpU(pszuTagNameBegin + 1, pszuTagNameOpening) != 0)
					{
					pError->SetErrorCodeFormatted(errParseXmlElementMismatch_ssi, pszuTagNameBegin, pszuTagNameOpening, pNode->m_lParam); // The closing element <$s> does not match its opening element <$s> from line $i
					pError->SetLineError(nLineTag);
					return errParseXmlElementMismatch_ssi;
					}
				pNode = pNode->m_pParent;	// Move back in the hierarchy
				continue;
				}
			else
				{
				// No tag value yet, so search for the closing tag by searching for the character >
				CXmlNode * pNodeAttributesList = NULL;
				while (TRUE)
					{
					chu = *pchuData++;
					if (chu == '\0')
						goto PrematureEndOfFile;
					if (chu == '/' && *pchuData == '>')
						{
						// We got a self closing element.
						*(pchuData - 1) = '\0';
						pchuData++;	// Skip the > character
						goto AllocateElement;
						}
					else if (chu == '>')
						{
						// We got the end of the opening tag
						*(pchuData - 1) = '\0';
						goto AllocateElement;
						}
					else
						{
						if (chu == '\n')
							nLineNumber++;
						if (pNodeAttributesList == NULL)
							{
							if (Ch_FIsWhiteSpace(chu))
								{
								// This is the first white space before the closing of the tag, so we may have attributes
								*(pchuData - 1) = '\0';		// Terminate the string
								while (TRUE)
									{
									while (TRUE)
										{
										chu = *pchuData;
										if (Ch_FIsWhiteSpace(chu))
											{
											if (chu == '\n')
												nLineNumber++;
											pchuData++;
											}
										else
											break;
										} // while
									if (chu == '/' && pchuData[1] == '>')
										{
										// Self closing tag having attributes without elements
										pchuData += 2;	// Skip the characters
										goto AllocateElement;	// Allocate the node
										}
									else if (chu == '>')
										{
										// Normal termination of a tag
										pchuData++;	// Skip the character
										goto AllocateElement;	// Allocate the node
										}

									const int nLineNumberAttribute = nLineNumber;	// Remember the line number of the attribute, because as we will be parsing the attribute, we may skip to another line
									PSZU pszuAttributeName = pchuData;
									// Scan until the end of the attribute name
									while (TRUE)
										{
										chu = *pchuData++;
										if (chu == '\0')
											goto PrematureEndOfFile;
										if (Ch_FIsWhiteSpace(chu))
											{
											*(pchuData - 1) = '\0';	// Terminate the string to get the attribute name
											while (Ch_FIsWhiteSpace(chu))
												{
												if (chu == '\n')
													nLineNumber++;
												chu = *pchuData++;
												}
											break;
											}
										if (chu == '=')
											{
											*(pchuData - 1) = '\0';	// Terminate the string to get the attribute name
											break;
											}
										} // while
									if (chu == '=')
										{
										CHS chuQuote = *pchuData;
										while (Ch_FIsWhiteSpace(chuQuote))
											{
											if (chuQuote == '\n')
												nLineNumber++;
											chuQuote = *++pchuData;
											}
										if (chuQuote == '\"' || chuQuote == '\'')
											{
											PSZU pszuAttributeValue = ++pchuData;
											while (TRUE)
												{
												chu = *pchuData++;
												if (chu == chuQuote)
													{
													*(pchuData - 1) = '\0';	// Truncate the string to get the attribute value
													XmlDecode(INOUT pszuAttributeValue);

													// Check if the attribute name contains a quote
													const CHU * pchuAttributeName = pszuAttributeName;
													while (TRUE)
														{
														switch (*pchuAttributeName++)
															{
														case '\0':
															goto AllocateAttribute;
														case '\"':
														case '\'':
															pError->SetErrorCodeFormatted(errParseXmlAttributeInvalidName_s, pszuAttributeName); // "The attribute name '$s' is invalid"
															pError->SetLineError(nLineNumberAttribute);
															return errParseXmlAttributeInvalidName_s;
															} // switch
														} // while

													// Allocate the attribute
													AllocateAttribute:
													CXmlNode * pNodeAttribute = (CXmlNode *)m_accumulatorNodes.PvAllocateData(sizeof(CXmlNode));
													Assert(IS_ALIGNED_32(pNodeAttribute));
													pNodeAttribute->m_pszuTagName = pszuAttributeName;
													pNodeAttribute->m_pszuTagValue = pszuAttributeValue;
													pNodeAttribute->m_lParam = nLineNumberAttribute;
													pNodeAttribute->m_pNextSibling = pNodeAttributesList;
													pNodeAttribute->m_pAttributesList = NULL;
													pNodeAttribute->m_pElementsList = NULL;
													pNodeAttributesList = pNodeAttribute;
													break;
													}
												if (chu == '\0')
													goto PrematureEndOfFile;
												if (chu == '\n')
													nLineNumber++;
												else if (chu == '<')
													{
													// An attribute is not allowed to have an opening < in it.  This is typically an error when the user forgot the closing quote
													pError->SetErrorCodeFormatted(errParseXmlAttributeMissingClosingQuote_s, pszuAttributeName); // "The attribute '$s' is missing its closing quote for its value"
													pError->SetLineError(nLineNumberAttribute);
													return errParseXmlAttributeMissingClosingQuote_s;
													}
												} // while
											}
										else
											{
											// We got an error; the attribute value must start with a single quote or a double quote
											pError->SetErrorCodeFormatted(errParseXmlAttributeMissingOpeningQuote_s, pszuAttributeName); // "The attribute '$s' is missing an opening quote for its value"
											pError->SetLineError(nLineNumberAttribute);
											return errParseXmlAttributeMissingOpeningQuote_s;

											}
										}
									else
										{
										// We got an error, since there must be an equal sign between the attribute name and its value.
										// This error however may occur when an attribute is missing its closing quote.
										// Therefore, analyze the value of the previous attribute to determine the best error to display.
										Endorse(pNodeAttributesList == NULL);	// This happens when an attribute does not have any value
										if (pNodeAttributesList != NULL)
											{
											Assert(pNodeAttributesList->m_pszuTagValue != NULL);
											PSZU pszuAttributeValuePrev = pNodeAttributesList->m_pszuTagValue;
											if (pszuAttributeValuePrev != NULL && pszuAttributeValuePrev[0] != '\0')
												{
												// Go to the end of the string
												while (*pszuAttributeValuePrev++ != '\0')
													;
												if (*(pszuAttributeValuePrev - 2) == '=')
													{
													// The value of the previous attribute ends with an equal (=) sign, which is probably because the closing quote was missing.
													// The error was in the previous attribute
													pError->SetErrorCodeFormatted(errParseXmlAttributeMissingClosingQuote_s, pNodeAttributesList->m_pszuTagName); // "The attribute '$s' is missing its closing quote for its value"
													pError->SetLineError(pNodeAttributesList->m_lParam);	// The line number is stored in the lParam
													return errParseXmlAttributeMissingClosingQuote_s;
													}
												}
											}
										pError->SetErrorCodeFormatted(errParseXmlAttributeMissingValue_s, pszuAttributeName); // "The attribute '$s' is missing a value"
										pError->SetLineError(nLineNumberAttribute);
										return errParseXmlAttributeMissingValue_s;
										}
									} // while
								Assert(pNodeAttributesList != NULL);
								} // if
							} // if
						} // if...else
					continue;
					AllocateElement:
					Assert(pszuTagNameBegin != NULL);
					Endorse(pszuTagNameBegin == '\0');	// We got an empty name because the tag is empty.  It is an error to have the <></> in an XML file, however the parser must handle this gracefully
					CXmlNode * pNodeNew = (CXmlNode *)m_accumulatorNodes.PvAllocateData(sizeof(CXmlNode));
					Assert(IS_ALIGNED_32(pNodeNew));
					pNodeNew->m_pszuTagName = pszuTagNameBegin;
					pNodeNew->m_pszuTagValue = pszuTagValue;
					pNodeNew->m_lParam = nLineTag;
					pNodeNew->m_pParent = pNode;
					pNodeNew->m_pNextSibling = pNode->m_pElementsList;
					pNodeNew->m_pElementsList = NULL;
					pNode->m_pElementsList = pNodeNew;
					pNodeNew->SetAttributesListReverse(pNodeAttributesList);
					#ifdef DEBUG
					AssertValidUtf8(pNode->m_pszuTagName);
					AssertValidUtf8(pNode->m_pszuTagValue);
					#endif
					pNodeAttributesList = NULL;
					if (chu != '/')
						pNode = pNodeNew;
					Assert(pszuTagValue == NULL);
					pszuTagValue = NULL;
					break;
					} // while
				//EmptyElement:
				continue;
				} // if...else
			} // if (chu == '<')

		if (chu == '\n')
			{
			nLineNumber++;
			continue;
			}
		if (chu == '\0')
			break;
		if (pszuTagValue == NULL)
			pszuTagValue = pchuData - 1;
		} // while


	// Before returning success, reverse the linked list of elements.  The attributes have already been reversed.
	// This is because the implementation does always insert a new element at the beginning of the linked list.
	ReverseLinkedListsElementsOnly();

	// The root element always has an empty name, and therefore can be collapsed
	Assert(m_pszuTagName != NULL);
	Assert(*m_pszuTagName == '\0');
	Assert(m_pAttributesList == NULL);
	Assert(m_pParent == NULL);
	Assert(m_pNextSibling == NULL);

	if (m_pElementsList != NULL && m_pszuTagValue == NULL)
		{
		memcpy(OUT this, IN m_pElementsList, sizeof(*m_pElementsList));
		// Typically, there is only one element since there should be a single node, however walk to all the roots and attributes element and change the parent pointer
		CXmlNode * pThis = this;
		while (TRUE)
			{
			pThis->m_pParent = NULL;
			pNode = pThis->m_pElementsList;
			while (pNode != NULL)
				{
				pNode->m_pParent = pThis;
				pNode = pNode->m_pNextSibling;
				}
			pNode = pThis->m_pAttributesList;
			while (pNode != NULL)
				{
				pNode->m_pParent = pThis;
				pNode = pNode->m_pNextSibling;
				}
			pThis = pThis->m_pNextSibling;
			if (pThis == NULL)
				break;
			} // while
		Assert(*m_pszuTagName != '\0');
		}
	Assert(m_pszuTagName != NULL);
	if (m_pszuTagName[0] != '\0')
		return errSuccess;
	// If we reach this point, it is because we parsed the whole XML stream and were unable to extract a single element
	/*
		// The root element do

		}
	Endorse(*m_pszuTagName == '\0');	// This may happen if the root is invalid
	return errSuccess;
	*/

PrematureEndOfFile:
	#ifdef DEBUG
	PSZAC pszEllipsis = NULL;
	int ibEndOfFile = pchuData - pszuFileDataToParse;
	if (ibEndOfFile > 100)
		{
		pszuFileDataToParse = pchuData - 100;	// Just display the tailing 100 characters
		pszEllipsis = " [...]";
		}
	TRACE6("Unexpected end of XML data at offset $I (chStop = $i, '{s1}'):$s {Pn}", ibEndOfFile, *pchuData, pchuData, pszEllipsis, pszuFileDataToParse, pchuData);
	#endif

	pError->SetErrorCodeFormatted(errParseXmlPrematureEof_i, nLineTag);	// Unexpected end of file at line $i
	pError->SetLineErrorSilent(nLineTag);
	return errParseXmlPrematureEof_i;
	} // EParseFileDataToXmlNodes()

EError
CXmlTree::EParseFileDataToXmlNodesModify_ML(INOUT CBin * pbinXmlFileData)
	{
	m_binXmlFileData.BinInitFromCBinStolen(INOUT pbinXmlFileData);	// TODO: BinInitFromCBinStolenWithVirtualNullTerminator()
	m_binXmlFileData.BinAppendNullTerminatorVirtualSzv();
	return EParseFileDataToXmlNodes_ML();
	}

EError
CXmlTree::EParseFileDataToXmlNodesCopy_ML(IN const CBin & binXmlFileData)
	{
	m_binXmlFileData.BinInitFromCBinWithVirtualNullTerminator(IN &binXmlFileData);
	return EParseFileDataToXmlNodes_ML();
	}

EError
CXmlTree::EParseFileDataToXmlNodesCopy_ML(const QByteArray & arraybXmlFileData)
	{
	m_binXmlFileData.BinInitFromByteArray(arraybXmlFileData);
	return EParseFileDataToXmlNodes_ML();
	}

//	Parse the XML into nodes and display any error to the Message Log and Error Log
EError
CXmlTree::EParseFileDataToXmlNodes_ML()
	{
	CErrorMessage error;
	EError err = EParseFileDataToXmlNodes(INOUT_F_VALID &error);
	if (err != errSuccess)
		{
		Assert(error.EGetErrorCode() == err);
		(void)error.FDisplayToMessageLog();
		}
	return err;
	}

/*
VOID
CXmlTree::ParseXmlNodesWithoutError(PSZUC pszuXmlNodes, int cbXmlNodes)
	{
	Assert(pszuXmlNodes != NULL);
	Assert(strlenU(pszuXmlNodes) + 1 == (UINT)cbXmlNodes);
	m_binXmlFileData.BinInitFromBinaryData(IN pszuXmlNodes, cbXmlNodes);
	CError error;
	DEBUG_CODE( APIERR err = ) EParseFileDataToXmlNodes(INOUT IGNORED &error, INOUT m_binXmlFileData.PbGetData());
	Assert(err == ERROR_SUCCESS);
	}
*/


//	Return the root node only if it matches the name pszuNameRootNode
//	Return NULL if the file is empty, or the name of the root node does not match pszuNameRootNode, or if there are multiple root nodes.
CXmlNode *
CXmlTree::PGetRootNodeValidate(PSZAC pszuNameRootNode) const
	{
	Assert(pszuNameRootNode != NULL);
	Assert(*pszuNameRootNode != '\0');

	CXmlNode * pNodeRoot = (CXmlNode *)this;
	while (TRUE)
		{
		CXmlNode * pNextSibling = pNodeRoot->m_pNextSibling;
		if (!pNodeRoot->FIsNodeComment())
			{
			// Validate the root node name
			if (pNodeRoot->FCompareTagName(pszuNameRootNode))
				{
				// Make sure there is only a single root node
				if (pNextSibling == NULL)
					return pNodeRoot;
				}
			break;
			}
		pNodeRoot = pNextSibling;
		if (pNodeRoot == NULL)
			break;
		} // while
	return NULL;
	} // PGetRootNodeValidate()

/*
CXmlNode *
CXmlTree::PGetRootNodeValidateUI(PSZAC pszuNameRootNode, HWND hwndEditLog) const
	{
	Assert(IsWindow(hwndEditLog));

	CXmlNode * pNodeRoot = PGetRootNodeValidate(pszuNameRootNode);
	if (pNodeRoot == NULL)
		EditW_AppendTextFormatSev(hwndEditLog, eSeverityErrorWarning, "Unable to find root node named <$U>\n", pszuNameRootNode);
	return pNodeRoot;
	}
*/

//	Make sure the root node is not empty.
//	This method is used after constructing an XML tree where the root node
//	may be an empty string and the actual root node is the first element.
VOID
CXmlTree::RootNodeEnsureNotEmpty()
	{
	Assert(m_pszuTagName != NULL);
	if (m_pszuTagName[0] != '\0')
		return;

	if (m_pElementsList != NULL)
		{
		Assert(m_pElementsList->m_pParent == this);
		m_pElementsList->m_pParent = NULL;
		m_pszuTagName = m_pElementsList->m_pszuTagName;
		CXmlNode * pXmlNodeElement = m_pElementsList->m_pElementsList;
		m_pElementsList = pXmlNodeElement;
		while (pXmlNodeElement != NULL)
			{
			pXmlNodeElement->m_pParent = this;
			pXmlNodeElement = pXmlNodeElement->m_pNextSibling;
			}
		}
	} // RootNodeEnsureNotEmpty()


CXmlNode *
CXmlTree::_PAllocateElementOrAttribute(PSZAC pszaElementOrAttributeName)
	{
	CXmlNode * pNode = (CXmlNode *)m_accumulatorNodes.PvAllocateData(sizeof(CXmlNode));
	Assert(IS_ALIGNED_32(pNode));
	InitToZeroes(OUT pNode, sizeof(*pNode));
	pNode->m_pszuTagName = (PSZU)pszaElementOrAttributeName;
	return pNode;
	}


//	Notice the element name is not allocated, simply a pointer reference to pszaElementName
CXmlNode *
CXmlTree::_PAllocateElement(INOUT CXmlNode * pNodeElementParent, PSZAC pszaElementName)
	{
	Assert(pNodeElementParent != NULL);
	Assert(pNodeElementParent->PFindRootNodeXmlTreeParent() == this);
	CXmlNode * pNode = (CXmlNode *)m_accumulatorNodes.PvAllocateData(sizeof(CXmlNode));
	Assert(IS_ALIGNED_32(pNode));
	InitToZeroes(OUT pNode, sizeof(*pNode));
	pNode->m_pszuTagName = (PSZU)pszaElementName;
	pNode->m_pParent = pNodeElementParent;
	pNode->m_pNextSibling = pNodeElementParent->m_pElementsList;
	pNodeElementParent->m_pElementsList = pNode;
	return pNode;
	}

CXmlNode *
CXmlTree::_PAllocateAttribute(CXmlNode * pNodeElementParent, PSZAC pszaAttributeName, PSZUC pszuAttributeValue)
	{
	Assert(pNodeElementParent != NULL);
	Assert(pNodeElementParent->PFindRootNodeXmlTreeParent() == this);
	Assert(pszaAttributeName != NULL);
	Assert(*pszaAttributeName != '\0');
	Endorse(pszuAttributeValue == NULL);

	CXmlNode * pNode = (CXmlNode *)m_accumulatorNodes.PvAllocateData(sizeof(CXmlNode));
	Assert(IS_ALIGNED_32(pNode));
	InitToZeroes(OUT pNode, sizeof(*pNode));
	pNode->m_pszuTagName = (PSZU)pszaAttributeName;
	pNode->m_pszuTagValue = (PSZU)pszuAttributeValue;
	pNode->m_pParent = pNodeElementParent;
	pNode->m_pNextSibling = pNodeElementParent->m_pAttributesList;
	pNodeElementParent->m_pAttributesList = pNode;
	return pNode;
	}

CXmlNode *
CXmlTree::_PAllocateAttributeWithExtraStorage(CXmlNode * pNodeElementParent, PSZAC pszaAttributeName, int cbAllocExtra)
	{
	Assert(pNodeElementParent != NULL);
	Assert(pNodeElementParent->PFindRootNodeXmlTreeParent() == this);
	Assert(pszaAttributeName != NULL);
	Assert(*pszaAttributeName != '\0');
	Assert(cbAllocExtra > 0);
	Assert(FALSE && "NYI");
	return 0;
	}

VOID
CXmlTree::_AppendNodeTagValue(INOUT CXmlNode * pNode, INOUT PSZU pszuTagValue, BOOL fCDATA)
	{
	Assert(pNode != NULL);
	Assert(pszuTagValue != NULL);
	UINT chu;
	CHU * pchu;
	// First, determine if the pszuTagValue contains something other than white spaces
	if (!fCDATA)
		{
		pchu = pszuTagValue;
		while (TRUE)
			{
			chu = *pchu++;
			if (chu == '\0')
				return;	// We have reached the end of file without any non-white space character
			if (!Ch_FIsWhiteSpace(chu))
				break;
			}
		XmlDecode(INOUT pszuTagValue);
		}
	PSZU pszuTagValuePrevious = pNode->m_pszuTagValue;
	if (pszuTagValuePrevious == NULL)
		{
		pNode->m_pszuTagValue = pszuTagValue;
		#ifdef DEBUG
		AssertValidUtf8(pNode->m_pszuTagValue);
		#endif
		return;
		}
	// Concatenate the two strings
	pchu = pszuTagValuePrevious;
	while (*pchu != '\0')
		pchu++;
	int cbTagValuePrevious = pchu - pszuTagValuePrevious;
	pchu = pszuTagValue;
	while (*pchu++ != '\0')
		;
	int cbTagValue = pchu - pszuTagValue;

	PSZU pszuTagValueNew = (PSZU)m_accumulatorNodes.PvAllocateData(NEXT_UINT32(cbTagValuePrevious + 1 + cbTagValue));
	pNode->m_pszuTagValue = pszuTagValueNew;
	memcpy(OUT pszuTagValueNew, pszuTagValuePrevious, cbTagValuePrevious);
	pszuTagValueNew += cbTagValuePrevious;
	pszuTagValueNew[0] = ' ';	// Insert a space between the two strings
	memcpy(OUT pszuTagValueNew + 1, pszuTagValue, cbTagValue);
	AssertValidUtf8(IN pszuTagValueNew);
	} // _AppendNodeTagValue()

/*
class CHashElementPointerSerializer : public CHashElement
{
public:
	PCVOID m_pvKey;		// The pointer itself is the key
	UINT m_uID;			// The identifier of the pointer when serializing
};

//	Hash table to serialize pointers
class CHashTablePointerSerializer : public CHashTable
{
public:
	UINT UGetPointerID(PCVOID pvKey);
};
*/

/*
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
CHashTableXmlNodes::CHashTableXmlNodes() : CHashTable((PFn_PszGetHashKey)PszuGetHashKey, eHashFunctionStringNoCase)
	{
	SetHashElementSize(sizeof(CHashElementXmlNode));
	}

VOID
CHashTableXmlNodes::BuildHashTableOfIDsFromNodeParent(const CXmlNode * pXmlNodeParent)
	{
	if (pXmlNodeParent != NULL)
		BuildHashTableOfIDsFromFirstElement(IN pXmlNodeParent->m_pElementsList, c_szID);
	}

#ifdef DEBUG
CHashTableAllocateUniqueStringsU g_hashtableUniqueStrings;
#endif

VOID
CHashTableXmlNodes::BuildHashTableOfIDsFromFirstElement(const CXmlNode * pXmlNodeFirstElement, PSZAC pszaAttributeNameID)
	{
	Endorse(pXmlNodeFirstElement == NULL);
	Assert(pszaAttributeNameID != NULL);
	Assert(pszaAttributeNameID[0] != '\0');

	while (pXmlNodeFirstElement != NULL)
		{
		PSZUC pszuID = pXmlNodeFirstElement->PszuFindAttributeValue(pszaAttributeNameID);
		if (pszuID != NULL && *pszuID != '\0')
			{
			#ifdef DEBUG
			PSZUC pszuUniqueString = g_hashtableUniqueStrings.PszuAllocateUniqueStringU(pszuID);
			Assert(strcmpequalUU(pszuUniqueString, pszuID));
			#endif

			// Add the node to the hash table
			UINT uHashValue;
			if (PFindHashElementWithHashValue((PSZAC)pszuID, OUT &uHashValue) == NULL)
				{
				CHashElementXmlNode * pHashElement = (CHashElementXmlNode *)PAllocateHashElement();
				Assert(IS_ALIGNED_32(pHashElement));
				pHashElement->m_pszuNodeValue = pszuID;
				pHashElement->m_pNodeXML = const_cast<CXmlNode *>(pXmlNodeFirstElement);
				pHashElement->SetHashValue(uHashValue);
				Add(INOUT pHashElement);
				}
			#ifdef DEBUG
			else
				{
				TRACE2("Duplicate IDs '$s' at line $i.\n", pszuID, pXmlNodeFirstElement->GetLineNumber());
				} // if...else
			#endif
			}
		#ifdef DEBUG
		else if (!pXmlNodeFirstElement->FIsNodeComment())
			{
			TRACE2("XML node at line $i is missing the attribute '$s'.\n", pXmlNodeFirstElement->GetLineNumber(), pszaAttributeNameID);
			} // if...else
		#endif // DEBUG

		pXmlNodeFirstElement = pXmlNodeFirstElement->m_pNextSibling;	// Go to the next node
		} // while
	} // BuildHashTableOfIDsFromFirstElement()
*/
/*
//	Return TRUE if the hash table is build of element names, or FALSE by IDs.
BOOL
CHashTableXmlNodes::BuildHashTableOfElementNamesOrAttributeIDs(const CXmlNode * pXmlNodeFirstElement, PSZAC pszaAttributeID)
	{
	Assert(pXmlNodeFirstElement != NULL);
	Assert(pszaAttributeID != NULL && pszaAttributeID[0] != '\0');

	RemoveAll();	// Empty the previous hash table


	// Determine if we must build the hash table from the attribute ID, or from the element names
	PSZUC pszuID = pXmlNodeFirstElement->PszuFindAttributeValue(pszaAttributeID);
	if (pszuID != NULL && *pszuID != '\0')
		{
		// We have an attribute, so build the hash table from
		BuildHashTableOfIDsFromFirstElement(IN pXmlNodeFirstElement);
		return FALSE;
		}

	while (TRUE)
		{
		// Add the node to the hash table
		PSZUC pszuNodeName = pXmlNodeFirstElement->m_pszuTagName;

		UINT uHashValue;
		if (PFindHashElementWithHashValue((PSZAC)pszuNodeName, OUT &uHashValue) == NULL)
			{
			CHashElementXmlNode * pHashElement = (CHashElementXmlNode *)PAllocateHashElement();
			Assert(IS_ALIGNED_32(pHashElement));
			pHashElement->m_pszuNodeValue = pszuNodeName;
			pHashElement->m_pNodeXML = const_cast<CXmlNode *>(pXmlNodeFirstElement);
			pHashElement->SetHashValue(uHashValue);
			Add(INOUT pHashElement);
			}
		#ifdef DEBUG
		else
			{
			TRACE2("Line $i: Node <$s> is already in hash table.\n", pXmlNodeFirstElement->GetLineNumber(), pszuNodeName);
			} // if...else
		#endif

		pXmlNodeFirstElement = pXmlNodeFirstElement->m_pNextSibling;	// Go to the next node
		if (pXmlNodeFirstElement == NULL)
			break;
		} // while

	return TRUE;
	} // BuildHashTableOfElementNamesOrAttributeIDs()
*/
/*
VOID
CHashTableXmlNodes::AddHashElementXmlNode(PSZAC pszaNodeValue, CXmlNode * pNodeXML)
	{
	Assert(pszaNodeValue != NULL);
	Assert(*pszaNodeValue != '\0');
	Endorse(pNodeXML == NULL);

	// Allocate a unique hash element
	BOOL fElementNewlyAllocated = FALSE;
	CHashElementXmlNode * pHashElement = (CHashElementXmlNode *)PFindHashElementAllocate(pszaNodeValue, INOUT &fElementNewlyAllocated);
	Assert(pHashElement != NULL);
	Assert(IS_ALIGNED_32(pHashElement));
	#ifdef DEBUG
	if (!fElementNewlyAllocated)
		{
		// The hash element was already there
		Assert(strcmpequalUU(pHashElement->m_pszuNodeValue, (PSZUC)pszaNodeValue));
		Assert(pHashElement->m_pNodeXML == pNodeXML);
		}
	#endif
	pHashElement->m_pszuNodeValue = (PSZUC)pszaNodeValue;
	pHashElement->m_pNodeXML = pNodeXML;
	} // AddHashElementXmlNode()
*/

//	Serialize all the XML nodes into an UTF-8 string stored in m_binXmlFileData.
VOID
CXmlTree::SerializeTreeToXml(UINT uSerializeFlagsExtra) CONST_OUTPUT_BUFFER
	{
	Assert(m_binXmlFileData.CbGetData() == 0 && "The buffer to serialize the XML tree should be empty");
	m_binXmlFileData.PvSizeAlloc(2000);		// Make sure we start with a minimum of 2 KB
	/*
	if ((uSerializeFlagsExtra & CXmlNode::STBF_kfNoHeaderXml) == 0)
		m_binXmlFileData.InitXmlHeaderForUtf8();
	*/
	SerializeToBinUtf8(IOUT &m_binXmlFileData, CXmlNode::STBF_kfIndentTags | CXmlNode::STBF_kfCRLF | uSerializeFlagsExtra);
	m_binXmlFileData.BinAppendNullTerminatorVirtualSzv();
	} // SerializeTreeToXml()
/*
APIERR
CXmlTree::EWriteToFileW(PSZWC pszwFileName) const
	{
	Assert(pszwFileName != NULL);

	CBin binFile;
	SerializeFileToUtf8(OUT &binFile);
	return binFile.EWriteToFileW(pszwFileName);
	} // EWriteToFileW()
*/
/*
CXmlNode *
CXmlTree::_PCloneNode(const CXmlNode * pNodeToClone)
	{
	Assert(pNodeToClone != NULL);

	m_allocator.AlignToUInt32();
	CXmlNode * pNodeCloned = (CXmlNode *)m_allocator.PvAllocateData(sizeof(CXmlNode));
	Assert(IS_ALIGNED_32(pNodeCloned));
	pNodeCloned->m_pszuTagName = m_allocator.PszuAllocateStringNullU(pNodeToClone->m_pszuTagName);
	pNodeCloned->m_pszuTagValue = m_allocator.PszuAllocateStringNullU(pNodeToClone->m_pszuTagValue);
	pNodeCloned->m_lParam = pNodeToClone->m_lParam;
	pNodeCloned->m_pParent = NULL;
	pNodeCloned->m_pNextSibling = NULL;
	pNodeCloned->m_pAttributesList = NULL;
	pNodeCloned->m_pElementsList = NULL;
	return pNodeCloned;
	} // _PCloneNode()

/////////////////////////////////////////////////////////////////////
VOID
CXmlNode::NodesClone(INOUT CXmlTree * pTreeClone, INOUT CXmlNode * pNodeFileXmlAttachTo) const
	{
	Assert(pTreeClone != NULL);
	Endorse(pNodeFileXmlAttachTo == NULL);	// Attach to the root of pTreeClone

	if (pNodeFileXmlAttachTo == NULL)
		pNodeFileXmlAttachTo = pTreeClone;

	Assert(pNodeFileXmlAttachTo->PFindRootNodeXmlTreeParent() == pTreeClone);

	// Clone the current node
	CXmlNode * pNodeCloned = pTreeClone->_PCloneNode(this);
	pNodeCloned->m_pParent = pNodeFileXmlAttachTo;
	pNodeCloned->m_pNextSibling = pNodeFileXmlAttachTo->m_pNextSibling;
	pNodeFileXmlAttachTo->m_pElementsList = pNodeCloned;

	// Clone the attributes
	CXmlNode * pAttributeToClone = m_pAttributesList;
	while (pAttributeToClone != NULL)
		{
		Assert(pAttributeToClone->m_pElementsList == NULL);
		Assert(pAttributeToClone->m_pAttributesList == NULL);
		Assert(pAttributeToClone->m_pParent == this);

		CXmlNode * pNode = pTreeClone->_PCloneNode(pAttributeToClone);
		pNode->m_pParent = pNodeCloned;
		pNode->m_pNextSibling = pNodeCloned->m_pAttributesList;
		pNodeCloned->m_pAttributesList = pNode;
		pAttributeToClone = pAttributeToClone->m_pNextSibling;		// Go to the next attribute
		} // while

	// Clone the elements
	CXmlNode * pElementToClone = m_pElementsList;
	while (pElementToClone != NULL)
		{
		pElementToClone->NodesClone(INOUT pTreeClone, pNodeCloned);	// Recursively clone the elements
		pNodeCloned->m_pNextSibling = pNodeFileXmlAttachTo->m_pNextSibling;
		pNodeFileXmlAttachTo->m_pElementsList = pNodeCloned;


		pElementToClone = pElementToClone->m_pNextSibling;	// Go to the next element
		}

	} // NodesClone()

VOID
CXmlNode::NodesClonedRestore(const CXmlNode * pNodesXmlCloned)
	{

	}
*/

//	Return the root node parent which is the CXmlTree.
//	This method never returns NULL
//
CXmlTree *
CXmlNode::PFindRootNodeXmlTreeParent() const
	{
	Assert(this != NULL);
	CXmlTree * pTreeParent = (CXmlTree *)this;
	CXmlNode * pNodeParent = m_pParent;
	while (pNodeParent != NULL)
		{
		pTreeParent = (CXmlTree *)pNodeParent;
		pNodeParent = pNodeParent->m_pParent;
		}
	return pTreeParent;
	} // PFindRootNodeXmlTreeParent()

CMemoryAccumulator *
CXmlNode::PFindMemoryAccumulatorOfXmlTreeParent() const
	{
	return &PFindRootNodeXmlTreeParent()->m_accumulatorNodes;
	}

//	XmlExchange(), virtual
void
IXmlExchange::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	Assert(FALSE && "Must be implemented");
	}

//	IXmlExchangeObjectID::IRuntimeObject::PGetRuntimeInterface()
POBJECT
IXmlExchangeObjectID::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	if (rti == RTI(IXmlExchangeObjectID))
		return (void *)this;
	return IXmlExchange::PGetRuntimeInterface(rti, piParent);
	}

//	IXmlExchangeObjectID::IXmlExchange::XmlExchange()
void
IXmlExchangeObjectID::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	pXmlExchanger->XmlExchangeUInt("_ID", INOUT_F_UNCH_S &mu_Cookie.uSerializeObjectId);
	}


IXmlExchangeObjectID *
CArrayPtrXmlSerializableObjects::PFindObjectById(UINT uObjectId) const
	{
	if (uObjectId != 0)
		{
		IXmlExchangeObjectID ** ppObject;
		IXmlExchangeObjectID ** ppObjectStop;
		ppObject = PrgpGetObjectsStop(OUT &ppObjectStop);
		while (ppObject != ppObjectStop)
			{
			IXmlExchangeObjectID * pObject = *ppObject++;
			Assert(pObject != NULL);
			Assert(pObject->PGetRuntimeInterface(RTI(IXmlExchangeObjectID), NULL) == pObject);
			Assert(pObject->mu_Cookie.uSerializeObjectId != 0);
			if (pObject->mu_Cookie.uSerializeObjectId == uObjectId)
				return pObject;
			} // while
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "PFindObjectById() - Identifier $u is out of range\n", uObjectId);
		} // if
	return NULL;
	}

IXmlExchangeObjectID *
CArrayPtrXmlSerializableObjects::PFindObjectByIdFromXmlNodeValue(const CXmlNode * pXmlNode) const
	{
	if (pXmlNode != NULL)
		return PFindObjectById(NStringToNumber_ZZR_ML(pXmlNode->m_pszuTagValue));
	return NULL;
	}
//	Chain the assignment of Object IDs
UINT
CArrayPtrXmlSerializableObjects::ForEach_UAssignObjectIds(UINT uObjectIdPrevious) const
	{
	IXmlExchangeObjectID ** ppObject;
	IXmlExchangeObjectID ** ppObjectStop;
	ppObject = PrgpGetObjectsStop(OUT &ppObjectStop);
	while (ppObject != ppObjectStop)
		{
		IXmlExchangeObjectID * pObject = *ppObject++;
		Assert(pObject != NULL);
		Assert(pObject->PGetRuntimeInterface(RTI(IXmlExchangeObjectID), NULL) == pObject);
		pObject->mu_Cookie.uSerializeObjectId = ++uObjectIdPrevious;
		}
	return uObjectIdPrevious;
	}

void
CArrayPtrXmlSerializableObjects::ForEach_SetCookieValue(LPARAM lParam) const
	{
	IXmlExchangeObjectID ** ppObject;
	IXmlExchangeObjectID ** ppObjectStop;
	ppObject = PrgpGetObjectsStop(OUT &ppObjectStop);
	while (ppObject != ppObjectStop)
		{
		IXmlExchangeObjectID * pObject = *ppObject++;
		Assert(pObject != NULL);
		Assert(pObject->PGetRuntimeInterface(RTI(IXmlExchangeObjectID), NULL) == pObject);
		pObject->mu_Cookie.lParam = lParam;
		}
	}

void
CArrayPtrXmlSerializableObjects::ForEach_SetCookieValueToNULL() const
	{
	IXmlExchangeObjectID ** ppObject;
	IXmlExchangeObjectID ** ppObjectStop;
	ppObject = PrgpGetObjectsStop(OUT &ppObjectStop);
	while (ppObject != ppObjectStop)
		{
		IXmlExchangeObjectID * pObject = *ppObject++;
		Assert(pObject != NULL);
		Assert(pObject->PGetRuntimeInterface(RTI(IXmlExchangeObjectID), NULL) == pObject);
		pObject->mu_Cookie.lParam = 0;
		}
	}

/*
void
CArrayPtrXmlSerializableObjects::AppendObjectsWithoutDuplicates(const CArrayPtrXmlSerializableObjects & arraypAppend)
	{
	}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
CXmlExchanger::CXmlExchanger(BOOL fSerialize)
	{
	m_fSerializing = fSerialize;
	m_pXmlNodeSerialize = NULL;
	m_uVersionFileFormat = d_zNA;
	}

EError
CXmlExchanger::ELoadFileAndParseToXmlNodes(const QString & sFileName)
	{
	return CXmlTree::ELoadFileAndParseToXmlNodes(sFileName, INOUT &m_error);
	}

void
CXmlExchanger::StackPushNode(CXmlNode * pXmlNodeSerialize)
	{
	Assert(pXmlNodeSerialize != NULL);
	Endorse(m_pXmlNodeSerialize == NULL);
	m_arraypStack.StackPushElement(m_pXmlNodeSerialize);
	m_pXmlNodeSerialize = pXmlNodeSerialize;
	}

CXmlNode **
CXmlExchanger::PpStackPushNodes(CXmlNode * pXmlNodeSerialize)
	{
	Assert(pXmlNodeSerialize != NULL);
	Endorse(m_pXmlNodeSerialize == NULL);
	m_arraypStack.StackPushElement(m_pXmlNodeSerialize);
	m_arraypStack.StackPushElement(pXmlNodeSerialize);
	DEBUG_CODE( m_pXmlNodeSerialize = (CXmlNode *)d_pszuGarbageValue);
	return &m_pXmlNodeSerialize;
	}

void
CXmlExchanger::StackPopNodes()
	{
	StackPopNode();
	StackPopNode();
	}

void
CXmlExchanger::StackPopNode()
	{
	m_pXmlNodeSerialize = (CXmlNode *)m_arraypStack.PvStackPopElement();
	}

PSZUC
CXmlExchanger::XmlExchange_PszGetValue(PSZAC pszuTagName) const
	{
	return m_pXmlNodeSerialize->PszuFindElementOrAttributeValue(pszuTagName);
	}

//	Return FALSE if an error occurred.
BOOL
CXmlExchanger::XmlExchangeNodeRootF(PSZAC pszuNameRootNode)
	{
	Assert(pszuNameRootNode != NULL);
	Assert(pszuNameRootNode[0] != '\0');
	if (m_fSerializing)
		{
		m_pszuTagName = (PSZU)pszuNameRootNode;	// Set the name of the root node.  There is no need to allocate an element, since CXmlExchanger inherits CXmlTree which inherits CXmlNode
		StackPushNode(this);
		(void)_PAllocateAttribute(this, c_szaVersion, (PSZUC)d_szVersionFileFormatCurrent);
		}
	else
		{
		CXmlNode * pNodeRoot = PGetRootNodeValidate(pszuNameRootNode);
		if (pNodeRoot == NULL)
			return FALSE;
		StackPushNode(pNodeRoot);
		m_uVersionFileFormat = UFindAttributeValueDecimal_ZZR(c_szaVersion);
		if (m_uVersionFileFormat == 0)
			m_uVersionFileFormat = UFindAttributeValueDecimal_ZZR(d_chXmlAttributeVersion);	// Prepare to eventually remove c_szaVersion
		}
	return TRUE;
	} // XmlExchangeNodeRootF()

void
CXmlExchanger::XmlExchangeElementBegin(PSZAC pszuElementName)
	{
	CXmlNode * pNodeElement = m_pXmlNodeSerialize->PFindElement(pszuElementName);
	if (pNodeElement == NULL)
		pNodeElement = _PAllocateElement(m_pXmlNodeSerialize, pszuElementName);		// Create a temporary element so we may read or write from the node
	StackPushNode(pNodeElement);
	Assert(FCompareStrings(m_pXmlNodeSerialize->m_pszuTagName, pszuElementName));
	}

void
CXmlExchanger::XmlExchangeElementEnd(PSZAC pszuElementName)
	{
	Assert(FCompareStrings(m_pXmlNodeSerialize->m_pszuTagName, pszuElementName));
	Assert(m_pXmlNodeSerialize->m_pszuTagValue == NULL);	// This kind of element should not have any value
	if (m_fSerializing)
		{
		// If the element is empty, then remove it so we do not serialize unnecessary data
		if (m_pXmlNodeSerialize->m_pElementsList == NULL && m_pXmlNodeSerialize->m_pAttributesList == NULL)
			m_pXmlNodeSerialize->RemoveFromParent();
		}
	StackPopNode();
	}

void
CXmlExchanger::XmlExchangeBinary(PSZAC pszuTagName, INOUT_ZZR BYTE prgbDataBinary[], int cbDataBinary)
	{
	Assert(pszuTagName != NULL);
	Assert(pszuTagName[0] != '\0');
	Assert(prgbDataBinary != NULL);
	Assert(cbDataBinary > 0);
	if (m_fSerializing)
		{
		PSZU pszBase85 = (PSZU)m_accumulatorText.PvAllocateData(Base85_CbEncodeAlloc(cbDataBinary));
		Base85_CchEncodeToText(OUT pszBase85, IN prgbDataBinary, cbDataBinary);
		(void)_PAllocateAttribute(INOUT m_pXmlNodeSerialize, pszuTagName, pszBase85);
		}
	else
		{
		(void)Base85_FDecodeToBinary_ZZR_ML(IN m_pXmlNodeSerialize->PszuFindElementOrAttributeValue_NZ(pszuTagName), OUT_ZZR prgbDataBinary, cbDataBinary);
		/*
		CXmlNode * pNodeBinary = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagName);
		if (pNodeBinary == NULL)
			return;
		PSZUC pszBinary = pNodeBinary->m_pszuTagValue;
		Assert(pszBinary != NULL);
		if (pszBinary != NULL && pszBinary[0] != '\0')
			{
			if (Base85_FCanDecodeToBinary(pszBinary, cbDataBinary))
				Base85_CbDecodeToBinary(IN pszBinary, OUT_F_INV prgbDataBinary);
			else
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The Base85 string '$s' cannot be decoded into a buffer of $i bytes\n", pszBinary, cbDataBinary);
			}
		*/
		}
	} // XmlExchangeBinary()

void
CXmlExchanger::XmlExchangeBin(PSZAC pszuTagName, INOUT CBin * pbinValue)
	{
	Assert(pszuTagName != NULL);
	Assert(pszuTagName[0] != '\0');
	Assert(pbinValue != NULL);
	Assert(m_pXmlNodeSerialize != NULL);
	if (m_fSerializing)
		{
		// Serialize only non-empty binary
		int cbDataBinary = pbinValue->CbGetData();
		if (cbDataBinary > 0)
			{
			XmlExchangeBinary(pszuTagName, IN pbinValue->PbGetData(), cbDataBinary);	// Reuse the same method as the encoding fixed-length binary data
			/*
			CStr strBase85;	// Content of the binary is encoded in Base85, which uses less storage than Base64 and does not require any XML/HTML encoding.
			strBase85.BinAppendStringBase85FromBinaryData(IN pbinValue);
			strBase85.BinAppendNullTerminator();	// Make sure we are inserting the null-terminator so the method PszuAllocateCopyStrU() get the exact length of the CStr object
			AssertValidStr(strBase85);
			(void)_PAllocateAttribute(INOUT m_pXmlNodeSerialize, pszuTagName, m_accumulatorText.PszuAllocateCopyStrU(strBase85));	// Serialize the bin as an attribute to save on storage
			*/
			}
		}
	else
		{
		pbinValue->Empty();
		if (m_uVersionFileFormat > d_uVersionFileFormat0)
			(void)pbinValue->BinAppendBinaryDataFromBase85Szv_ML(m_pXmlNodeSerialize->PszuFindElementOrAttributeValue(pszuTagName));
		else
			(void)pbinValue->BinAppendBinaryDataFromBase64Szv(m_pXmlNodeSerialize->PszuFindElementOrAttributeValue(pszuTagName));	// Old file format was using Base64
		}
	}

//	Store a string encoded in base85.
//	The motivation for this storing an XML string within an XML document.  The encoding in Base85 offers three benefits:
//	1. There is no need to unserialize and serialize the XML string each time the XML is loaded, thus reducing CPU and memory requirements.
//	2. No need to write extra code to handle an XML tree structure.  Manipulating a string is much simpler than manipulating an XML tree structure.
//	3. Reduce disk storage.  Typically storing an XML within an XML result in a much larger storage requirement (since the entities <>&'" are encoded as &lt; &gt; &amp; &apos; &quot;),
//	   the disk storags may be 200% to 300% than the Base85 encoding which is always 25% larger.
void
CXmlExchanger::XmlExchangeStrBase85(PSZAC pszuTagName, INOUT CStr * pstrValue)
	{
	Assert(m_pXmlNodeSerialize != NULL);
	if (m_fSerializing)
		{
		// Serialize only non-empty strings
		int cchString = pstrValue->CchGetLength();
		if (cchString > 0)
			XmlExchangeBinary(pszuTagName, IN pstrValue->PbGetData(), cchString);	// Reuse the same method as the encoding fixed-length binary data
		}
	else
		{
		pstrValue->InitFromTextEncodedInBase85(IN m_pXmlNodeSerialize->PszuFindElementOrAttributeValue(pszuTagName));
		}

	}

void
CXmlExchanger::XmlExchangeStr(PSZAC pszuTagName, INOUT CStr * pstrValue)
	{
	Assert(pszuTagName != NULL);
	Assert(pszuTagName[0] != '\0');
	Assert(pstrValue != NULL);
	Assert(m_pXmlNodeSerialize != NULL);
	if (m_fSerializing)
		{
		// Serialize only non-empty strings
		AssertValidStr(*pstrValue);
		PSZUC pszuValue = pstrValue->PszuGetDataNZ();	// Typecast
		if (pszuValue != NULL && pszuValue[0] != '\0')
			(void)_PAllocateAttribute(INOUT m_pXmlNodeSerialize, pszuTagName, pszuValue);	// Serialize the string as an attribute to save on storage

		}
	else
		{
		*pstrValue = m_pXmlNodeSerialize->PszuFindElementOrAttributeValue(pszuTagName);
		AssertValidStr(*pstrValue);
		}
	} // XmlExchangeStr()

void
CXmlExchanger::XmlExchangeStrConditional(PSZAC pszuTagName, INOUT CStr * pstrValue, BOOL fuConditionToSerialize)
	{
	if (m_fSerializing && !fuConditionToSerialize)
		return;
	XmlExchangeStr(pszuTagName, pstrValue);
	}

//	Serialize a number to XML.
//	If the number is zero, do not serialize.
//	If the XML attribute or element is missing, do nothing.
void
CXmlExchanger::XmlExchangeInt(PSZAC pszuTagName, INOUT int * pnValue)
	{
	Assert(pszuTagName != NULL);
	Assert(pszuTagName[0] != '\0');
	Assert(pnValue != NULL);
	Assert(m_pXmlNodeSerialize != NULL);
	if (m_fSerializing)
		{
		AllocateAttributeValueInteger(pszuTagName, *pnValue);
		/*
		const int nValue = *pnValue;
		if (nValue != 0)
			{
			CHU szuString[16];
			IntegerToString(OUT szuString, nValue);
			(void)_PAllocateAttribute(INOUT m_pNodeSerialize, pszuTagName, IN m_accumulatorSerialize.PszuAllocateStringU(IN szuString));	// Serialize the number as an attribute to save on storage
			}
		*/
		}
	else
		{
		CXmlNode * pNode = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagName);
		if (pNode != NULL)
			{
			PSZUC pszuTagValue = pNode->m_pszuTagValue;
			Assert(pszuTagValue != NULL);
			if (pszuTagValue != NULL && pszuTagValue[0] != '\0')
				{
				SStringToNumber stn;
				stn.uFlags = STN_mskfAllowHexPrefix;
				stn.pszuSrc = pszuTagValue;
				if (FStringToNumber(INOUT &stn))
					{
					*pnValue = stn.u.nData;
					return;
					}
				// An error occurred while parsing the integer
				}
			}
		*pnValue = 0;
		} // if...else
	} // XmlExchangeInt()

void
CXmlExchanger::XmlExchangeUInt(PSZAC pszuTagName, INOUT_F_UNCH_S UINT * puValue)
	{
	XmlExchangeInt(pszuTagName, INOUT_F_UNCH_S (int *)puValue);
	}
void
CXmlExchanger::XmlExchangeUIntHex(PSZAC pszuTagName, INOUT_F_UNCH_S UINT * puValueHex)
	{
	Assert(pszuTagName != NULL);
	Assert(pszuTagName[0] != '\0');
	Assert(puValueHex != NULL);
	Assert(m_pXmlNodeSerialize != NULL);
	if (m_fSerializing)
		{
		if (*puValueHex != 0)
			{
			CHU szuString[16];
			IntegerToString(OUT szuString, *puValueHex, ITS_mskfHexadecimal);
			AllocateAttributeValueCopyPsz(pszuTagName, IN szuString);
			}
		}
	else
		{
		CXmlNode * pNode = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagName);
		if (pNode != NULL)
			*puValueHex	= UStringToNumberHexadecimal_ZZR_ML(pNode->m_pszuTagValue);
		}
	}

void
CXmlExchanger::XmlExchangeUIntHexFlagsMasked(PSZAC pszuTagNameFlags, INOUT UINT * puFlags, UINT uFlagsSerializeMask)
	{
	Assert(uFlagsSerializeMask != 0 && "NOOP");
	UINT uFlagsOld = *puFlags;
	UINT uValueSerialize = (uFlagsOld & uFlagsSerializeMask);
	XmlExchangeUIntHex(pszuTagNameFlags, INOUT_F_UNCH_S &uValueSerialize);
	Assert((uValueSerialize & ~uFlagsSerializeMask) == 0 && "Those unserializable flags should not be present");
	*puFlags = uValueSerialize | (uFlagsOld & ~uFlagsSerializeMask);
	}

void
CXmlExchanger::XmlExchangeUShort(PSZAC pszuTagName, INOUT_F_UNCH_S quint16 * pusValue)
	{
	int nValue = *pusValue;
	XmlExchangeInt(pszuTagName, INOUT_F_UNCH_S &nValue);
	*pusValue = nValue;
	}

void
CXmlExchanger::XmlExchangeGuid(PSZAC pszuTagNameGuid, INOUT_F_UNCH_S GUID * pGuid)
	{
	XmlExchangeBinary(pszuTagNameGuid, INOUT_F_UNCH_S (BYTE *)pGuid, sizeof(*pGuid));
	/*
	Assert(pGuid != NULL);
	if (m_fSerializing)
		{
		AllocateAttributeValueGuid(pszuTagNameGuid, pGuid);
		}
	else
		{
		CXmlNode * pNodeGuid = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagNameGuid);
		if (pNodeGuid == NULL)
			return;
		PSZAC pszGuid = (PSZAC)pNodeGuid->m_pszuTagValue;
		Assert(pszGuid != NULL);
		if (pszGuid != NULL && pszGuid[0] != '\0')
			{
			if (!Guid_FInitFromStringHex(OUT pGuid, pszGuid))
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Invalid GUID $s\n", pszGuid);
				}
			}
		}
	*/
	} // XmlExchangeGuid()

void
CXmlExchanger::XmlExchangeSha1(PSZAC pszuTagNameSha1, INOUT_F_UNCH_S SHashSha1 * pHashSha1)
	{
	XmlExchangeBinary(pszuTagNameSha1, INOUT_F_UNCH_S (BYTE *)pHashSha1, sizeof(*pHashSha1));
	}


void
CXmlExchanger::XmlExchangeTimestamp(PSZAC pszuTagNameTimestamp, INOUT_F_UNCH_S TIMESTAMP * ptsValue)
	{
	if (m_fSerializing)
		{
		if (*ptsValue != 0)
			{
			CHU szTimestamp[16];
			Timestamp_CchToString(*ptsValue, OUT szTimestamp);
			AllocateAttributeValueCopyPsz(pszuTagNameTimestamp, IN szTimestamp);
			}
		}
	else
		{
		CXmlNode * pNodeTimestamp = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagNameTimestamp);
		if (pNodeTimestamp == NULL)
			return;
		*ptsValue = Timestamp_FromString_ML(pNodeTimestamp->m_pszuTagValue);
		}
	}

void
CXmlExchanger::XmlExchangeDateTime(PSZAC pszuTagNameDateTime, INOUT_F_UNCH_S QDateTime * pDateTime)
	{
	if (m_fSerializing)
		{
		AllocateAttributeValueStringQ(pszuTagNameDateTime, pDateTime->toString(Qt::ISODate));
		}
	else
		{
		CXmlNode * pNodeDateTime = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagNameDateTime);
		if (pNodeDateTime == NULL)
			return;
		*pDateTime = QDateTime::fromString((PSZAC)pNodeDateTime->m_pszuTagValue, Qt::ISODate);
		if (!pDateTime->isValid())
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Invalid QDateTime $s\n", pNodeDateTime->m_pszuTagValue);
		}
	}

void
CXmlExchanger::XmlExchangeObjects(CHS chTagNameObjects, INOUT_F_UNCH_S CArrayPtrTreeItems * parraypaObjects, PFn_PaAllocateXmlObject pfnPaAllocatorObject, PVOID pvContextAllocate)
	{
	PSZAC pszuTagNameObjects = (PSZAC)m_accumulatorText.PszuAllocateStringFromSingleCharacter(chTagNameObjects);
	PSZAC pszuTagNameObject = (PSZAC)m_accumulatorText.PszuAllocateStringFromSingleCharacter(chTagNameObjects + 32);	// Use a lowercase version for each object
	XmlExchangeObjects(pszuTagNameObjects, pszuTagNameObject, parraypaObjects, pfnPaAllocatorObject, pvContextAllocate);
	}

//	Serialize Tree Item objects.
//	Originally this method was serializing CArrayPtrXmlSerializable, however since some objects may be undeletable (and every serializable object inherits from ITreeItem),
//	the array parraypaObjects is of type CArrayPtrTreeItems.
//
//	INTERFACE NOTES
//	The pointers pszuTagNameObjects and pszuTagNameObject are assumed to be static.  If they are dynamic, it is the responsibility of the caller to allocate them.
void
CXmlExchanger::XmlExchangeObjects(PSZAC pszuTagNameObjects, PSZAC pszuTagNameObject, INOUT_F_UNCH_S CArrayPtrTreeItems * parraypaObjects, PFn_PaAllocateXmlObject pfnPaAllocatorObject, PVOID pvContextAllocate)
	{
	Assert(pszuTagNameObjects != NULL);
	Assert(pszuTagNameObjects[0] != '\0');
	Assert(parraypaObjects != NULL);
	Assert(m_pXmlNodeSerialize != NULL);

	if (m_fSerializing)
		{
		ITreeItem ** ppObject;
		ITreeItem ** ppObjectStop;
		ppObject = parraypaObjects->PrgpGetTreeItemsStop(OUT &ppObjectStop);
		if (ppObject == ppObjectStop)
			return;	// The array is empty, therefore there is nothing to serialize
		CXmlNode * pXmlNodeObjects = _PAllocateElement(m_pXmlNodeSerialize, pszuTagNameObjects);	// Allocate the element for the objects
		CXmlNode ** ppXmlNodeObjectStack = PpStackPushNodes(pXmlNodeObjects);
		while (TRUE)
			{
			// An object is always serialized as an element
			ITreeItem * pObject = *ppObject++;
			Assert(PGetRuntimeInterfaceOf_ITreeItem(pObject) == pObject);
			if (!pObject->TreeItemFlags_FuIsDeleted())
				{
				CXmlNode * pXmlNodeObject = _PAllocateElement(pXmlNodeObjects, pszuTagNameObject);
				*ppXmlNodeObjectStack = pXmlNodeObject;
				pObject->XmlExchange(INOUT this);
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "The Tree Item 0x$p '$S' is NOT serialized because it is considered deleted\n", pObject, &pObject->m_strNameDisplayTyped);
				}
			if (ppObject == ppObjectStop)
				break;
			} // while
		}
	else
		{
		Assert(pfnPaAllocatorObject != NULL);
		CXmlNode * pXmlNodeObjects = m_pXmlNodeSerialize->PFindElement(pszuTagNameObjects);
		if (pXmlNodeObjects == NULL)
			return;
		CXmlNode ** ppXmlNodeObjectStack = PpStackPushNodes(pXmlNodeObjects);
		CXmlNode * pXmlNodeObject = pXmlNodeObjects->m_pElementsList;
		while (pXmlNodeObject != NULL)
			{
			*ppXmlNodeObjectStack = pXmlNodeObject;
			IXmlExchange * paObject = pfnPaAllocatorObject(pvContextAllocate);
			Report(paObject != NULL);
			parraypaObjects->Add(PA_CHILD paObject);
			paObject->XmlExchange(INOUT this);
			pXmlNodeObject = pXmlNodeObject->m_pNextSibling;
			} // while
		} // if...else
	StackPopNodes();
	} // XmlExchangeObjects()

//	The method XmlExchangeObjects2() is very similar to XmlExchangeObjects(), however pfnPaAllocatorObject2 takes two (2) parameters:
//		1. The first parameter is identical to XmlExchangeObjects()
//		2. The second parameter of pfnPaAllocatorObject2 is a pointer to the XML element to unserialize.  This allows the allocator pfnPaAllocatorObject2 to fetch additional information necessary to create (allocate) the object.
void
CXmlExchanger::XmlExchangeObjects2(CHS chTagNameObjects, INOUT_F_UNCH_S CArrayPtrXmlSerializable * parraypaObjects, PFn_PaAllocateXmlObject2_YZ pfnPaAllocatorObject2_YZ, PVOID pvContextAllocate)
	{
	Assert(chTagNameObjects >= 'A' && chTagNameObjects <= 'Z');
	Assert(parraypaObjects != NULL);
	Assert(m_pXmlNodeSerialize != NULL);
	if (m_fSerializing)
		{
		IXmlExchange ** ppObject;
		IXmlExchange ** ppObjectStop;
		ppObject = parraypaObjects->PrgpGetObjectsStop(OUT &ppObjectStop);
		if (ppObject == ppObjectStop)
			return;	// The array is empty, therefore there is nothing to serialize
		PSZU pszTagNameObject = m_accumulatorText.PszuAllocateStringFromSingleCharacter(chTagNameObjects + 32);	// Use a lowercase version for each object
		CXmlNode * pXmlNodeObjects = _PAllocateElement(m_pXmlNodeSerialize, (PSZA)m_accumulatorText.PszuAllocateStringFromSingleCharacter(chTagNameObjects));	// Allocate the element for the objects
		CXmlNode ** ppXmlNodeObjectStack = PpStackPushNodes(pXmlNodeObjects);
		while (TRUE)
			{
			// An object is always serialized as an element
			CXmlNode * pXMlNodeObject = _PAllocateElement(pXmlNodeObjects, (PSZA)pszTagNameObject);
			*ppXmlNodeObjectStack = pXMlNodeObject;
			IXmlExchange * pObject = *ppObject++;
			pObject->XmlExchange(INOUT this);
			if (ppObject == ppObjectStop)
				break;
			} // while
		}
	else
		{
		CXmlNode * pXmlNodeObjects = m_pXmlNodeSerialize->PFindElement(chTagNameObjects);
		if (pXmlNodeObjects == NULL)
			return;
		CXmlNode ** ppXmlNodeObjectStack = PpStackPushNodes(pXmlNodeObjects);
		CXmlNode * pXmlNodeObject = pXmlNodeObjects->m_pElementsList;
		while (pXmlNodeObject != NULL)
			{
			*ppXmlNodeObjectStack = pXmlNodeObject;
			IXmlExchange * paObject = pfnPaAllocatorObject2_YZ(pvContextAllocate, pXmlNodeObject);
			if (paObject != NULL)
				{
				parraypaObjects->Add(PA_CHILD paObject);
				paObject->XmlExchange(INOUT this);
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to allocate object from the data of the following element:\n^N", pXmlNodeObject);
				}
			pXmlNodeObject = pXmlNodeObject->m_pNextSibling;
			} // while
		} // if...else
	StackPopNodes();
	}

void
CXmlExchanger::XmlExchangePointer(CHS chTagNamePointer, INOUT_F_UNCH_S ITreeItem ** ppObject, const CArrayPtrTreeItems * parraypObjectsLookup)
	{
	Assert(ppObject != NULL);
	if (m_fSerializing)
		{
		ITreeItem * pObject = *ppObject;
		if (pObject != NULL)
			{
			Assert(PGetRuntimeInterfaceOf_ITreeItem(pObject) == pObject);
			Assert(!pObject->TreeItemFlags_FuIsDeleted() && "Attempting to serialize a pointer to a deleted object is a bug!");
			AllocateAttributeValueInteger((PSZA)m_accumulatorText.PszuAllocateStringFromSingleCharacter(chTagNamePointer), pObject->UGetObjectId());
			}
		}
	else
		{
		Assert(parraypObjectsLookup != NULL);
		CXmlNode * pXmlNodePointer = m_pXmlNodeSerialize->PFindAttribute(chTagNamePointer);
		if (pXmlNodePointer == NULL)
			return;
		PSZUC pszuTagValue = pXmlNodePointer->m_pszuTagValue;
		Assert(pszuTagValue != NULL);
		if (pszuTagValue != NULL && pszuTagValue[0] != '\0')
			{
			SStringToNumber stn;
			stn.uFlags = STN_mskfAllowHexPrefix;
			stn.pszuSrc = pszuTagValue;
			if (FStringToNumber(INOUT &stn))
				{
				if (stn.u.nData > 0)
					{
					*ppObject = (ITreeItem *)parraypObjectsLookup->PFindObjectById(stn.u.nData);
					return;
					}
				}
			// An error occurred while parsing the integer
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "XmlExchangePointer('$b') - Invalid data '$s'\n", chTagNamePointer, pszuTagValue);
			}
		} // if...else
	} // XmlExchangePointer()

void
CXmlExchanger::XmlExchangePointers(PSZAC pszuTagNamePointers, INOUT_F_UNCH_S CArrayPtrTreeItems * parraypObjects, const CArrayPtrXmlSerializableObjects & arraypObjectsLookup)
	{
	Assert(pszuTagNamePointers != NULL);
	Assert(parraypObjects != NULL);
	Assert(&arraypObjectsLookup != NULL);
	Assert(parraypObjects != &arraypObjectsLookup);
	if (m_fSerializing)
		{
		// Serialize the pointers
		ITreeItem ** ppObject;
		ITreeItem ** ppObjectStop;
		ppObject = parraypObjects->PrgpGetTreeItemsStop(OUT &ppObjectStop);
		if (ppObject == ppObjectStop)
			return;	// The array is empty, therefore there is nothing to serialize
		g_strScratchBufferStatusBar.Empty();	// For performance, reuse the global variable as a CBin
		while (TRUE)
			{
			ITreeItem * pObject = *ppObject++;
			if (pObject != NULL)
				{
				Assert(PGetRuntimeInterfaceOf_ITreeItem(pObject) == pObject);
				Assert(!pObject->TreeItemFlags_FuIsDeleted() && "Attempting to serialize a pointer to a deleted object is a bug!");
				Assert(pObject->UGetObjectId() > 0);
				g_strScratchBufferStatusBar.BinAppendTextInteger(pObject->UGetObjectId());
				}
			if (ppObject == ppObjectStop)
				break;
			g_strScratchBufferStatusBar.BinAppendByte(' ');	// Put a space between the pointers
			}
		AllocateAttributeValueCBinString(pszuTagNamePointers, g_strScratchBufferStatusBar);
		}
	else
		{
		CXmlNode * pXmlNode = m_pXmlNodeSerialize->PFindElementOrAttribute(pszuTagNamePointers);
		if (pXmlNode == NULL)
			return;
		SStringToNumber stn;
		stn.uFlags = STN_mskzSkipLeadingSpaces | STN_mskzSkipTailingSpaces | STN_mskfAllowRandomTail;
		stn.pszuSrc = pXmlNode->m_pszuTagValue;
		Assert(stn.pszuSrc != NULL);
		if (stn.pszuSrc != NULL && stn.pszuSrc[0] != '\0')
			{
			while (TRUE)
				{
				if (FStringToNumber(INOUT &stn))
					{
					if (stn.u.nData > 0)
						{
						IXmlExchangeObjectID * pObject = arraypObjectsLookup.PFindObjectById(stn.u.nData);
						if (pObject != NULL)
							{
							Assert(pObject->PGetRuntimeInterface(RTI(IXmlExchangeObjectID), NULL) == pObject);
							parraypObjects->Add(pObject);
							stn.pszuSrc = stn.pchStop;
							Assert(stn.pszuSrc != NULL);
							if (stn.pszuSrc[0] != '\0')
								continue;	// Keep going until there are no more IDs
							return;	// We reached the end of the string, therefore we are done
							}
						}
					} // if
				// An error occurred while parsing the integer
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The object identifier '$s' is not valid\n", stn.pszuSrc);
				break;	// We got an error, therefore stop
				} // while
			} // if
		} // if...else
	} // XmlExchangePointers()

void
CXmlExchanger::XmlExchangeWriteAttribute(PSZAC pszNameAttribute, PSZUC pszAttributeValue)
	{
	if (m_fSerializing)
		AllocateAttributeValueCopyPsz(pszNameAttribute, pszAttributeValue);
	}

void
CXmlExchanger::XmlExchangeWriteAttributeUSZU(PSZAC pszNameAttribute, USZU uszuAttributeValue)
	{
	XmlExchangeWriteAttribute(pszNameAttribute, PszFromUSZU(uszuAttributeValue));
	}

void
CXmlExchanger::XmlExchangeWriteAttributeRtiSz(PSZAC pszNameAttribute, RTI_ENUM rtiAttributeValue)
	{
	XmlExchangeWriteAttribute(pszNameAttribute, PszFromUSZU(rtiAttributeValue));
	}

//	This method does NOT make a copy of pszValue
void
CXmlExchanger::AllocateAttributePsz(PSZAC pszaAttributeName, PSZUC pszValue)
	{
	(void)_PAllocateAttribute(INOUT m_pXmlNodeSerialize, IN pszaAttributeName, IN pszValue);
	}

void
CXmlExchanger::AllocateAttributeValueCopyPsz(PSZAC pszaAttributeName, PSZUC pszValue)
	{
	AllocateAttributePsz(pszaAttributeName, IN m_accumulatorText.PszuAllocateCopyStringU(IN pszValue));
	}

//	Allocate the attribute only if the integer is non-zero
void
CXmlExchanger::AllocateAttributeValueInteger(PSZAC pszaAttributeName, int nValue)
	{
	if (nValue != 0)
		{
		CHU szuString[16];
		IntegerToString(OUT szuString, nValue);
		AllocateAttributeValueCopyPsz(pszaAttributeName, szuString);
		}
	}

//	Allocate the attribute only if the GUID is not empty
void
CXmlExchanger::AllocateAttributeValueGuid(PSZAC pszaAttributeName, const GUID * pGuid)
	{
	Assert(pGuid != NULL);
	if (!Guid_FIsEmpty(pGuid))
		{
		PSZU pszGuid = (PSZU)m_accumulatorText.PvAllocateData(32 + 1);
		Guid_ToStringHex(OUT pszGuid,  IN pGuid);
		AllocateAttributePsz(pszaAttributeName, IN pszGuid);
		}
	}

void
CXmlExchanger::AllocateAttributeValueStringQ(PSZAC pszaAttributeName, const QString & sString)
	{
	if (!sString.isEmpty())
		AllocateAttributePsz(pszaAttributeName, IN m_accumulatorText.PszuAllocateEncodeStringQ(sString));
	}

//	Allocate the content of binary data (CBin) with the assumption it contains a string.
void
CXmlExchanger::AllocateAttributeValueCBinString(PSZAC pszaAttributeName, IN_MOD_TMP CBin & binString)
	{
	if (!binString.FIsEmptyBinary())
		AllocateAttributePsz(pszaAttributeName, IN m_accumulatorText.PszuAllocateCopyCBinString(IN_MOD_TMP binString));
	}
