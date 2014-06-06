///////////////////////////////////////////////////////////////////////////////////////////////////
//	CMemoryAccumulator.cpp
//
//	Class specialized for allocating small blocks of memory and accumulating them.
//	The benefits of this class is an incredibly small overhead since there is
//	no housekeeping of allocated blocks.  Instead the allocated blocks are appended
//	in the allocator.
//	Another benefit is there is no need to free a memory block allocated by the allocator,
//	since the destructor of CMemoryAccumulator deletes all memory blocks at once.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_cbAllocMin		4//000	// Minimum block size to allocate
#define d_cbAllocExtraMax	100000	// Don't pre-allocate more than 100 KB of extra memory

CMemoryAccumulator::~CMemoryAccumulator()
	{
	while (m_paData != NULL)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		SHeaderWithData * paDataNext = m_paData->pNext;
		delete m_paData;
		m_paData = paDataNext;
		}
	}

//	Initialize the "ideal" size to allocate a buffer, otherwise the allocator will allocate block of 2 KB.
//
//	If there is already allocated buffers, this method does nothing
VOID
CMemoryAccumulator::SetAllocSize(int cbAlloc)
	{
	if (m_paData == NULL)
		{
		if (cbAlloc < d_cbAllocMin)
			cbAlloc = d_cbAllocMin;
		_AllocateMemoryBlock(cbAlloc);	// Pre-allocate a buffer to hold the allocated blocks of memory
		}
	}

BOOL
CMemoryAccumulator::FIsEmpty()
	{
	if (m_paData != NULL)
		{
		if (m_paData->cbData > 0)
			return FALSE;
		SHeaderWithData * pDataNext = m_paData->pNext;
		return (pDataNext == NULL || pDataNext->cbData == 0);
		}
	return TRUE;
	}

//	Flush all allocated buffer, except one buffer.
//	This method avoids the allocation/deletion of blocks of memory
//
//	The implementation of this method keeps the largest allocated buffer, which will reduce future memory allocations.
void
CMemoryAccumulator::RecycleBuffer()
	{
	if (m_paData == NULL)
		return;
	while (TRUE)
		{
		SHeaderWithData * paDataNext = m_paData->pNext;
		if (paDataNext == NULL)
			break;
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		Assert(paDataNext->cbData <= paDataNext->cbAlloc);
		// Keep the largest buffer, by deleting the smallest allocated buffer
		if (m_paData->cbAlloc <= paDataNext->cbAlloc)
			{
			delete m_paData;
			m_paData = paDataNext;
			}
		else
			{
			// Skip the node
			m_paData->pNext = paDataNext->pNext;
			Endorse(m_paData->pNext == NULL);
			delete paDataNext;
			}
		} // while
	Assert(m_paData != NULL);
	m_paData->cbData = 0;	// No data in buffer
	Assert(m_paData->cbAlloc > 0);
	InitToGarbage(OUT m_paData->rgbData, m_paData->cbAlloc);
	Assert(FIsEmpty());
	} // RecycleBuffer()


void
CMemoryAccumulator::AppendData(const void * pvData, int cbData)
	{
	Assert(pvData != NULL);
	Assert(cbData >= 0);
	if (m_paData != NULL)
		{
		// We have an existing block
		const int cbAllocated = m_paData->cbAlloc;
		const int cbDataUsed = m_paData->cbData;
		BYTE * pbDst = m_paData->rgbData + cbDataUsed;
		const int cbAvailable = cbAllocated - cbDataUsed;
		Assert(cbAvailable >= 0);
		if (cbData <= cbAvailable)
			{
			// Copy the entire buffer to append
			memcpy(OUT pbDst, IN pvData, cbData);
			m_paData->cbData = cbDataUsed + cbData;
			return;
			}
		// Copy as much as we can into the existing buffer
		memcpy(OUT pbDst, IN pvData, cbAvailable);
		pvData = (BYTE *)pvData + cbAvailable;
		cbData -= cbAvailable;
		Assert(cbData > 0);
		m_paData->cbData = cbAllocated;
		goto CopyRemaining;
		}
	if (cbData > 0)
		{
		CopyRemaining:
		(void)PvAllocateDataCopy(cbData, pvData);
		}
	} // AppendData()

void
CMemoryAccumulator::AppendTextU(PSZUC pszString)
	{
	if (pszString != NULL)
		AppendData(pszString, strlenU(pszString));
	}

void
CMemoryAccumulator::AppendStringFormat_Gsb(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	int cbData;
	PSZUC pszData = g_strScratchBufferStatusBar.BinAppendTextVirtualSzv_VL(OUT &cbData, IN pszFmtTemplate, vlArgs);
	//MessageLog_AppendTextFormatCo(d_coAqua, "AppendStringFormat_Gsb('{pn}')\n", pszData, cbData);
	AppendData(pszData, cbData);
	}

