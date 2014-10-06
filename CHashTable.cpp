///////////////////////////////////////////////////////////////////////////////////////////////////
//	CHashTable.cpp
//
//	Class capable of searching within a large collection of elements.
//
//	PERFORMANCE NOTES
//	- Insert element O(1)
//	- Remove element O(1)
//	- Find element O(1). The worst case is O(n) if all the elements map to the same hash index.
//
//	IMPLEMENTATION NOTES
//	This class is implemented to maximize performance.
//	The code is designed to take advantage of hardware architecture to reduce computing branching.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#ifdef DEBUG_WANT_ASSERT
	//#define TRACE_HASH_PERFORMANCE	// Display statistics about the performance of a hash table
	volatile BOOL g_fAlreadyHere_HashTable_Add = FALSE;
	volatile BOOL g_fAlreadyHere_HashTable_Rehash = FALSE;
	volatile BOOL g_fAlreadyHere_HashTable_Destructor = FALSE;

#define d_cDebugAllowanceAssertValidHashTableDefault		5000
long g_cDebugAllowanceAssertValidHashTable = d_cDebugAllowanceAssertValidHashTableDefault;

BOOL
FIsDebugAllowanceReachedAssertValidHashTable()
	{
	return FIsDebugAllowanceReached(INOUT &g_cDebugAllowanceAssertValidHashTable, "the CHashTableCore verification module", d_cDebugAllowanceAssertValidHashTableDefault);
	}

VOID
DebugAllowanceAssertValidHashTableDisplayToMessageLog()
	{
	MessageLog_AppendTextFormatCo(d_coBlueDark, "\t CHashTableCore verification module: $I$s\n", d_cDebugAllowanceAssertValidHashTableDefault - g_cDebugAllowanceAssertValidHashTable, (g_cDebugAllowanceAssertValidHashTable <= 0) ? "+" : NULL);
	}

#endif // DEBUG_WANT_ASSERT


CHashTableCore::CHashTableCore()
	{
	m_cHashElements = 0;
	m_mskmHashTable = 0;
	m_pargpHashElements = NULL;
	m_cbHashElementAndExtraEstimated = 0;
	m_paTableList = NULL;
	}

CHashTableCore::~CHashTableCore()
	{
	// AssertValid();	// Cannot call AssertValid() after in the base destructor the derived class may have deleted the elements
	// Destroy the linked list of pre-allocated tables
	#ifdef DEBUG
	Assert(g_fAlreadyHere_HashTable_Destructor == FALSE);
	Assert(g_fAlreadyHere_HashTable_Add == FALSE);
	g_fAlreadyHere_HashTable_Destructor = TRUE;
	#endif

	delete m_pargpHashElements;
	_DestroyTableList();

	DEBUG_CODE( g_fAlreadyHere_HashTable_Destructor = FALSE; )
	}

VOID CHashTableCore::_BackupCreate(INOUT CHashTableBackup * pHashTableBackup)
	{
	Assert(pHashTableBackup->m_pHashTable == this);
	pHashTableBackup->m_cHashElements = m_cHashElements;
	if (m_cHashElements <= 0)
		return;	// There is nothing to backup
	m_cHashElements = 0;

	// Remember the state of the hash table
	pHashTableBackup->m_mskmHashTable = m_mskmHashTable;
	m_mskmHashTable = 0;

	pHashTableBackup->m_pargpHashElements = m_pargpHashElements;
	m_pargpHashElements = NULL;

	pHashTableBackup->m_paTableList = m_paTableList;
	m_paTableList = NULL;
	} // _BackupCreate()

VOID
CHashTableCore::_BackupRestore(const CHashTableBackup * pHashTableBackup)
	{
	Assert(pHashTableBackup->m_pHashTable == this);

	if (pHashTableBackup->m_cHashElements <= 0)
		{
		// The previous hash table was empty, so remove empty it again
		Empty();
		return;
		}

	// Restore the previous state of the hash table
	_DestroyTableList();
	Assert(m_paTableList == NULL);
	m_paTableList = pHashTableBackup->m_paTableList;

	delete m_pargpHashElements;
	m_pargpHashElements = pHashTableBackup->m_pargpHashElements;
	m_mskmHashTable = pHashTableBackup->m_mskmHashTable;
	m_cHashElements = pHashTableBackup->m_cHashElements;
	} // _BackupRestore()

CHashTableBackup::CHashTableBackup(INOUT CHashTableCore * pHashTable)
	{
	Assert(pHashTable != NULL);
	InitToGarbage(OUT this, sizeof(*this));
	m_pHashTable = pHashTable;
	pHashTable->_BackupCreate(INOUT this);
	}

CHashTableBackup::~CHashTableBackup()
	{
	m_pHashTable->_BackupRestore(IN this);
	}

/////////////////////////////////////////////////////////////////////
CHashTable::CHashTable(PFn_PszGetHashKey pfnPszGetHashKey, EHashFunction eHashFunction)
	{
	Assert(pfnPszGetHashKey != NULL);
	m_pfnPszGetHashKey = pfnPszGetHashKey;
	SetHashFunction(eHashFunction);
	}


//	Set the following pointers:
//		- m_pfnUComputeHashValueForHashKey - routine create the hash key (index)
//		- m_pfnFCompareHashKeys - routine to compare two hash keys
VOID
CHashTable::SetHashFunction(EHashFunction eHashFunction)
	{
	switch (eHashFunction)
		{
	default:
		Assert(FALSE && "Unknown hash function");
	case eHashFunctionStringCase:
		m_pfnFCompareHashKeys = FCompareHashKeysStringCase;
		m_pfnUComputeHashValueForHashKey = UComputeHashValueFromStringCase;
		break;
	case eHashFunctionStringNoCase:
		m_pfnFCompareHashKeys = FCompareHashKeysStringNoCase;
		m_pfnUComputeHashValueForHashKey = UComputeHashValueFromStringNoCase;
		break;
	case eHashFunctionStringUnicodeCase:
		m_pfnFCompareHashKeys = (PFn_FCompareHashKeys)FCompareHashKeysStringUnicodeCase;
		m_pfnUComputeHashValueForHashKey = (PFn_UComputeHashValueForHashKey)UComputeHashValueFromStringUnicodeCase;
		break;
	case eHashFunctionStringUnicodeNoCase:
		Assert(FALSE && "NYI");
		/*
		m_pfnFCompareHashKeys = (PFn_FCompareHashKeys)FCompareHashKeysStringUnicodeNoCase;
		m_pfnUComputeHashValueForHashKey = (PFn_UComputeHashValueForHashKey)UComputeHashValueFromStringUnicodeNoCase;
		break;
		*/
	case eHashFunctionStringUnicodeNoCaseAnsi:
		m_pfnFCompareHashKeys = (PFn_FCompareHashKeys)FCompareHashKeysStringUnicodeNoCaseAnsi;
		m_pfnUComputeHashValueForHashKey = (PFn_UComputeHashValueForHashKey)UComputeHashValueFromStringUnicodeNoCaseAnsi;
		break;
	case eHashFunctionUInt32:
		m_pfnFCompareHashKeys = (PFn_FCompareHashKeys)FCompareHashKeysUInt32;
		m_pfnUComputeHashValueForHashKey = (PFn_UComputeHashValueForHashKey)UComputeHashValueFromUInt32;
		break;
	case eHashFunctionHashSha1:
		m_pfnFCompareHashKeys = (PFn_FCompareHashKeys)FCompareHashKeysHashSha1;
		m_pfnUComputeHashValueForHashKey = (PFn_UComputeHashValueForHashKey)UComputeHashValueFromHashSha1;
		break;
		} // switch
	} // SetHashFunction()

