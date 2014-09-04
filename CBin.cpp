///////////////////////////////////////////////////////////////////////////////////////////////////
//	CBin.cpp
//
//	Storage-efficient class to handle binary large objects.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#ifdef DEBUG
//	#define _ALLOCATE_EXACT_BINARY_SIZE	// Force the bin to allocate the exact size (this is very slow, but good for debugging)
#endif

#ifdef _ALLOCATE_EXACT_BINARY_SIZE
	#define _cbAllocSizeExtra	1		// Just allocate one extra byte
#else
	#define _cbAllocSizeExtra	32		// Always add 32 more bytes for each allocations (to reduce the number of allocations)
#endif
#define _cbAllocSizeExtraForBinAppendText	20	// Allocate an extra 20 bytes when calling BinAppendText_VE() or BinAppendTextSzv_VL()

///////////////////////////////////////////////////////////////////////////////////////////////////
//	S_PaAllocateBytes(), static
//
//	Allocate memory to hold a number of bytes
//
CBin::SHeaderWithData *
CBin::S_PaAllocateBytes(int cbDataAlloc)
	{
	Assert(cbDataAlloc > 0);
	SHeaderWithData * paHeader = (SHeaderWithData *)new BYTE[sizeof(SHeaderOnly) + cbDataAlloc];
	Assert(paHeader!= NULL);
	#ifdef DEBUG
	#ifdef Q_OS_WIN
	const int cbAllocatedDebug = _msize(paHeader);	// malloc_usable_size()
	const int cbAllocatedExtraDebug = cbAllocatedDebug - (sizeof(SHeaderOnly) + cbDataAlloc);
	Assert(cbAllocatedExtraDebug >= 0);
	if (cbAllocatedExtraDebug > 0)
		TRACE1("S_PaAllocateBytes() - %d bytes extra allocated\n", cbAllocatedExtraDebug);
	#endif // Q_OS_WIN
	#endif // DEBUG
	InitToGarbage(OUT paHeader, sizeof(SHeaderOnly) + cbDataAlloc);
	paHeader->cbAlloc = cbDataAlloc;
	//ValidateHeapPointer(paHeader);
	return paHeader;
	}

CBin::CBin(const CBin & bin)
	{
	m_paData = NULL;
	BinAppendCBin(bin);
	}

//	Return TRUE if the bin is empty
BOOL
CBin::FIsEmptyBinary() const
	{
	return (m_paData == NULL || m_paData->cbData == 0);
	}

//	Return the number of bytes stored in the buffer.
int
CBin::CbGetData() const
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		return m_paData->cbData;
		}
	return 0;
	}

#ifdef DEBUG
//	Return the number of bytes allocated by the bin.
//	The number of bytes allocated is always larger or equal to the size of the data.
int
CBin::CbGetDataAllocated() const
	{
	if (m_paData == NULL)
		return 0;
	Assert(m_paData->cbAlloc >= m_paData->cbData);
	return m_paData->cbAlloc;
	}

//	Return the number of bytes have been allocated more than the actual data.
//	This number includes the size of SHeaderOnly
int
CBin::CbGetDataAllocatedOverhead() const
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		return sizeof(SHeaderOnly) + (m_paData->cbAlloc - m_paData->cbData);
		}
	return 0;
	}
#endif // DEBUG

//	Return the pointer to data stored in the buffer.
const void *
CBin::PvGetData() const
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		return m_paData->rgbData;
		}
	return NULL;
	}


//	Return a byte pointer to the content of the bin.
//	This pointer can be used to directly manipulate the content of the bin.
BYTE *
CBin::PbGetData()
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		return m_paData->rgbData;
		}
	return NULL;
	}


//	Somewhat the equivalent of calling BinAppendBinaryData(NULL, cbData) if the memory has already been allocated.
//	This method is useful with the PbAllocateExtraMemory() as a substitute of TruncateData() or TruncateDataAt().
//	This method can accept a negative number of bytes to remove bytes from the buffer.
void
CBin::BinAppendBytesEmpty(int cbData)
	{
	Endorse(cbData < 0);	// Remove some bytes
	Assert(m_paData != NULL);
	m_paData->cbData += cbData;
	Assert(m_paData->cbData >= 0);
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Return a pointer to a string.
//	This routine never return NULL.
//
//	Typically the string is null-terminated, however the caller
//	is responsible to use the CBin object in a way the content
//	of the CBin contains a null-terminated string.
//
PSZUC
CBin::PszuGetDataNZ() const
	{
	if (m_paData != NULL && m_paData->cbData > 0)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		return m_paData->rgbData;
		}
	return c_szuEmpty;
	}


//	Return TRUE if a pointer is within the address range of the bin.
//	This method is used to debugging to avoid a pointer from the bin
//	being used as a parameter to append to the same bin
BOOL
CBin::FIsPointerAddressWithinBinaryObject(const void * pvData) const
	{
	if (pvData == NULL)
		return FALSE;
	if (m_paData == NULL)
		return FALSE;
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	return (pvData >= m_paData->rgbData && pvData <= m_paData->rgbData + m_paData->cbAlloc);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	PvAllocateMemoryAndEmpty()
//
//	Allocate a block of memory to hold a desired number of bytes.
//
//	USAGE
//	Use this method to allocate bytes before using the object.
//	Pre-allocating the binary will avoid the need to reallocate and copy the binary data when more bytes are added.
//
//	INTERFACE NOTES
//	This method destroys the content of the object.
//	After calling this method, the size returned by CbGetData() is always zero.
//
//	Return pointer to the buffer of the binary data.
//
void *
CBin::PvAllocateMemoryAndEmpty(int cbDataAlloc)
	{
	Assert(cbDataAlloc >= 0);
	if (m_paData == NULL)
		{
		if (cbDataAlloc == 0)
			return NULL;
		m_paData = S_PaAllocateBytes(cbDataAlloc);
		}
	else if (m_paData->cbAlloc < cbDataAlloc)
		{
		// We need to allocate a new block of memory
		//ValidateHeapPointer(m_paData);
		delete m_paData;
		m_paData = NULL;
		m_paData = S_PaAllocateBytes(cbDataAlloc);
		}
	m_paData->cbData = 0;
	return m_paData->rgbData;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Allocate bytes for the bin and set its size to cbData.
//	The method may expand or truncate the bin if necessary.
//
//	Return pointer to newly allocated buffer.
void *
CBin::PvAllocateMemoryAndSetSize(int cbData)
	{
	void * pv = PvAllocateMemoryAndEmpty(cbData);
	if (m_paData != NULL)
		m_paData->cbData = cbData;
	return pv;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	PvAllocateMemoryToGrowBy()
//
//	Allocate more bytes to the blob without changing its data size.
//
//	PERFORMANCE NOTES
//	To avoid unnecessary memory allocations and copying data,
//	each re-allocation is twice as large as its previous.
//
//	INTERFACE NOTES
//	This method does NOT destroys the content of the bin.
//	After calling this method, the size returned by CbGetData() the
//	same as prior the call.
//
//	Return pointer to the BEGINNING of buffer of the bin.
//
//	SEE ALSO
//	PbAllocateExtraMemory() this method does the same but return pointer pointer to the END of the bin.
//
void *
CBin::PvAllocateMemoryToGrowBy(int cbDataGrowBy)
	{
	Assert(cbDataGrowBy >= 0);
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		const int cbDataOld = m_paData->cbData;
		const int cbDataNew = cbDataOld + cbDataGrowBy;
		if (cbDataNew > m_paData->cbAlloc)
			{
			#ifdef _ALLOCATE_EXACT_BINARY_SIZE
			SHeaderWithData * paHeader = S_PaAllocateBytes(cbDataNew);
			#else
			SHeaderWithData * paHeader = S_PaAllocateBytes(cbDataNew + cbDataOld + _cbAllocSizeExtra);
			#endif
			memcpy(OUT paHeader->rgbData, IN m_paData->rgbData, cbDataOld);
			//ValidateHeapPointer(m_paData);
			delete m_paData;
			m_paData = paHeader;
			m_paData->cbData = cbDataOld;
			}
		return m_paData->rgbData;
		}
	return PvAllocateMemoryAndEmpty(cbDataGrowBy + _cbAllocSizeExtra);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Allocate more storage to the blob and return pointer to the end of the blob.
//	The allocated storage does NOT change the size of the blob.
//	To update the size of the blob, the caller must invoke either BinAppendBytesEmpty(), SetSizeGrowBy() or some other method appending data.
//
//	This method never return NULL.
BYTE *
CBin::PbAllocateExtraMemory(int cbAllocGrowBy)
	{
	Assert(cbAllocGrowBy >= 0);
	BYTE * pbData = (BYTE *)PvAllocateMemoryToGrowBy(cbAllocGrowBy);
	Assert(m_paData != NULL);
	return (pbData + m_paData->cbData);
	}

BYTE *
CBin::PbAllocateExtraData(int cbDataGrowBy)
	{
	Assert(cbDataGrowBy >= 0);
	BYTE * pbData = (BYTE *)PvAllocateMemoryToGrowBy(cbDataGrowBy);
	Assert(m_paData != NULL);
	const int cbData = m_paData->cbData;
	m_paData->cbData += cbDataGrowBy;
	return (pbData + cbData);
	}

BYTE *
CBin::PbAllocateExtraDataWithVirtualNullTerminator(int cbDataGrowBy)
	{
	Assert(cbDataGrowBy >= 0);
	BYTE * pbData = (BYTE *)PvAllocateMemoryToGrowBy(cbDataGrowBy + sizeof(CHU));
	Assert(m_paData != NULL);
	const int cbData = m_paData->cbData;
	m_paData->cbData += cbDataGrowBy;
	m_paData->rgbData[m_paData->cbData] = '\0';	// Append the virtual null-terminator
	return (pbData + cbData);
	}

//	Return the number of bytes available after an offset with a value smaller or equal to cbDataMax.
//	This method also returns the number of bytes remaining for a subsequent call.
int
CBin::CbGetDataAfterOffset(int ibData, int cbDataMax, OUT int * pcbDataRemaining) const
	{
	Assert(ibData >= 0);
	Assert(cbDataMax > 0);
	Assert(pcbDataRemaining != NULL);
	if (m_paData != NULL)
		{
		Report(ibData <= m_paData->cbData && "Should not request an offset larger than the data");
		int cbAvailable = m_paData->cbData - ibData;
		if (cbAvailable > 0)
			{
			int cbRemain = cbAvailable - cbDataMax;
			if (cbRemain > 0)
				{
				// The amount of data is larger than the maximum buffer size
				*pcbDataRemaining = cbRemain;
				return cbDataMax;
				}
			*pcbDataRemaining = 0;	// There is no more data remaining
			return cbAvailable;
			}
		}
	*pcbDataRemaining = 0;
	return 0;
	}

//	Return a pointer of the data at an offset.
//	Return NULL if the offset is out of range.
PVOID
CBin::PvGetDataAtOffset(int ibData) const
	{
	Assert(ibData >= 0);
	if (m_paData != NULL)
		{
		Assert(ibData <= m_paData->cbData);
		if (m_paData->cbData >= ibData)
			return m_paData->rgbData + ibData;
		}
	return NULL;
	}

//	Same as DataTruncateAtOffset() however return pointer to where the data was truncated (or return NULL if there is an error because the offset is out of range)
PVOID
CBin::DataTruncateAtOffsetPv(UINT cbDataKeep)
	{
	if (m_paData != NULL)
		{
		if ((UINT)m_paData->cbData > cbDataKeep)
			m_paData->cbData = cbDataKeep;
		return (PVOID)(m_paData->rgbData + m_paData->cbData);
		}
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Truncate the binary to contain at most cbData.
//
//	INTERFACE NOTES
//	If the buffer is truncated, the data will remain in
//	the buffer as long as no other data is appended to the bin.
//
void
CBin::DataTruncateAtOffset(UINT cbDataKeep)
	{
	if (m_paData != NULL)
		{
		if ((UINT)m_paData->cbData > cbDataKeep)
			m_paData->cbData = cbDataKeep;
		}
	}

//	Truncate the binary to have its end of buffer at pvDataEnd.
//	This method is used with PbAllocateExtraMemory() and/or PvAllocateMemoryToGrowBy().
//
//	SEE ALSO: CStr::StringTruncateAt()
void
CBin::DataTruncateAtPointer(const void * pvDataEnd)
	{
	Assert(FIsPointerAddressWithinBinaryObject(pvDataEnd));
	Assert(m_paData != NULL);
	DEBUG_CODE( int cbDataOld = m_paData->cbData; )
	m_paData->cbData = (BYTE *)pvDataEnd - m_paData->rgbData;
	Assert(m_paData->cbData >= 0 && m_paData->cbData <= m_paData->cbAlloc);
	#ifdef DEBUG
	// Fill the tail with garbages, to help debugging
	int cchTruncate = cbDataOld - m_paData->cbData;
	if (cchTruncate > 0)
		{
		InitToGarbage(OUT (void *)pvDataEnd, cchTruncate);
		}
	#endif
	ValidateHeapPointer(m_paData);
	}

//	Remove data until reaching the pointer.
//	In other words pvDataStart will be the new beginning of the binary data.
void
CBin::DataRemoveUntilPointer(const void * pvDataStart)
	{
	Assert(FIsPointerAddressWithinBinaryObject(pvDataStart));
	Assert(m_paData != NULL);
	const int cbRemove = (BYTE *)pvDataStart - m_paData->rgbData;
	Assert(cbRemove > 0 && cbRemove <= m_paData->cbData);
	RemoveData(0, cbRemove);
	DEBUG_CODE( GarbageInitializedUnusedBytes() );
	}


//	Ensure the content of the bin does not contain any null-terminator, however
//	the bin itself has a virtual null-terminator (to make a valid null-terminated string)
void
CBin::BinEnsureContentHasNoNullTerminatorAndIsTerminatedWithVirtualNullTerminator()
	{
	Assert(m_paData != NULL);
	Assert(m_paData->cbData < m_paData->cbAlloc);	// There should be room for a virtual null-terminator
	CHU * pchNullTerminatorLast = NULL;
	CHU * pchData = m_paData->rgbData;
	CHU * pch = pchData;
	CHU * pchStop = pchData + m_paData->cbData;
	while (pch != pchStop)
		{
		if (*pch++ == '\0')
			pchNullTerminatorLast = pch;		// The bin contains a null-terminator
		} // while
	if (pchNullTerminatorLast != NULL)
		{
		// Since there is no easy way to recover from that type of error, remove any previous string until we get a bin without a null-termiator (this may imply an empty bin)
		#ifdef DEBUG
		int cbRemoved = pchNullTerminatorLast - m_paData->rgbData;
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning,
			"Removing data from bin because it contains a null-terminator:\n"
			"Data Removed ($I bytes): $s\n"
			"Data Remain ($I bytes): $s\n",
			cbRemoved, m_paData->rgbData,  m_paData->cbData - cbRemoved, pchNullTerminatorLast);
		#endif
		DataRemoveUntilPointer(pchNullTerminatorLast);
		}
	Assert(pchData == m_paData->rgbData);
	pchData[m_paData->cbData] = '\0';	// Insert the virtual null-terminator
	} // BinEnsureContentHasNoNullTerminatorAndIsTerminatedWithVirtualNullTerminator()

///////////////////////////////////////////////////////////////////////////////////////////////////
//	BinInitFromBinaryData()
//
//	Initialize the object with binary data.
//
//	The method may re-allocate memory if the existing block of memory
//	is not large enough to hold the new data.
//
void
CBin::BinInitFromBinaryData(const void * pvData, int cbData)
	{
	Assert(pvData != NULL);
	Assert(!FIsPointerAddressWithinBinaryObject(pvData));
	if (cbData > 0)
		{
		(void)PvAllocateMemoryAndEmpty(cbData);
		Assert(m_paData != NULL);
		Assert(m_paData->cbData == 0);
		memcpy(OUT m_paData->rgbData, IN pvData, cbData);
		m_paData->cbData = cbData;
		}
	else
		{
		if (m_paData != NULL)
			m_paData->cbData = 0;
		}
	} // BinInitFromBinaryData()

void
CBin::BinInitFromBinaryDataWithVirtualNullTerminator(const void * pvData, int cbData)
	{
	BinInitFromBinaryDataWithExtraVirtualZeroes(pvData, cbData, sizeof(BYTE));
	}

//	Same as BinInitFromBinaryData() however extra virtual zeroes are added to the end.
//	Those virtual zeroes are not included as part of the size of the bin.
//	Useful to create a null-terminated string from some data.
void
CBin::BinInitFromBinaryDataWithExtraVirtualZeroes(const void * pvData, int cbData, int cbExtraVirtualZeroes)
	{
	Assert(pvData != NULL);
	Assert(cbData > 0);
	Endorse(cbData == 0);
	Assert(cbExtraVirtualZeroes > 0);
	Assert(!FIsPointerAddressWithinBinaryObject(pvData));

	BYTE * pbDst = (BYTE *)PvAllocateMemoryAndEmpty(cbData + cbExtraVirtualZeroes);
	Assert(m_paData != NULL);
	Assert(m_paData->cbData == 0);
	m_paData->cbData = cbData;				// Set the data size
	memcpy(OUT pbDst, IN pvData, cbData);	// Copy the data
	// Pad with zeroes
	pbDst += cbData;
	while (cbExtraVirtualZeroes-- > 0)
		*pbDst++ = '\0';
	} // BinInitFromBinaryDataWithExtraVirtualZeroes()


//	Copy a string into the bin.
//	The method copies the null-terminator.
void
CBin::BinInitFromStringWithNullTerminator(PSZAC pszData)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszData));
	if (pszData != NULL && *pszData != '\0')
		{
		BinInitFromBinaryData(IN pszData, strlen(pszData) + 1);
		}
	else
		{
		if (m_paData != NULL)
			m_paData->cbData = 0;
		}
	}

