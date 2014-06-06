///////////////////////////////////////////////////////////////////////////////////////////////////
//	CMemoryHeap.cpp
//
//	Class to allocate memory.
//	This class creates a heap and deletes its allocated blocks when destructed.
//	It is a convenient way to efficiently allocate memory without having to delete it when no longer needed.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

CMemoryHeap::CMemoryHeap()
	{
	InitToZeroes(OUT this, sizeof(*this));
	}

CMemoryHeap::~CMemoryHeap()
	{
	Destroy();
	}

VOID
CMemoryHeap::Destroy()
	{
	while (m_paAllocatedList != NULL)
		{
		SAllocHdr * pNext = m_paAllocatedList->pNext;
		delete m_paAllocatedList;
		m_paAllocatedList = pNext;
		}
	Assert(m_paAllocatedList == NULL);
	InitToZeroes(OUT this, sizeof(*this));
	} // Destroy()

VOID
CMemoryHeap::GrowHeap(int cbAlloc)
	{
	Assert(cbAlloc > cbMinimumNodeSizeSplit);
	SAllocHdr * paAllocated = (SAllocHdr *)new BYTE[cbAlloc];
	InitToGarbage(OUT paAllocated, cbAlloc);
	#ifdef ENABLE_DEBUG_HEAP
	paAllocated->cbAlloc = cbAlloc;
	#endif
	paAllocated->pNext = m_paAllocatedList;
	m_paAllocatedList = paAllocated;

	AddAddressRange(paAllocated + 1, cbAlloc - sizeof(*paAllocated));
	} // GrowHeap()

VOID
CMemoryHeap::AddAddressRange(VOID * pvStart, int cbSize)
	{
	Assert(pvStart != NULL);
	Assert(cbSize >= cbMinimumNodeSizeSplit);
	Assert(IS_ALIGNED_32(pvStart));
	Assert(IS_ALIGNED_32(cbSize));
	InitToGarbage(pvStart, cbSize);

	SNodeHdr * pNode = (SNodeHdr *)pvStart;
	List_InsertNodeAtTail(INOUT (SList *)&m_pHead, INOUT (SListNode *)pNode);

	pNode->cbNodeSize = cbSize;
	#ifdef ENABLE_DEBUG_HEAP
	pNode->uSignature = _HEAP_uDebugSignature;
	SNodeFreeHdr * pNodeFree = (SNodeFreeHdr *)pNode;
	UNUSED_VARIABLE(pNodeFree);
	#endif

	// Insert the free node at the beginning
	List_InsertNodeAtTail(INOUT (SList *)&m_pFreeHead, INOUT (SListNode *)(pNode + 1));

	#ifdef DEBUG
	AssertValidNode((const SNodeHdr *)pvStart);
	#endif
	} // AddAddressRange()


VOID *
CMemoryHeap::PvAllocateData(int cbRequest)
	{
	Assert(cbRequest >= 0);
	#ifdef ENABLE_DEBUG_HEAP
	int ibSignature = cbRequest;	// Remember the exact offset
	cbRequest += SNodeHdr::c_cbSignature;
	#endif

	cbRequest = sizeof(SNodeHdr) + NEXT_UINT32(cbRequest);
	if ((unsigned)cbRequest < sizeof(SNodeHdr) + sizeof(SNodeFreeHdr))
		cbRequest = sizeof(SNodeHdr) + sizeof(SNodeFreeHdr);

	// Search the list of free blocks to satisfy the request
	while (TRUE)
		{
		// It is possible to satisfy the request, so try to find a memory block
		for (SNodeFreeHdr * pNodeFree = m_pFreeHead; pNodeFree != NULL; pNodeFree = pNodeFree->pFreeNext)
			{
			SNodeHdr * pNode = (SNodeHdr *)pNodeFree - 1;
			#ifdef DEBUG
			AssertValidNode(pNode);
			#endif

			int cbRemain = (pNode->cbNodeSize - cbRequest);
			if (cbRemain >= 0)
				{
				// We have a node with enough space to satisfy the request, however for optimization, we will look at the next node just in case
				SNodeFreeHdr * pNodeFreeNext = pNodeFree->pFreeNext;
				if (pNodeFreeNext != NULL)
					{
					SNodeHdr * pNodeNext = (SNodeHdr *)pNodeFreeNext - 1;
					const int cbRemainNext = (pNodeNext->cbNodeSize - cbRequest);
					if (cbRemainNext >= 0 && cbRemainNext < cbRemain)
						{
						// The next node is smaller, so use this node instead
						cbRemain = cbRemainNext;
						pNodeFree = pNodeFreeNext;
						pNode = pNodeNext;
						}
					} // if

				// Remove the free node from the free list
				List_DetachNode(INOUT (SList *)&m_pFreeHead, INOUT (SListNode *)pNodeFree);

				if (cbRemain >= cbMinimumNodeSizeSplit)
					{
					// Free node is large enough to be slitted
					SNodeHdr * pNodeSplit = (SNodeHdr *)((BYTE *)pNode + cbRequest);	// Create a new node
					InitToGarbage(pNodeSplit, sizeof(*pNodeSplit));
					pNodeSplit->cbNodeSize = cbRemain;
					#ifdef ENABLE_DEBUG_HEAP
					pNodeSplit->uSignature = _HEAP_uDebugSignature;
					#endif

					pNode->cbNodeSize = -cbRequest;
					List_InsertNodeAfter(
						INOUT (SList *)&m_pHead,
						INOUT (SListNode *)pNode,
						INOUT (SListNode *)pNodeSplit);

					// Insert the free node at the beginning
					List_InsertNodeAtHead(
						INOUT (SList *)&m_pFreeHead,
						INOUT (SListNode *)(pNodeSplit + 1));
					}
				else
					{
					cbRequest = pNode->cbNodeSize;	// The request becomes the entire node size
					pNode->cbNodeSize = -pNode->cbNodeSize;	// Negate the value to indicate the block is allocated
					} // if...else
				#ifdef ENABLE_DEBUG_HEAP
				Assert(pNode->uSignature == _HEAP_uDebugSignature);
				pNode->ibchSignature = ibSignature;
				pNode->cbNodeSizeT = pNode->cbNodeSize;
				BYTE * pbDst = (BYTE *)(pNode + 1);
				InitToGarbage(OUT pbDst, pNode->ibchSignature);
				pbDst[pNode->ibchSignature] = _HEAP_chDebugSignature;
				#endif
				#ifdef DEBUG
				AssertValidNode(pNode);
				#endif
				return (pNode + 1);
				} // if
			} // for

		GrowHeap((cbRequest * 2) + 4000);	// Allocate larger blocks of memory extra 4 KB
		} // while
	} // PvAllocateData()