/////////////////////////////////////////////////////////////////////
//	Initialize the size of the hash table.
//
//	The method will keep all previous hash element in the table.
//
//	IMPLEMENTATION NOTES
//	The method will allocate about four times as many buckets to minimize collisions.
void
CHashTable::SetHashTableSize(int cElements)
	{
	Assert(cElements >= 0 && cElements < 10000000);	// Arbitrary chosen
	AssertValid();

	#define _cElementsMinimum	16	// Minimum number of elements for a hash table (must be a power of 2).  This number will be multiplied by two anyway.
	if (cElements < _cElementsMinimum)
		cElements = _cElementsMinimum;
	if (cElements <= (int)m_mskmHashTable)
		return;		// We don't want to shrink the hash table
	else if (m_mskmHashTable > 0)
		cElements <<= 2;	// This is a re-allocation of the hash table, so multiply by four to grow the number of buckets by four
	// Remember the content of the previous hash table
	CHashElement ** pargpHashElementsOld =  m_pargpHashElements;
	CHashElement ** ppHashElementsOldLast = pargpHashElementsOld + m_mskmHashTable;

	m_mskmHashTable = _cElementsMinimum << 1;	// Compute the mask to use get the hash index
	while ((int)m_mskmHashTable < cElements)
		m_mskmHashTable <<= 1;	// Multiply by two
	Assert(UCountBits(m_mskmHashTable) == 1);
	m_pargpHashElements = new CHashElement *[m_mskmHashTable];	// Allocate the new bigger hash table
	InitToZeroes(OUT m_pargpHashElements, m_mskmHashTable * sizeof(CHashElement *));
	//m_arraypHashElements.ZeroInitialize(m_mskmHashTable);	// Allocate the new hash table
	m_mskmHashTable--;	// Make the mask

	#ifdef TRACE_HASH_PERFORMANCE
	TRACE1("CHashTable::SetHashTableSize() - cBuckets=%d\n", m_mskmHashTable + 1);
	#endif

	if (m_cHashElements > 0)
		{
		// Re-hash the elements
		#ifdef DEBUG
		m_cReHash++;
		#endif

		CHashElement ** ppHashElementsOld = pargpHashElementsOld;
		Assert(ppHashElementsOld != NULL);
		#ifdef DEBUG_WANT_ASSERT
		int cElementsDebug = 0;
		#endif
		while (ppHashElementsOld <= ppHashElementsOldLast)
			{
			CHashElement * pHashElement = *ppHashElementsOld++;
			while (pHashElement != NULL)
				{
				#ifdef DEBUG_WANT_ASSERT
				cElementsDebug++;
				#endif
				Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
				CHashElement * pHashElementNext = pHashElement->m_pNextHashElement;
				Endorse(pHashElementNext == NULL);
				Assert(pHashElement->m_uHashValue != 0);
				#ifdef DEBUG
				const UINT uHashValueDebug = m_pfnUComputeHashValueForHashKey(m_pfnPszGetHashKey(pHashElement));
				Assert(uHashValueDebug == pHashElement->m_uHashValue);
				#endif
				int iHashElement = (pHashElement->m_uHashValue & m_mskmHashTable);
				pHashElement->m_pNextHashElement = m_pargpHashElements[iHashElement];
				m_pargpHashElements[iHashElement] = pHashElement;
				Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
				pHashElement = pHashElementNext;
				}
			} // while
		Assert(cElementsDebug == m_cHashElements);
		} // if
	delete pargpHashElementsOld;
	AssertValid();
	} // SetHashTableSize()


//	Set the size (in bytes) of the hash element and
//	optionally the estimated number of extra bytes per hash element.
void
CHashTableCore::SetHashElementSize(int cbHashElement, int cbExtraEstimated)
	{
	Assert(cbHashElement >= (int)sizeof(CHashElement));
	Assert(cbHashElement < 0xFFFF);
	Assert(cbExtraEstimated >= 0 && cbExtraEstimated < 0xFFFF);
	m_cbHashElementAndExtraEstimated = cbHashElement | (cbExtraEstimated << 16);
	}

//	Use a static buffer to improve the performance by avoiding a dynamic memory allocation.
//	The static buffer can also be shared with many hash tables.
//
//	INTERFACE NOTES
//	If the first 16 bytes at the beginning of pvBuffer are zeroes, the routine assumes
//	the static buffer is new and therefore set the appropriate pointers to belong
//	to the hash table, otherwise the routine assumes the buffer is being re-used
//	by a different hash table.
//
void
CHashTableCore::BindToStaticBuffer(INOUT void * pvBuffer, int cbBuffer)
	{
	Assert(pvBuffer != NULL);
	Assert(cbBuffer >= 100);	// Minimum of 100 bytes
	Assert(m_paTableList == NULL);	// This assert is not important, but can be handy to detect memory leaks
	m_paTableList = (THashElementTableHdr *)pvBuffer;
	Assert(m_paTableList->pNext == NULL);
	if (m_paTableList->cbAlloc == 0)
		{
		// Initialize the static buffer
		Assert(m_paTableList->cbData == 0);
		m_paTableList->cbAlloc = cbBuffer - sizeof(THashElementTableHdr);
		}
	} // BindToStaticBuffer()

void
CHashTableCore::UnbindStaticBuffer()
	{
	Assert(m_paTableList != NULL);
	Assert(m_paTableList->pNext == NULL && "Static buffers cannot be dynamically linked together.  Increase the size of the static buffer!!!");
	m_paTableList = NULL;	// Prevent the destructor to free a static buffer (it would be catastrophic!)
	}

//	Allocate a table (or buffer) to hold multiple hash elements
void
CHashTableCore::_AllocateTableList(int cbAlloc)
	{
	Assert(cbAlloc >= 250);		// Minimum of 250 bytes
	cbAlloc = NEXT_UINT32(cbAlloc);
	THashElementTableHdr * paTable = (THashElementTableHdr *)new BYTE[sizeof(THashElementTableHdr) + cbAlloc];
	InitToGarbage(OUT paTable, sizeof(THashElementTableHdr) + cbAlloc);
	paTable->pNext = m_paTableList;	// Append at the beginning of the list
	paTable->cbData = 0;
	paTable->cbAlloc = cbAlloc;
	m_paTableList = paTable;
	}

VOID
CHashTableCore::_DestroyTableList()
	{
	#ifdef TRACE_HASH_PERFORMANCE
	int cMemoryBlocks = 0;
	#endif
	while (m_paTableList != NULL)
		{
		#ifdef TRACE_HASH_PERFORMANCE
		cMemoryBlocks++;
		int cbFree = m_paTableList->cbAlloc - m_paTableList->cbData;
		Assert(cbFree >= 0);
		TRACE5("CHashTableCore::~CHashTableCore() - Deleting memory block #%d: %d bytes free (%.01f%% free)  [%d bytes of %d bytes allocated]\n",
			cMemoryBlocks, cbFree, (float)cbFree * 100 / m_paTableList->cbAlloc, m_paTableList->cbData, m_paTableList->cbAlloc);
		#endif
		THashElementTableHdr * paTableNext = m_paTableList->pNext;
		#ifdef DEBUG
		InitToGarbage(OUT m_paTableList, sizeof(THashElementTableHdr) + m_paTableList->cbAlloc);
		#endif
		delete m_paTableList;
		m_paTableList = paTableNext;
		}
	} // _DestroyTableList()

//	Return a pointer to a buffer of a minimum cbBuffer.
//	This routine may pre-allocate memory, but does not commit.
//	In other words, calling this method multiple times does nothing
//
//	Core routine to allocate small block memory from the pre-allocated table/buffer.
//	This data does not need to be deleted because the hash table holds the allocation to the memory.
//
//	To increase speed and reduce allocation overhead, the hash table allocates larger blocks
//	of memory and returns pointer to it when the user requires memory.  If there is not enough
//	remaining storage in the pre-allocated block, then a new block is allocated.
//
//	INTERFACE NOTES
//	You must call method SetHashElementSizeAllocate() once before using PvGetBuffer().
//	The caller is responsible of calling the method CommitBuffer() to bind the data to the buffer.