void
CBin::BinInitFromText(PSZAC pszText)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszText));
	if (pszText != NULL && *pszText != '\0')
		{
		BinInitFromBinaryData(IN pszText, strlen(pszText));
		}
	else
		{
		if (m_paData != NULL)
			m_paData->cbData = 0;
		}
	}

//	Initialize the object to contain one byte of data
void
CBin::BinInitFromByte(BYTE bData)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;	// Empty the object
	BinAppendByte(bData);
	}

PSZU
CBin::BinInitFromTextSzv_VE(PSZAC pszFmtTemplate, ...)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;	// Empty the object
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	return BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	}

PSZU
CBin::BinInitFromTextSzv_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;	// Empty the object
	return BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	}


void
CBin::InsertData(int ibData, const void * pvData, int cbData)
	{
	if (cbData == 0)
		return;
	Endorse(pvData == NULL);	// Don't write any data - just make room for the new data
	Assert(!FIsPointerAddressWithinBinaryObject(pvData));
	int cbDataOld = 0;
	int cbDataAllocated = 0;
	if (m_paData != NULL)
		{
		cbDataOld = m_paData->cbData;
		cbDataAllocated = m_paData->cbAlloc;
		}
	Assert(ibData >= 0 && ibData <= cbDataOld);
	int cbDataAlloc = cbDataOld + cbData;
	if (cbDataAllocated < cbDataAlloc)
		{
		SHeaderWithData * paHeader = S_PaAllocateBytes(cbDataAlloc + cbDataOld + _cbAllocSizeExtra);
		if (m_paData != NULL)
			{
			BYTE * pbDest = paHeader->rgbData;
			memcpy(OUT pbDest, IN m_paData->rgbData, ibData);
			if (pvData != NULL)
				memcpy(pbDest + ibData, pvData, cbData);
			memcpy(OUT pbDest + ibData + cbData, m_paData->rgbData + ibData, cbDataOld - ibData);
			ValidateHeapPointer(m_paData);
			delete m_paData;
			}
		else
			{
			Assert(ibData == 0);
			if (pvData != NULL)
				memcpy(OUT paHeader->rgbData, pvData, cbData);
			}
		m_paData = paHeader;
		m_paData->cbData = cbDataOld + cbData;
		return;
		}
	Assert(m_paData != NULL);
	BYTE * pbDest = m_paData->rgbData + ibData;
	memmove(OUT pbDest + cbData, IN pbDest, cbDataOld - ibData);		// OLDBUG: memmove(OUT pbDest + cbData, IN pbDest, cbData);
	if (pvData != NULL)
		memcpy(OUT pbDest, pvData, cbData);
	m_paData->cbData = cbDataOld + cbData;
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	ValidateHeapPointer(m_paData);
	} // InsertData()

void
CBin::InsertDataAtBeginning(const void * pvData, int cbData)
	{
	InsertData(0, pvData, cbData);
	}

//	Insert one byte at the beginning of the bin
void
CBin::InsertByteAtBeginning(BYTE bData)
	{
	InsertDataAtBeginning(IN &bData, sizeof(bData));
	}

int
CBin::IbFindDataAfterString(PSZAC pszString) const
	{
	Assert(pszString != NULL);
	if (m_paData != NULL)
		{
		PSZUC pszData = PszrStringContainsSubString(m_paData->rgbData, pszString);
		if (pszData != NULL)
			{
			Assert(pszData > m_paData->rgbData);
			return pszData - m_paData->rgbData;
			}
		}
	return -1;
	}

//	Remove a number of bytes at a given offset
void
CBin::RemoveData(int ibData, int cbData)
	{
	Assert(ibData >= 0);
	Assert(cbData >= 0);
	if (m_paData != NULL)
		{
		Assert(ibData + cbData <= m_paData->cbData);
		int cbRemain = m_paData->cbData - (ibData + cbData);
		Assert(cbRemain >= 0);
		BYTE * pbDest = m_paData->rgbData + ibData;
		memmove(OUT pbDest, pbDest + cbData, cbRemain);
		m_paData->cbData -= cbData;
		}
	else
		{
		Assert(ibData == 0);
		Assert(cbData == 0);
		}
	ValidateHeapPointer(m_paData);
	} // RemoveData()

//  Substitutes a sub-part of the bin's data with other data.
void
CBin::SubstituteData(int ibRepl, int cbRepl, const void * pvSubst, int cbSubst)
	{
	if (m_paData != NULL)
		{
		Assert((ibRepl >= 0) && (ibRepl < m_paData->cbData));
		Assert(cbRepl >= 0);
		Assert(ibRepl + cbRepl <= m_paData->cbData);
		if (cbSubst < cbRepl)
			{
			memcpy(m_paData->rgbData + ibRepl, pvSubst, cbSubst);
			RemoveData(ibRepl + cbSubst, cbRepl - cbSubst);
			}  // if
		else
			{
			memcpy(m_paData->rgbData + ibRepl, pvSubst, cbRepl);
			InsertData(ibRepl + cbRepl, ((BYTE*)pvSubst) + cbRepl, cbSubst - cbRepl);
			}  // else
		}  // if
	else
		{
		Assert(ibRepl == 0);
		Assert(cbRepl == 0);
		}  // else
	ValidateHeapPointer(m_paData);
	}  // SubstituteData()


//	Remove a number of bytes from the end of the bin
void
CBin::RemoveBytesFromEnd(int cbData)
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbData >= cbData);
		m_paData->cbData -= cbData;
		}
	else
		{
		Assert(cbData == 0);
		}
	} // RemoveBytesFromEnd()

///////////////////////////////////////////////////////////////////////////////////////////////////
//	BinAppendBinaryData()
//
//	Append data to the end of the bin.
//
//	The method may re-allocate memory if the existing block of memory
//	is not large enough to hold the new data.
//
void
CBin::BinAppendBinaryData(const void * pvData, int cbData)
	{
	Endorse(pvData == NULL);	// Allocate virtual bytes.  The pointer pvData is often NULL when cbData is zero.
	Assert(cbData >= 0);
	if (cbData > 0)
		{
		Assert(!FIsPointerAddressWithinBinaryObject(pvData));
		Assert(!FIsPointerAddressWithinBinaryObject((BYTE *)pvData + cbData));
		if (m_paData != NULL)
			{
			int cbDataOld = m_paData->cbData;
			int cbDataNew = cbDataOld + cbData;
			if (cbDataNew > m_paData->cbAlloc)
				{
				SHeaderWithData * paHeader = S_PaAllocateBytes(cbDataNew + cbDataOld + _cbAllocSizeExtra);	// Allocate at least twice the data
				memcpy(OUT paHeader->rgbData, IN m_paData->rgbData, cbDataOld);
				ValidateHeapPointer(m_paData);
				delete m_paData;
				m_paData = paHeader;
				}
			m_paData->cbData = cbDataNew;
			if (pvData != NULL)
				memcpy(OUT m_paData->rgbData + cbDataOld, IN pvData, cbData);
			}
		else
			{
			m_paData = S_PaAllocateBytes(cbData + _cbAllocSizeExtra);
			m_paData->cbData = cbData;
			if (pvData != NULL)
				memcpy(OUT m_paData->rgbData, IN pvData, cbData);
			} // if...else
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		} // if
	ValidateHeapPointer(m_paData);
	} // BinAppendBinaryData()

//	Append all the data between two pointers
void
CBin::BinAppendBinaryDataPvPv(const void * pvDataStart, const void * pvDataStop)
	{
	Assert(pvDataStart <= pvDataStop);
	BinAppendBinaryData(pvDataStart, (const BYTE *)pvDataStop - (const BYTE *)pvDataStart);
	}

//	Append a string without its null-terminator
void
CBin::BinAppendCStr(const CStr & strSrc)
	{
	const SHeaderWithData * pDataSrc = strSrc.m_paData;
	if (pDataSrc == NULL)
		return;
	const int cchString = pDataSrc->cbData - 1;
	if (cchString > 0)
		{
		Assert(strlenU(pDataSrc->rgbData) == (unsigned)cchString);
		BinAppendBinaryData(pDataSrc->rgbData, cchString);
		}
	}

void
CBin::BinAppendCBin(const CBin & binSrc)
	{
	Assert(&binSrc != NULL);
	Assert(&binSrc != this);
	const SHeaderWithData * pDataSrc = binSrc.m_paData;
	if (pDataSrc == NULL)
		return;
	BinAppendBinaryData(pDataSrc->rgbData, pDataSrc->cbData);
	}

//	Append the content of the binary from an offset
void
CBin::BinAppendCBinFromOffset(const CBin & binSrc, int ibDataBegin)
	{
	Assert(&binSrc != NULL);
	Assert(&binSrc != this);
	Assert(ibDataBegin >= 0);
	const SHeaderWithData * pDataSrc = binSrc.m_paData;
	if (pDataSrc == NULL)
		return;
	BinAppendBinaryData(IN pDataSrc->rgbData + ibDataBegin, pDataSrc->cbData - ibDataBegin);
	}

