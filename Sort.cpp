///////////////////////////////////////////////////////////////////////////////////////////////////
//	Sort.cpp
//
//	General-purpose super efficient stable sort routine.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Generic routine to determine if an array is sorted ascending, descending or both.
ESortOrder
Sort_EGetSortedOrder(
	PCVOID prgpvElements[],			// IN: Array of pointers to verify the sorting
	int cElements,					// IN: Number of elements in array
	PFn_NCompareSortElements pfnSortCompare,	// IN: Comparison routine
	LPARAM lParamCompare)			// IN: Optional parameter to the comparison routine
	{
	Assert(prgpvElements != NULL);
	Assert(cElements >= 0);
	Assert(pfnSortCompare != NULL);
	ESortOrder eSortOrder = eSortOrder_kmBoth;	// Pretent the array is sorted until otherwise
	PCVOID * ppvElement = prgpvElements;
	while (--cElements > 0)
		{
		int nResult = pfnSortCompare(*ppvElement, *(ppvElement+1), lParamCompare);
		if (nResult < 0)
			{
			eSortOrder = (ESortOrder)(eSortOrder & ~eSortOrder_kfDescending);
			if (eSortOrder == eSortOrder_kzNone)
				return eSortOrder_kzNone;
			}
		else if (nResult > 0)
			{
			eSortOrder = (ESortOrder)(eSortOrder & ~eSortOrder_kfAscending);
			if (eSortOrder == eSortOrder_kzNone)
				return eSortOrder_kzNone;
			}
		ppvElement++;
		}
	return eSortOrder;
	} // Sort_EGetSortedOrder()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Specialized routine to return TRUE if the array is sorted in ascending order.
BOOL
Sort_FIsSortedAscending(
	PCVOID prgpvElements[],			// IN: Array of pointers to verify the sorting
	int cElements,					// IN: Number of elements in array
	PFn_NCompareSortElements pfnSortCompare,	// IN: Comparison routine
	LPARAM lParamCompare)			// IN: Optional parameter to the comparison routine
	{
	Assert(prgpvElements != NULL);
	Assert(cElements >= 0);
	Assert(pfnSortCompare != NULL);
	PCVOID * ppvElement = prgpvElements;
	while (--cElements > 0)
		{
		if (pfnSortCompare(*ppvElement, *(ppvElement+1), lParamCompare) > 0)
			return FALSE;
		ppvElement++;
		}
	return TRUE;
	} // Sort_FIsSortedAscending()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Specialized routine to return TRUE if the array is sorted in descending order.
BOOL
Sort_FIsSortedDescending(
	PCVOID prgpvElements[],			// IN: Array of pointers to verify the sorting
	int cElements,					// IN: Number of elements in array
	PFn_NCompareSortElements pfnSortCompare,	// IN: Comparison routine
	LPARAM lParamCompare)			// IN: Optional parameter to the comparison routine
	{
	Assert(prgpvElements != NULL);
	Assert(cElements >= 0);
	Assert(pfnSortCompare != NULL);
	PCVOID * ppvElement = prgpvElements;
	while (--cElements > 0)
		{
		if (pfnSortCompare(*ppvElement, *(ppvElement+1), lParamCompare) < 0)
			return FALSE;
		ppvElement++;
		}
	return TRUE;
	} // Sort_FIsSortedDescending()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Reverse only the identical elements