void *
CHashTableCore::PvGetBuffer(int cbBuffer)
	{
	Assert(cbBuffer > 0);
	Assert(m_cbHashElementAndExtraEstimated != 0);
	const int cbAlloc = cbBuffer;	// TODO: Rename this variable in the future
	Endorse(!IS_ALIGNED_32(cbAlloc));	// This is not necessary to have the allocation 32-bit aligned
	if (m_paTableList == NULL)
		_AllocateTableList(100 * ((WORD)m_cbHashElementAndExtraEstimated + (m_cbHashElementAndExtraEstimated >> 16)) + cbAlloc);	// Pre-allocate room for 100 extra hash elements
	Assert(m_paTableList != NULL);
	BYTE * pbHashElement;
	int cbAllocated = m_paTableList->cbAlloc;
	int cbDataAllocated = m_paTableList->cbData;
	int cbDataAvailable = cbAllocated - cbDataAllocated;
	if (cbAlloc <= cbDataAvailable)
		{
		// The hash element fit within the pre-allocated block
		pbHashElement = ((BYTE *)(m_paTableList + 1)) + cbDataAllocated;
		}
	else
		{
		// We need to allocate a new block to hold the new hash element
		_AllocateTableList(cbAllocated + cbAlloc);	// Grow the bloc size to include the previous, and the new hash element
		pbHashElement = (BYTE *)(m_paTableList + 1);
		}
	InitToGarbage(OUT pbHashElement, cbAlloc);
	return pbHashElement;
	} // PvGetBuffer()

//	Complement to PvGetBuffer()
void
CHashTableCore::CommitBuffer(int cbBuffer)
	{
	Assert(cbBuffer >= 0);
	Assert(m_paTableList != NULL);
	m_paTableList->cbData += cbBuffer;
	Assert(m_paTableList->cbData <= m_paTableList->cbAlloc);
	}

//	Allocate raw data from the pre-allocated buffer.
//	The content of the allocated buffer is not initialized.
//
//	INTERFACE NOTES
//	You must call method SetHashElementSizeAllocate() once before using PvAllocateData()
void *
CHashTableCore::PvAllocateData(int cbAlloc)
	{
	void * pvBuffer = PvGetBuffer(cbAlloc);
	InitToGarbage(OUT pvBuffer, cbAlloc);
	m_paTableList->cbData += cbAlloc;
	return pvBuffer;
	} // PvAllocateData()


//	Small allocator which copies a buffer
void *
CHashTableCore::PvAllocateDataCopy(int cbAlloc, PCVOID pvDataSrcCopy)
	{
	Assert(pvDataSrcCopy != NULL);
	void * pvData = PvAllocateData(cbAlloc);
	memcpy(OUT pvData, IN pvDataSrcCopy, cbAlloc);
	return pvData;
	}

//	INTERFACE NOTES
//	After calling this method, the data may no longer be 32-bit aligned.
//	Use void AlignToUInt32() before allocating a new hash element.
PSZWC
CHashTableCore::PszwAllocateStringW(PSZWC pszwSrcCopy)
	{
	Assert(pszwSrcCopy != NULL);
	Assert(*pszwSrcCopy != '\0');
	// Calculate how many bytes to allocate
	const CHW * pchw = pszwSrcCopy;
	while (*pchw++ != '\0')
		;
	// Allocate and copy the data
	return (PSZWC)PvAllocateDataCopy((INT_P)pchw - (INT_P)pszwSrcCopy, IN pszwSrcCopy);
	}


//	Allocate an UTF-U string to the allocator
PSZUC
CHashTableCore::PszuAllocateStringU(PSZUC pszuSrcCopy)
	{
	if (pszuSrcCopy == NULL)
		return NULL;
	Assert(*pszuSrcCopy != '\0');
	// Calculate how many bytes to allocate
	const CHU * pchu = pszuSrcCopy;
	while (*pchu++ != '\0')
		;
	Assert((UINT)(pchu - pszuSrcCopy) == strlenU(pszuSrcCopy) + 1);
	// Allocate and copy the data
	return (PSZUC)PvAllocateDataCopy(pchu - pszuSrcCopy, IN pszuSrcCopy);
	}

//	Align the internal buffer to a 32-bit boundary.  This is a requirement
//	to enhance the speed accessing hash elements (all hash elements are 32-bit aligned).
//
void
CHashTableCore::AlignToUInt32()
	{
	if (m_paTableList != NULL)
		{
		Assert(IS_ALIGNED_32(m_paTableList));
		Assert(IS_ALIGNED_32(m_paTableList->cbAlloc));
		m_paTableList->cbData = NEXT_UINT32(m_paTableList->cbData);
		}
	}

//	This routine allocate a CHashElement (or any derived class).
//	The content of CHashElement is filled with zeroes.
//
//	This is a pure allocator as no element is added to the hash table.
//	If there is not enough remaining storage in the pre-allocated block,
//	then a new block is allocated.
//
//	INTERFACE NOTES
//	You must call method SetHashElementSizeAllocate() once before using PAllocateHashElement()
//
CHashElement *
CHashTable::PAllocateHashElement(int cbExtra)
	{
	Assert(cbExtra >= 0);
	Assert(m_cbHashElementAndExtraEstimated >= (int)sizeof(CHashElement) && "Use method SetHashElementSize() to initialize the allocator");
	Endorse(m_paTableList == NULL);	// No table allocated yet

	const int cbAlloc = (WORD)m_cbHashElementAndExtraEstimated + cbExtra;
	Endorse(!IS_ALIGNED_32(cbAlloc));	// This is not necessary 32-bit aligned
	Assert(m_paTableList == NULL || IS_ALIGNED_32(m_paTableList->cbData));	// The allocation of a hash element must always be on a 32-bit boundary.  Use AlignToUInt32() if necessary.
	BYTE * pbHashElement = (BYTE *)PvAllocateData(NEXT_UINT32(cbAlloc));
	Assert(m_paTableList != NULL);
	Assert(IS_ALIGNED_32(m_paTableList->cbData));
	Assert(pbHashElement != NULL);
	InitToZeroes(OUT pbHashElement, cbAlloc);	// Initialize the hash element to zeroes
	return (CHashElement *)pbHashElement;
	} // PAllocateHashElement()

void
CHashTable::Add(INOUT CHashElement * pHashElement)
	{
	Assert(g_fAlreadyHere_HashTable_Add == FALSE);
	Assert(g_fAlreadyHere_HashTable_Destructor == FALSE);
	DEBUG_CODE( g_fAlreadyHere_HashTable_Add = TRUE; )
	// AssertValid();		// This assert has been commented out because it seriously affects the performance.  Now the hash table is O(n^2) which takes several minutes to import a large XML file.
	Assert(pHashElement != NULL);
	Endorse(pHashElement->m_pNextHashElement != NULL);
	Assert(m_pfnPszGetHashKey != NULL);
	if (m_cHashElements >= (int)m_mskmHashTable)
		{
		#ifdef TRACE_HASH_PERFORMANCE
		if (m_cHashElements > 50000)
			MessageLog_AppendTextFormatCo(coRed, "CHashTable::Add() - Hash table has %d elements and must be increased...\n", m_cHashElements);
		#endif
		SetHashTableSize(m_cHashElements * 2);
		}
	Assert(m_mskmHashTable > 0);
	Assert(m_pargpHashElements != NULL);

	if (pHashElement->m_uHashValue == 0)
		pHashElement->m_uHashValue = m_pfnUComputeHashValueForHashKey(m_pfnPszGetHashKey(pHashElement));
	Assert(pHashElement->m_uHashValue != 0);
	int iHashElement = (pHashElement->m_uHashValue & m_mskmHashTable);
	pHashElement->m_pNextHashElement = m_pargpHashElements[iHashElement];
	Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
	m_pargpHashElements[iHashElement] = pHashElement;
	m_cHashElements++;
	Assert(PFindHashElement(m_pfnPszGetHashKey(pHashElement)) == pHashElement);

	// Verify the integrity of the hash table
	// AssertValid();		// Same as above.
	DEBUG_CODE( g_fAlreadyHere_HashTable_Add = FALSE; )
	} // Add()