void
CBin::BinAppendCBinLowercaseAscii(const CBin & binSrc)
	{
	Assert(&binSrc != NULL);
	Assert(&binSrc != this);
	const SHeaderWithData * pDataSrc = binSrc.m_paData;
	if (pDataSrc == NULL)
		return;
	const BYTE * pbSrc = pDataSrc->rgbData;
	int cbData = pDataSrc->cbData;
	const BYTE * pbSrcStop = pbSrc + cbData;
	BYTE * pbDst = PbAllocateExtraData(cbData);
	while (pbSrc != pbSrcStop)
		*pbDst++ = Ch_GetCharLowercase(*pbSrc++);
	}

void
CBin::BinAppendByteArray(const QByteArray & arraybData)
	{
	BinAppendBinaryData(arraybData.data(), arraybData.size());
	}

//	Append a single byte to the bin.
void
CBin::BinAppendByte(UINT bData)
	{
	if (m_paData != NULL)
		{
		const int cbData = m_paData->cbData;
		if (cbData < m_paData->cbAlloc)
			{
			m_paData->rgbData[cbData] = bData;
			m_paData->cbData = cbData + sizeof(BYTE);
			}
		else
			{
			BinAppendBinaryData(IN &bData, sizeof(BYTE));	// Re-allocate more memory
			}
		}
	else
		{
		m_paData = S_PaAllocateBytes(_cbAllocSizeExtra);	// Allocate minimum of 32 bytes
		m_paData->rgbData[0] = bData;
		m_paData->cbData = 1;
		}
	} // BinAppendByte()


//	Append a Byte multiple times
void
CBin::BinAppendBytes(UINT bData, int cCount)
	{
	Endorse(cCount < 0);	// A negative value is allowed
	while (cCount-- > 0)
		BinAppendByte(bData);
	}

//	Append a null-terminator without increasing the size of the binary object.
//	Return a pointer to the beginning of the binary data (string)
PSZUC
CBin::BinAppendNullTerminatorVirtualSzv() CONST_TEMPORARY_MODIFIED
	{
	Endorse(m_paData == NULL);
	BinAppendByte('\0');
	Assert(m_paData != NULL);
	m_paData->cbData--;
	return m_paData->rgbData;
	}

#pragma GCC diagnostic ignored "-Wstrict-aliasing"	// The instruction "*(WORD *)(m_paData->rgbData + cbData) = wData" is causing a compiler warning in the release build
void
CBin::BinAppendUInt16(UINT wData)
	{
	Assert(sizeof(WORD) == 2);
	if (m_paData != NULL)
		{
		// Optimization for the common case
		const int cbData = m_paData->cbData;
		const int cbDataNew = cbData + sizeof(WORD);
		if (cbDataNew <= m_paData->cbAlloc)
			{
			*(WORD *)(m_paData->rgbData + cbData) = wData;
			m_paData->cbData = cbDataNew;
			ValidateHeapPointer(m_paData);
			return;
			}
		}
	BinAppendBinaryData(IN &wData, sizeof(WORD));
	ValidateHeapPointer(m_paData);
	}

void
CBin::BinAppendUInt32(UINT uData)
	{
	Assert(sizeof(uData) == 4);
	BinAppendBinaryData(IN &uData, sizeof(uData));
	}

//	Append bytes until the size of the binary data is 32 bit aligned.
void
CBin::BinAppendZeroBytesUntilAligned32()
	{
	if (m_paData != NULL)
		{
		int cbData = m_paData->cbData;
		if (cbData & 0x3)
			{
			UINT cbExtra = 4 - (cbData & 0x3);
			Assert(cbExtra > 0 && cbExtra < 4);
			const UINT uZero = 0;
			BinAppendBinaryData(IN &uZero, cbExtra);
			}
		}
	}

void
CBin::BinAppendTextInteger(int nInteger, UINT uFlagsITS)
	{
	CHU szValue[16];
	IntegerToString(OUT szValue, nInteger, uFlagsITS);
	BinAppendText((PSZAC)szValue);
	}

void
CBin::BinAppendTextBytesKiB(L64 cbBytes)
	{
	if (cbBytes < 100000)
		BinAppendText_VE((cbBytes > 0) ? "$L bytes" : "? bytes", cbBytes);
	else
		BinAppendText_VE("$L KiB", cbBytes / 1024);
	}

//	Format a string such as:
//		"17%"
//		"17%"
//		"123 bytes"
//		"4.5 KiB"
void
CBin::BinAppendTextBytesKiBPercent(L64 cbBytesReceived, L64 cbBytesTotal)
	{
	Endorse(cbBytesTotal < 0);		// This is the case when the total is unknown, and therefore to display the amount of data received without any percentage
//	Assert(cbBytesTotal >= 0);		// This value should be positive
	if (cbBytesTotal > 0)
		{
		// If we have the total number of bytes, we may calculate the percentage
		int nPercent = (cbBytesReceived * 100) / cbBytesTotal;
		if (nPercent > 0)
			{
			BinAppendTextInteger(nPercent);
			BinAppendByte('%');
			return;
			}
		// If the percentage is zero, display the number of bytes (this may happen if the file is very large, and it is important to display to the user some feedback the transfer has begun
		}
	BinAppendTextBytesKiB(cbBytesReceived);
	}

void
CBin::BinAppendTextBytesKiBPercentProgress(L64 cbBytesReceived, L64 cbBytesTotal)
	{
	if (cbBytesReceived > 0)
		{
		BinAppendTextBytesKiBPercent(cbBytesReceived, cbBytesTotal);
		BinAppendStringWithNullTerminator(" of ");
		}
	else
		BinAppendStringWithNullTerminator(" total of ");
	BinAppendTextBytesKiB(cbBytesTotal);
	}

void
CBin::BinAppendTimestamp(TIMESTAMP ts)
	{
	m_paData->cbData += Timestamp_CchToString(ts, OUT PbAllocateExtraMemory(16));
	}

void
CBin::BinAppendTimestampSpace(TIMESTAMP ts)
	{
	BinAppendTimestamp(ts);
	m_paData->rgbData[m_paData->cbData++] = ' ';	// Append an extra space
	}

//	Copy from the source string until a character or the end of the string.
//
//	Return the pointer of the remaining of the string.  This pointer is never NULL and is either a null termnator or a string beginning with chCopyUntil.
//
//	See also AppendTextUntilCharacterPszr()
PSZR
CBin::BinAppendTextUntilCharacterPszr(PSZUC pszuSource, UINT chCopyUntil)
	{
	Assert(pszuSource != NULL);
	PSZUC pszSourceRemaining = pszuSource;
	while (TRUE)
		{
		const UINT ch = *pszSourceRemaining;
		if (ch == '\0' || ch == chCopyUntil)
			break;
		pszSourceRemaining++;
		} // while
	BinAppendBinaryData(pszuSource, pszSourceRemaining - pszuSource);
	return pszSourceRemaining;
	}

//	No virtual null-terminator is appended
void
CBin::BinAppendText(PSZAC pszText)
	{
	Assert(pszText != NULL);
	Assert(!FIsPointerAddressWithinBinaryObject(pszText));
	BinAppendBinaryData(IN pszText, strlenU(pszText));
	}

//	Append a string, including its null-terminator to the array
void
CBin::BinAppendStringWithNullTerminator(PSZAC pszString)
	{
	Assert(pszString != NULL);
	Assert(!FIsPointerAddressWithinBinaryObject(pszString));
	BinAppendBinaryData(IN pszString, CbAllocUtoU((PSZUC)pszString));
	}


//	Append a Unicode string to an UTF-8 bin.
//	The Unicode string is converted from UTF-16 to UTF-8.
//	A virtual null-terminator is appended to the string
void
CBin::BinUtf8AppendStringW(PSZWC pszwString)
	{
	if (pszwString != NULL)
		{
		Assert(!FIsPointerAddressWithinBinaryObject(pszwString));
		CHU * pchuDst = PbAllocateExtraMemory(CbAllocWtoU(pszwString));
		m_paData->cbData += EncodeUnicodeToUtf8(OUT pchuDst, IN pszwString);
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		}
	}

void
CBin::BinUtf8AppendStringQ(const QString & sString)
	{
	if (&sString != NULL)
		{
		const int cchwString = sString.length();
		CHU * pchuDst = PbAllocateExtraMemory(CbAllocCchwToU(cchwString));
		m_paData->cbData += EncodeUnicodeToUtf8(OUT pchuDst, IN sString.utf16());
		}
	}

const char c_sz_a_href_http[] = "<a href='http://";
#define d_cb_a_href			9
#define d_cb_a_href_http	16

PCHRO
CBin::BinAppendHtmlHyperlinkPchro(PCHC pchTextBeforeHyperlink, PCHC pchHyperlinkBegin, EScheme eScheme)
	{
	Assert(pchTextBeforeHyperlink <= pchHyperlinkBegin);
	BinAppendHtmlTextUntilPch(pchTextBeforeHyperlink, pchHyperlinkBegin);
	// Find the end of the hyperlink
	PCHC pchEnd = pchHyperlinkBegin;
	while (TRUE)
		{
		CHS ch = *pchEnd;
		if (ch == '<' || ch == '\'' || ch == '"')
			break;		// An hyperlink ends if reaching the beginning of an HTML tag, or a single quote or double quote
		if (!Ch_FIsWhiteSpaceOrNullTerminator(ch))
			pchEnd++;
		else
			break;
		}
	// Trim the remaining punctuation
	while (TRUE)
		{
		CHS ch = *--pchEnd;
		if (ch == '/' || ch == '\\')
			break;
		if (!Ch_FIsPunctuation(ch))
			break;
		}
	PCHRO pchrNext = pchEnd + 1;
	BinAppendBinaryData(c_sz_a_href_http, eScheme == eSchemeHttpImplicit ? d_cb_a_href_http : d_cb_a_href);
	BinAppendBinaryDataPvPv(pchHyperlinkBegin, pchrNext);
	BinAppendBinaryData("'>", 2);
	BinAppendBinaryDataPvPv(pchHyperlinkBegin, pchrNext);
	BinAppendBinaryData("</a>", 4);
	return pchrNext;
	} // BinAppendHtmlHyperlinkPchro()

//	At the moment, this is the lazy implementation for a method inserting hyperlinks
void
CBin::BinAppendHtmlTextWithAutomaticHyperlinks(PSZUC pszText)
	{
	#if 1
	#ifdef DEBUG
	PSZUC pszTextBegin = pszText;
	Assert(pszTextBegin != NULL);
	#endif
	PCHUC pchBegin = pszText;
	while (TRUE)
		{
		FetchNextCharacter:
		CHS ch = *pszText;
		if (ch == ':')
			{
			// Search for a scheme
			PCHUC pchScheme = pszText;
			while (--pchScheme >= pchBegin)
				{
				ch = Ch_GetCharLowercase(*pchScheme);
				if (Ch_FIsAlphaNumeric(ch) || ch == '-' || ch == '+' || ch == '.')
					continue;
				break;
				}
			EScheme eScheme = EGetScheme(++pchScheme);
			if (eScheme != eSchemeUnknown)
				{
				pchBegin = pszText = BinAppendHtmlHyperlinkPchro(pchBegin, pchScheme, eScheme);
				continue;
				}
			}
		else if (ch == '.')
			{
			// The dot is the core delimiter to determine if there is an hyperlink.  What we need to check if there is a domain extension after the dot such as (.com, .org, .uk, .tv and so on)
			PSZUC pszTextExtension = pszText + 1;
			PSZR pszAfterExtension = PszrFindDomainExtension(pszTextExtension);
			if (pszAfterExtension != NULL)
				{
				// Search for the beginning of the domain name
				while (pszText > pchBegin)
					{
					ch = Ch_GetCharLowercase(*--pszText);
					if (Ch_FIsAlphaNumeric(ch) || ch == '-')
						continue;
					if (ch == '.')
						{
						if (pszText[1] == '.')
							{
							// A domain name cannot have consecutive dots
							pszText += 2;				// Skip the dots
							goto FetchNextCharacter;	// And continue
							}
						continue;
						}
					if (ch == '@')
						{
						pszText = pszAfterExtension;
						goto FetchNextCharacter;	// Do not create hyperlink for email addresses.  This is because Cambrian uses JIDs which look like emails.  Besides, communicating by email is insecure and there is no need to use emails if already using Cambrian.
						}
					pszText++;
					break;
					} // while
				if (*pszText == '.')
					{
					pszText++;	// Skip the leading dot because a domain name cannot being with a dot
					if (pszText == pszTextExtension)
						continue;
					}
				pchBegin = pszText = BinAppendHtmlHyperlinkPchro(pchBegin, pszText, eSchemeHttpImplicit);
				continue;
				}
			} // if (dot)
		else if (ch == '\0')
			{
			BinAppendHtmlTextUntilPch(pchBegin, pszText);
			break;
			}
		pszText++;
		} // while
	#else
	CStr str;
	str.BinAppendHtmlTextU(pszText);
	str.BinAppendNullTerminator();
	CString s = str;
	s.replace(QRegularExpression("((?:https?|ftp)://\\S+)"), "<a href=\"\\1\">\\1</a>");
	str = s;
	BinAppendText(str);
	//BinAppendHtmlTextU(str);
	#endif
	}

void
CBin::BinAppendHtmlListItems(const CArrayPsz & arraypszListItems)
	{
	PSZUC * ppszStringStop;
	PSZUC * ppszString = arraypszListItems.PrgpszGetStringsStop(OUT &ppszStringStop);
	while (ppszString != ppszStringStop)
		BinAppendHtmlTextWithinTag("li", *ppszString++);	// Insert a list item "<li> ... </li>"
	}