VOID
Sort_DoReverseIdenticalElements(
	PCVOID prgpvElements[],			// INOUT: Array of pointers to sort
	int cElements,					// IN: Number of elements in the array
	PFn_NCompareSortElements pfnSortCompare,	// IN: Comparison routine
	LPARAM lParamCompare)			// IN: Optional parameter to the comparison routine
	{
	#ifdef DEBUG_WANT_ASSERT
	const int cElementsOld = cElements;
	const ESortOrder eSortOlderOld = Sort_EGetSortedOrder(prgpvElements, cElements, pfnSortCompare, lParamCompare);
	#endif

	PCVOID * ppvElement = prgpvElements;
	while (--cElements > 0)
		{
		if (pfnSortCompare(*ppvElement, *(ppvElement+1), lParamCompare) == 0)
			{
			// We have identical elements, so find how many of them to reverse them
			PCVOID * ppvElementStart = ppvElement;
			PCVOID * ppvElementStop = ppvElement + 2;
			while (--cElements > 0)
				{
				if (pfnSortCompare(*ppvElementStart, *ppvElementStop, lParamCompare) != 0)
					break;
				ppvElementStop++;
				}
			ppvElement = ppvElementStop;
			// Reverse the array
			for (int cElementsReverse = (ppvElementStop - ppvElementStart) / 2; cElementsReverse > 0; cElementsReverse--)
				{
				PCVOID pvTemp = *ppvElementStart;
				*ppvElementStart++ = *--ppvElementStop;
				*ppvElementStop = pvTemp;
				}
			continue;
			} // if
		ppvElement++;
		} // while
	Assert(Sort_EGetSortedOrder(prgpvElements, cElementsOld, pfnSortCompare, lParamCompare) == eSortOlderOld);
	} // Sort_DoReverseIdenticalElements()

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Merge two sorted arrays into one array.  The routine assumes both
//	source arrays prgpvSrc1, and prgpvSrc2 are sorted and prgpvDst is large
//	enough to store the resulting array.
//
//	This routine is used internally by the sort module and does not check
//	for any error.
//
inline void _MergeSortedArraysFast(
	PCVOID prgpvSrc1[],				// IN: Source array 1
	int cElementsSrc1,				// IN: Number of elements in source array 1
	PCVOID prgpvSrc2[],				// IN: Source array 2
	int cElementsSrc2,				// IN: Number of elements in source array 2
	PCVOID prgpvDst[],				// OUT: Resulting sorted array
	PFn_NCompareSortElements pfnSortCompare,	// IN: Comparison routine
	LPARAM lParamCompare)			// IN: Optional parameter to the comparison routine
	{
	while (TRUE)
		{
		if (cElementsSrc1 <= 0)
			{
			// Copy the remaining array 2 to destination
			memcpy(OUT prgpvDst, IN prgpvSrc2, cElementsSrc2 * sizeof(PCVOID));
			return;
			}
		if (cElementsSrc2 <= 0)
			{
			// Copy the remaining array 1 to destination
			memcpy(OUT prgpvDst, IN prgpvSrc1, cElementsSrc1 * sizeof(PCVOID));
			return;
			}
		// Compare two elements
		if (pfnSortCompare(*prgpvSrc1, *prgpvSrc2, lParamCompare) <= 0)
			{
			*prgpvDst++ = *prgpvSrc1++;
			cElementsSrc1--;
			}
		else
			{
			*prgpvDst++ = *prgpvSrc2++;
			cElementsSrc2--;
			}
		} // while
	} // _MergeSortedArraysFast()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Sort_DoSorting()