//	Allocate a block of memory and initialize its buffer
VOID *
CMemoryHeap::PvAllocateDataCopy(int cbAlloc, PCVOID pvDataSrcCopy)
	{
	Assert(pvDataSrcCopy != NULL);
	void * pvData = PvAllocateData(cbAlloc);
	memcpy(OUT pvData, IN pvDataSrcCopy, cbAlloc);
	return pvData;
	}


PSZU
CMemoryHeap::PszuAllocateStringU(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	const CHU * pch = pszuString;
	while (*pch++ != '\0')
		;
	return (PSZU)PvAllocateDataCopy(pch - pszuString, IN pszuString);
	}
/*
//	Make a lowercase copy of an UTF-8 string string
PSZUC
CMemoryHeap::PszuAllocateStringLowercaseU(PSZUC pszuString)
	{
	Assert(pszuString != NULL);
	Assert(*pszuString != '\0');

	// First, convert UTF-8 to Unicode
	const int cbStringUppercase = CbAllocUtoW(IN pszuString);
	PSZW pszwString = (PSZW)alloc_stack(cbStringUppercase);
	_DecodeUtf8ToUnicode(OUT pszwString, pszuString);

	// Make the Unicode string lowercase
	int cchwLowercase = StringLowerCaseW(INOUT pszwString, cbStringUppercase / sizeof(CHW)) + 1;

	#ifdef DEBUG_
	//int cchwLowercase = strlenW(pszwString);
	if (ChatDebugger_GetHwndEditW() != NULL)
		ChatDebugger_AppendTextFormatCo(coGreyDark, "Converting '$s' to '$w'.\n", pszuString, pszwString);
	#endif

	// Convert Unicode back to UTF-8
	PSZU pszuLowercase = (PSZU)alloc_stack(cchwLowercase * 3);

	int cbStringLowercase = _EncodeUnicodeToUtf8(OUT pszuLowercase, IN pszwString) + 1;
	return (PSZUC)PvAllocateDataCopy(cbStringLowercase, pszuLowercase);
	} // PszuAllocateStringLowercaseU()
*/