void
CBin::BinAppendHtmlTextWithinTag(PSZAC pszTagHtml, PSZUC pszText)
	{
	Assert(pszTagHtml != NULL);
	Assert(pszTagHtml[0] != '\0');
	Assert(pszTagHtml[0] != '<');
	BinAppendText_VE("<$s>{sH}</$s>", pszTagHtml, pszText, pszTagHtml);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	BinAppendHtmlTextUntilPch()
//
//	Method to encode text into HTML.
//	This method does NOT append a virtual null-terminator.
//
//	This method is different than QString::toHtmlEscaped() because toHtmlEscaped() only encodes HTML metacharacters <, >, &, and " (without encoding the single quote ').
//	This method is somewhat similar as BinAppendXmlTextU() however with a special understanding of HTML:
//
//		"		&quot;
//		'		&#39;
//		&		&amp;
//		<		&lt;
//		>		&gt;
//		{CRLF}	<br />
//		{CR}	<br /> or ignored
//		{LF}	<br /> or ignored
//		TBD: {TAB}	&nbsp;&nbsp;&nbsp;&nbsp;
//		TBD: {SPACE}	&#32; if more than one consecutive space, or if the space is the first or last character.
//
void
CBin::BinAppendHtmlTextUntilPch(PCHUC pchTextBegin, PCHUC pchTextEnd)
	{
	Assert(pchTextBegin != NULL);
//	Assert(pchTextBegin <= pchTextEnd);
	Assert(!FIsPointerAddressWithinBinaryObject(pchTextBegin));
	Assert(!FIsPointerAddressWithinBinaryObject(pchTextEnd));
	// First check if there is a necessity to encode in HTML.
	CHS chu;
	const CHU * pchu = pchTextBegin;
	while (TRUE)
		{
		if (pchu >= pchTextEnd)
			{
			// We have reached the end of the text without any HTML entity or special white space.  This is the case of most short strings
			BinAppendBinaryData(pchTextBegin, pchu - pchTextBegin);
			return;
			}
		chu = *pchu;
		Assert(chu != '\0');
		switch (chu)
			{
		case '&':
		case '<':
		case '>':
		case '\"':	// Double quote
		case '\'':	// Single quote
		case '\n':
		case '\r':
			goto NeedEncoding;
		default:
			if (chu < ' ')
				goto NeedEncoding;
			} // switch
		pchu++;
		} // while

	NeedEncoding:
	(void)PbAllocateExtraMemory(pchTextEnd - pchTextBegin + 20);	// Allocate memory enough to hold the text and encode at least 3 HTML entities
	BinAppendBinaryData(IN pchTextBegin, pchu - pchTextBegin);		// Append the data which does not require any encoding
	Assert(m_paData != NULL);
	CHS chHtmlBr = '\0';	// Which character to use to insert the <br/>.  Only the '\n' or '\r' may be used, however not both

	while (TRUE)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		Assert(chu != '\0');
		switch (chu)
			{
		case '&':
			BinAppendBinaryData("&amp;", 5);
			break;
		case '<':
			BinAppendBinaryData("&lt;", 4);
			break;
		case '>':
			BinAppendBinaryData("&gt;", 4);
			break;
		case '\"':	// Double quote
			BinAppendBinaryData("&quot;", 6);
			break;
		case '\'':	// Single quote
			BinAppendBinaryData("&#39;", 5);	// This should be encoded as &apos; however it does not work with IE, so we encode it as a numeric value
			//BinAppendBinaryData("&apos;", 6);
			break;
		case '\r':
		case '\n':
			if (chu == chHtmlBr || chHtmlBr == '\0')
				{
				chHtmlBr = chu;
				BinAppendBinaryData(" <br/>", 6);	// Put a space before the <br/> so the RegEx will not include the <br/> in the URL
				}
			break;
		default:
			if (chu >= ' ')
				{
				// This is a normal character, so append it as usual
				const int cbData = m_paData->cbData;
				if (cbData < m_paData->cbAlloc)
					{
					m_paData->rgbData[cbData] = chu;
					m_paData->cbData = cbData + sizeof(BYTE);
					}
				else
					{
					BinAppendBinaryData(IN &chu, sizeof(BYTE));	// Re-allocate more memory
					}
				}
			else
				{
				// Any character less than $32 are encoded in a numeric value
				BYTE * pb = PbAllocateExtraMemory(6);
				*(CHW *)pb = UINT16_FROM_CHARS('&', '#');	// Append "&#"
				if (chu < 10)
					{
					pb[2] = '0' + chu;
					pb[3] = ';';
					m_paData->cbData += 4;
					}
				else
					{
					pb[2] = '0' + (chu / 10);
					pb[3] = '0' + (chu % 10);
					pb[4] = ';';
					m_paData->cbData += 5;
					}
				} // if...else
			} // switch

		chu = *++pchu;
		if (pchu >= pchTextEnd)
			return;
		} // while
	Assert(FALSE && "Unreachable code");
	} // BinAppendHtmlTextUntilPch()

void
CBin::BinAppendHtmlTextCch(PCHUC pchText, int cchuText)
	{
	BinAppendHtmlTextUntilPch(pchText, pchText + cchuText);
	}


void
CBin::BinAppendUrlPercentEncode(PSZUC pszUrl)
	{
	while (TRUE)
		{
		CHS ch = *pszUrl++;
		if (!Ch_FIsAlphaNumeric(ch))
			{
			switch (ch)
				{
			case '\0':
				return;
			case ' ':
				ch = '+';	// This is the special case where the space is encoded as the plus sign
				break;
			case '@':
			case '-':
			case '_':
			case '.':
			case '~':
				// Those characters do not need to be encoded in a URL
				break;
			default:
				BinAppendByte('%');
				BinAppendByte(c_rgchHexDigitsLowercase[ch >> 4]);
				BinAppendByte(c_rgchHexDigitsLowercase[ch & 0x0F]);
				continue;
				} // switch
			} // if
		BinAppendByte(ch);
		} // while
	} // BinAppendUrlPercentEncode()

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Super optimized method to encode an UTF-8 string to XML.  This method is the opposite of XmlDecode().
//	This method does NOT append a virtual null-terminator.
void
CBin::BinAppendXmlTextU(PSZUC pszText)
	{
	AssertValidUtf8(pszText);
	Assert(!FIsPointerAddressWithinBinaryObject(pszText));
	if (pszText == NULL)
		return; // Nothing to do
	if (pszText[0] == '\0')
		return;	// Nothing to do
	if (pszText[0] == d_chuXmlAlreadyEncoded)
		{
		// The content of pszuString is already XML encoded, so just copy it to the blob
		BinAppendText((PSZAC)pszText + 1);
		return;
		}
	// First check if there is a necessity to encode in XML.
	UINT chu;
	const CHU * pchu = pszText;
	while (TRUE)
		{
		chu = *pchu;
		switch (chu)
			{
		case '\0':
			// We have reached the end of the string without any XML entity.  This is the case of most simple and/or short strings
			BinAppendBinaryData(pszText, pchu - pszText);
			return;
		case '&':
		case '<':
		case '>':
		case '\"':	// Double quote
		case '\'':	// Single quote
			goto NeedEncoding;
			} // switch
		pchu++;
		} // while

	NeedEncoding:
	int cbAppend = pchu - pszText;
	(void)PbAllocateExtraMemory(cbAppend + CbAllocUtoU(pchu) + 20);	// Allocate memory enough to hold the string and encode at least 3 XML entities
	BinAppendBinaryData(IN pszText, cbAppend);	// Append the data which does not require any encoding
	Assert(m_paData != NULL);

	while (TRUE)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		switch (chu)
			{
		case '\0':
			return;
		case '&':
			BinAppendBinaryData("&amp;", 5);
			break;
		case '<':
			BinAppendBinaryData("&lt;", 4);
			break;
		case '>':
			BinAppendBinaryData("&gt;", 4);
			break;
		case '\"':	// Double quote
			BinAppendBinaryData("&quot;", 6);
			break;
		case '\'':	// Single quote
			BinAppendBinaryData("&#39;", 5);	// This should be encoded as &apos; however it does not work with IE, so we encode it as a numeric value
			//BinAppendBinaryData("&apos;", 6);
			break;
		default:
			const int cbData = m_paData->cbData;
			if (cbData < m_paData->cbAlloc)
				{
				m_paData->rgbData[cbData] = chu;
				m_paData->cbData = cbData + sizeof(BYTE);
				}
			else
				{
				BinAppendBinaryData(IN &chu, sizeof(BYTE));	// Re-allocate more memory
				}
			} // switch

		chu = *++pchu;
		} // while
	Assert(FALSE && "Unreachable code");
	} // BinAppendXmlTextU()

void
CBin::BinAppendXmlTextStr(const CStr & strText)
	{
	BinAppendXmlTextU(strText.PszuGetDataNZ());
	}

void
CBin::BinAppendXmlTextW(PSZWC pszwText)
	{
	if (pszwText == NULL || pszwText[0] == '\0')
		return;
	// First, convert from Unicode to UTF-8
	const int cbAlloc = CbAllocWtoU(pszwText);
	Assert(cbAlloc > 1);
	PSZU paszu = new CHU[cbAlloc];
	EncodeUnicodeToUtf8(OUT paszu, IN pszwText);	// This process could be optimized by skipping the XML encoding if the string does not contain any XML entities
	// Encode the UTF-8 into XML
	BinAppendXmlTextU(IN paszu);
	delete paszu;
	}

//	The binary object is NOT appended with a virtual null-terminator.
void
CBin::BinAppendXmlNode(const CXmlNode * pXmlNode)
	{
	if (pXmlNode != NULL)
		pXmlNode->SerializeToBinUtf8(IOUT this, CXmlNode::STBF_kfIndentTags | CXmlNode::STBF_kfCRLF | 1);
	}

void
CBin::BinAppendXmlNodeNoWhiteSpaces(const CXmlNode * pXmlNode)
	{
	if (pXmlNode != NULL)
		pXmlNode->SerializeToBinUtf8(IOUT this);
	}

PSZUC
CBin::BinAppendNullTerminatorSz()
	{
	BinAppendByte('\0');
	return m_paData->rgbData;
	}


PSZUC
CBin::BinAppendNullTerminatorVirtualAndEmptySzv()
	{
	BinAppendByte('\0');
	m_paData->cbData = 0;
	return m_paData->rgbData;
	}


BOOL
CBin::FCompareBinary(const CBin & binCompare) const
	{
	Assert(&binCompare != NULL);
	int cbData = 0;
	if (m_paData != NULL)
		cbData = m_paData->cbData;
	if (cbData != binCompare.CbGetData())
		return FALSE;	// The bin cannot be identical if they have different lengths
	return (0 == memcmp(m_paData->rgbData, binCompare.m_paData->rgbData, cbData));
	}

BOOL
CBin::FCompareBinary(const QByteArray & arraybCompare) const
	{
	const int cbDataCompare = arraybCompare.size();
	if (m_paData != NULL)
		{
		if (cbDataCompare != m_paData->cbData)
			return FALSE;
		return (0 == memcmp(m_paData->rgbData, arraybCompare.data(), cbDataCompare));
		}
	return (cbDataCompare == 0);
	}

void
CBin::BinInitFromCBin(const CBin * pbinSrc)
	{
	Assert(pbinSrc != NULL);
	Assert(pbinSrc != this);
	int cbSrc = pbinSrc->CbGetData();
	if (cbSrc > 0)
		{
		BinInitFromBinaryData(pbinSrc->m_paData->rgbData, cbSrc);
		}
	else
		{
		if (m_paData != NULL)
			m_paData->cbData = 0;
		}
	}

//	Same as Copy() however make sure the binary data is null-terminated
void
CBin::BinInitFromCBinWithVirtualNullTerminator(const CBin * pbinSrc)
	{
	Assert(pbinSrc != NULL);
	Assert(pbinSrc != this);

	SHeaderWithData * pDataSrc = pbinSrc->m_paData;
	if (pDataSrc != NULL)
		{
		const int cbData = pDataSrc->cbData;
		BYTE * pbDst = (BYTE *)PvAllocateMemoryAndEmpty(cbData + sizeof(BYTE));	// Allocate memory for the data and the null-terminator
		memcpy(OUT pbDst, IN pDataSrc->rgbData, cbData);
		pbDst[cbData] = '\0';		// Append the null-terminator
		m_paData->cbData = cbData;
		}
	else
		{
		BinAppendNullTerminatorVirtualSzv();	// Make sure the bin has a null-terminator
		}
	} // BinInitFromCBinWithVirtualNullTerminator()

//	Steal copy the content of the other binary
void
CBin::BinInitFromCBinStolen(INOUT CBin * pbinSrc)
	{
	Assert(pbinSrc != NULL);
	Assert(pbinSrc != this);
	ValidateHeapPointer(m_paData);
	delete m_paData;
	m_paData = pbinSrc->m_paData;
	pbinSrc->m_paData = NULL;
	}

//	Swap the content of 'this' and pbinSrc
VOID
CBin::BinInitFromCBinSwapped(INOUT CBin * pbinSrc)
	{
	Assert(pbinSrc != NULL);
	Assert(pbinSrc != this);
	SHeaderWithData * paDataTemp = m_paData;
	m_paData = pbinSrc->m_paData;
	pbinSrc->m_paData = paDataTemp;
	}

void
CBin::BinInitFromByteArray(const QByteArray & arraybData)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BinAppendBinaryData(arraybData.data(), arraybData.size() + 1);
	m_paData->cbData--;	// Remove the null-terminator
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Assignment operator.
//
//	INTERFACE NOTES
//	This operator is implemented for convenience to allow the copying of
//	a class/structure using the = operator.
//	This operator is used whatever a class contains inherits CBin).
//
void
CBin::operator = (const CBin & binSrc)
	{
	Assert(&binSrc != this);
	BinInitFromCBin(IN &binSrc);
	}
