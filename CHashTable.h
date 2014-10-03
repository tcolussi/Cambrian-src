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
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHASHTABLE_H
#define CHASHTABLE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CHashTableBackup;
class CHashTableCore;
class CHashTable;
class CHashTableAllocateUniqueStringsU;

/////////////////////////////////////////////////////////////////////
//	Class for a single hash element.
//
//	All hash elements mapping to the same hash index linked together
//
//	INTERFACE NOTES
//	All the fields in this base class are private, however it is permitted
//	to initialize them to zeroes using InitToZeroes().
//
class CHashElement
{
private:
	UINT m_uHashValue;					// Hash value of the hash element (this value is cached to avoid recomputing the hash value each time we wish to compare hash elements)
	CHashElement * m_pNextHashElement;	// Pointer to the next hash element when a collision occurs

public:
	inline CHashElement() { m_uHashValue = 0; }
	inline void ClearHashValue() { m_uHashValue = 0; }
	inline VOID SetHashValue(UINT uHashValue) { m_uHashValue = uHashValue; }
	inline CHashElement * PGetNextHashElementInSameBucket() const { return m_pNextHashElement; }

	#ifdef DEBUG
	private:
	void operator = (const CHashElement & UNUSED_PARAMETER(oHashElement)) { Assert(FALSE && "This operation has no meaning!!!!"); }
	#endif

protected:
	// This method is used only when the hash value is the index.  It is used in rare cases for finest optimization.
	inline UINT _UGetHashValue() const { return m_uHashValue; }

	friend class CHashTableCore;
	friend class CHashTable;
	friend class CHashTableAllocateUniqueStringsU;
}; // CHashElement

class CArrayPtrHashElement : public CArray
{
public:
	inline CHashElement * PGetAt(int iElement) const { return (CHashElement *)PvGetElementAt(iElement); }
};

/////////////////////////////////////////////////////////////////////
//	Return the pointer to the key of the hash element.
//	Since the majority of hashing will done with strings, this interface returns a string.
//	In the case of other hask keys, such as a 128-bit GUID, the interface will return
//	a pointer to a string, however the comparison routine PFn_FCompareHashKeys() will understand
//	the keys are GUIDs.
//
//	The return value cannot be NULL.
//
typedef PSZUC (* PFn_PszGetHashKey)(const CHashElement * pHashElement);


/////////////////////////////////////////////////////////////////////
//	Interface to compare two hash keys.
//
//	Return TRUE if the two hash keys are identical, otherwise return FALSE.
//	The hash key pszHashKey is typically a string returned by PFn_PszGetHashKey()
//	however may be something else, such as a 128-bit GUID.  What is important is
//	the compare routines understand what they are comparing.
//
typedef BOOL (* PFn_FCompareHashKeys)(PSZUC pszHashKey1, PSZUC pszHashKey2);
	BOOL FCompareHashKeysStringCase(PSZUC pszHashKey1, PSZUC pszHashKey2);
	BOOL FCompareHashKeysStringNoCase(PSZUC pszHashKey1, PSZUC pszHashKey2);
	BOOL FCompareHashKeysStringUnicodeCase(PSZWC pszwHashKey1, PSZWC pszwHashKey2);
	BOOL FCompareHashKeysStringUnicodeNoCase(PSZWC pszwHashKey1, PSZWC pszwHashKey2);
	BOOL FCompareHashKeysStringUnicodeNoCaseAnsi(PSZWC pszwHashKey1, PSZWC pszwHashKey2);
	BOOL FCompareHashKeysUInt32(UINT uHashKeyA, UINT uHashKeyB);
	BOOL FCompareHashKeysHashSha1(const SHashSha1 * pHashA, const SHashSha1 * pHashB);

/////////////////////////////////////////////////////////////////////
//	Return a hash value for the hash key.  The hash key pszHashKey is typically
//	a string returned by PFn_PszGetHashKey() however may be anything else hashable.
//
//	The returned value cannot be zero.  It is the responsibility of routine implementing this
//	interface to ensure the value is NEVER zero.
//
typedef UINT (* PFn_UComputeHashValueForHashKey)(PSZUC pszHashKey);
	UINT UComputeHashValueFromStringCase(PSZUC pszHashKey);
	UINT UComputeHashValueFromStringNoCase(PSZUC pszHashKey);
	UINT UComputeHashValueFromStringUnicodeCase(PSZWC pszwHashKey);
	UINT UComputeHashValueFromStringUnicodeNoCase(PSZWC pszwHashKey);
	UINT UComputeHashValueFromStringUnicodeNoCaseAnsi(PSZWC pszwHashKey);
	UINT UComputeHashValueFromUInt32(UINT uHashKey);
	UINT UComputeHashValueFromIntegerIndex(UINT iHashKey);
	UINT UComputeHashValueFromHashSha1(const SHashSha1 * pHash);