VOID
CMemoryHeap::FreeData(PA_DELETING PCVOID pvData)
	{
	if (pvData == NULL)
		return;
	SNodeHdr * pNode = ((SNodeHdr *)pvData) - 1;

	#ifdef DEBUG
	Assert(pNode->cbNodeSize < 0);	// Assert node was previously allocated
	#endif
	#ifdef ENABLE_DEBUG_HEAP
	Assert(pNode->cbNodeSize == pNode->cbNodeSizeT);
	BYTE * pbDst = (BYTE *)(pNode + 1);
	InitToGarbage(OUT pbDst, pNode->ibchSignature);
	Assert(pbDst[pNode->ibchSignature] == _HEAP_chDebugSignature);
	InitToGarbage(OUT pbDst, pNode->ibchSignature);
	AssertValidNode(pNode);
	#endif

	int cbNodeSize = -pNode->cbNodeSize;
	pNode->cbNodeSize = cbNodeSize;	// Mark the node as free

	// Insert the free node at the head of the list
	List_InsertNodeAtHead(
		INOUT (SList *)&m_pFreeHead,
		INOUT (SListNode *)pvData);

	// Attempt to merge adjacent free nodes
	if (pNode->pNext != NULL)
		_MergeAdjecentFreeNodes(pNode, pNode->pNext);

	#ifdef DEBUG
	AssertValidNode(pNode);
	#endif
	if (m_pHead != pNode)
		{
		Assert(m_pTail != NULL);
		Assert(m_pTail != m_pHead);
		Assert(pNode->_pPrev != NULL);
		Assert((void *)pNode->_pPrev != (void *)this);
		_MergeAdjecentFreeNodes(pNode->_pPrev, pNode);
		}
	} // FreeData()


//	Conditionally merge two nodes if the following conditions are satisfied:
//		1)  Both nodes are free
//		2)  Both nodes are adjacent in the address space.
//
//	INTERFACE NOTES
//	Content of pNode2 is undefined when function returns.
//
VOID
CMemoryHeap::_MergeAdjecentFreeNodes(INOUT SNodeHdr * pNode1, INOUT SNodeHdr * pNode2)
	{
	Assert(pNode1 != NULL);
	Assert(pNode2 != NULL);

	// Assert both nodes belong to the same heap
	#ifdef DEBUG
	AssertValidNode(pNode1);
	AssertValidNode(pNode2);
	#endif

	int cbNodeSize1 = pNode1->cbNodeSize;
	if (cbNodeSize1 < 0)
		return;	// The node is not free
	int cbNodeSize2 = pNode2->cbNodeSize;
	if (cbNodeSize2 < 0)
		return;	// The node is not free
	if (((BYTE *)pNode1) + cbNodeSize1 != (BYTE *)pNode2)
		return; // The nodes are not in adjacent memory locations

	// Merge pNode2 nodes into pNode1
	pNode1->cbNodeSize += cbNodeSize2;
	#ifdef ENABLE_DEBUG_HEAP
	Assert(pNode1->uSignature == _HEAP_uDebugSignature && "Invalid signature - beginning of memory block damaged");
	#endif
	List_DetachNode(
		INOUT (SList *)&m_pHead,
		INOUT (SListNode *)pNode2);

	// Remove also node2 from the free list
	List_DetachNode(
		INOUT (SList *)&m_pFreeHead,
		INOUT (SListNode *)(pNode2 + 1));

	#ifdef DEBUG
	Assert(pNode1->_pPrev != NULL);
	AssertValidNode(pNode1);
	#endif
	} // _MergeAdjecentFreeNodes()

#ifdef DEBUG
VOID
CMemoryHeap::AssertChildNode(const SNodeHdr * pNode)
	{
	Assert(pNode != NULL);
	AssertValidNode(pNode);
	BOOL fIsChildNode = List_FIsNodeChildren(
		(SList *)&m_pHead,
		(SListNode *)pNode);
	Assert(fIsChildNode);
	}

//	AssertValidNode(), static
VOID
CMemoryHeap::AssertValidNode(const SNodeHdr * pNode)
	{
	Assert(pNode != NULL);
	Assert(pNode->_pPrev != NULL);
	Assert(pNode->_pPrev->pNext == pNode);

	Assert(IS_ALIGNED_32(pNode->cbNodeSize));
	#ifdef ENABLE_DEBUG_HEAP
	if (pNode->uSignature != _HEAP_uDebugSignature)
		{
		TRACE3("\nERROR: pNode %p : Wrong memory signature %p (%d).\n",
			pNode, pNode->uSignature, pNode->uSignature);
		}
	if (pNode->cbNodeSize < 0)
		{
		// The memory is allocated, so verify the signature
		Assert(pNode->cbNodeSizeT == pNode->cbNodeSize);
		Assert(pNode->ibchSignature >= 0);
		Assert(pNode->ibchSignature < -pNode->cbNodeSize);
		if (_HEAP_chDebugSignature != *(BYTE *)((BYTE *)(pNode + 1) + pNode->ibchSignature))
			{
			Assert(FALSE && "Memory corruption - end block damaged");
			}
		}
	else
		{
		// The memory is free, and therefore validate the pointers
		const SNodeFreeHdr * pNodeFree = (const SNodeFreeHdr *)(pNode + 1);
		List_AssertValidNode((SListNode *)pNodeFree);
		}
	#endif
	List_AssertValidNode((SListNode *)pNode);
	} // AssertValidNode()

#endif // DEBUG
