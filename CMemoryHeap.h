#ifndef CMEMORYHEAP_H
#define CMEMORYHEAP_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#ifdef DEBUG
	#define ENABLE_DEBUG_HEAP

	#define _HEAP_chDebugSignature  'D'
	#define _HEAP_uDebugSignature  0xccddcccc
#endif


class CMemoryHeap
{
protected:
	enum { cbMinimumNodeSizeSplit = 32 };		// Minimum size to split a node, otherwise we will end up with many tiny [useless] blocks of free memory

	struct SNodeHdr
		{
		SNodeHdr * pNext;		// Next adjacent memory block
		SNodeHdr * _pPrev;		// Previous adjacent memory block
		int cbNodeSize;			// Allocated number of bytes (including header).  If this value is negative, then the node is allocated, otherwise it is free.
		#ifdef ENABLE_DEBUG_HEAP
		int ibchSignature;      // Offset of signature at end of block
		int cbNodeSizeT;		// Redundant information about of the size of the block
		UINT uSignature;		// Signature at beginning of block
		enum { c_cbSignature = sizeof(UINT) };
		#endif
		};

	struct SNodeFreeHdr
		{
		// Have a double-linked list of free blocks of memory
		SNodeFreeHdr * pFreeNext;
		SNodeFreeHdr * _pFreePrev;
		};

	SNodeHdr * m_pHead;
	SNodeHdr * m_pTail;
	SNodeFreeHdr * m_pFreeHead;	// Pointer to the first free block of memory
	SNodeFreeHdr * m_pFreeTail;

	struct SAllocHdr
		{
		SAllocHdr * pNext;	// Pointer to the next allocated memory block (if any)
		#ifdef ENABLE_DEBUG_HEAP
		int cbAlloc;
		#endif
		};
	SAllocHdr * m_paAllocatedList;	// Linked list of allocated memory blocks

public:
	CMemoryHeap();
	~CMemoryHeap();
	VOID Destroy();
	VOID GrowHeap(int cbAlloc);
	VOID AddAddressRange(INOUT VOID * pvStart, int cbSize);
	VOID * PvAllocateData(int cbAlloc);
	VOID * PvAllocateDataCopy(int cbAlloc, PCVOID pvDataSrcCopy);
	PSZU PszuAllocateStringW(PSZWC pszwString);
	PSZU PszuAllocateStringU(PSZUC pszuString);
	PSZUC PszuAllocateStringLowercaseU(PSZUC pszuString);
	VOID FreeData(PA_DELETING PCVOID pavData);

protected:
	VOID _MergeAdjecentFreeNodes(INOUT SNodeHdr * pNode1, INOUT SNodeHdr * pNode2);

public:
#ifdef DEBUG
	VOID HeapWalk(BOOL fDisplayTraceInfo = FALSE) const;
	VOID AssertValid() const { HeapWalk(); }
	static VOID AssertValidNode(const SNodeHdr * pNode);
	VOID AssertChildNode(const SNodeHdr * pNode);
#endif

}; // CMemoryHeap

#ifdef ENABLE_DEBUG_HEAP
	#define d_cbDebugHeapExtra		16		// 16 bytes per extra allocation for debugging
#else
	#define d_cbDebugHeapExtra		0
#endif

#endif // CMEMORYHEAP_H