//
//	Flexible and efficient general-purpose sort routine.
//
//	INTERFACE NOTES
//	This routine will not sort if there is not enough memory for the pointers.
//
//	IMPLEMENTATION NOTES
//	The routine has been written for speed resulting in harder-to-read code.
//	This routine uses the divide and conquer approach to sort portions
//	of the array and then merge the sorted portions into a larger array.
//
void
Sort_DoSorting(
	PCVOID prgpvElements[],			// INOUT: Array of pointers to sort
	const int cElements,			// IN: Number of elements to sort
	BOOL fSortAscending,			// IN: TRUE => Sort ascending, FALSE => Sort descending
	PFn_NCompareSortElements pfnSortCompare,	// IN: Comparison routine
	LPARAM lParamCompare)			// IN: Optional parameter to the comparison routine
	{
	Assert(prgpvElements != NULL);
	Assert(cElements >= 0);
	Assert(pfnSortCompare != NULL);
	if (cElements <= 1)
		return;		// An empty array or an array with one element is already sorted
	PCVOID * pargpvElementsTemp = new PCVOID[cElements];	// Allocate temporary storage
	if (pargpvElementsTemp == NULL)
		return;	// This line of code is there only if the new() operator does not throw an exception.

	int cElementsGroupForward = 1;
	int cElementsGroupDivision = 2;
	PCVOID * prgpvElementsSource = pargpvElementsTemp;
	PCVOID * prgpvElementsDestination = prgpvElements;
	while (TRUE)
		{
		// Sort each division
		int cElementsRemain = cElements - cElementsGroupDivision;
		// Swap the arrays
		PCVOID * ppvDst = prgpvElementsDestination;
		prgpvElementsDestination = prgpvElementsSource;
		prgpvElementsSource = ppvDst;
		ppvDst = prgpvElementsDestination;
		PCVOID * ppvSrc1 = prgpvElementsSource;
		PCVOID * ppvSrc2 = prgpvElementsSource + cElementsGroupForward;	
		while (cElementsRemain >= 0)
			{
			_MergeSortedArraysFast(
				ppvSrc1, cElementsGroupForward,
				ppvSrc2, cElementsGroupForward,
				OUT ppvDst,
				pfnSortCompare, lParamCompare);
			ppvSrc1 += cElementsGroupDivision;
			ppvSrc2 += cElementsGroupDivision;
			ppvDst += cElementsGroupDivision;
			cElementsRemain -= cElementsGroupDivision;
			} // while
		// Merge the partial divisions
		cElementsRemain += cElementsGroupDivision;
		if (cElementsRemain <= cElementsGroupForward)
			{
			memcpy(OUT ppvDst, IN ppvSrc1, cElementsRemain * sizeof(PCVOID));
			}
		else
			{
			// We need to merge partial divisions
			cElementsRemain -= cElementsGroupForward;
			Assert(cElementsRemain < cElementsGroupForward);
			_MergeSortedArraysFast(
				ppvSrc1, cElementsGroupForward,
				ppvSrc2, cElementsRemain,
				OUT ppvDst,
				pfnSortCompare, lParamCompare);
			}
		if (cElements <= cElementsGroupDivision)
			break;

		// Double the size of the division
		Assert(cElementsGroupForward * 2 == cElementsGroupDivision);
		cElementsGroupForward = cElementsGroupDivision;
		cElementsGroupDivision *= 2;
		} // while

	Assert(prgpvElementsSource == prgpvElements || prgpvElementsSource == pargpvElementsTemp);
	Assert(prgpvElementsDestination == prgpvElements || prgpvElementsDestination == pargpvElementsTemp);
	if (fSortAscending)
		{
		if (prgpvElementsDestination != prgpvElements)
			{
			// Copy the sorted array to the destination buffer
			memcpy(OUT prgpvElements, IN prgpvElementsDestination, cElements * sizeof(PCVOID));
			}
		// Verify for consistency
		Assert(Sort_FIsSortedAscending(IN prgpvElements, cElements, pfnSortCompare, lParamCompare));
		Assert(Sort_EGetSortedOrder(prgpvElements, cElements, pfnSortCompare, lParamCompare) & eSortOrder_kfAscending);
		}
	else
		{
		if (prgpvElementsDestination != prgpvElements)
			{
			// Copy the sorted array to the destination buffer
			PCVOID * ppvSrcRev = prgpvElementsDestination + cElements;
			PCVOID * ppvDst = prgpvElements;
			PCVOID * ppvDstLast = ppvDst + cElements;
			while (ppvDst != ppvDstLast)
				*ppvDst++ = *--ppvSrcRev;
			}
		else
			{
			// Reverse the array in the destination buffer
			PCVOID * ppvFirst = prgpvElements;
			PCVOID * ppvLastRev = prgpvElements + cElements;
			for (int cElementsReverse = cElements / 2; cElementsReverse > 0; cElementsReverse--)
				{
				PCVOID pvTemp = *ppvFirst;
				*ppvFirst++ = *--ppvLastRev;
				*ppvLastRev = pvTemp;
				}
			}
		Sort_DoReverseIdenticalElements(INOUT prgpvElements, cElements, pfnSortCompare, lParamCompare);
		Assert(Sort_FIsSortedDescending(IN prgpvElements, cElements, pfnSortCompare, lParamCompare));
		Assert(Sort_EGetSortedOrder(prgpvElements, cElements, pfnSortCompare, lParamCompare) & eSortOrder_kfDescending);
		} // if...else
	delete pargpvElementsTemp;
	} // Sort_DoSorting()

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Compare two Ascii strings for sorting purpose.  The comparison is NOT case sensitive.
int
NCompareSortStringAsciiNoCase(PSZAC pszStringA, PSZAC pszStringB)
	{
	if (pszStringA == NULL)
		pszStringA = c_szaEmpty;
	if (pszStringB == NULL)
		pszStringB = c_szaEmpty;
	while (TRUE)
		{
		CHS chA = Ch_GetCharLowercase(*pszStringA++);
		int nResult = (chA - Ch_GetCharLowercase(*pszStringB++));
		if (nResult != 0)
			return nResult;
		if (chA == '\0')
			return 0;
		}
//	QCollatorSortKey
	}