#define d_kfHashValueNonZero	0x10000000	// To guarantee the the routine UComputeHashValueFrom*() never returns zero, this constant is ORed with the computed hash value.

/////////////////////////////////////////////////////////////////////
//	Routine to enumerate the hash elements in the hash table.
//
//	This callback routine can be used to store the hash elements
//	in an array or delete the hash table.
//
typedef void (* PFn_EnumHashElement)(CHashElement * pHashElement, LPARAM lParam);

//	Routine to compare a hash element with a given key.
//	Return a pointer/handle of if the hash element matches the key, otherwise return NULL.
//	The caller is responsible to interpret the meaning of the returned pointer/handle.
typedef CHashElement * (* PFn_PCompareHashElement)(CHashElement * pHashElement, LPARAM lParamKeyCompare);

//	The hash table is a block of memory to store consecutive CHashElement.
//	The idea of using a single block of memory is to reduce the number of
//	memory allocations required for each CHashElement.
struct THashElementTableHdr
	{
	THashElementTableHdr * pNext;	// Pointer to the next table (if any)
	int cbAlloc;		// Number of bytes allocated after the header
	int cbData;			// Number of bytes in the buffer after the header
	};



/////////////////////////////////////////////////////////////////////
//	Core hash table without the pointer to routines to hash functions.
//	This hash table can only allocate hash elements without knowing what they are.
//
class CHashTableCore
{
private:
	UINT m_mskmHashTable;						// Mask to compute the hash index
	CHashElement ** m_pargpHashElements;		// Array of pointers of all the hash elements

	int m_cbHashElementAndExtraEstimated;		// Size of a single hash element.  The high-word contains the estimated number of bytes allocated for the hash element.
	THashElementTableHdr * m_paTableList;		// List of pre-allocated block of memory to hold the CHashElements. This field is valid only if m_cbHashElement is non-zero.  The implementation of this allocation is almost identical to class CMemoryAccumulator.
	#ifdef DEBUG
	int m_cReHash;								// Number of time the the elements have been re-hashed.  This is useful to determine the performance of the hash table.
	#endif

protected:
	int m_cHashElements;						// Number of hash elements in the table

public:
	CHashTableCore();
	~CHashTableCore();
	BOOL FIsHashTableSizeInitialized() const { return (m_mskmHashTable != 0); }
	void SetHashElementSize(int cbHashElement, int cbExtraEstimated = 0);
	void BindToStaticBuffer(INOUT void * pvBuffer, int cbBuffer);
	void UnbindStaticBuffer();

	void * PvGetBuffer(int cbBuffer);
	void CommitBuffer(int cbBuffer);
	void * PvAllocateData(int cbAlloc);
	void * PvAllocateDataCopy(int cbAlloc, PCVOID pvDataSrcCopy);
	PSZWC PszwAllocateStringW(PSZWC pszwSrcCopy);
	PSZUC PszuAllocateStringU(PSZUC pszuSrcCopy);
	void AlignToUInt32();

	void RemoveAll();
	int GetElementsCount() const { return m_cHashElements; }

	#ifdef DEBUG
	void TraceHashPerformance(PSZAC pszHashTableName) const;
	VOID TraceHashMemoryAllocations(PSZAC pszHashTableName) const;
	#endif

protected:
	void _AllocateTableList(int cbAlloc);
	VOID _DestroyTableList();

private:
	VOID _BackupCreate(INOUT CHashTableBackup * pHashTableBackup);
	VOID _BackupRestore(const CHashTableBackup * pHashTableBackup);
	friend class CHashTableBackup;
	friend class CHashTable;
	friend class CHashTableAllocateUniqueStringsU;
}; // CHashTableCore

//	Class to backup the state of a hash table.
//	This class is used on the stack when the same hash table can be re-used multiple times.
class CHashTableBackup
{
private:
	CHashTableCore * m_pHashTable;				// Source hash table for the backup

	// Remember the important fields from CHashTableCore
	int m_cHashElements;						// Number of hash elements in the table

	UINT m_mskmHashTable;
	CHashElement ** m_pargpHashElements;
	THashElementTableHdr * m_paTableList;

public:
	CHashTableBackup(INOUT CHashTableCore * pHashTable);
	~CHashTableBackup();
	friend class CHashTableCore;
};

