///////////////////////////////////////////////////////////////////////////////////////////////////
//	List.cpp
//
//	Code to handle doubly-linked list.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#ifdef DEBUG_WANT_ASSERT

#define d_cNodesInfiniteLoop				1000000		// If this limit is reached, it probably means the list is in an infinite loop

#define d_cDebugAllowanceListDefault		1000000
long g_cDebugAllowanceList = d_cDebugAllowanceListDefault;

BOOL
FIsDebugAllowanceReachedList()
	{
	return FIsDebugAllowanceReached(INOUT &g_cDebugAllowanceList, "the list module", d_cDebugAllowanceListDefault);
	}

VOID
DebugAllowanceListDisable()
	{
	DebugAllowanceDisable(INOUT &g_cDebugAllowanceList);
	}

VOID
DebugAllowanceListEnable()
	{
	DebugAllowanceEnable(INOUT &g_cDebugAllowanceList);
	}

VOID
DebugAllowanceListDisplayToChatDebugger()
	{
	MessageLog_AppendTextFormatCo(d_coBlueDark, "\t list module: $I$s\n", d_cDebugAllowanceListDefault - g_cDebugAllowanceList, (g_cDebugAllowanceList <= 0) ? "+" : NULL);
	}

#endif // DEBUG_WANT_ASSERT


/////////////////////////////////////////////////////////////////////
//	Return the next node in the list.  Return NULL if pNode
//	was the last node in the list.
//
SListNode *
List_PNodeGetNext(IN const SListNode * pNode)
	{
	Assert(pNode != NULL);
	Assert(pNode->_pPrev != NULL);
	return pNode->pNext;
	} // List_PNodeGetNext()


/////////////////////////////////////////////////////////////////////
//	Return the previous node in the list.  Return NULL if pNode
//	was the first node in the list.
//
SListNode *
List_PNodeGetPrev(IN const SListNode * pNode)
	{
	Assert(pNode != NULL);
	Assert(pNode->_pPrev != NULL);
	const SListNode * pNodePrev = pNode->_pPrev;
	Assert(pNodePrev->_pPrev != NULL);
	if (pNodePrev->_pPrev->pNext == NULL)
		{
		// We have found the last node in the list
		// Therefore pNode was the first node in list
		return NULL;
		}
	return (SListNode *)pNodePrev;
	} // List_PNodeGetPrev()


/////////////////////////////////////////////////////////////////////
//	Generic routine to insert a node next to another node
void
List_InsertNodeAfter(
	INOUT SList * pList,
	INOUT SListNode * pNodeBefore,
	OUT SListNode * pNodeInsert)
	{
	Assert(pList != NULL);
	Assert(pNodeBefore != NULL);
	Assert(pNodeInsert != NULL);
	#ifdef DEBUG
	if (!FIsDebugAllowanceReachedList())
		{
		Assert(pNodeBefore == (SListNode *)pList
			|| List_FIsNodeChildren(pList, pNodeBefore));
		}
	#endif

	pNodeInsert->pNext = pNodeBefore->pNext;
	if (pNodeBefore->pNext != NULL)
		pNodeBefore->pNext->_pPrev = pNodeInsert;
	else
		pList->pTail = pNodeInsert;
	pNodeBefore->pNext = pNodeInsert;
	pNodeInsert->_pPrev = pNodeBefore;
	List_AssertValidNode(pNodeInsert);
	List_AssertValidList(pList);
	} // List_InsertNodeAfter()


//	Insert a node before another node
//	This routine must be called on a non-empty list
VOID
List_InsertNodeBefore(
	INOUT SListNode * pNodeAfterInsertion,	// Tailing node
	OUT SListNode * pNodeInsertBefore)		// Node to insert
	{
	Assert(pNodeAfterInsertion != NULL);
	Assert(pNodeAfterInsertion->_pPrev != NULL && "The list is empty");
	Assert(pNodeInsertBefore != NULL);

	pNodeInsertBefore->pNext = pNodeAfterInsertion;
	pNodeInsertBefore->_pPrev = pNodeAfterInsertion->_pPrev;
	pNodeInsertBefore->_pPrev->pNext = pNodeInsertBefore;
	pNodeAfterInsertion->_pPrev = pNodeInsertBefore;

	List_AssertValidNode(pNodeAfterInsertion);
	List_AssertValidNode(pNodeInsertBefore);
	} // List_InsertNodeBefore()

/*
//	Insert a node between two nodes
VOID
List_InsertNodeBetweenNodes(
	INOUT SListNode * pNodeBefore,
	INOUT SListNode * pNodeAfter,
	OUT SListNode * pNodeInsert)
	{
	Assert(pNodeBefore != NULL);
	Assert(pNodeAfter != NULL);
	Assert(pNodeAfter->_pPrev != NULL);
	Assert(FALSE && "NYI");
	}
*/