void
CHashTable::RemoveElement(INOUT CHashElement * pHashElementRemove)
	{
	Assert(pHashElementRemove != NULL);
	Assert(m_pfnPszGetHashKey != NULL);
	Assert(pHashElementRemove->m_uHashValue != 0 && "Hash element should have been added");
	#ifdef DEBUG
	if (!g_fAlreadyHere_HashTable_Rehash)
		{
		// If we are not re-hashing, then verify if the table is valid
		AssertValid();
		}
	#endif

	int iHashElement = (pHashElementRemove->m_uHashValue & m_mskmHashTable);
	pHashElementRemove->m_uHashValue = 0;	// Force a re-hashing next time

	Assert(m_pargpHashElements != NULL);
	CHashElement * pHashElementTemp = m_pargpHashElements[iHashElement];
	Assert(pHashElementTemp != NULL);
	if (pHashElementTemp == pHashElementRemove)
		{
		// The first hash element is the one to remove
		Endorse(pHashElementTemp->m_pNextHashElement == NULL);
		m_pargpHashElements[iHashElement] = pHashElementTemp->m_pNextHashElement;
		}
	else
		{
		// Find the hash element in the chain
		CHashElement * pHashElementPrev = pHashElementTemp;
		while (TRUE)
			{
			Assert(pHashElementTemp != NULL);
			if (pHashElementTemp == NULL)
				return;	// Just in case (to avoid Judgment Day)
			CHashElement * pHashElementNext = pHashElementTemp->m_pNextHashElement;
			if (pHashElementTemp == pHashElementRemove)
				{
				Endorse(pHashElementTemp->m_pNextHashElement == NULL);
				pHashElementPrev->m_pNextHashElement = pHashElementNext;
				break;
				}
			pHashElementPrev = pHashElementTemp;
			pHashElementTemp = pHashElementNext;
			} // while
		} // if...else
	// Decrease the counter
	Assert(m_cHashElements > 0);
	m_cHashElements--;
	AssertValid();
	} // RemoveElement()

//	This method is similar as removing an element and re-inserting it.
//	The purpose of this method is to simplify the code but also make the
//	hash table coherent via the AssertValid() because it is possible
//	the the hash table not be valid at the beginning of the method.
//
VOID
CHashTable::RehashElement(INOUT CHashElement * pHashElement)
	{
	Assert(g_fAlreadyHere_HashTable_Rehash == FALSE);
	DEBUG_CODE( g_fAlreadyHere_HashTable_Rehash = TRUE; )

	Assert(pHashElement != NULL);
	Assert(m_pfnPszGetHashKey != NULL);
	if (pHashElement->m_uHashValue != 0)
		{
		RemoveElement(INOUT pHashElement);
		Add(INOUT pHashElement);
		}
	AssertValid();
	DEBUG_CODE( g_fAlreadyHere_HashTable_Rehash = FALSE; )
	} // RehashElement()


/////////////////////////////////////////////////////////////////////
//	Call the routine pfnEnumHashElement for each element in the hash table.
//
//	INTERFACE NOTES
//	To avoid using the const_cast<> operator each time, the method ForEachHashElement()
//	has been made 'const'.  Of course, depending on the behavior of pfnEnumHashElement,
//	the method ForEachHashElement() may change the hash table.
//
void
CHashTable::ForEachHashElement(PFn_EnumHashElement pfnEnumHashElement, LPARAM lParam) const
	{
	Assert(pfnEnumHashElement != NULL);
	Endorse(lParam == d_zNA);
	AssertValid();
	if (m_cHashElements == 0)
		return;
	//Assert(m_arraypHashElements.GetSize() == (int)m_mskmHashTable + 1);
	CHashElement ** prgpHashElements = m_pargpHashElements;
	CHashElement ** prgpHashElementsLast = prgpHashElements + m_mskmHashTable;
	while (prgpHashElements <= prgpHashElementsLast)
		{
		CHashElement * pHashElement = *prgpHashElements++;
		while (pHashElement != NULL)
			{
			CHashElement * pHashElementNext = pHashElement->m_pNextHashElement;
			pfnEnumHashElement(pHashElement, lParam);
			pHashElement = pHashElementNext;
			}
		}
	// AssertValid();	// Cannot call AssertValid() after enumerating the elements.  This is because the pfnEnumHashElement() may delete the hash elements
	} // ForEachHashElement()

/////////////////////////////////////////////////////////////////////
//	Return an array containing all the elements in the hash table
//	Return the number of elements added to the array (ie the value of m_cHashElements)
UINT
CHashTable::GetAllElements(OUT CArrayPtrHashElement * parraypHashElements) const
	{
	Assert(parraypHashElements != NULL);
	AssertValid();
	parraypHashElements->PrgpvAllocateElementsEmpty(m_cHashElements);
	Assert(parraypHashElements->GetSize() == 0);
	ForEachHashElement((PFn_EnumHashElement)S_PFn_EnumHashElementGetAll, INOUT (LPARAM)parraypHashElements);
	Assert(parraypHashElements->GetSize() == m_cHashElements);
	AssertValid();
	return m_cHashElements;
	} // GetAllElements()

#define E_mskfConsolidateMemoryBlocks
#define E_mskfAllowHashTableToShrink

//	Flush all the elements in the hash table.
//
//	IMPLEMENTATION NOTES
//	For better performance, the memory remains allocated and the pointers are set to NULL.
//	In the case of the pre-allocated buffers, they are consolidated into a single buffer
//	so next time, there is no need to re-allocate any memory.
//
void
CHashTableCore::Empty()
	{
	//AssertValid();
	if (m_cHashElements > 0)
		{
		// Initialize all pointers to NULL
		m_cHashElements = 0;
		Assert(m_pargpHashElements != NULL);
		Assert(m_mskmHashTable >= 16);
		InitToZeroes(m_pargpHashElements, (m_mskmHashTable + 1) * sizeof(CHashElement *));
		}
	if (m_paTableList != NULL)
		{
		if (m_paTableList->pNext != NULL)
			{
			// We have more than one buffer
			#ifdef TRACE_HASH_PERFORMANCE
			int cMemoryBlocks = 0;
			#endif
			int cbAllocated = 0;
			while (m_paTableList != NULL)
				{
				#ifdef TRACE_HASH_PERFORMANCE
				cMemoryBlocks++;
				#endif
				cbAllocated += m_paTableList->cbData;
				THashElementTableHdr * paTableNext = m_paTableList->pNext;
				#ifdef DEBUG
				InitToGarbage(OUT m_paTableList, sizeof(THashElementTableHdr) + m_paTableList->cbAlloc);
				#endif
				delete m_paTableList;
				m_paTableList = paTableNext;
				}
			#ifdef TRACE_HASH_PERFORMANCE
			TRACE2("CHashTable::~Empty() - Consolidating %d blocks into a single block of %d bytes\n", cMemoryBlocks, cbAllocated);
			//Report(FALSE && "Testing Empty()");
			#endif
			_AllocateTableList(cbAllocated);
			Assert(m_paTableList != NULL);
			Assert(m_paTableList->cbData == 0);
			}
		Assert(m_paTableList != NULL);
		m_paTableList->cbData = 0;
		} // if
	} // Empty()

//	Delete all the elements in the hash table.
//
//	WARNING
//	Any derived class should overwrite this method and supply their
//	own DeleteAllElements() method, otherwise there may be a memory leak.
//
void
CHashTable::DeleteAllElements()
	{
	AssertValid();
	ForEachHashElement(S_PFn_EnumHashElementDelete);
	Empty();
	AssertValid();
	}


//	Return NULL if no element matches the string
CHashElement *
CHashTable::PFindHashElement(PSZUC pszHashKey) const
	{
	Endorse(pszHashKey == NULL);	// Under some cases, the hash key is not a pointer but some other value such as an integer which may be zero.
	Assert(m_pfnPszGetHashKey != NULL);
	//AssertValid();
	if (m_cHashElements > 0)
		{
		Assert(m_pargpHashElements != NULL);
		// Compute the hash index
		int iElement = (m_pfnUComputeHashValueForHashKey(pszHashKey) & m_mskmHashTable);
		CHashElement * pHashElement = m_pargpHashElements[iElement];
		while (pHashElement != NULL)
			{
			#ifdef DEBUG
			PSZUC pszHashKeyDebug = m_pfnPszGetHashKey(pHashElement);
			Assert(pszHashKeyDebug != NULL);
			Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
			const UINT uHashValueDebug = m_pfnUComputeHashValueForHashKey(m_pfnPszGetHashKey(pHashElement));
			Assert(uHashValueDebug == pHashElement->m_uHashValue);
			Assert((int)(pHashElement->m_uHashValue & m_mskmHashTable) == iElement);
			#endif
			if (m_pfnFCompareHashKeys(m_pfnPszGetHashKey(pHashElement), pszHashKey))
				return pHashElement;
			pHashElement = pHashElement->m_pNextHashElement;
			} // while
		} // if
	return NULL;
	} // PFindHashElement()