//	Append the content of the allocator to a file and empty the content of the allocator.
EError
CMemoryAccumulator::EAppendToFileAndEmpty(const QString & sFileName)
	{
	SHeaderWithData * pData = m_paData;
	if (pData == NULL)
		return errSuccess;	// Nothing to do

	EError err = errFileWriteError;
	QFile oFile(sFileName);
	if (oFile.open(QIODevice::WriteOnly | QIODevice::Append))
		{
		err = errSuccess;
		// Since new memory blocks are appended at the beginning of the linked list, we need to reverse the order
		m_paData = NULL;
		while (TRUE)
			{
			SHeaderWithData * pNext = pData->pNext;
			pData->pNext = m_paData;
			m_paData = pData;	// Always re-insert at the beginning
			if (pNext == NULL)
				break;
			pData = pNext;
			} // while
		Assert(pData == m_paData);
		while (TRUE)
			{
			if (oFile.write((const char *)pData->rgbData, pData->cbData) != pData->cbData)
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to read write the data to file $Q.\n", &sFileName);
				err = errFileWriteError;
				}
			pData = pData->pNext;
			if (pData == NULL)
				break;
			} // while
		RecycleBuffer();
		if (!oFile.flush())
			err = errFileWriteError;
		oFile.close();
		}
	return err;
	} // EAppendToFileAndEmpty()


//	Align the buffer on a 32-bit boundary to enhance performance when accessing data structures.
//	This method is used before calling PvGetBuffer() to make sure the returned buffer is 32-bit aligned.
void
CMemoryAccumulator::AlignToUInt32()
	{
	if (m_paData != NULL)
		{
		Assert(IS_ALIGNED_32(m_paData));			// Each allocation pointer is 32-bit aligned
		Assert(IS_ALIGNED_32(m_paData->cbAlloc));	// The size of each allocation is always 32-bit aligned
		m_paData->cbData = NEXT_UINT32(m_paData->cbData);
		}
	}

//	Return a pointer to a buffer of a minimum cbBuffer.
//	This routine may pre-allocate memory, but does not commit.
//	In other words, calling this method multiple times does nothing
//
//	The caller is responsible of calling the method CommitBuffer() to bind the data to the buffer.
//	An improvement would be to try to allocate to the next buffer (in case there is some memory left)
void *
CMemoryAccumulator::PvGetBuffer(int cbBufferMin)
	{
	Assert(cbBufferMin > 0);
	Endorse(!IS_ALIGNED_32(cbBufferMin));
	if (m_paData == NULL)
		_AllocateMemoryBlock(cbBufferMin + d_cbAllocMin);	// Pre-allocate a buffer of about 2 KB
	Assert(m_paData != NULL);
	BYTE * pbData;
	int cbAllocated = m_paData->cbAlloc;
	int cbDataAllocated = m_paData->cbData;
	int cbDataAvailable = cbAllocated - cbDataAllocated;
	if (cbBufferMin <= cbDataAvailable)
		{
		// The requested buffer fits within the pre-allocated block
		pbData = m_paData->rgbData + cbDataAllocated;
		}
	else
		{
		// We need to allocate a new block to hold the requested buffer
		if (cbAllocated > d_cbAllocExtraMax)
			cbAllocated = d_cbAllocExtraMax;	// Make sure we don't allocate blocks over 100 KB.  This is a slight protection to prevent a previous large block to force any subsequent blocks to become larger and larger.
		_AllocateMemoryBlock(cbBufferMin + cbAllocated);	// Allocate a memory block for the requsted buffer plus extra bytes about the same size as the previous block
		pbData = m_paData->rgbData;
		}
	InitToGarbage(OUT pbData, cbBufferMin);
	return pbData;
	} // PvGetBuffer()

//	Complement to PvGetBuffer()
void
CMemoryAccumulator::CommitBuffer(int cbBufferUsed)
	{
	Assert(cbBufferUsed >= 0);
	Endorse(!IS_ALIGNED_32(cbBufferUsed));
	Assert(m_paData != NULL);
	m_paData->cbData += cbBufferUsed;
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	}


//	Core routine to allocate small block memory from the pre-allocated larger buffer.
//	This data does not need to be deleted because the allocator holds the allocation to the memory.
//
//	The returned memory block is not initialized and may not be 32-bit aligned.
void *
CMemoryAccumulator::PvAllocateData(int cbAlloc)
	{
	Assert(cbAlloc > 0);
	Endorse(!IS_ALIGNED_32(cbAlloc));
	void * pvBuffer = PvGetBuffer(cbAlloc);
	Endorse(!IS_ALIGNED_32(pvBuffer));
	InitToGarbage(OUT pvBuffer, cbAlloc);
	Assert(m_paData != NULL);
	m_paData->cbData += cbAlloc;
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	return pvBuffer;
	} // PvAllocateData()