/*
CBin::operator QByteArray() const
	{
	if (m_paData != NULL)
		return QByteArray((const char *)m_paData->rgbData, m_paData->cbData);	// Make a deep copy of the array. DO NOT USE fromRawData() as it does NOT copy the data.
	return QByteArray();	// Return an empty QByteArray
	}
*/

QByteArray
CBin::ToQByteArrayDeepCopy() const
	{
	if (m_paData != NULL)
		return QByteArray((const char *)m_paData->rgbData, m_paData->cbData);	// Make a deep copy of the array. DO NOT USE fromRawData() as it does NOT copy the data.
	return c_arraybEmpty;
	}

QByteArray
CBin::ToQByteArrayShared() const
	{
	if (m_paData != NULL)
		return QByteArray::fromRawData((const char *)m_paData->rgbData, m_paData->cbData);
	return c_arraybEmpty;
	}


//	Method to convert a CBin to a QString
QString
CBin::ToQString() const
	{
	if (m_paData != NULL)
		{
		const int cchUtf8 = m_paData->cbData;
		if (cchUtf8 > 0)
			return QString::fromUtf8((const char *)m_paData->rgbData, cchUtf8);
		}
	return c_sEmpty;
	}

void
CBin::Empty()
	{
	if (m_paData != NULL)
		{
		Assert(m_paData->cbAlloc > 0);
		InitToGarbage(OUT m_paData->rgbData, m_paData->cbAlloc);
		m_paData->cbData = 0;
		}
	#ifdef _ALLOCATE_EXACT_BINARY_SIZE
	FreeExtra();
	#endif
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	FreeExtra()
//
//	This member function free any extra memory previously allocated by
//	the bin but no longer needed.
//
//	NYI: The function reallocates the buffer to the exact length returned by CbGetData().
//
void
CBin::FreeExtra()
	{
	if (m_paData == NULL)
		return;
	if (m_paData->cbData == 0)
		{
		ValidateHeapPointer(m_paData);
		delete m_paData;
		m_paData = NULL;
		}
	}

VOID
CBin::FreeBuffer()
	{
	ValidateHeapPointer(m_paData);
	delete m_paData;
	m_paData = NULL;
	}

#ifdef DEBUG
//	Initialize the remaining bytes to garbage, to prevent
VOID
CBin::GarbageInitializedUnusedBytes() const
	{
	if (m_paData != NULL)
		{
		int ibStart = m_paData->cbData;
		Assert(ibStart >= 0);
		Assert(ibStart <= m_paData->cbAlloc);
		InitToGarbage(m_paData->rgbData + ibStart, m_paData->cbAlloc - ibStart);
		}
	}
#endif // DEBUG


//	See also PszFindString()
PCHU
CBin::PchFindCharacter(UINT ch) const
	{
	if (m_paData != NULL)
		{
		const BYTE * pbStart = m_paData->rgbData;
		const BYTE * pbStop = pbStart + m_paData->cbData;
		while (pbStart != pbStop)
			{
			if (*pbStart == ch)
				return (CHU *)pbStart;
			pbStart++;
			}
		}
	return NULL;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Read the content of a file and store it into the bin.
//
//	Return errSuccess if the entire content of the file was read and stored in the bin.
//	Return errFileOpenError if the file could not be opened, typically because the file cannot be located on disk (file not found).
//	Return errFilePermissionsError if unable to read file because of security permissions (or maybe the file is already opened by another process).
//	Return errFileReadError if an error occurred while reading from the file.
//
//	INTERFACE NOTES
//	The bin is ALWAYS appended by a unicode null-terminator, but these extra bytes are not accounted into CbGetData().
//	In other words, the value returned by CbGetData() is the EXACT file size on disk.
//
EError
CBin::BinFileReadE(const QString & sFileName)
	{
	Assert(sFileName != NULL);
	Empty();	// Flush previous content (if any)
	QFileDevice::FileError eFileError;
	QFile oFile(sFileName);
	if (oFile.open(QIODevice::ReadOnly))
		{
		UINT cbFileLength = oFile.size();
		char * prgbBuffer = (char *)PvAllocateMemoryAndEmpty(cbFileLength + sizeof(CHW));	// Allocate room for a unicode null-terminator
		UINT cbDataRead = oFile.read(OUT prgbBuffer, cbFileLength);
		eFileError = oFile.error();	// Get the file error (if any)
		oFile.close();	// Close the file
		if (eFileError == QFileDevice::NoError && cbDataRead == cbFileLength)
			{
			*(CHW *)(prgbBuffer + cbFileLength) = '\0';	// Append the unicode null-terminator
			m_paData->cbData = cbFileLength;
			return errSuccess;
			}
		}
	else
		{
		eFileError = oFile.error();	// Get the file error
		}
	Assert(CbGetData() == 0);	// The content of the bin should be empty
	switch (eFileError)
		{
	case QFileDevice::OpenError:
		return errFileOpenError;
	case QFileDevice::PermissionsError:
		return errFilePermissionsError;
	case QFileDevice::NoError:
		Assert(FALSE && "Inconsistent result");
	default:
		return errFileReadError;	// Return a generic error code indicating the file could not be read
		} // switch
	} // BinFileReadE()

//	Write the content of the bin to a file.
EError
CBin::BinFileWriteE(const QString & sFileName, QIODevice::OpenModeFlag uFlagsExtra) const
	{
	EError err = errFileWriteError;

	QFile oFile(sFileName);
	if (oFile.open(QIODevice::WriteOnly | uFlagsExtra))
		{
		err = errSuccess;
		if (m_paData != NULL)
			{
			if (oFile.write((const char *)m_paData->rgbData, m_paData->cbData) != m_paData->cbData)
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to write all the data to file $Q.\n", &sFileName);
				err = errFileWriteError;
				}
			}
		if (!oFile.flush())
			err = errFileWriteError;
		oFile.close();
		}
	return err;
	} // BinFileWriteE()


#define d_chTemplateColor				'c'
#define d_chTemplateByte				'b'

//	The source characters are used by method BinAppendTextSzv_VL() when using { } for an argument.
#define d_chSourcePCSZU_copy_until_ch	'c'	// Copy until the character specified as the 'encoding'.  Examples: {c@} means copy until the '@', {c/} means copy until the '/'.  This is a good way to extract a usename from an email address or strip the resource from a JID.
#define d_chSourcePCStr_copy_until_ch	'C'	// Same as above, except the source string is a CStr*
//	'u' 'U'			'a'						'A' // Copy after the character
#define d_chSourcePCSZW					'w'
#define d_chSourcePCSZU					's'
#define d_chSourcePCStr					'S'
#define d_chSourcePCBin					'B'	// This is very similar to CStr* however different, since CBin returns the size of the binary data rather than the length of the string.
#define d_chSourcePCBinOffset			'o'	// 32-bit offset of data within a CBin*.  This field has 3 parameters: CBin*, ibDataOffset, cbDataMax
#define d_chSourcePQString				'Q'
#define d_chSourcePQByteArray			'Y'
#define d_chSourceHashGuid				'g'	// GUID, MD5 or any 128-bit value
#define d_chSourceHash160				'h'	// SHA-1 or RIPEMD-160
#define d_chSourceHash256				'H'	// Any 256-bit value, such as SHA-256
#define d_chSourcePvCb					'p'	// A pointer and byte count representing a binary value.  This one contains two arguments: a pointer pvData followed by cbData
#define d_chSourcePvPv					'P'	// Two pointers (one is the beginning and the other is the end).  The nummber of bytes representing the source is the difference between the pointers.
#define d_chSourcePCXmlNode				'N'	// Serialize the content of a CXmlNode* (at the moment only {Nf} is allowed)
#define d_chSourcePCXmlNodeAttribute	'A' // Interpret the content of CXmlNode* as an attribute (at the moment, only ^A is allowed)
#define d_chSourcePCArrayPsz			'L' // Encode the content of a CArrayPsz* into an HTML list items (example: "<ul><li>example.com</li><li>cambrian.org</li></ul>")
#define d_chSourceContactIdentifier		'i'
#define d_chSourceJidBare				'j'
#define d_chSourceJidFull				'J'
#define d_chSourcePQDateTime			'D'	// Could be removed, not really used
#define d_chSourceTIMESTAMP				't'	// {t_} {tL} {tU}
#define d_chSourceTIMESTAMP_DELTA		'T'	// {T-} {T_}
#define d_chSourceAmount				'A' // {AB} {Am} {A$} {A*}
#define d_chSourceKiB_32				'k'	// {kT} {kK}
#define d_chSourceKiB_64				'K'


// d = QDate
// D = QDateTime
// t = QTime
// T = QDateTime now
// U = Encode in UTC

//	The encoding characters are used by methods BinAppendTextSzv_VL() and BinAppendDataEncoded().
#define d_chEncodingNone				'n'	// No encoding - just copy the source directly into the bin.  {Bn} is the same as $B
#define d_chEncodingHexadecimal			'f'	// Encode the source in Hexadecimal (Base16)
//#define d_chEncodingHexadecimalReversed	'F'	// This may be never used because Base85 is superior
#define d_chEncodingBase41				'!'	// Encode the source in Base41 (the last character of Base41 is the character !).  Base41 is useful to encode hashes into filenames.
#define d_chEncodingBase58				'z' // Encode the source in Base58 (the last character of Base58 is the letter z)
#define d_chEncodingBase64				'/'	// Encode the source in Base64 (typical values are {B/}, {S/}, {s/} and {p/}
#define d_chEncodingBase64Url			'_'	// Encode the source in Base64url (RFC 4648)
#define d_chEncodingBase85				'|'	// Encode the source in Base85 (one of the last character of Base85 is |)
#define d_chEncodingHtml				'H'	// Encode the source string in HTML.  The source MUST be a null-terminated string.  Since method BinAppendDataEncoded() ignores the parameter cbData, the valid options are {sH}, {SH}, {BH} and {YH}.
#define d_chEncodingXmlAttributeCh		'a' // Encode the source string as an XML attribute (of course if the value is not empty).  Valid options {Sa}, {sa}
#define d_chEncodingXmlAttributePsz		'A'
#define d_chEncodingCharacterSingle		'1'	// Encode (add) a single Unicode character (this is useful to report an invalid character when parsing data).  Typically {s1} is supported
#define d_chEncodingFingerprint			'G'	// Encode the CBin* into a fingerprint.  At the moment, only {hG} and {BG} are valid.
#define d_chEncodingPercentURL			'%'	// Encode a URL with the % escape (it is called percent encode)
#define d_chEncodingKiB					'K'	// Show the value in Bytes or KiB
#define d_chEncodingKiB_Percent			'%'	// Show the percentage (%) by dividing two values (this is useful to show the progress of a download)
#define d_chEncodingKiB_PercentOfTotal	'T'	// Show the the percentage (%) as well as the total

#define d_chEncodingQDateTimeLocal		'L'	// Display a TIMESTAMP in the local date and time	{tL}
#define d_chEncodingQDateTimeUTC		'U'	// Display a TIMESTAMP in the UTC date and time		{tU}

void
CBin::BinAppendDataEncoded(const void * pvData, int cbData, UINT chEncoding)
	{
	switch (chEncoding)
		{
	case d_chEncodingNone:
		BinAppendBinaryData(pvData, cbData);
		return;
	case d_chEncodingHexadecimal:
		BinAppendStringBase16FromBinaryData(pvData, cbData);
		return;
	case d_chEncodingBase41:
		BinAppendStringBase41FromBinaryData(pvData, cbData);
		return;
	case d_chEncodingBase64:
		BinAppendStringBase64FromBinaryData(pvData, cbData);
		return;
	case d_chEncodingBase85:
		BinAppendStringBase85FromBinaryData(pvData, cbData);
		return;
	case d_chEncodingCharacterSingle:	// Typically {s1}
		BinAppendBinaryDataPvPv(pvData, PchroNextCharacterUtf8((PSZUC)pvData));
		return;
	case d_chEncodingFingerprint:
		BinAppendTextFormattedFromFingerprintBinary(pvData, cbData);
		return;
	case d_chEncodingHtml:
		BinAppendHtmlTextCch((PCHUC)pvData, cbData);
		return;
	case d_chEncodingPercentURL:
		if (cbData > 0)
			{
			Assert((int)strlenU(pvData) == cbData);
			(void)PvAllocateMemoryToGrowBy(cbData + 32);	// Since we already know the length of the string, use it to allocate enough memory to hold the string and encode a few extra entities
			BinAppendUrlPercentEncode((PSZUC)pvData);
			}
		return;
		} // switch
	Assert(FALSE && "Invalid Encoding!");
	} // BinAppendDataEncoded()

//	Similar as BinAppendText_VE() returning the beginning offset
//	This offset is useful to truncate
void
CBin::BinAppendTextOffsets_VE(OUT SOffsets * pOffsets, PSZAC pszFmtTemplate, ...)
	{
	Assert(pOffsets != NULL);
	pOffsets->ibReset = (m_paData != NULL) ? m_paData->cbData : 0;
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	pOffsets->ibDataBegins = m_paData->cbData;
	}

void
CBin::BinAppendText_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	}