//	Similar to PFindHashElement(), however the method computes the hash value to be used
//	with the SetHashValue() method.  The benefit is a speed increase since there is no
//	need to compute the hash value twice when populating the table.  Most of the time, computing
//	the hash value is quick, however there may be cases where it is costly to compute it.
CHashElement *
CHashTable::PFindHashElementWithHashValue(PSZUC pszHashKey, OUT UINT * puHashValue) const
	{
	Assert(pszHashKey != NULL);
	Assert(puHashValue != NULL);
	Assert(m_pfnPszGetHashKey != NULL);

	const UINT uHashValue = m_pfnUComputeHashValueForHashKey(pszHashKey);
	*puHashValue = uHashValue;
	if (m_cHashElements > 0)
		{
		Assert(m_pargpHashElements != NULL);
		// Compute the hash index
		int iElement = (uHashValue & m_mskmHashTable);
		CHashElement * pHashElement = m_pargpHashElements[iElement];
		while (pHashElement != NULL)
			{
			#ifdef DEBUG
			Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
			const UINT uHashValueDebug = m_pfnUComputeHashValueForHashKey(m_pfnPszGetHashKey(pHashElement));
			Assert(uHashValueDebug == pHashElement->m_uHashValue);
			Assert((int)(pHashElement->m_uHashValue & m_mskmHashTable) == iElement);
			#endif
			if (m_pfnFCompareHashKeys(m_pfnPszGetHashKey(pHashElement), pszHashKey))
				return pHashElement;
			pHashElement = pHashElement->m_pNextHashElement;
			} // while
		} // if
	return NULL;
	} // PFindHashElementWithHashValue()

//	Find a hash element, and if not present, allocate it.
//	The benefit is a significant speed increase, since the hashing overhead is done only once.
//
//	This method returns the pointer to the hash element which cannot be NULL.
//	If the hash element was allocated, this method returns TRUE.
//
//	PERFORMANCE NOTES
//	Since this routine is optimized for performance, the allocated hash element is not initialized to zeroes.
//	This routine also assumes the hash element is 32-bit aligned.
//
CHashElement *
CHashTable::PFindHashElementAllocate(PSZUC pszHashKey, INOUT BOOL * pfElementNewlyAllocated)
	{
	Assert(pszHashKey != NULL);
	Assert(pfElementNewlyAllocated != NULL);
	Assert(*pfElementNewlyAllocated == FALSE);

	Assert(m_cbHashElementAndExtraEstimated > 0);
	Assert((m_cbHashElementAndExtraEstimated <= 1024) && "Hash element quite large!!! Are you sure?");
	Assert(IS_ALIGNED_32(m_cbHashElementAndExtraEstimated));
	Assert(m_paTableList == NULL || IS_ALIGNED_32(m_paTableList->cbData));	// The allocation of a hash element must always be on a 32-bit boundary.  Use AlignToUInt32() if necessary.
	Assert(m_pfnPszGetHashKey != NULL);

	if (m_cHashElements >= (int)m_mskmHashTable)
		SetHashTableSize(m_cHashElements * 2);	// Make sure there is enough room in case we need to allocate an element
	Assert(m_mskmHashTable > 0);
	Assert(m_pargpHashElements != NULL);

	// Try to find the hash element
	const UINT uHashValue = m_pfnUComputeHashValueForHashKey(pszHashKey);
	CHashElement ** ppElement = m_pargpHashElements + (uHashValue & m_mskmHashTable);
	CHashElement * pHashElementFirst = *ppElement;
	CHashElement * pHashElement = pHashElementFirst;
	while (pHashElement != NULL)
		{
		Assert(IS_ALIGNED_32(pHashElement));
		if (m_pfnFCompareHashKeys(m_pfnPszGetHashKey(pHashElement), pszHashKey))
			return pHashElement;
		pHashElement = pHashElement->m_pNextHashElement;
		} // while

	// We have not found the hash element, so allocate it
	m_cHashElements++;
	*pfElementNewlyAllocated = TRUE;

	Endorse(m_paTableList == NULL);
	pHashElement = (CHashElement *)PvGetBuffer(m_cbHashElementAndExtraEstimated);
	Assert(IS_ALIGNED_32(pHashElement));
	Assert(m_paTableList != NULL);
	m_paTableList->cbData += m_cbHashElementAndExtraEstimated;	// Mark the memory as reserved
	Assert(IS_ALIGNED_32(m_paTableList->cbData));
	pHashElement->m_uHashValue = uHashValue;

	pHashElement->m_pNextHashElement = pHashElementFirst;
	Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
	*ppElement = pHashElement;
	return pHashElement;
	} // PFindHashElementAllocate()

#ifdef DEBUG
int g_cbMemorySavedByHashTableAllocateUniqueStringsU = 0;
#endif


//	Method to allocate a unique UTF-8 string.  If the string is already in the hash table,
//	then return pointer to it, otherwise allocate a new hash element.
//
//	The implementation of this method was inspired from PFindHashElementAllocate().
PSZUC
CHashTableAllocateUniqueStringsU::PszuAllocateUniqueStringU(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	Assert(pszuString[0] != '\0');

	Assert(IS_ALIGNED_32(m_cbHashElementAndExtraEstimated));
	Assert(m_paTableList == NULL || IS_ALIGNED_32(m_paTableList->cbData));	// The allocation of a hash element must always be on a 32-bit boundary.  Use AlignToUInt32() if necessary.

	if (m_cHashElements >= (int)m_mskmHashTable)
		{
		// Remember the content of the previous hash table
		CHashElement ** pargpHashElementsOld =  m_pargpHashElements;
		CHashElement ** ppHashElementsOldLast = pargpHashElementsOld + m_mskmHashTable;

		if (m_mskmHashTable < 127)
			m_mskmHashTable = 128;
		else
			m_mskmHashTable = (m_mskmHashTable + 1) << 3;	// Multiply by 8 to grow the hash table fast enough to avoid unnecessary re-allocations
		Assert(UCountBits(m_mskmHashTable) == 1);
		m_pargpHashElements = new CHashElement *[m_mskmHashTable];	// Allocate the new bigger hash table
		InitToZeroes(OUT m_pargpHashElements, m_mskmHashTable * sizeof(CHashElement *));
		//	m_arraypHashElements.ZeroInitialize(m_mskmHashTable);	// Allocate the new hash table
		m_mskmHashTable--;	// Make the mask

		if (m_cHashElements > 0)
			{
			// Re-hash the elements
			DEBUG_CODE( m_cReHash++; )
			CHashElement ** ppHashElementsOld = pargpHashElementsOld;
			#ifdef DEBUG_WANT_ASSERT
			int cElementsDebug = 0;
			#endif
			while (ppHashElementsOld <= ppHashElementsOldLast)
				{
				CHashElement * pHashElement = *ppHashElementsOld++;
				while (pHashElement != NULL)
					{
					#ifdef DEBUG_WANT_ASSERT
					cElementsDebug++;
					#endif
					Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
					CHashElement * pHashElementNext = pHashElement->m_pNextHashElement;
					Endorse(pHashElementNext == NULL);
					Assert(pHashElement->m_uHashValue != 0);
					#ifdef DEBUG
					const UINT uHashValueDebug = UComputeHashValueFromStringCase((PSZUC)(pHashElement + 1));
					Assert(uHashValueDebug == pHashElement->m_uHashValue);
					#endif
					const int iHashElement = (pHashElement->m_uHashValue & m_mskmHashTable);
					pHashElement->m_pNextHashElement = m_pargpHashElements[iHashElement];
					m_pargpHashElements[iHashElement] = pHashElement;
					Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
					pHashElement = pHashElementNext;
					}
				} // while
			Assert(cElementsDebug == m_cHashElements);
			} // if
		delete pargpHashElementsOld;
		// Make sure there is enough room in case we need to allocate a new string
		m_cbHashElementAndExtraEstimated = sizeof(CHashElement) + 8;	// We need to set this value for PvGetBuffer().  Since a string is allocated at the end of the hash element, we can estimate about 8 bytes per string
		} // if
	Assert(m_mskmHashTable > 0);

	// Try to find the string in the hash table
	const UINT uHashValue = UComputeHashValueFromStringCase(pszuString);
	CHashElement ** ppElement = m_pargpHashElements + (uHashValue & m_mskmHashTable);
	CHashElement * pHashElementFirst = *ppElement;
	CHashElement * pHashElement = pHashElementFirst;
	while (pHashElement != NULL)
		{
		Assert(IS_ALIGNED_32(pHashElement));
		if (FCompareHashKeysStringCase((PSZUC)(pHashElement + 1), pszuString))
			{
			// The string is already there, so return pointer to it
			#ifdef DEBUG
			const int cbMemory = sizeof(CHashElement) + strlenU(pszuString) + 1;
			g_cbMemorySavedByHashTableAllocateUniqueStringsU += NEXT_UINT32(cbMemory);
			#endif
			return (PSZUC)(pHashElement + 1);
			}
		pHashElement = pHashElement->m_pNextHashElement;
		} // while


	// At this point, we have not found the string, so allocate a new one
	m_cHashElements++;
	const CHU * pchu = pszuString;
	while (*pchu++ != '\0')
		;

	const int cbString = pchu - pszuString;
	const int cbAlloc = sizeof(CHashElement) + NEXT_UINT32(cbString);

	Endorse(m_paTableList == NULL);
	pHashElement = (CHashElement *)PvGetBuffer(cbAlloc);
	Assert(IS_ALIGNED_32(pHashElement));
	Assert(m_paTableList != NULL);
	m_paTableList->cbData += cbAlloc;	// Mark the memory as reserved
	Assert(IS_ALIGNED_32(m_paTableList->cbData));
	pHashElement->m_uHashValue = uHashValue;
	pHashElement->m_pNextHashElement = pHashElementFirst;
	Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
	*ppElement = pHashElement;
	PSZU pszuStringDst = (PSZU)(pHashElement + 1);
	memcpy(OUT pszuStringDst, IN pszuString, cbString);
	return pszuStringDst;
	} // PszuAllocateUniqueStringU()