/////////////////////////////////////////////////////////////////////
void
List_InsertNodeAtHead(
	INOUT SList * pList,
	OUT SListNode * pNodeInsert)
	{
	InitToGarbage(OUT pNodeInsert, sizeof(*pNodeInsert));
	List_InsertNodeAfter(
		pList,
		(SListNode *)pList,
		pNodeInsert);
	} // List_InsertNodeAtHead()


/////////////////////////////////////////////////////////////////////
void
List_InsertNodeAtTail(
	INOUT SList * pList,
	OUT SListNode * pNodeInsert)
	{
	List_InsertNodeAfter(
		pList,
		(pList->pTail != NULL) ? pList->pTail : (SListNode *)pList,
		pNodeInsert);
	} // List_InsertNodeAtTail()


/////////////////////////////////////////////////////////////////////
//	Insert a node in a sorted list using a user-defined callback
//	function to sort the elements.
void
List_InsertNodeSortClbk(
	INOUT SList * pList,
	OUT SListNode * pNodeInsert,
	IN List_PFnSortCallback pfnSortCallback)
	{
	Assert(pList != NULL);
	Assert(pNodeInsert != NULL);
	Assert(pfnSortCallback != NULL);

	SListNode * pNodeT = pList->pHead;
	SListNode * pNodePrev;
	if ((pNodeT == NULL) ||
		(pfnSortCallback(pNodeT, pNodeInsert) > 0))
		{
		// Insert node at beginning of list
		List_InsertNodeAfter(
			pList,
			(SListNode *)pList,
			pNodeInsert);
		return;
		}
	pNodePrev = pNodeT;
	while (pfnSortCallback(pNodeT, pNodeInsert) <= 0)
		{
		pNodePrev = pNodeT;
		pNodeT = pNodeT->pNext;
		if (pNodeT == NULL)
			{
			break;
			}
		} // while
	Assert(pNodePrev != NULL);
	List_InsertNodeAfter(pList, pNodePrev, pNodeInsert);
	} // List_InsertNodeSortClbk()


/////////////////////////////////////////////////////////////////////
//	Detach a node from a linked list
//
//	INTERFACE NOTES
//	The routine does not free the node.
//
void
List_DetachNode(
	INOUT SList * pList,
	INOUT SListNode * pNode)
	{
	Assert(pList != NULL);
	Assert(pNode != NULL);
	#ifdef DEBUG
	if (!FIsDebugAllowanceReachedList())
		Assert(List_FIsNodeChildren(pList, pNode));
	List_AssertValidNode(pNode);
	List_AssertValidList(pList);
	#endif

	Assert(pNode->_pPrev != NULL);
	pNode->_pPrev->pNext = pNode->pNext;
	if (pNode->pNext != NULL)
		{
		// We detached a node at the beginning or middle of the list
		pNode->pNext->_pPrev = pNode->_pPrev;
		}
	else
		{
		// We detached a node at the end of the list
		Assert(pNode == pList->pTail);
		Assert(pNode->pNext == NULL);
		if (pList->pHead == NULL)
			{
			pList->pTail = NULL;
			}
		else
			{
			pList->pTail = pNode->_pPrev;
			}
		}

	pNode->pNext = NULL;
	pNode->_pPrev = NULL;
	List_AssertValidNode(pNode);
	List_AssertValidList(pList);
	} // List_DetachNode()