union _UnionForBinAppendTextSzv_VL	// Private union.  This union is defined outside BinAppendTextSzv_VL() because Qt Creator' auto-complete does not handle the declaration inside of a method
	{
	PSZUC pszuString;
	PSZWC pszwString;
	const CStr * pstr;
	const CBin * pbin;
	const QString * psString;
	const QByteArray * parrayb;
	const CArrayPsz * parraypsz;
	const ITreeItem * piTreeItem;
	const TContact * pContact;
	const TAccountXmpp * pAccount;
	const CXmlNode * pXmlNode;
	QDateTime * pdtuDateTime;
	UINT uValue;
	UINT bValue;
	BOOL fValue;
	};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	BinAppendTextSzv_VL()
//
//	Append formatted text to the blob.
//
//	The binary object is ALWAYS appended with a null-terminator, but this null-terminator is not included in the length/size of the binary object.
//	Return pointer to the BEGINNING of the blob, which is a null-terminated string.
//
//	FORMATS SUPPORTED
//	This method supports rudimentary formats
//		$s - Append a UTF-8 string
//		$S - Append the content of CStr*
//		$B - Append the content of CBin* (the CBin may NOT be null-terminated, and/or may contain binary characters including several null-terminators).  This is the same as {Bn}
//		$w - Append a Unicode string (wide char) to the bin, converting UTF-16 to UTF-8
//		$Q - Append the content of QString* to UTF-8
//		$Y - Append the content of QByteArray* to UTF-8
//		$U - Append a USZU or USZUF (which is a string within a 32-bit value)
//		$n - The current time, $N the current date and time
//		$t - Append a TIMESTAMP in Base85.  Use {tL} append a date in local time, and {tU} to append a date in UTC.
//		$T - Append a TIMESTAMP_DELTA in seconds, minutes, hours or days.
//		$@ - Append the current time in hh:mm::ss (useful for debugging a log).  This format does not have any input parameter.
//		$x - Append an unsigned integer in hexadecimal format (without any leading zeroes).  For an hexadecimal with leading zeroes, use $p.
//		$u - Append an unsigned integer
//		$i - Append an integer
//		$I - Append an decimal integer with thousand separators
//		$l - Append a 64-bit integer without thousand separators
//		$L - Append a 64-bit integer with thousand separators
//		$p - Append a pointer in hexadecimal - the equivalent of sprintf("%08X")
//		$c - Append a color in HTML (such as #ff0000 for red)
//		$g - Format a float/double with three decimal points
//		$f - Format a float/double with one decimal point
//		$b - Append a byte if its value is non-zero.
//		$Z - FALSE => Continue/skip.  TRUE => Terminate the string and stop any further formatting
//		$z - FALSE => Terminate the string and stop any further formatting (opposite of $Z)
//		^  - Encode to XML
//		^s - Append/encode to XML the content of a UTF-8 string
//		^S - Append/encode to XML the content of CStr*
//		^w - Append/encode to XML the content of Unicode string (wide char)
//		^Q - Append/encode to XML the content of QString*
//		^Y - Append/encode to XML the content of QByteArray* (the QByteArray is always null-terminated)
//		^N - Append/encode to XML the content of CXmlNode*
//		^A - Append/incode an XML attribute of a CXmlNode*
//		^L - Append an HTML list items from a CArrayPsz*
//		^Lo
//		^:		Append an attribute named "xmlns".  Of course this may appear as pollution in this routine, however those xmlns attributes are everywhere
//		^:#s	Append an attribute named "xmlns" with the string value from the stack
//		^:ft ^:fn ^:si ^:ib
//		^*ft ^*fn ^*si ^*ib - Just append the attribute value without the attribute name "xmlns="
//		^j - Append the bare JID of a TAccountXmpp* or TContact*
//		^J - Append the full JID of a TAccountXmpp* or TContact*
//
//		{SE} - S is the source of the data, and E is the encoding of the data
//		{S } - The source is the content of CStr*
//		{B } - The source is the content of CBin*
//		{ f} - Encode in Hexadecimal (Base16)
//		{ /} - Encode in Base64
//		{ _} - Encode in Base64url
//
PSZU
CBin::BinAppendTextSzv_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	Assert(pszFmtTemplate != NULL);
	Assert(!FIsPointerAddressWithinBinaryObject(pszFmtTemplate));
	(void)PbAllocateExtraMemory(CbAllocUtoU((PSZUC)pszFmtTemplate) + _cbAllocSizeExtraForBinAppendText);	// Pre-allocate memory for the template.  This ensures m_paData is never NULL and also optimizes the destination buffer by reducing the number of memory allocations
	Assert(m_paData != NULL);
	_UnionForBinAppendTextSzv_VL u;

	while (TRUE)
		{
		Assert(m_paData->cbData <= m_paData->cbAlloc);
		const UINT chTemplate = *pszFmtTemplate++;
		switch (chTemplate)
			{
		default:
			{
			AppendCharacter:
			const int cbData = m_paData->cbData;
			if (cbData < m_paData->cbAlloc)
				{
				m_paData->rgbData[cbData] = chTemplate;
				m_paData->cbData = cbData + sizeof(BYTE);
				}
			else
				{
				BinAppendBinaryData(IN &chTemplate, sizeof(BYTE));	// Re-allocate more memory
				}
			}
			break;

		case '\0':
			goto TerminateString;

		case '{':	// {Source and Encoding}
			if (pszFmtTemplate[2] == '}')
				{
				const UINT chSource = pszFmtTemplate[0];
				const UINT chEncoding = pszFmtTemplate[1];
				switch (chSource)
					{
				case d_chSourcePCStr_copy_until_ch:
					u.pstr = va_arg(vlArgs, CStr *);
					Assert(u.pstr != NULL);
					u.pszuString = u.pstr->PszuGetDataNZ();
					goto IncludeStringUntilCharacter;
				case d_chSourcePCSZU_copy_until_ch:
					u.pszuString = va_arg(vlArgs, PSZUC);
					IncludeStringUntilCharacter:
					if (u.pszuString != NULL)
						BinAppendTextUntilCharacterPszr(u.pszuString, chEncoding);
					break;
				case d_chSourcePCSZU:
					u.pszuString = va_arg(vlArgs, PSZUC);
					if (u.pszuString != NULL)
						BinAppendDataEncoded(u.pszuString, strlenU(u.pszuString), chEncoding);
					break;
				case d_chSourcePCStr:
					u.pstr = va_arg(vlArgs, CStr *);
					if (u.pstr != NULL)
						BinAppendDataEncoded(u.pstr->PvGetData(), u.pstr->CchGetLength(), chEncoding);
					break;
				case d_chSourcePCBin:
					u.pbin = va_arg(vlArgs, CBin *);
					if (u.pbin != NULL)
						BinAppendDataEncoded(u.pbin->PvGetData(), u.pbin->CbGetData(), chEncoding);
					break;
				case d_chSourcePCBinOffset:
					{
					// Fetch the 3 arguments from the stack
					u.pbin = va_arg(vlArgs, CBin *);
					const int ibDataOffset = va_arg(vlArgs, int);
					const int cbDataMax = va_arg(vlArgs, int);
					Assert(u.pbin != NULL);
					Assert(ibDataOffset >= 0);
					Assert(cbDataMax > 0);
					if (u.pbin->m_paData != NULL)
						{
						const int cbDataAvailable = u.pbin->m_paData->cbData - ibDataOffset;
						if (cbDataAvailable > 0)
							BinAppendDataEncoded(u.pbin->m_paData->rgbData + ibDataOffset, (cbDataAvailable < cbDataMax) ? cbDataAvailable : cbDataMax, chEncoding);
						}
					}
					break;
				case d_chSourcePQByteArray:
					u.parrayb = va_arg(vlArgs, QByteArray *);
					if (u.parrayb != NULL)
						BinAppendDataEncoded(u.parrayb->constData(), u.parrayb->size(), chEncoding);
					break;
				case d_chSourceHashGuid:
					Assert(sizeof(GUID) == sizeof(SHashMd5));
					BinAppendDataEncoded(va_arg(vlArgs, SHashMd5 *), sizeof(SHashMd5), chEncoding);
					break;
				case d_chSourceHash160:
					BinAppendDataEncoded(va_arg(vlArgs, SHashSha1 *), sizeof(SHashSha1), chEncoding);
					break;
				case d_chSourcePvCb:
				case d_chSourcePvPv:
					{
					Assert(sizeof(PCVOID) == sizeof(INT_P));	// The argument PvCb should use the same stack space as PvPv
					PCVOID pvSourceDataStart = va_arg(vlArgs, PCVOID);
					INT_P cbSourceData =  va_arg(vlArgs, INT_P);
					if (chSource == d_chSourcePvPv)
						cbSourceData -= (INT_P)pvSourceDataStart;		// The destination is a pointer, so the cbSourceData is the difference between pvSourceDataStart and pvSourceDataStop
					Report(cbSourceData >= 0 && "Number of bytes should be positive");	// If the number of bytes is negative, it is not necessary a bug, however should draw the attention of the developer
					BinAppendDataEncoded(pvSourceDataStart, cbSourceData, chEncoding);
					}
					break;
				case d_chSourcePCXmlNode:
					{
					Assert(chEncoding == 'f');
					// For performance reasons, serialize the XML node in the blob, and then calculate the MD5 value
					int cbPrevious = m_paData->cbData;
					BinAppendXmlNode(va_arg(vlArgs, CXmlNode *));
					SHashMd5 md5;
					HashMd5_CalculateFromBinary(OUT &md5, m_paData->rgbData + cbPrevious, m_paData->cbData - cbPrevious);
					m_paData->cbData = cbPrevious;	// Flush (truncate) the content of the XML node to restore it to its original state
					BinAppendStringBase16FromBinaryData(IN &md5, sizeof(md5));
					}
					break;
				case d_chSourcePQDateTime: // {DU}
					Assert(chEncoding == 'U');
					u.pdtuDateTime = va_arg(vlArgs, QDateTime *);
					if (u.pdtuDateTime != NULL)
						BinUtf8AppendStringQ(IN u.pdtuDateTime->toString(Qt::ISODate));
					break;
				case d_chSourceTIMESTAMP:
					{
					TIMESTAMP ts = va_arg(vlArgs, TIMESTAMP);
					if (chEncoding == d_chEncodingBase64Url)			// {t_}
						{
						m_paData->cbData += Timestamp_CchEncodeToBase64Url(ts, OUT PbAllocateExtraMemory(16));
						break;
						}
					Assert(chEncoding == d_chEncodingQDateTimeLocal);	// {tL}
					QDateTime date = QDateTime::fromMSecsSinceEpoch(ts);
					BinUtf8AppendStringQ(date.toString(Qt::DefaultLocaleShortDate));
					}
					break;
				case d_chSourceAmount:
					m_paData->cbData += Amount_CchFormat(OUT PbAllocateExtraMemory(64), va_arg(vlArgs, AMOUNT), chEncoding);
					break;
				case d_chSourceKiB_32:
				case d_chSourceKiB_64:
					Assert(chEncoding == d_chEncodingKiB || chEncoding == d_chEncodingKiB_Percent || chEncoding == d_chEncodingKiB_PercentOfTotal);
					{
					u.fValue = (chSource == d_chSourceKiB_32);
					L64 cblValue = u.fValue ? va_arg(vlArgs, int) : va_arg(vlArgs, L64);
					if (chEncoding == d_chEncodingKiB)
						BinAppendTextBytesKiB(cblValue);		// {KK} {kK}
					else
						{
						L64 cblTotal = u.fValue ? va_arg(vlArgs, int) : va_arg(vlArgs, L64);
						if (chSource == d_chEncodingKiB_Percent)
							BinAppendTextBytesKiBPercent(cblValue, cblTotal);	// {K%} {k%}
						else
							BinAppendTextBytesKiBPercentProgress(cblValue, cblTotal);	// {KT} {kT}
						}
					}
					break;
				default:
					Assert(FALSE && "Unknown source template character for { }");
					goto AppendCharacter;
					} // switch (chSource)
				pszFmtTemplate += 3;	// Skip the Source, Encoding and closing curly brace }
				break;
				}
			else
				{
				// This case is somewhat 'normal' as two braces may be displayed.  As long as they are not within two spaces.
				//MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "CBin::BinAppendTextSzv_VL() - Incorrect syntax for {  } starting at: $s\n", pszFmtTemplate - 1);
				goto AppendCharacter;
				}

		case '^':  // Encode to XML
			{
			const UINT chFormat = *pszFmtTemplate++;	// Get the second character
			switch (chFormat)
				{
			case d_chSourcePCSZU:	// ^s - Append/encode to XML the content of a UTF-8 string
				BinAppendXmlTextU(va_arg(vlArgs, PSZUC));
				break;
			case d_chSourcePCStr:	// ^S - Append/encode to XML the content of CStr*
				u.pstr = va_arg(vlArgs, CStr *);
				if (u.pstr != NULL)
					BinAppendXmlTextStr(*u.pstr);
				break;
			case d_chSourcePCSZW:	// ^w - Append/encode to XML the content of Unicode string (wide char)
				BinAppendXmlTextW(va_arg(vlArgs, PSZWC));
				break;
			case d_chSourcePQString:	// ^Q - Append/encode to XML the content of QString*
				u.psString = va_arg(vlArgs, QString *);
				if (u.psString != NULL)
					BinAppendXmlTextW(u.psString->utf16());
				break;
			case d_chSourcePQByteArray:	// ^Y - Append/encode to XML the content of QByteArray*
				u.parrayb = va_arg(vlArgs, QByteArray *);
				if (u.parrayb != NULL)
					BinAppendXmlTextU((PSZUC)u.parrayb->constData());
				break;
			case d_chSourcePCXmlNode:	// ^N - Append/encode to XML the content of CXmlNode*
				BinAppendXmlNode(va_arg(vlArgs, CXmlNode *));
				break;
			case d_chSourcePCXmlNodeAttribute:	// ^A
				u.pXmlNode = va_arg(vlArgs, CXmlNode *);
				if (u.pXmlNode != NULL)
					BinAppendXmlAttributeText((PSZAC)u.pXmlNode->m_pszuTagName, u.pXmlNode->m_pszuTagValue);
				break;
			case d_chSourcePCArrayPsz:	// ^L
				u.parraypsz = va_arg(vlArgs, CArrayPsz *);
				if (u.parraypsz != NULL)
					{
					BinAppendText("<div style='margin:-25px;'><ul>");	// By default Qt indents the list items by about 40 pixels.  By using a negative margin of 30 pixels, we can display the text indented about 5 pixels.
					//BinAppendText("<div style='margin-left:-25px;'><ul>");
					//BinAppendText("<div style='text-indent:-25px;'><ul>");	//This indents only the first list item
					BinAppendHtmlListItems(IN *u.parraypsz);
					BinAppendText("</ul></div>");
					}
				break;
			case d_chSourceContactIdentifier: // ^i
				u.pContact = va_arg(vlArgs, TContact *);
				if (u.pContact != NULL)
					{
					Assert(u.pContact->EGetRuntimeClass() == RTI(TContact));
					BinAppendXmlTextStr(u.pContact->m_strJidBare);
					}
				break;
			case d_chSourceJidBare:		// ^j
			case d_chSourceJidFull:		// ^J
				u.piTreeItem = va_arg(vlArgs, ITreeItem *);
				Assert(PGetRuntimeInterfaceOf_ITreeItem(u.piTreeItem) == u.piTreeItem);
				if (u.piTreeItem != NULL)
					{
					if (u.piTreeItem->EGetRuntimeClass() == RTI(TAccountXmpp))
						{
						TAccountXmpp * pAccount = (TAccountXmpp *)u.piTreeItem;
						Assert(!pAccount->m_strJID.FIsEmptyString());
						Assert(!pAccount->m_strJIDwithResource.FIsEmptyString());
						BinAppendXmlTextStr((chFormat == d_chSourceJidFull) ? pAccount->m_strJIDwithResource : pAccount->m_strJID);
						}
					else if (u.piTreeItem->EGetRuntimeClass() == RTI(TContact))
						{
						TContact * pContact = (TContact *)u.piTreeItem;
						Assert(!pContact->m_strJidBare.FIsEmptyString());
						BinAppendXmlTextStr(pContact->m_strJidBare);
						if (chFormat == d_chSourceJidFull)
							BinAppendXmlTextStr(pContact->m_strRessource);
						}
					else
						{
						Report(FALSE && "Invalid runtime class for ^J");
						}
					}
				break;
			case '*':	// ^*
				BinAppendXmlTextU((PSZUC)PszGetXmlnsFromWord(*(WORD *)pszFmtTemplate));
				pszFmtTemplate += 2;
				break;
			case ':':	// ^:
				{
				const UINT wXmlns = *(WORD *)pszFmtTemplate;
				if (wXmlns != d_wXmlnsPCSZU)
					u.pszuString = (PSZUC)PszGetXmlnsFromWord(wXmlns);
				else
					u.pszuString = va_arg(vlArgs, PSZUC);
				BinAppendXmlAttributeText(c_sza_xmlns, u.pszuString);
				pszFmtTemplate += 2;
				}
				break;
			case '~':	// ^~
			case '_':	// ^_
				BinAppendUInt16(d_w_nbsp);	// Insert an &nbsp; when formatting HTML.  There are two options (^~ and ^_) as I have not decided which one I prefer more.
				break;
			case '$':	// ^$ to insert the $ in the string
				BinAppendByte('$');
				break;
			default:
				Assert(FALSE && "Unknown template character for ^");
				// Just put back the sequence to the output string
				BinAppendByte('^');
				BinAppendByte(chFormat);
				} // switch
			} // case '^'
			break;

		case '$':
			UINT chFormat = *pszFmtTemplate++;	// Get the second character
			switch (chFormat)
				{
			case d_chSourcePCSZU:	// $s
				u.pszuString = va_arg(vlArgs, PSZUC);
				if (u.pszuString != NULL)
					BinAppendText((PSZAC)u.pszuString);
				break;
			case d_chSourcePCStr:	// $S
				u.pstr = va_arg(vlArgs, CStr *);
				if (u.pstr != NULL)
					BinAppendCStr(*u.pstr);
				break;
			case d_chSourcePCBin:	// $B
				u.pbin = va_arg(vlArgs, CBin *);
				if (u.pbin != NULL)
					BinAppendBinaryData(u.pbin->PvGetData(), u.pbin->CbGetData());
				break;
			case d_chSourcePCSZW:
				BinUtf8AppendStringW(va_arg(vlArgs, PSZWC));
				break;
			case d_chSourcePQString:	// $Q
				BinUtf8AppendStringQ(*va_arg(vlArgs, QString *));
				break;
			case d_chSourcePQByteArray:
				u.parrayb = va_arg(vlArgs, QByteArray *);
				if (u.parrayb != NULL)
					BinAppendText((PSZAC)u.parrayb->constData());
				break;
			case d_chTemplateByte:
				u.bValue = va_arg(vlArgs, UINT);	// A byte is pushed on the stack as 32 bits, so we have to extract it as a UINT
				if (u.bValue != 0)
					BinAppendByte(u.bValue);
				break;
			case 'u':	// $u - Unsigned integer
			case 'i':	// $i
			case 'I':	// $I
				BinAppendTextInteger(va_arg(vlArgs, int), (chFormat == 'i') ? d_zNA : (chFormat == 'I') ? ITS_mskfThousandSeparator : ITS_mskfUnsigned);
				break;
			case 'l':	// $l - 64-bit integer without thousand separators
			case 'L':	// $L
				{
				L64 llValue = va_arg(vlArgs, L64);
				CHU szValue[32];
				Integer64ToString(OUT szValue, llValue, (chFormat == 'L') ? ITS_mskfThousandSeparator : d_zNA);
				BinAppendText((PSZAC)szValue);
				}
				break;
			case 'x':	// $x - hexadecimal value without any leading zeroes
			case 'p':	// $p - Pointer in hexadecimal
				BinAppendTextInteger(va_arg(vlArgs, UINT), (chFormat == 'x') ? ITS_mskfHexadecimal : ITS_mskfHexadecimal | 8);
				break;
			case 'U':	// $U
				u.uValue = USZU_from_USZUF(va_arg(vlArgs, USZU));	// Just in case, remove the flags from the USZU
				BinAppendText((PSZAC)PszFromUSZU(u.uValue));
				break;
			case d_chTemplateColor:	// $c
				BinAppendByte('#');
				BinAppendTextInteger(va_arg(vlArgs, UINT), ITS_mskfHexadecimal | 6);
				break;
			case d_chSourceTIMESTAMP: // $t
				BinAppendTimestamp(va_arg(vlArgs, TIMESTAMP));
				break;
			case d_chSourceTIMESTAMP_DELTA:	// $T
				#if 1
				m_paData->cbData += TimestampDelta_CchToString(va_arg(vlArgs, TIMESTAMP_DELTA), OUT PbAllocateExtraMemory(32));
				#else
				int TimestampDelta_CchToStringLongText(TIMESTAMP_DELTA dts, OUT CHU pszTimestampDelta[64]);
				m_paData->cbData += TimestampDelta_CchToStringLongText(va_arg(vlArgs, TIMESTAMP_DELTA), OUT PbAllocateExtraMemory(64));
				#endif
				break;
			case '@':	// $@
				{
				QTime timeNow = QTime::currentTime();
				CHU * pszTime = PbAllocateExtraData(2 + 1 + 2 + 1 + 2);	// Allocate enough data for the following format "hh:mm:ss"
				UINT uValue = timeNow.hour();
				pszTime[0] = '0' + (uValue / 10);
				pszTime[1] = '0' + (uValue % 10);
				pszTime[2] = ':';
				uValue = timeNow.minute();
				pszTime[3] = '0' + (uValue / 10);
				pszTime[4] = '0' + (uValue % 10);
				pszTime[5] = ':';
				uValue = timeNow.second();
				pszTime[6] = '0' + (uValue / 10);
				pszTime[7] = '0' + (uValue % 10);
				}
				break;

			case 'f':	// $f
			case 'g':	// $g
				{
				double flValue = va_arg(vlArgs, double);	// Floats are never pushed on the stack (the compiler always pushes doubles)
				CHA szValue[32];
				sprintf(OUT szValue, (chFormat == 'g') ? "%.05g" : "%.03g", flValue);
				BinAppendText(szValue);
				}
				break;

			case 'Z':
				if (va_arg(vlArgs, BOOL))
					goto TerminateString;
				break;
			case 'z':
				if (!va_arg(vlArgs, BOOL))
					goto TerminateString;
				break;

			default:
				Assert(FALSE && "Unknown template character for $");
				// Just put back the sequence to the output string
				BinAppendByte('$');
				BinAppendByte(chFormat);
				} // switch
			} // switch
		} // while

