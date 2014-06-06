///////////////////////////////////////////////////////////////////////////////////////////////////
//	Sort.h
//
//	General-purpose super efficient sort routine.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//	typedef PFn_NCompareSortElements()
//
//	Interface for a routine to compare two elements for sorting purpose.
//
//	Return negative if pvElementA should precede pvElementB.
//	Return 0 if pvElementA is equivalent to pvElementB.
//	Return positive if pvElementA should follow pvElementB.
//
//	PARAMETERS
//	The lParamCompareSort is an additional parameter to perform a comparison between two elements.
//
typedef int (* PFn_NCompareSortElements)(PCVOID pvElementA, PCVOID pvElementB, LPARAM lParamCompareSort);
	int NCompareSortElementsByBytesZ(const BYTE * prgbzSortKey1, const BYTE * prgbzSortKey2, LPARAM lParamCompare);
	int NCompareSortElementsByStringIntegerAscii(PSZAC pszString1, PSZAC pszString2, LPARAM lParamCompareSort);

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Returned values for GetSortOrder()
enum ESortOrder
	{
	eSortOrder_kzNone		= 0x0,	// The array is not sorted
	eSortOrder_kfAscending	= 0x1,	// The array is sorted in ascending order
	eSortOrder_kfDescending	= 0x2,	// The array is sorted in descending order
	eSortOrder_kmBoth		= 0x3,	// The array is sorted in both ascending and descending order (which means all the array elements are the same, or the array is empty)
	};

///////////////////////////////////////////////////////////////////////////////////////////////////
ESortOrder Sort_EGetSortedOrder(PCVOID prgpvElements[], int cElements, PFn_NCompareSortElements pfnSortCompare, LPARAM lParamCompare = d_zNA);
BOOL Sort_FIsSortedAscending(PCVOID prgpvElements[], int cElements, PFn_NCompareSortElements pfnSortCompare, LPARAM lParamCompare = d_zNA);
BOOL Sort_FIsSortedDescending(PCVOID prgpvElements[], int cElements, PFn_NCompareSortElements pfnSortCompare, LPARAM lParamCompare = d_zNA);
void Sort_DoSorting(INOUT PCVOID prgpvElements[], const int cElements, BOOL fSortAscending, PFn_NCompareSortElements pfnSortCompare, LPARAM lParamCompare = d_zNA);
void Sort_DoReverseIdenticalElements(INOUT PCVOID prgpvElements[], const int cElements, PFn_NCompareSortElements pfnSortCompare, LPARAM lParamCompare = d_zNA);