/////////////////////////////////////////////////////////////////////
//	Collection of CHashElement
//
//	The implementation of this hash table is dedicated to hash strings.
//	By default, the string comparison is case sensitive, but can be easily changed to ignore case.
//	If a different hashing is required, then the methods m_pfnFCompareHashKeys and m_pfnUComputeHashValueForHashKey
//	must be supplied.
//
class CHashTable : public CHashTableCore
{
public:
	enum EHashFunction	// The purpose of this enumeration is to provide an easy mechanism to set the hash function without having to manually specify PFn_FCompareHashKeys() and PFn_UComputeHashValueForHashKey().
		{
		eHashFunctionHashSha1,
		eHashFunctionStringCase,				// The comparison is case sensitive.  A binary comparison is performed to enhance speed.
		eHashFunctionStringNoCase,				// The comparison is NOT case sensitive.  Only the letters A..Z are compared. This comparison is good when the string is known to have only alpha numeric values (such as a tag name, Permanent ID, or something not having any accents)
		eHashFunctionStringUnicodeCase,			// The comparison is case sensitive.  A binary comparison is performed to enhance speed.
		eHashFunctionStringUnicodeNoCase,		// The comparison is NOT case sensitive.  An uppercase and lowercase letter with an accent are treated as identical, however different from the letter without the accent.  This is the slowest comparison routine, since it does use the Windows API to perform the comparison.
		eHashFunctionStringUnicodeNoCaseAnsi,	// The comparison is NOT case sensitive.  This is an optimization for unicode strings having only ansi text.  Any characters from A..Z will be compared without case, and the rest will be compared using the binary operator.
		eHashFunctionUInt32						// Hash a 32-bit value.  This may appear obvious, but there is some optimization to be done to avoid collisions
		};

protected:
	PFn_PszGetHashKey m_pfnPszGetHashKey;		// Pointer to the routine to get the string of the hash element
	PFn_FCompareHashKeys m_pfnFCompareHashKeys;	// Pointer to the routine to compare two hash keys
	PFn_UComputeHashValueForHashKey m_pfnUComputeHashValueForHashKey;	// Pointer to the routine to compute the hash value for a given key

public:
	CHashTable(PFn_PszGetHashKey pfnPszGetHashKey, EHashFunction eHashFunction);
	VOID SetHashFunction(EHashFunction eHashFunction);
	void SetHashTableSize(int cElements);

	CHashElement * PAllocateHashElement(int cbExtra = 0);

	void Add(INOUT CHashElement * pHashElement);
	void RemoveElement(INOUT CHashElement * pHashElement);
	VOID RehashElement(INOUT CHashElement * pHashElement);
	VOID ForEachHashElement(PFn_EnumHashElement pfnEnumHashElement, LPARAM lParam = d_zNA) const;
	UINT GetAllElements(OUT CArrayPtrHashElement * parraypHashElements) const;
	void DeleteAllElements();
	CHashElement * PFindHashElement(PSZUC pszHashKey) const;		// O(1)
	CHashElement * PFindHashElementWithHashValue(PSZUC pszHashKey, OUT UINT * puHashValue) const; // O(1)
	CHashElement * PFindHashElementAllocate(PSZUC pszHashKey, INOUT BOOL * pfElementNewlyAllocated);	// O(1)
	CHashElement * PFindHashElementAndRemove(PSZUC pszHashKey);	// O(1)
	CHashElement * PFindHashElementCompare(PFn_PCompareHashElement pfnCompareHashElement, LPARAM lParamKeyCompare) const;	// O(n)

	#ifdef DEBUG_WANT_ASSERT
	VOID AssertValid() const;
	#endif

public:
	// The following static methods must have the same interface as PFn_EnumHashElement()
	static void S_PFn_EnumHashElementGetAll(CHashElement * pHashElement, INOUT CArrayPtrHashElement * parraypHashElements);
	static void S_PFn_EnumHashElementDelete(CHashElement * pHashElement, LPARAM lParam = d_zNA);

}; // CHashTable

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class CHashElementPszaPv : public CHashElement
{
public:
	PSZAC m_pszKey;
	PVOID m_pvValue;
};

/////////////////////////////////////////////////////////////////////
//	This is the most typical use of a hash table: to map a string to a pointer (which can be another string)
//
//	The strings are NOT allocated by the table.
//
class CHashTablePszaPv : public CHashTable
{
public:
	CHashTablePszaPv() : CHashTable((PFn_PszGetHashKey)S_PszaGetHashKey, eHashFunctionStringNoCase)
		{
		SetHashElementSize(sizeof(CHashElementPszaPv));
		}
	PVOID PvFindKey(PSZAC pszKey) const
		{
		Assert(pszKey != NULL);
		Assert(*pszKey != '\0');
		CHashElementPszaPv * pHashElement = (CHashElementPszaPv *)PFindHashElement((PSZUC)pszKey);
		if (pHashElement != NULL)
			return pHashElement->m_pvValue;		// May return NULL
		return NULL;
		}
	VOID AddKey(PSZAC pszKey, PCVOID pvValue)
		{
		Assert(pszKey != NULL);
		Assert(*pszKey != '\0');
		Endorse(pvValue == NULL);	// Who cares
		Assert(PFindHashElement((PSZUC)pszKey) == NULL);	// Key already in hash table

		CHashElementPszaPv * pHashElement = (CHashElementPszaPv *)PAllocateHashElement();
		pHashElement->m_pszKey = pszKey;
		pHashElement->m_pvValue = (PVOID)pvValue;
		Add(pHashElement);	// Add the string pair to the hash table
		}
	BOOL FAddedKeyUnique(PSZAC pszKey, PCVOID pvValue);

public:
	static PSZAC S_PszaGetHashKey(CHashElementPszaPv * pHashElement) { return pHashElement->m_pszKey; }	// This routine must have a compatible interface as PFn_PszGetHashKey()

}; // CHashTablePszaPv


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
class CHashElementPszwPv : public CHashElement
{
public:
	PSZWC m_pszwKey;
	PVOID m_pvValue;
};