/////////////////////////////////////////////////////////////////////
//	Detach a node from its list.  Since each node is part of a doubly-linked list, it is
//	possible to detach a node from its list without knowing the list.
//
//	IMPLEMENTATION NOTES
//	This routine was inspired from the other List_DetachNode() and is used when
//	it is difficult to determine the head of the list.  Therefore it is preferable to have
//	the routine take care of removing the node from the list.
//
VOID
List_DetachNode(INOUT SListNode * pNode)
	{
	Assert(pNode != NULL);
	List_AssertValidNode(pNode);
	List_AssertValidNode(pNode->pNext);

	SListNode * pNodePrev = pNode->_pPrev;
	SListNode * pNodeNext = pNode->pNext;
	if (pNodeNext != NULL)
		{
		// We are detaching a a node which is not at the end of the list (this node may be at the beginning or in the middle of the list)
		if (pNodePrev->pNext == pNode)
			pNodePrev->pNext = pNodeNext;	// pNodePrev->pNext is in fact SList.pHead
		pNodeNext->_pPrev = pNodePrev;
		pNode->_pPrev = NULL;
		pNode->pNext = NULL;
		return;
		}
	if (pNodePrev == NULL)
		return;	// Both pNodeNext and pNodePrev are NULL, so we assume the node has already been detached

	// We are detaching a node which is at the end of the list
	pNodePrev->pNext = NULL;
	SListNode * pNodePrevPrev = pNodePrev->_pPrev;
	Assert(pNodePrevPrev != NULL);
	if (pNodePrevPrev == pNode)
		{
		// We have found the list header SList just before the node we wish to detach.
		Assert(pNodePrev->_pPrev == pNode);
		Assert(pNode->pNext == NULL);
		pNodePrev->pNext = NULL;
		pNodePrev->_pPrev = NULL;	// pNodePrev->_pPrev is in fact SList.pTail
		pNode->_pPrev = NULL;
		return;
		}

	// At this point, we have to walk each node until we find the list header SList.
	#ifdef DEBUG_WANT_ASSERT
	int cGuard = 100000;
	#endif
	while (TRUE)
		{
		Assert(cGuard-- > 0 && "Infinite loop");
		List_AssertValidNode(pNodePrev);
		pNodePrev = pNodePrevPrev;
		pNodePrevPrev = pNodePrevPrev->_pPrev;
		if (pNodePrevPrev == pNode)
			{
			// We have found the list header update the tail of the list
			pNodePrev->_pPrev = pNode->_pPrev;	// pNodePrev->_pPrev is in fact SList.pTail
			Assert(pNodePrev->_pPrev != NULL);
			Assert(pNode->pNext == NULL);
			pNode->pNext = NULL;	// This is redundant
			pNode->_pPrev = NULL;
			return;
			}
		} // while
	} // List_DetachNode()


/////////////////////////////////////////////////////////////////////
//	Detach (dequeue) the first node and return a pointer to it.
//	This routine may return NULL if the list was empty.
//
//	INTERFACE NOTES
//	The routine does not free the node.
//
SListNode *
List_PDetachNodeFromHead(
	INOUT SList * pList)
	{
	Assert(pList != NULL);
	if (pList->pHead == NULL)
		{
		// List was empty
		return NULL;
		}
	// Detach the first node in the linked list
	SListNode * pNode = pList->pHead;
	List_DetachNode(INOUT pList, INOUT pNode);
	return pNode;
	} // List_PDetachNodeFromHead()


#ifdef DEBUG_WANT_ASSERT

#define d_cDebugAllowanceErrorCheckingLinkedListDefault		10000
long g_cDebugAllowanceErrorCheckingLinkedList = d_cDebugAllowanceErrorCheckingLinkedListDefault;

/////////////////////////////////////////////////////////////////////
//	Return TRUE if the node is found in the list.
//	This routine is mostly used for debugging purposes.
//
BOOL
List_FIsNodeChildren(
	IN const SList * pList,
	IN const SListNode * pNodeSearch)
	{
	Assert(pList != NULL);
	Endorse(pNodeSearch == NULL);

	if (pNodeSearch == NULL)
		return FALSE;
	List_AssertValidNode(pNodeSearch);
	int cNodeCountGuard = 0;
	const SListNode * pNodeT = pList->pHead;
	while (pNodeT != NULL)
		{
		cNodeCountGuard++;
		// Watchdog counter to detect circular lists
		Assert(cNodeCountGuard < d_cNodesInfiniteLoop && "Infinite loop");
		List_AssertValidNode(pNodeT);
		if (pNodeT == pNodeSearch)
			return TRUE;
		pNodeT = pNodeT->pNext;
		} // while
	return FALSE;
	} // List_FIsNodeChildren()

UINT
List_UCalculateLength(IN const SList * pList)
	{
	Assert(pList != NULL);
	UINT cNodes = 0;
	const SListNode * pNode = pList->pHead;
	while (pNode != NULL)
		{
		pNode = pNode->pNext;
		cNodes++;
		}
	return cNodes;
	}

/////////////////////////////////////////////////////////////////////
//	Verify the consistency of a node in a linked list.
void
List_AssertValidNode(
	IN const SListNode * pNode)
	{
	if (pNode == NULL)
		return;
	if (pNode->pNext == NULL && pNode->_pPrev == NULL)
		return;
	// There should always be something in the _pPrev field
	Assert(pNode->_pPrev != NULL);
	Assert(pNode->_pPrev->pNext == pNode);
	if (pNode->pNext != NULL)
		{
		Assert(pNode->pNext->_pPrev == pNode);
		}
	} // List_AssertValidNode()


