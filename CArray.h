/////////////////////////////////////////////////////////////////////
//	CArray.h
//
//	Storage for efficient high-perforformance dynamic array with wrappers for commonly used array operations.
/////////////////////////////////////////////////////////////////////

#ifndef CARRAY_H
#define CARRAY_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
/*
#define ARRAY_SORT
#ifdef ARRAY_SORT
	#include "Sort.h"
#endif
*/
/////////////////////////////////////////////////////////////////////
//	class CArray
//
//	Base class to manipulare an array of pointers.
//
//	To make the code easier to write with the auto-complete of modern IDEs, the return types are at the end of the method rather than at the beginning,
//	so the methods are sorted alphabetically by the IDE and making it easier to find a method by functionality.
//	For instance, if a method returns an index, the method will be named FindElementI() rather than IFindElement() as it would be in the rest of the code.  After all, almost all the
//	methods for the CArray will return pointer or an integer, so the possible return types are quite limited.
//
class CArray
{
protected:
	struct SHeaderOnly
		{
		int cElements;			// Number of elements in array
		int cElementsAlloc;		// Number of allocated elements
		};
	struct SHeaderWithData : public SHeaderOnly
		{
		const void * rgpvData[d_nLengthArrayDebug];	// Array of pointers (the true length of the array is determined by cElements)
		};

	SHeaderWithData * m_paArrayHdr;		// Pointer to allocated array

public:
	inline CArray() { m_paArrayHdr = NULL; }
	inline CArray(const CArray & array) { m_paArrayHdr = NULL; Copy(&array); }
	~CArray() { if (m_paArrayHdr != NULL) delete m_paArrayHdr; }

	void ** PrgpvGetElementsStop(OUT void *** pppvElementStop) const;
	inline const void ** PrgpvGetElementsUnsafe() const { return m_paArrayHdr->rgpvData; }	// For performance, when you are sure the array has been allocated
	void ** PrgpvGetElements() const;
	//void ** PrgpvGetElements();
	void ** PrgpvGetElements(OUT int * pcElements) const;
	void KeepMaximumElements(int cElementsMax);
	inline void SetSizeTruncate(int cElements);
	inline void SetSizeTruncateAt(const void ** ppvDataEnd);
	void SetSizeGrow(int cElements, BOOL fInitializeWithZeroes);
	void ZeroInitialize(int cElements);
	inline int GetSize() const;
	inline void * PvGetElementAt(int iElement) const;
	void * PvGetElementAtSafe_YZ(int iElement) const;
	void * PvGetElementAtSafeModuloSize_YZ(UINT iElement) const;
	void * PvGetElementAtSafeRandom_YZ() const;
	void * PvGetElementFirst_YZ() const;
	void * PvGetElementLast_YZ() const;
	void * PvGetElementUnique_YZ() const;

	void ** PrgpvAllocateElementsSetSize(int cElements);
	void ** PrgpvAllocateElementsEmpty(int cElementsAlloc);
	void ** PrgpvAllocateElementsEmpty(const CArray * parray);
	void ** PrgpvAllocateElementsAppend(int cElementsAllocGrowBy);
	void RemoveAllElements();
	void FreeExtra();

	int Add(const void * pvElement);
	int AddReplaceNULL(const void * pvElement);
	void AddMRU(const void * pvElement, int cElementsMax);
	BOOL AddUniqueF(const void * pvElement);
	void RemoveAllAndAdd(const void * pvElement);
	void InsertElementAtHead(const void * pvElement);
	void InsertElementAtHeadFast(const void * pvElement);
	void InsertElementAt(int iElement, const void * pvElement);
	void InsertElementsAtHead(int cElementsInsert);
	inline void SetAt(int iElement, const void * pvElement);
	inline void SetAtNULL(int iElement) { SetAt(iElement, NULL); }
	void SetAtGrowOptional(int iElement, const void * pvElement);
	void SetAtGrow(int iElement, const void * pvElement);