TerminateString:
	Assert(!FIsPointerAddressWithinBinaryObject(pszFmtTemplate));
	BinAppendByte('\0');
	m_paData->cbData -= sizeof(BYTE);	// Remove the null-terminator from the size of the bin
	ValidateHeapPointer(m_paData);
	return m_paData->rgbData;
	} // BinAppendTextSzv_VL()

void
CBin::BinAppendXmlAttributeUInt(PSZAC pszAttributeName, UINT uAttributeValue)
	{
	if (uAttributeValue != 0)
		{
		CHU szValue[16];
		IntegerToString(OUT szValue, uAttributeValue, ITS_mskfUnsigned);
		BinAppendXmlAttributeText(pszAttributeName, szValue);
		}
	}

void
CBin::BinAppendXmlAttributeUInt(CHS chAttributeName, UINT uAttributeValue)
	{
	const CHA szAttributeName[2] = { (CHA)chAttributeName, '\0' };
	BinAppendXmlAttributeUInt(szAttributeName, uAttributeValue);
	}

void
CBin::BinAppendXmlAttributeUIntHexadecimal(CHS chAttributeName, UINT uAttributeValueHexadecimal)
	{
	if (uAttributeValueHexadecimal != 0)
		{
		CHU szValue[16];
		IntegerToString(OUT szValue, uAttributeValueHexadecimal, ITS_mskfHexadecimal);
		BinAppendXmlAttributeText(chAttributeName, szValue);
		}
	}

void
CBin::BinAppendXmlAttributeInt(CHS chAttributeName, int nAttributeValue)
	{
	if (nAttributeValue != 0)
		{
		CHU szValue[16];
		IntegerToString(OUT szValue, nAttributeValue);
		BinAppendXmlAttributeText(chAttributeName, szValue);
		}
	}

void
CBin::BinAppendXmlAttributeL64(PSZAC pszAttributeName, L64 lAttributeValue)
	{
	if (lAttributeValue != 0)
		{
		CHU szValue[32];
		Integer64ToString(OUT szValue, lAttributeValue);
		BinAppendXmlAttributeText(pszAttributeName, szValue);
		}
	}

void
CBin::BinAppendXmlAttributeL64(CHS chAttributeName, L64 lAttributeValue)
	{
	const CHA szAttributeName[2] = { (CHA)chAttributeName, '\0' };
	BinAppendXmlAttributeL64(szAttributeName, lAttributeValue);
	}

//	" a='v'"
void
CBin::BinAppendXmlAttributeText(PSZAC pszaAttributeName, PSZUC pszAttributeValue)
	{
	Assert(pszaAttributeName != NULL);
	if (pszAttributeValue == NULL || pszAttributeValue[0] == '\0')
		return;
	// Calculate how many bytes we need to allocate
	BYTE * pb = (BYTE *)pszaAttributeName;
	while (TRUE)
		{
		const UINT ch = *pb;
		if (ch == '^' || ch == '\0')
			break;
		pb++;
		}
	const int cchAttributeName = pb - (BYTE *)pszaAttributeName;
	const int cbAttributeName = cchAttributeName + 3;
	pb = PbAllocateExtraMemory(cbAttributeName);
	Assert(m_paData != NULL);
	*pb++ = ' ';
	memcpy(OUT pb, pszaAttributeName, cchAttributeName);
	pb[cchAttributeName] = '=';
	pb[cchAttributeName + 1] = '\'';
	m_paData->cbData += cbAttributeName;
	BinAppendXmlTextU(pszAttributeValue);
	BinAppendByte('\'');
	Assert(m_paData->cbData > 0);
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	} // BinAppendXmlAttributeText()

void
CBin::BinAppendXmlAttributeText(CHS chAttributeName, PSZUC pszAttributeValue)
	{
	Assert(chAttributeName != '\0');
	const CHA szAttributeName[2] = { (CHA)chAttributeName, '\0' };
	BinAppendXmlAttributeText(szAttributeName, pszAttributeValue);
	}

void
CBin::BinAppendXmlAttributeTimestamp(CHS chAttributeName, const TIMESTAMP & tsAttributeValue)
	{
	if (tsAttributeValue != 0)
		{
		CHU szTimestamp[16];
		Timestamp_CchToString(tsAttributeValue, OUT szTimestamp);
		BinAppendXmlAttributeText(chAttributeName, szTimestamp);
		}
	}

void
CBin::BinAppendXmlAttributeCStr(CHS chAttributeName, const CStr & strAttributeValue)
	{
	BinAppendXmlAttributeText(chAttributeName, strAttributeValue);
	}