//	PFindHashElementCompare()
//
//	Search for the hash element matching a key other than the primary hash key.
//	This method is used when search speed is critical and therefore bypassing the need to collect
//	all the elements from the hash table in the array and then search the array.
//
//	The implementation of this method was inspired from the method ForEachHashElement()
//
//	PERFORMANCE NOTES
//	Although this method is the fastest to search for a match, its performance is O(n).
//
CHashElement *
CHashTable::PFindHashElementCompare(PFn_PCompareHashElement pfnCompareHashElement, LPARAM lParamKeyCompare) const
	{
	Assert(pfnCompareHashElement != NULL);
	Endorse(lParamKeyCompare == d_zNA);	// It is up to the caller to determine the key to compare
	AssertValid();
	CHashElement ** prgpHashElements = m_pargpHashElements;
	CHashElement ** prgpHashElementsLast = prgpHashElements + m_mskmHashTable;
	while (prgpHashElements <= prgpHashElementsLast)
		{
		CHashElement * pHashElement = *prgpHashElements++;
		while (pHashElement != NULL)
			{
			CHashElement * pHashElementCompare = pfnCompareHashElement(pHashElement, lParamKeyCompare);
			if (pHashElementCompare != NULL)
				{
				AssertValid();
				return pHashElementCompare;
				}
			pHashElement = pHashElement->m_pNextHashElement;
			}
		} // while
	AssertValid();
	return NULL;
	} // PFindHashElementCompare()

//	Return pointer to the hash element found, however remove it from the hash table.
CHashElement *
CHashTable::PFindHashElementAndRemove(PSZUC pszHashKey)
	{
	CHashElement * pElement = PFindHashElement(pszHashKey);
	if (pElement != NULL)
		RemoveElement(INOUT pElement);
	return pElement;
	}


#ifdef DEBUG_WANT_ASSERT
VOID
CHashTable::AssertValid() const
	{
	// Verify the integrity of the hash table
	if (FIsDebugAllowanceReachedAssertValidHashTable())
		return;

	if (m_cHashElements > 0)
		{
		Assert(m_pargpHashElements != NULL);
		int cElementsDebug = 0;
		int cBuckets = m_mskmHashTable + 1;
		for (int iBucket = 0; iBucket < cBuckets; iBucket++)
			{
			int cCollisions = 0;
			CHashElement * pHashElement = m_pargpHashElements[iBucket];
			while (pHashElement != NULL)
				{
				cElementsDebug++;
				Assert(pHashElement != pHashElement->m_pNextHashElement);	// Infinite loop
				Assert(pHashElement->m_uHashValue != 0);
				const UINT uHashValueDebug = m_pfnUComputeHashValueForHashKey(m_pfnPszGetHashKey(pHashElement));
				Assert(uHashValueDebug == pHashElement->m_uHashValue);
				const int iHashElement = (pHashElement->m_uHashValue & m_mskmHashTable);
				Assert(iHashElement == iBucket);
				pHashElement = pHashElement->m_pNextHashElement;
				cCollisions++;
				if (cCollisions >= 10)
					{
					TRACE2("  Bucket[%d] has %d collisions\n", iBucket, cCollisions);
					}
				} //if
			} // for
		Assert(cElementsDebug == m_cHashElements);
		} // if
	} // AssertValid()
#endif

#ifdef DEBUG
//	Display to the debugger the performance of the hash function
void
CHashTableCore::TraceHashPerformance(PSZAC pszHashTableName) const
	{
	Endorse(pszHashTableName == NULL);
	#ifdef TRACE_HASH_PERFORMANCE
	MessageLog_AppendTextFormatCo(coiBlack, "HashTable: #A has %D elements\n", pszHashTableName, m_cHashElements);
	#endif
	if (m_cHashElements == 0)
		return;
	int cBucketsNonEmpty = 0;
	int cBucketCollisions = 0;		// Number of buckets having collisions
	int cCollisionsMax = 0;		// Maximum number of collisions
	int cEmptyBuckets0 = 0;		// Number of even empty buckets
	int cEmptyBuckets1 = 0;		// Number of odd empty buckets
	int cCollisionsBuckets0 = 0;
	int cCollisionsBuckets1 = 0;

	for (int iBucket = 0; iBucket <= (int)m_mskmHashTable; iBucket++)
		{
		CHashElement * pHashElement = m_pargpHashElements[iBucket];
		if (pHashElement == NULL)
			{
			// The bucket is empty
			if (iBucket & 0x01)
				cEmptyBuckets1++;
			else
				cEmptyBuckets0++;
			continue;
			}
		cBucketsNonEmpty++;
		int cCollisions = 0;
		while (pHashElement != NULL)
			{
			cCollisions++;
			pHashElement = pHashElement->m_pNextHashElement;
			} // while
		if (cCollisions > 1)
			{
			cBucketCollisions++;
			if (iBucket & 0x01)
				cCollisionsBuckets1++;
			else
				cCollisionsBuckets0++;
			}
		if (cCollisions > 3)
			{
			// Display buckets of 3 or more collisions.  Displaying buckets of two or less is a bit redundant
			#ifdef TRACE_HASH_PERFORMANCE
			MessageLog_AppendTextFormatCo((cCollisions >= 5) ? coOrange : coDarkGrey, "\t Bucket[%D] has %d elements (collisions).\n", iBucket, cCollisions);
			#endif
			}
		if (cCollisions > cCollisionsMax)
			cCollisionsMax = cCollisions;
		} // for

	float flPerformance = (float)m_cHashElements / cBucketsNonEmpty;
	Assert(flPerformance >= 1);
	#ifdef TRACE_HASH_PERFORMANCE
	MessageLog_AppendTextFormatCo(coiBlack, "Performance: O(%f)  [cBucketCollisions=%D of cBuckets=%D]  Largest number of collisions=%d\n", flPerformance, cBucketCollisions, m_mskmHashTable + 1, cCollisionsMax);
	MessageLog_AppendTextFormatCo(coiBlack, "\tAllocation Efficiency: %f%%  [%D elements for %D buckets]\n", (float)m_cHashElements * 100 / (m_mskmHashTable + 1), m_cHashElements, m_mskmHashTable + 1);
	MessageLog_AppendTextFormatCo(coiBlack, "\tEmpty Statistics: cEven=%D, cOdd=%D\n", cEmptyBuckets0, cEmptyBuckets1);
	MessageLog_AppendTextFormatCo(coiBlack, "\tCollision Statistics: cEven=%D, cOdd=%D\n", cCollisionsBuckets0, cCollisionsBuckets1);
	#endif
	} // TraceHashPerformance()