	void RemoveElementAt(int iElement);
	void RemoveElementAtPpv(const void ** ppvElement);
	void RemoveElementAtFast(int iElement);
	void RemoveElementAtPpvFast(const void ** ppvElement);
	void * RemoveElementFirstPv();
	int ReplaceElementI(const void * pvElementOld, const void * pvElementNew);
	int RemoveElementI(const void * pvElement);
	BOOL RemoveElementFastF(const void * pvElement);
	inline int RemoveElementAssertI(const void * pvElement);
	inline BOOL RemoveElementFastAssertF(const void * pvElement);
	int RemoveElementAllInst(const void * pvElement);
	int RemoveElementAllInstNULL() { return RemoveElementAllInst(NULL); }
	UINT RemoveTailingNULLsU();
	void ElementTransferFrom(const void * pvElementTransfer, INOUT CArray * pArraySrc);

	VOID InsertAtHead(const CArray * pArrayInsert);
	void Append(const CArray * pArrayAppend);
	void AppendUnique(const CArray * pArrayAppend);
	void Copy(const CArray * pArraySrc);
	void CopyStealFromSource(INOUT CArray * pArraySrc);
	void AppendStealFromSource(INOUT CArray * pArraySrc);
	VOID Swap(INOUT CArray * pArraySrc);
	VOID SetDataRaw(PCVOID prgpvData[], int cElements);
	VOID BuildArrayFromArray(PCVOID prgArray, int cbElement, int cElements);
	VOID BuildArrayFromArrayZ(PCVOID prgzArray, int cbElement);
	void operator = (const CArray & arraySrc);

	// Stack Interface (use an array to implement a stack)
	inline BOOL FIsEmpty() const { return (m_paArrayHdr == NULL || m_paArrayHdr->cElements == 0); }
	inline BOOL FHasUniqueElement() const { return (m_paArrayHdr != NULL && m_paArrayHdr->cElements == 1); }
	inline int StackPushElement(const void * pvElement) { return Add(pvElement); }
	void * PvStackPopElement();
	void StackPopElement();
	//void * PvStackGetTopELement(int iElement = 0) const;

	void * PFindNextElementFromCircularRing(const void * pvElement) const;
	void * PFindPrevElementFromCircularRing(const void * pvElement) const;
	int FindElementI(const void * pvElement) const;
	inline BOOL FindElementF(const void * pvElement) const;
	BOOL FAllElementsNULL() const;
	BOOL FHasNonNullPointer(int iElement, ...) const;
	UINT CountNonNullPointers() const;
	UINT UFindRemainingElements(const void * pvElement) const;

	BOOL FIsContentIdentical(const CArray * pArrayCompare) const;
	void Reverse(int iElementFirst = 0, int cElementsReverse = -1);
	int ShiftElementBy(int iElement, int diShift);