/////////////////////////////////////////////////////////////////////
//	Verify the consistency of the whole linked list.
void
List_AssertValidList(
	IN const SList * pList)
	{
	Assert(pList != NULL);

	if (FIsDebugAllowanceReachedList())
		return;	// Don't perform the checking

	int cNodeCount = 0;
	const SListNode * pNodeT = pList->pHead;
	while (pNodeT != NULL)
		{
		Assert(pNodeT->_pPrev != NULL);
		List_AssertValidNode(pNodeT);
		cNodeCount++;
		// Watchdog counter to detect circular lists
		Assert(cNodeCount < d_cNodesInfiniteLoop && "Infinite loop");
		pNodeT = pNodeT->pNext;
		} // while
	if (cNodeCount == 0)
		{
		Assert(pList->pHead == NULL);
		Assert(pList->pTail == NULL);
		}
	if (cNodeCount == 1)
		{
		Assert(pList->pHead == pList->pTail);
		Assert(List_PNodeGetPrev(pList->pTail) == NULL);
		}
	if (pList->pTail != NULL)
		{
		Assert(pList->pTail->pNext == NULL);
		List_AssertValidNode(pList->pTail);
		Assert(List_PNodeGetPrev(pList->pHead) == NULL);
		Assert(List_FIsNodeChildren(pList, pList->pTail));
		}

	} // List_AssertValidList()

//	Small unit test for the detaching a node from a list
VOID
List_Test()
	{
	SList list = { NULL, NULL };
	SListNode node1;
	SListNode node2;
	SListNode node3;
	SListNode node4;

	// Detach a node from a list congaing a single node
	List_InsertNodeAtHead(INOUT &list, OUT &node1);
	List_DetachNode(INOUT &node1);
	Assert(node1.pNext == NULL);
	Assert(node1._pPrev == NULL);
	Assert(list.pHead == NULL);
	Assert(list.pTail == NULL);

	// Detach two nodes from the end of the list
	List_InsertNodeAtTail(INOUT &list, OUT &node1);
	List_InsertNodeAtTail(INOUT &list, OUT &node2);
	List_AssertValidList(IN &list);

	List_DetachNode(INOUT &node2);
	Assert(node2.pNext == NULL);
	Assert(node2._pPrev == NULL);
	Assert(list.pHead == &node1);
	Assert(list.pTail == &node1);
	List_AssertValidList(IN &list);

	List_DetachNode(INOUT &node1);
	Assert(list.pHead == NULL);
	Assert(list.pTail == NULL);
	List_AssertValidList(IN &list);

	// Detach two nodes from the beginning of the list
	List_InsertNodeAtTail(INOUT &list, OUT &node1);
	List_InsertNodeAtTail(INOUT &list, OUT &node2);
	List_AssertValidList(IN &list);

	List_DetachNode(INOUT &node1);
	Assert(node1.pNext == NULL);
	Assert(node1._pPrev == NULL);
	Assert(list.pHead == &node2);
	Assert(list.pTail == &node2);
	List_AssertValidList(IN &list);

	List_DetachNode(INOUT &node2);
	Assert(list.pHead == NULL);
	Assert(list.pTail == NULL);
	List_AssertValidList(IN &list);

	// Detach a node from the middle of the list
	List_InsertNodeAtTail(INOUT &list, OUT &node1);
	List_InsertNodeAtTail(INOUT &list, OUT &node2);
	List_InsertNodeAtTail(INOUT &list, OUT &node3);
	List_InsertNodeAtTail(INOUT &list, OUT &node4);
	List_AssertValidList(IN &list);
	List_DetachNode(INOUT &node2);
	Assert(node2.pNext == NULL);
	Assert(node2._pPrev == NULL);
	Assert(list.pHead == &node1);
	Assert(list.pTail == &node4);
	List_AssertValidList(IN &list);

	List_DetachNode(INOUT &node4);
	Assert(node4.pNext == NULL);
	Assert(node4._pPrev == NULL);
	Assert(list.pHead == &node1);
	Assert(list.pTail == &node3);
	List_AssertValidList(IN &list);

	List_InsertNodeAtTail(INOUT &list, OUT &node4);
	List_DetachNode(INOUT &node1);
	Assert(node1.pNext == NULL);
	Assert(node1._pPrev == NULL);
	Assert(list.pHead == &node3);
	Assert(list.pTail == &node4);
	List_AssertValidList(IN &list);

	List_InsertNodeAtHead(INOUT &list, OUT &node2);
	List_InsertNodeAtHead(INOUT &list, OUT &node1);
	List_DetachNode(INOUT &node3);
	Assert(node3.pNext == NULL);
	Assert(node3._pPrev == NULL);
	Assert(list.pHead == &node1);
	Assert(list.pTail == &node4);
	List_AssertValidList(IN &list);

	} // List_Test()
#endif // DEBUG_WANT_ASSERT