/*
void
CBin::BinAppendXmlAttributeCStr2(CHS chAttributeName, const CStr & strAttributeValue, PSZUC pszAttributeValuePriority)
	{
	BinAppendXmlAttributeCStr(chAttributeName, (pszAttributeValuePriority != NULL) ? pszAttributeValuePriority : strAttributeValue);
	}
*/

void
CBin::BinAppendXmlAttributeCBin(CHS chAttributeName, const CBin & binAttributeValue)
	{
	Assert(chAttributeName != '\0');
	if (!binAttributeValue.FIsEmptyBinary())
		BinAppendText_VE(" $b='{B|}'", chAttributeName, &binAttributeValue);
	}

void
CBin::BinAppendXmlElementText(PSZAC pszElementName, PSZUC pszElementValue)
	{
	Assert(pszElementName != NULL);
	Assert(pszElementName[0] != '\0');
	if (pszElementValue != NULL && pszElementValue[0] != '\0')
		BinAppendText_VE("<$s>^s</$s>\n", pszElementName, pszElementValue, pszElementName);
	}

void
CBin::BinAppendXmlElementText(PSZAC pszElementName, WEdit * pwEdit)
	{
	if (pwEdit != NULL)
		{
		CStr strValue = *pwEdit;
		BinAppendXmlElementText(pszElementName, strValue);
		}
	}

void
CBin::BinAppendXmlElementInt(PSZAC pszElementName, int nElementValue)
	{
	if (nElementValue != 0)
		BinAppendText_VE("<$s>$i</$s>\n", pszElementName, nElementValue, pszElementName);
	}

void
CBin::BinAppendXmlElementBinaryBase64(PSZAC pszElementName, const CBin & binElementValue)
	{
	Assert(pszElementName != NULL);
	Assert(pszElementName[0] != '\0');
	if (!binElementValue.FIsEmptyBinary())
		BinAppendText_VE("<$s>{B/}</$s>\n", pszElementName, &binElementValue, pszElementName);
	}

void
CBin::BinAppendXmlForSelfClosingElement()
	{
	BinAppendBinaryData("/>\n", 3);	// Close the XML element
	}

void
CBin::BinAppendXmlForSelfClosingElementQuote()
	{
	BinAppendText("'/>");
	}

void
CBin::BinAppendXmlForSelfClosingElementQuoteTruncateAtOffset(const SOffsets * pOffsets)
	{
	Assert(pOffsets != NULL);
	Assert(pOffsets->ibReset <= pOffsets->ibDataBegins);
	Assert(m_paData != NULL);
	if (m_paData->cbData == pOffsets->ibDataBegins)
		m_paData->cbData = pOffsets->ibReset;
	else
		BinAppendXmlForSelfClosingElementQuote();
	}

//	Create a string containing multiple XML attributes, ready to be inserted in the file.
//
//	This method is very similar to BinAppendText_VE() with the exception the empty values are ignored.
PSZUC
CBin::PszAppendVirtualXmlAttributes(PSZAC pszFmtTemplateAttributes, ...) CONST_TEMPORARY_MODIFIED
	{
	Assert(pszFmtTemplateAttributes != NULL);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplateAttributes);
	const int cbDataBefore = (m_paData != NULL) ? m_paData->cbData : 0;		// Remember the initial
	PSZAC pszAttributeName = (PSZAC)pszFmtTemplateAttributes;
	while (TRUE)
		{
		const UINT chTemplate = *pszFmtTemplateAttributes++;
		switch (chTemplate)
			{
		case '\0':
			goto TerminateString;

		case '^':  // Encode to XML
			const UINT chFormat = *pszFmtTemplateAttributes++;	// Get the second character
			switch (chFormat)
				{
			case d_chSourcePCSZU:
				BinAppendXmlAttributeText(pszAttributeName, va_arg(vlArgs, PSZUC));
				break;
			case d_chSourcePCStr:
				{
				CStr * pstr = va_arg(vlArgs, CStr *);
				if (pstr != NULL)
					BinAppendXmlAttributeText(pszAttributeName, pstr->PbGetData());
				}
				break;
			case 'd':
				Report(FALSE && "Please replace ^d by ^u");
			case 'u':
				{
				UINT uValue = va_arg(vlArgs, UINT);
				if (uValue != 0)
					{
					CHU szValue[16];
					IntegerToString(OUT szValue, uValue, ITS_mskfUnsigned);
					BinAppendXmlAttributeText(pszAttributeName, szValue);
					}
				}
				break;
			case 'l':
				{
				L64 llValue = va_arg(vlArgs, L64);
				if (llValue != 0)
					{
					CHU szValue[32];
					Integer64ToString(OUT szValue, llValue);
					BinAppendXmlAttributeText(pszAttributeName, szValue);
					}
				}
				break;
			default:
				Assert(FALSE && "Unknown template character for ^");
				goto TerminateString;
				} // switch
			if (*pszFmtTemplateAttributes == ',')
				pszFmtTemplateAttributes++;
			pszAttributeName = (PSZAC)pszFmtTemplateAttributes;
			} // switch
		} // while
TerminateString:
	Assert(!FIsPointerAddressWithinBinaryObject(pszFmtTemplateAttributes));
	BinAppendByte('\0');
	m_paData->cbData = cbDataBefore;
	ValidateHeapPointer(m_paData);
	return m_paData->rgbData + cbDataBefore;
	} // PszAppendVirtualXmlAttributes()

//	Encode binary data into Hexadecimal
void
CBin::BinAppendStringBase16FromBinaryData(const void * pvBinaryData, int cbBinaryData)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pvBinaryData));
	Assert(cbBinaryData >= 0);
	if (cbBinaryData > 0)
		{
		PSZU pszHex = PbAllocateExtraDataWithVirtualNullTerminator(cbBinaryData * 2);
		Base16_StringFromBinary(OUT pszHex, (const BYTE *)pvBinaryData, cbBinaryData);
		}
	}
//	Encode binary data into Base41
void
CBin::BinAppendStringBase41FromBinaryData(const void * pvDataBinary, int cbDataBinary)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pvDataBinary));
	Assert(cbDataBinary >= 4);	// Don't attempt to convert odd number of bytes to Base41
	PSZU pszBase41 = PbAllocateExtraMemory(cbDataBinary * 2);	// Allocate double the memory, this should be enough for the conversion
	pszBase41 = Base41_EncodeToText(OUT pszBase41, IN (const BYTE *)pvDataBinary, cbDataBinary);
	m_paData->cbData = pszBase41 - m_paData->rgbData;
	Assert(m_paData->cbData <= m_paData->cbAlloc);
	}

//	Encode binary data into Base64
void
CBin::BinAppendStringBase64FromBinaryData(const void * pvDataBinary, int cbDataBinary)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pvDataBinary));
	Assert(cbDataBinary >= 0);
	PSZU pszBase64 = PbAllocateExtraDataWithVirtualNullTerminator(Base64_CchEncoded(cbDataBinary));
	(void)Base64_CchEncodeToText(OUT pszBase64, (const BYTE *)pvDataBinary, cbDataBinary);
	}

void
CBin::BinAppendStringBase64FromBinaryData(const QByteArray & arraybData)
	{
	BinAppendStringBase64FromBinaryData(arraybData.data(), arraybData.size());
	//BinAppendByteArray(arraybData.toBase64());
	}

void
CBin::BinAppendStringBase64FromBinaryData(const CBin * pbinData)
	{
	BinAppendStringBase64FromBinaryData(pbinData->PszuGetDataNZ(), pbinData->CbGetData());
	}

//	Encode a string into Base64
void
CBin::BinAppendStringBase64FromStringU(PSZUC pszuString)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszuString));
	if (pszuString != NULL)
		BinAppendStringBase64FromBinaryData(pszuString, strlenU(pszuString));
	}

void
CStr::InitFromTextEncodedInBase85(PSZUC pszBase85)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BinAppendBinaryDataFromBase85Szv_ML(pszBase85);
	m_paData->cbData++;	// Include the virtual null-terminator as a null-terminator
	}

/*
Not used anymore... need testing
void
CStr::InitFromTextEncodedInBase64(const CStr & strText)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BinAppendStringBase64FromStringU(strText.PszuGetDataNZ());
	BinAppendNullTerminator();
	}
*/

PSZUC
CStr::InitFromBase64Sz(PSZUC pszBase64)
	{
	if (m_paData != NULL)
		m_paData->cbData = 0;
	BinAppendBinaryDataFromBase64Szv(pszBase64);
	m_paData->cbData++;	// Include the virtual null-terminator as a null-terminator
	return m_paData->rgbData;
	}

//	Decode a Base64 string into binary data (the Base64 string may contain just text without any binary character)
PSZUC
CBin::BinAppendBinaryDataFromBase64Szv(PSZUC pszuBase64)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszuBase64));
	if (pszuBase64 != NULL)
		{
		QByteArray arraybBase64((const char *)pszuBase64);
		BinAppendByteArray(QByteArray::fromBase64(arraybBase64));
		}
	return BinAppendNullTerminatorVirtualSzv();
	}

//	Decode a Base85 string into binary and return the number of bytes appended.
int
CBin::BinAppendBinaryDataFromBase85SCb_ML(PSZUC pszuBase85)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pszuBase85));
	if (pszuBase85 != NULL)
		{
		PCHRO pchroStop;
		BYTE * pbBinaryData = PbAllocateExtraMemory(Base85_CbDecodeAlloc(pszuBase85, OUT &pchroStop));
		const int cbDecoded = Base85_CbDecodeToBinary(IN pszuBase85, OUT_F_INV pbBinaryData);
		m_paData->cbData += cbDecoded;
		Assert(m_paData->cbData < m_paData->cbAlloc);	// The function Base85_CbDecodeAlloc() should return a value large for the null-terminator
		pbBinaryData[cbDecoded] = '\0';	// Insert a virtual null-terminator
		Assert(pchroStop != NULL);
		if (*pchroStop != '\0')
			{
			// We have a string conaining an illegal Base85 character
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The character '{s1}' at offset $i is not recognized in the Base85 text: $s\n", pchroStop, pchroStop - pszuBase85, pszuBase85);
			}
		return cbDecoded;
		}
	return 0;
	}

PSZUC
CBin::BinAppendBinaryDataFromBase85Szv_ML(PSZUC pszuBase85)
	{
	(void)BinAppendBinaryDataFromBase85SCb_ML(pszuBase85);
	return BinAppendNullTerminatorVirtualSzv();
	}

void
CBin::BinAppendStringBase85FromBinaryData(const void * pvDataBinary, int cbDataBinary)
	{
	Assert(!FIsPointerAddressWithinBinaryObject(pvDataBinary));
	Assert(cbDataBinary >= 0);
	PSZU pszBase85 = PbAllocateExtraMemory(Base85_CbEncodeAlloc(cbDataBinary));
	m_paData->cbData += Base85_CchEncodeToText(OUT pszBase85, (const BYTE *)pvDataBinary, cbDataBinary);
	}

void
CBin::BinAppendStringBase85FromBinaryData(const CBin * pbinData)
	{
	BinAppendStringBase85FromBinaryData(pbinData->PszuGetDataNZ(), pbinData->CbGetData());
	}


//	Append a string to the bin, however without increasing the size of the bin.
//	In other words, the appended string is there in the bin, however not counted as the size.
//	The motivation for this method is to reuse an existing CBin or CStr for temporary storage, without modifying its content.
//	Return pointer to the appended string.
PSZUC
CBin::BinAppendTextVirtualSzv_VL(OUT int * pcbDataFormatted, IN PSZAC pszFmtTemplate, va_list vlArgs) CONST_TEMPORARY_MODIFIED
	{
	Assert(pcbDataFormatted != NULL);
	Assert(pszFmtTemplate != NULL);
	const int cbDataBefore = (m_paData != NULL) ? m_paData->cbData : 0;
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	*pcbDataFormatted = m_paData->cbData - cbDataBefore;
	m_paData->cbData = cbDataBefore;
	return m_paData->rgbData + cbDataBefore;
	}

void
CBin::CalculateHashMd5FromFormattedString_VL(OUT SHashMd5 * pHashMd5, PSZAC pszFmtTemplate, va_list vlArgs) CONST_TEMPORARY_MODIFIED
	{
	Assert(pHashMd5 != NULL);
	const int cbDataBefore = (m_paData != NULL) ? m_paData->cbData : 0;
	BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	HashMd5_CalculateFromBinary(OUT pHashMd5, IN m_paData->rgbData + cbDataBefore, m_paData->cbData - cbDataBefore);
	m_paData->cbData = cbDataBefore;
	}


//	Return TRUE if pFileSizeAndMd5 has been modified.
//	Return FALSE if the content of the bin produces an identical SFileSizeAndMd5.
BOOL
CBin::FAssignFileSizeAndMd5(INOUT_F_UNCH SFileSizeAndMd5 * pFileSizeAndMd5) const
	{
	Assert(pFileSizeAndMd5 != NULL);
	SFileSizeAndMd5 hashBin;
	if (m_paData != NULL)
		{
		hashBin.cbFileSize = m_paData->cbData;
		HashMd5_CalculateFromBinary(OUT &hashBin.md5, IN m_paData->rgbData, hashBin.cbFileSize);
		}
	else
		{
		InitToZeroes(OUT &hashBin, sizeof(hashBin));
		}
	if (0 == memcmp(&hashBin, pFileSizeAndMd5, sizeof(SFileSizeAndMd5)))
		return FALSE;				// The values are identical
	*pFileSizeAndMd5 = hashBin;	// Copy the new values
	return TRUE;
	} // FAssignFileSizeAndMd5()