	// Sorting
	ESortOrder EGetSortedOrder(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA) const;
	BOOL FIsSortedAscending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA) const;
	BOOL FIsSortedDescending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA) const;
	void DoSortAscending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void DoSortDescending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void DoSort(BOOL fSortAscending, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void Sort(BOOL fSortAscending, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void Sort(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void SortNeverReverse(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void ToggleSort(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	//VOID SortByStringsW(PFn_PszwGetString pfnPszwGetString);
	//VOID SortByKey(PFn_PvAllocateSortKey pfnPvAllocateSortKey, PFn_NCompareSortElements pfnCompareSort);
	void MergeAppend(const void * pvElement, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	void MergeAppend(const CArray * pArrayAppend, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);
	int BinarySearch(const void * pvElement, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare = d_zNA);

private:
	static SHeaderWithData * S_PaAllocateElements(int cElementsAlloc);
}; // CArray


//	The following routines are inline to improve performance

/////////////////////////////////////////////////////////////////////
//	GetSize()
//
//	Return the number of elements stored in the array
int
CArray::GetSize() const
	{
	if (m_paArrayHdr == NULL)
		return 0;
	Assert(m_paArrayHdr->cElements <= m_paArrayHdr->cElementsAlloc);
	return m_paArrayHdr->cElements;
	}

//	Set the size by truncating the array
void
CArray::SetSizeTruncate(int cElements)
	{
	if (m_paArrayHdr != NULL)
		{
		Assert(cElements <= m_paArrayHdr->cElements);
		m_paArrayHdr->cElements = cElements;
		}
	}

//	Truncate the array at a given position
void
CArray::SetSizeTruncateAt(const void ** ppvDataEnd)
	{
	Endorse(ppvDataEnd == NULL);	// The array is already empty
	#ifdef DEBUG
	if (m_paArrayHdr == NULL)
		{
		Assert(ppvDataEnd == NULL);
		}
	#endif
	if (m_paArrayHdr != NULL)
		{
		Assert((ppvDataEnd - m_paArrayHdr->rgpvData) <= m_paArrayHdr->cElementsAlloc);
		m_paArrayHdr->cElements = (ppvDataEnd - m_paArrayHdr->rgpvData);
		Assert(m_paArrayHdr->cElements >= 0);
		Assert(m_paArrayHdr->cElements <= m_paArrayHdr->cElementsAlloc);
		}
	}

/////////////////////////////////////////////////////////////////////
//	PvGetElementAt()
//
//	Return the element at a given index.
//
void *
CArray::PvGetElementAt(int iElement) const
	{
	Assert(m_paArrayHdr != NULL);
	Assert(m_paArrayHdr->cElements <= m_paArrayHdr->cElementsAlloc);
	Assert(iElement >= 0 && iElement < m_paArrayHdr->cElements);
	return (void *)m_paArrayHdr->rgpvData[iElement];
	}

/////////////////////////////////////////////////////////////////////
//	SetAt()
//
//	Sets the array element at the specified index.
//
//	Method SetAt() does not cause the array to grow.
//	Use SetAtGrow() if you want the array to grow automatically.
//
void
CArray::SetAt(int iElement, const void * pvElement)
	{
	Assert(m_paArrayHdr != NULL);
	Assert(iElement >= 0 && iElement < m_paArrayHdr->cElements);
	m_paArrayHdr->rgpvData[iElement] = pvElement;
	}

//	Return TRUE if the element is found in the array
BOOL
CArray::FindElementF(const void * pvElement) const
	{
	return (FindElementI(pvElement) >= 0);
	}

/////////////////////////////////////////////////////////////////////
//	Same as RemoveElementI() but asserts if the element was not in the array
int
CArray::RemoveElementAssertI(const void * pvElement)
	{
	const int iElementRemoved = RemoveElementI(pvElement);
	Assert(iElementRemoved >= 0);
	return iElementRemoved;
	}

BOOL
CArray::RemoveElementFastAssertF(const void * pvElement)
	{
	const BOOL fElementRemoved = RemoveElementFastF(pvElement);
	Assert(fElementRemoved == TRUE);
	return fElementRemoved;
	}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//	Array of pointers to UTF-8 strings.
class CArrayPsz : public CArray
{
public:
	inline PSZUC * PrgpszGetStringsStop(OUT PSZUC ** pppszStringStop) const { return (PSZUC *)PrgpvGetElementsStop(OUT (void ***)pppszStringStop); }
	inline PSZUC PszGetAt(int iString) const { return (PSZUC)PvGetElementAt(iString); }
	void GetStrings(OUT CStr * pstrStrings, PSZAC pszaSeparator) const;
	int FindStringI(PSZUC pszStringSearch) const;
//	inline void Sort(BOOL fSortAscending = TRUE) { CArray::Sort(fSortAscending, (PFn_NCompareSortElements)SortCompareStrings); }
};

/////////////////////////////////////////////////////////////////////
//	Array of pointer of allocated UTF-8 strings
//
//	This class is optimized to reduce the memory requirements (allocations) to store the strings
//	by storing NULL for empty strings
//
class CArrayPasz : public CArrayPsz
{
public:
	~CArrayPasz();
	/*
	inline operator const CArrayPsz & () const { return *this; }	// This line is supposed to cast from CArrayPasz to CArrayPsz however it does not work
	inline BOOL FIsEmpty() const { return CArray::FIsEmpty(); }
	inline int GetSize() const { return CArray::GetSize(); }
	inline PSZU PszGetAt(int iString) const { return (PSZU)PvGetElementAt(iString); }
	inline PSZU PszGetAtSafe(int iString) const { return (PSZU)PvGetElementAtSafe_YZ(iString); }
	*/
	int AddStringAllocate(PSZUC pszString);
	int AddStringAllocate(const QString & sString);
	int AddStringAllocateUnique(const QString & sString);
	void AddStringsUniqueStealFromSource(INOUT CArrayPasz * parraypaszSource);
	void DeleteAllStrings();

//	inline void Sort(BOOL fSortAscending = TRUE) { CArray::Sort(fSortAscending, (PFn_NCompareSortElements)SortCompareStrings); }
};


#endif // CARRAY_H