//	Small allocator which copies a buffer
void *
CMemoryAccumulator::PvAllocateDataCopy(int cbAlloc, PCVOID pvDataSrcCopy)
	{
	Assert(cbAlloc > 0);
	Assert(pvDataSrcCopy != NULL);
	void * pvData = PvAllocateData(cbAlloc);
	memcpy(OUT pvData, IN pvDataSrcCopy, cbAlloc);
	return pvData;
	}

VOID *
CMemoryAccumulator::PvAllocateDataCopyAndAlignToDWORD(int cbAlloc, PCVOID pvDataSrcCopy)
	{
	Assert(cbAlloc > 0);
	Assert(pvDataSrcCopy != NULL);
	void * pvData = PvAllocateData(cbAlloc);
	memcpy(OUT pvData, IN pvDataSrcCopy, cbAlloc);
	m_paData->cbData = NEXT_UINT32(m_paData->cbData);	// Align to the next dword
	return pvData;
	}

PSZU
CMemoryAccumulator::PszuAllocateEncodeStringW(PSZWC pszwString)
	{
	if (pszwString != NULL)
		{
		PSZU pszuString = (PSZU)PvGetBuffer(CbAllocWtoU(pszwString));
		m_paData->cbData += 1 + EncodeUnicodeToUtf8(OUT pszuString, IN pszwString);
		return pszuString;
		}
	return (PSZU)c_szuEmpty;
	}

PSZU
CMemoryAccumulator::PszuAllocateEncodeStringQ(const QString & sString)
	{
	return PszuAllocateEncodeStringW(sString.utf16());
	}

PSZU
CMemoryAccumulator::PszuAllocateCopyCBinString(IN_MOD_TMP CBin & binString)
	{
	PSZUC pszuString = binString.BinAppendNullTerminatorVirtualSzv() CONST_TEMPORARY_MODIFIED;	// Make sure the binary data has a null-terminator
	Assert((int)strlenU(pszuString) == binString.CbGetData());
	return (PSZU)PvAllocateDataCopy(binString.CbGetData() + 1, IN pszuString);				// Allocate a string for the binary data
	}

PSZU
CMemoryAccumulator::PszuAllocateCopyStrU(const CStr & str)
	{
	return (PSZU)PvAllocateDataCopy(str.CbGetData(), IN str.PszuGetDataNZ());
	}

PSZU
CMemoryAccumulator::PszuAllocateCopyStringU(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	return (PSZU)PvAllocateDataCopy(CbAllocUtoU(pszuString), IN pszuString);
	}

PSZU
CMemoryAccumulator::PszuAllocateCopyStringNullU(PSZUC pszuString)
	{
	if (pszuString == NULL || *pszuString == '\0')
		return (PSZU)c_szuEmpty;
	return (PSZU)PvAllocateDataCopy(CbAllocUtoU(pszuString), IN pszuString);
	}

PSZU
CMemoryAccumulator::PszuAllocateStringFromSingleCharacter(CHS chString)
	{
	PSZU pszData = (PSZU)PvAllocateData(2);
	pszData[0] = (CHU)chString;
	pszData[1] = '\0';
	return pszData;
	}