/////////////////////////////////////////////////////////////////////
//	This is the most typical use of a hash table: to map a Unicode string to a pointer (which can be another string)
//
//	The strings are NOT allocated by the table.
//
class CHashTablePszwPv : public CHashTable
{
public:
	CHashTablePszwPv() : CHashTable((PFn_PszGetHashKey)S_PszwGetHashKey, eHashFunctionStringUnicodeCase)
		{
		SetHashElementSize(sizeof(CHashElementPszwPv));
		}
	PVOID PvFindKey(PSZWC pszwKey) const
		{
		Assert(pszwKey != NULL);
		Assert(*pszwKey != '\0');
		CHashElementPszwPv * pHashElement = (CHashElementPszwPv *)PFindHashElement((PSZUC)pszwKey);
		if (pHashElement != NULL)
			return pHashElement->m_pvValue;		// May return NULL
		return NULL;
		}
	VOID AddKey(PSZWC pszwKey, PCVOID pvValue)
		{
		Assert(pszwKey != NULL);
		Assert(*pszwKey != '\0');
		Endorse(pvValue == NULL);	// Who cares
		Assert(PFindHashElement((PSZUC)pszwKey) == NULL);	// Key already in hash table

		CHashElementPszwPv * pHashElement = (CHashElementPszwPv *)PAllocateHashElement();
		pHashElement->m_pszwKey = pszwKey;
		pHashElement->m_pvValue = (PVOID)pvValue;
		Add(pHashElement);	// Add the string pair to the hash table
		}

public:
	static PSZWC S_PszwGetHashKey(CHashElementPszwPv * pHashElement) { return pHashElement->m_pszwKey; }	// This routine must have a compatible interface as PFn_PszGetHashKey()

}; // CHashTablePszwPv

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
class CHashTableAllocateUniqueStringsU : public CHashTableCore
{
public:
	PSZUC PszuAllocateUniqueStringU(PSZUC pszuString);
};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
class CHashElementUniqueUnicodeStringsNoCase : public CHashElement
{
public:
	// The string is appended at the end
	inline PSZW PszwGetString() { return (PSZW)(this + 1); }
};

//	This hash table simply makes sure each string is unique.
//	The comparison is NOT case sensitive.
//	This hash table can be used to determine if there is a collision
//	with two file names.
class CHashTableUniqueUnicodeStringsNoCase : public CHashTable
{
public:
	CHashTableUniqueUnicodeStringsNoCase();
	PSZWC PszwAddUniqueStringW(PSZWC pszwString);

public:
	// This routine must have the same interface as PFn_PszGetHashKey()
	static PSZWC S_PszwGetHashKeyUniqueString(CHashElementUniqueUnicodeStringsNoCase * pHashElement) { return pHashElement->PszwGetString(); }

}; // CHashTableUniqueUnicodeStringsNoCase


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//	This is a hash element that keeps track of how many times it contains the value.
//	It is very useful to determine which element is the most popular, or to sort by the number of instances
class CHashElementInstanceCounter : public CHashElement
{
public:
	int m_cInstances;  // Number of instances
};

class CArrayPtrHashElementInstanceCounter : public CArrayPtrHashElement
{
public:
	inline CHashElementInstanceCounter * PGetAt(int iElement) const { return (CHashElementInstanceCounter *)PvGetElementAt(iElement); }
//	inline CHashElementInstanceCounter ** GetDataStop(OUT CHashElementInstanceCounter *** ppHashElementStop) const { return (CHashElementInstanceCounter **)CArrayEx::GetDataStop(OUT (void ***)ppHashElementStop); }
};


class CHashTableInstanceCounters : public CHashTable
{
public:
	CHashTableInstanceCounters(PFn_PszGetHashKey pfnPszGetHashKey, EHashFunction eHashFunction) : CHashTable(pfnPszGetHashKey, eHashFunction) { }
	VOID GetAllElementsSortedByMostPopular(OUT CArrayPtrHashElementInstanceCounter * parraypHashElements) const;
};

#endif // CHASHTABLE_H