//	Display in the debugger detailed information about the memory allocated by the hash table.
VOID
CHashTableCore::TraceHashMemoryAllocations(PSZAC pszHashTableName) const
	{
	int cMemoryBlocks = 0;
	int cbAllocTotal = 0;
	int cbDataTotal = 0;

	// Calculate some vital statistics
	THashElementTableHdr * pTable = m_paTableList;
	while (pTable != NULL)
		{
		cMemoryBlocks++;
		cbAllocTotal += pTable->cbAlloc;
		cbDataTotal += pTable->cbData;
		pTable = pTable->pNext;
		} // while
	int nPercentFree = 0;
	if (cbAllocTotal > 0)
		nPercentFree = 100 - ((cbDataTotal * 100) / cbAllocTotal);
	TRACE6("HashTable: %s has %d blocks with %d%% (%d bytes) free (%d data bytes of %d allocated bytes)\n",
		pszHashTableName, cMemoryBlocks, nPercentFree, cbAllocTotal - cbDataTotal, cbDataTotal, cbAllocTotal);

	// Display detailed information
	int iBlock = 0;
	pTable = m_paTableList;
	while (pTable != NULL)
		{
		cbAllocTotal = pTable->cbAlloc;
		cbDataTotal = pTable->cbData;
		nPercentFree = 100 - ((cbDataTotal * 100) / cbAllocTotal);
		TRACE5("\t[%d] %d%% (%d bytes) free (%d data bytes of %d allocated bytes)\n",
			++iBlock, nPercentFree, cbAllocTotal - cbDataTotal, cbDataTotal, cbAllocTotal);
		pTable = pTable->pNext;
		} // while

	} // TraceHashMemoryAllocations()

#endif // DEBUG_WANT_ASSERT


/////////////////////////////////////////////////////////////////////
//	S_PFn_EnumHashElementGetAll(), static
//
//	Accumulate each hash element into parraypHashElements.
void
CHashTable::S_PFn_EnumHashElementGetAll(CHashElement * pHashElement, INOUT CArrayPtrHashElement * parraypHashElements)
	{
	Assert(pHashElement != NULL);
	Assert(parraypHashElements != NULL);
	parraypHashElements->Add(pHashElement);
	}

/////////////////////////////////////////////////////////////////////
//	S_PFn_EnumHashElementDelete(), static
//
//	Delete each hash element.
//
//	REMARKS
//	Any hash element derived from CHashElement may use this method as long
//	as the derived hash element does not require a destructor.
//	This method should be used carefully.  It is preferable the derived
//	class provide its own delete routine.
//
void
CHashTable::S_PFn_EnumHashElementDelete(CHashElement * pHashElement, LPARAM UNUSED_PARAMETER(lParam))
	{
	Assert(pHashElement != NULL);
	UNUSED_PARAMETER(lParam);		// The delete function does not need a context lParam, however since the interface PFn_EnumHashElement() requires a lParam, this parameter must be in the prototype of the method.
	delete pHashElement;
	}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//	Return TRUE if the two strings are identical.
//	This routine is nearly identical to strcmp() but returns a boolean value instead
BOOL
FCompareHashKeysStringCase(PSZUC pszHashKey1, PSZUC pszHashKey2)
	{
	Assert(pszHashKey1 != NULL);
	Assert(pszHashKey2 != NULL);

	while (TRUE)
		{
		const UINT ch1 = *pszHashKey1++;
		if (ch1 != (UINT)*pszHashKey2++)
			return FALSE;
		if (ch1 == '\0')
			return TRUE;
		}
	}

BOOL
FCompareHashKeysStringNoCase(PSZUC pszHashKey1, PSZUC pszHashKey2)
	{
	Assert(pszHashKey1 != NULL);
	Assert(pszHashKey2 != NULL);

	while (TRUE)
		{
		UINT ch1 = *pszHashKey1++;
		if (ch1 >= 'A' && ch1 <= 'Z')
			ch1 += 32;	// Make lowercase
		UINT ch2 = *pszHashKey2++;
		if (ch2 >= 'A' && ch2 <= 'Z')
			ch2 += 32;	// Make lowercase
		if (ch1 != ch2)
			return FALSE;
		if (ch1 == '\0')
			return TRUE;
		}
	}


BOOL
FCompareHashKeysStringUnicodeCase(PSZWC pszwHashKey1, PSZWC pszwHashKey2)
	{
	Assert(pszwHashKey1 != NULL);
	Assert(pszwHashKey2 != NULL);

	while (TRUE)
		{
		UINT ch1 = *pszwHashKey1++;
		if (ch1 != (UINT)*pszwHashKey2++)
			return FALSE;
		if (ch1 == '\0')
			return TRUE;
		}
	}

/*
BOOL
FCompareHashKeysStringUnicodeNoCase(PSZWC pszwHashKey1, PSZWC pszwHashKey2)
	{
	Assert(pszwHashKey1 != NULL);
	Assert(pszwHashKey2 != NULL);
	return (StringCompareNoCaseW(pszwHashKey1, pszwHashKey2) == 0);
	}
*/

BOOL
FCompareHashKeysStringUnicodeNoCaseAnsi(PSZWC pszwHashKey1, PSZWC pszwHashKey2)
	{
	Assert(pszwHashKey1 != NULL);
	Assert(pszwHashKey2 != NULL);

	while (TRUE)
		{
		UINT ch1 = *pszwHashKey1++;
		if (ch1 >= 'A' && ch1 <= 'Z')
			ch1 += 32;	// Make lowercase
		UINT ch2 = *pszwHashKey2++;
		if (ch2 >= 'A' && ch2 <= 'Z')
			ch2 += 32;	// Make lowercase
		if (ch1 != ch2)
			return FALSE;
		if (ch1 == '\0')
			return TRUE;
		}
	}


BOOL
FCompareHashKeysUInt32(UINT uHashKeyA, UINT uHashKeyB)
	{
	return (uHashKeyA == uHashKeyB);
	}

BOOL
FCompareHashKeysHashSha1(const SHashSha1 * pHashA, const SHashSha1 * pHashB)
	{
	Assert(pHashA != NULL);
	Assert(pHashB != NULL);
	return (memcmp(pHashA, pHashB, sizeof(SHashSha1)) == 0);
	}