/*
//	Allocate a data structure, and append the string to the end of it
VOID *
CMemoryAccumulator::PvAllocateDataAndStringW(int cbAlloc, PSZWC pszwString)
	{
	Assert(pszwString != NULL);
	int cbString = CbAllocWtoW(pszwString);
	BYTE * pbData = (BYTE *)PvAllocateData(cbAlloc + cbString);
	memcpy(OUT pbData + cbAlloc, IN pszwString, cbString);
	return pbData;
	}


PSZU
CMemoryAccumulator::PszuAllocateStringNullUandAlignToDWORD(PSZUC pszuString)
	{
	if (pszuString == NULL || *pszuString == '\0')
		return (PSZU)szuNull;
	return (PSZU)PvAllocateDataCopyAndAlignToDWORD(CbAllocUtoU(pszuString), IN pszuString);
	}

//	Allocate an UTF-8 string from a Unicode string
PSZU
CMemoryAccumulator::PszuAllocateStringW(PSZWC pszwString)
	{
	Assert(pszwString != NULL);
	return (PSZU)PszaAllocateStringA((PSZAC)STR_WtoU(pszwString));
	}


PSZW
CMemoryAccumulator::PszwAllocateStringW(PSZWC pszwString)
	{
	Assert(pszwString != NULL);
	return (PSZW)PvAllocateDataCopy(CbAllocWtoW(pszwString), IN pszwString);
	}



//	This method never returns NULL.
//	For performance reasons, the allocated string is always allocated on a WORD boundary
PSZWC
CMemoryAccumulator::PszwAllocateStringWfromU(PSZUC pszuString)
	{
	if (pszuString == NULL || *pszuString == '\0')
		return szwNull;
	if (m_paData != NULL)
		{
		Assert(IS_ALIGNED_32(m_paData));				// Each allocation pointer is 32-bit aligned
		Assert(IS_ALIGNED_32(m_paData->cbAlloc));	// The size of each allocation is always 32-bit aligned
		m_paData->cbData = NEXT_UINT16(m_paData->cbData);
		}
	CHW * pchwDst = (CHW *)PvGetBuffer(CbAllocUtoW(pszuString));
	int cchwDst = _DecodeUtf8ToUnicode(OUT pchwDst, pszuString);
	Assert(cchwDst >= 0);	// The null-terminator is excluded
	m_paData->cbData += (cchwDst * sizeof(CHW)) + sizeof(CHW);	// Include the null-terminator
	return pchwDst;
	}



//	Generate a sort key from a string.
//	The sort key is null-terminated and must be compared using a byte comparison routine such as NCompareSortElementsByBytesZ().
//	Once the sort key is created, there is no more alphabet, but only series of
//	weight factors representing each character.
PSZUC
CMemoryAccumulator::PrgbzAllocateSortKeyW(PSZWC pszwString, UINT uFlagsASK)
	{
	const int cbAllocateExtra = (uFlagsASK & ASK_mskmAllocateExtraBytesMask);
	if (pszwString == NULL || *pszwString == '\0')
		{
		if (cbAllocateExtra == 0)
			return szuNull;	// No need to create a sort key, a null-terminated string does the trick
		pszwString = szwNull;
		}

	// Each unicode character may take up to 8 bytes, so compute the size of the string
	const CHW * pchw = pszwString;
	while (*pchw++ != '\0')
		;
	int cbBuffer = ((int)pchw - (int)pszwString) * 4;
	BYTE * prgbzSortKey = (BYTE *)PvGetBuffer(cbBuffer + cbAllocateExtra) + cbAllocateExtra;
	PFn_StringCreateSortKeyW pfnStringCreateSortKeyW = (uFlagsASK & ASK_mskfIgnorePunctuation) ? StringCreateSortKeyIgnorePunctuationW : StringCreateSortKeyW;
	int cbData = pfnStringCreateSortKeyW(OUT prgbzSortKey, cbBuffer, IN pszwString);
	Assert(cbData <= cbBuffer);
	m_paData->cbData += cbData + cbAllocateExtra;
	return prgbzSortKey;
	}

*/
/*
SORTKEY *
CMemoryAccumulator::PSortKeyAllocateForStringA(PSZAC pszaString)
	{
	Assert(pszaString != NULL);
	const char * pchSrc = pszaString;
	while (*pchSrc++ != '\0')
		;
	SORTKEY * pSortKey = (SORTKEY *)PvAllocateData(sizeof(SORTKEY) + (pchSrc - pszaString));
	// Build the sort key in lowercase
	char * pchDst = (char *)(pSortKey + 1);
	while (TRUE)
		{
		UINT chu = (BYTE)*pszaString++;
		if (chu >= 'A' && chu <= 'Z')
			chu += 32;	// Make lowercase
		*pchDst++ = chu;
		if (chu == '\0')
			break;
		} // while
	return pSortKey;
	} // PSortKeyAllocateForStringA()


SORTKEYINTEGER *
CMemoryAccumulator::PSortKeyAllocateForInteger(int nValue)
	{
	Assert(sizeof(SORTKEYINTEGER) == 8);
	SORTKEYINTEGER * pvSortKey = (SORTKEYINTEGER *)PvAllocateData(sizeof(SORTKEYINTEGER));	// Allocate 8 bytes
	pvSortKey->nValue = nValue;
	return pvSortKey;
	}
*/

//	Allocate a block of memory to hold multiple data elements.
void
CMemoryAccumulator::_AllocateMemoryBlock(int cbAlloc)
	{
	Assert(cbAlloc >= d_cbAllocMin);		// Minimum of 2 KB
	Endorse(!IS_ALIGNED_32(cbAlloc));

	cbAlloc = NEXT_UINT32(cbAlloc);
	SHeaderWithData * paData = (SHeaderWithData *)new BYTE[sizeof(SHeaderOnly) + cbAlloc];
	InitToGarbage(OUT paData, sizeof(SHeaderOnly) + cbAlloc);
	paData->pNext = m_paData;	// Append the block at the beginning of the list
	paData->cbData = 0;
	paData->cbAlloc = cbAlloc;
	m_paData = paData;
	} // _AllocateMemoryBlock()