/////////////////////////////////////////////////////////////////////
//	Return a hash value for a string.  The string comparison is case sensitive.
//
//	To guarantee the value is never zero, the constant d_kfHashValueNonZero is ORed
//	with the computed hash value.
//
//	INTERFACE NOTES
//	This routine has the same interface as PFn_UComputeHashValueForHashKey()
//
UINT
UComputeHashValueFromStringCase(PSZUC pszHashKey)
	{
	Assert(pszHashKey != NULL);
	Assert(strlenU(pszHashKey) > 0);
	UINT uHashValue = 0;
	while (TRUE)
		{
		// 431 elements: Performance: O(1.227920) [cBucketCollisions=69 of cBucket=1024] (1 collision has 4 strings per bucket, 5 collisions has 3 strings per bucket, all others have 2 strings per bucket)
		// 177 elements: Performance: O(1.041176) [cBucketCollisions=7 of cBucket=1024]  (all collisions have 2 strings per bucket)
		UINT ch = (BYTE)*pszHashKey++;
		if (ch == '\0')
			break;
		uHashValue = (uHashValue << 5) + uHashValue + ch;
		} // while
	/*
	while (TRUE)
		{
		// This hash function is not good
		// 431 elements: O(1.596296) [cBucketCollisions=85 of cBucket=1024] (3 buckets with more than 10 strings, and many collisions with more than 5 strings)
		// 177 elements: O(1.320896) [cBucketCollisions=22 of cBucket=1024] (5 buckets with more than 5 strings)
		BYTE ch = *pszHashKey++;
		if (ch == '\0')
			break;
		if (ch >= 96)
			ch -= 32;
		uHashValue = (uHashValue << 3) + (uHashValue >> 23) + ch;
		} // while
	*/
	/*
	while (TRUE)
		{
		// Java hash function (WORSE of all)
		// 431 elements: O(4.489583) [cBucketCollisions=59 of cBucket=1024] (up to 55 strings per bucket)
		// 177 elements: O(2.901639) [cBucketCollisions=29 of cBucket=1024] (up to 23 strings per bucket)
		BYTE ch = *pszHashKey++;
		if (ch == '\0')
			break;
		uHashValue += (uHashValue * 31) + ch;
		}
	*/
	/*
	// Defines the so called `hashpjw' function by P.J. Weinberger
	// [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
	// 1986, 1987 Bell Telephone Laboratories, Inc.]
	#define HASHWORDBITS 32		// We assume to have `unsigned long int' value with at least 32 bits.
	unsigned long int hval, g;
	const char * str = pszHashKey;
	hval = 0;
	while (*str != '\0')
		{
		// 431 Elements: O(1.873913) [cBucketCollisions=84 of cBucket=1024] (many times up to 10 strings per bucket)
		// 177 Elements: O(1.372093) [cBucketCollisions=30 of cBucket=1024] (up to 6 strings per bucket)
		hval <<= 4;
		hval += (unsigned long int) *str++;
		g = hval & ((unsigned long int) 0xf << (HASHWORDBITS - 4));
		if (g != 0)
			{
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
			}
		}
	uHashValue = hval;
	*/

	return (uHashValue | d_kfHashValueNonZero);
	} // UComputeHashValueFromStringCase()


//	Compute the hash value for a string, ignoring the case of each character.
//	In short, two strings with different case will have the same hash value.
//
//	This routine is nearly identical to UComputeHashValueFromStringCase(),
//	but will treat all characters of the alphabet as identical.  Punctuation and accents
//	are not expected in this algorithm.
//
//	USAGE
//	This routine is ideal for hashing alphanumeric strings (such as IDs) or raw UTF-8 strings.
//
UINT
UComputeHashValueFromStringNoCase(PSZUC pszHashKey)
	{
	Assert(pszHashKey != NULL);
	Assert(strlenU(pszHashKey) > 0);
	UINT uHashValue = 0;
	while (TRUE)
		{
		UINT ch = (BYTE)*pszHashKey++;
		if (ch >= 'A' && ch <= 'Z')
			ch += 32;	// Make lowercase
		uHashValue = (uHashValue << 5) + uHashValue + ch;
		if (ch == '\0')
			return (uHashValue | d_kfHashValueNonZero);
		} // while
	}


//	Compute the hash value for a Unicode string.
//	It is important to notice this routine is case sensitive, because
//	a lowercase in Unicode is a bit tricky.
//
//	USAGE
//	This routine is mostly used by the Gui to populate a combo with
//	unique unicode strings (for instance, a list of all the last names).
//	To the user, it is important to display all unique strings, and if
//	two strings differ by case, then they are different.
//
UINT
UComputeHashValueFromStringUnicodeCase(PSZWC pszwHashKey)
	{
	Assert(pszwHashKey != NULL);
	Endorse(pszwHashKey[0] == '\0');		// It is not recommended, but it may happen and it is not a big deal.
	UINT uHashValue = 0;
	while (TRUE)
		{
		UINT chw = *pszwHashKey++;
		uHashValue = (uHashValue << 5) + uHashValue + chw;
		if (chw == '\0')
			return (uHashValue | d_kfHashValueNonZero);
		} // while
	}

/*
UINT
UComputeHashValueFromStringUnicodeNoCase(PSZWC pszwHashKey)
	{
	Assert(pszwHashKey != NULL);
	Assert(strlenW(pszwHashKey) > 0);
	int cbHashKey = CbAllocWtoW(pszwHashKey);
	PSZW pszwStringLowerCase = (PSZW)alloc_stack(cbHashKey);
	StringLowerCaseW(OUT pszwStringLowerCase, IN cbHashKey / sizeof(CHW), pszwHashKey);
	return UComputeHashValueFromStringUnicodeCase(IN pszwStringLowerCase);
	}
*/

UINT
UComputeHashValueFromStringUnicodeNoCaseAnsi(PSZWC pszwHashKey)
	{
	Assert(pszwHashKey != NULL);
	Assert(pszwHashKey[0] != '\0');
	UINT uHashValue = 0;
	while (TRUE)
		{
		UINT ch = *pszwHashKey++;
		if (ch >= 'A' && ch <= 'Z')
			ch += 32;	// Make lowercase
		uHashValue = (uHashValue << 5) + uHashValue + ch;
		if (ch == '\0')
			return (uHashValue | d_kfHashValueNonZero);
		} // while
	}

//	Compute a hash value from a 32-bit value.  The 32-bit value may be a RGB color (COLORREF) or an IPv4 address.
//	Considering the function hashing a Unicode string has given very good results, we will
//	pretend uHashKey is a string containing only two Unicode characters.
UINT
UComputeHashValueFromUInt32(UINT uHashKey)
	{
	Assert(uHashKey != 0);
	UINT uHashValue = (uHashKey & 0xFFFF);
	return ((uHashValue << 5) + uHashValue + (uHashKey >> 16)) | d_kfHashValueNonZero;
	}

UINT
UComputeHashValueFromIntegerIndex(UINT iHashKey)
	{
	Assert(iHashKey != 0);
	return iHashKey;
	}

UINT
UComputeHashValueFromHashSha1(const SHashSha1 * pHash)
	{
	Assert(pHash != NULL);
	return *(UINT *)pHash | d_kfHashValueNonZero;
	}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//	Return TRUE if a new entry was added to the hash table.
//	Return FALSE if the key was already in the hash table.
BOOL
CHashTablePszaPv::FAddedKeyUnique(PSZAC pszKey, PCVOID pvValue)
	{
	Assert(pszKey != NULL);
	Assert(*pszKey != '\0');
	Endorse(pvValue == NULL);
	CHashElementPszaPv * pHashElement = (CHashElementPszaPv *)PFindHashElement((PSZUC)pszKey);
	if (pHashElement != NULL)
		return FALSE;	// Already in the hash table
	AddKey(pszKey, pvValue);
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CHashTableUniqueUnicodeStringsNoCase::CHashTableUniqueUnicodeStringsNoCase() : CHashTable((PFn_PszGetHashKey)S_PszwGetHashKeyUniqueString, eHashFunctionStringUnicodeNoCaseAnsi)
	{
	SetHashElementSize(sizeof(CHashElementUniqueUnicodeStringsNoCase));
	}

/*
// Add a unique unicode string to the hash table.
// If the string was already there, return NULL,
// otherwise copy the string to the hashtable's allocator and return pointer to it.
PSZWC
CHashTableUniqueUnicodeStringsNoCase::PszwAddUniqueStringW(PSZWC pszwString)
	{
	Assert(pszwString != NULL);
	Assert(*pszwString != '\0');
	if (PFindHashElement((PSZUC)pszwString) != NULL)
		return NULL;
	int cbString = CbAllocWtoW(pszwString);
	CHashElementUniqueUnicodeStringsNoCase * pHashElement = (CHashElementUniqueUnicodeStringsNoCase *)PAllocateHashElement(cbString);
	PSZW pszwStringUnique = pHashElement->PszwGetString();
	memcpy(OUT pszwStringUnique, IN pszwString, cbString);
	Add(pHashElement);	// Add the string to the hash table
	return pszwStringUnique;
	}
*/

