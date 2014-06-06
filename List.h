///////////////////////////////////////////////////////////////////////////////////////////////////
//	List.cpp
//
//	Code to handle doubly-linked list.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIST_H
#define LIST_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////
struct SListNode	// (node)
	{
	SListNode * pNext;	// Next node in linked list
	SListNode * _pPrev;	// Previous node in linked list (private to LIST module)
	};

struct SList		// (list)
	{
	SListNode * pHead;	// First node in linked list
	SListNode * pTail;	// Last node in linked list
	};


/////////////////////////////////////////////////////////////////////
//	typedef List_PFnSortCallback()
//
//	Interface of a user-defined callback function to compare two
//	nodes in the list.
//
//	RETURNS
//	-1	If content of Node1 < Node2
//	 0	If content of Node1 == Node2
//	+1	If content of Node1 > Node2
//
//	INTERFACE NOTES
//	Both pvNode1 and pvNode2 cannot be NULL.
//
typedef int (* List_PFnSortCallback)(void * pvNode1, void * pvNode2);

/////////////////////////////////////////////////////////////////////
//	Initialize the list to be empty
#define List_Init(/* OUT */ pList)	\
	{							\
	(pList)->pHead = NULL;		\
	(pList)->pTail = NULL;		\
	}

SListNode * List_PNodeGetNext(IN const SListNode * pNode);
SListNode * List_PNodeGetPrev(IN const SListNode * pNode);

void List_InsertNodeAfter(INOUT SList * pList, INOUT SListNode * pNodeBefore, OUT SListNode * pNodeInsert);
void List_InsertNodeBefore(INOUT SListNode * pNodeAfterInsertion, OUT SListNode * pNodeInsertBefore);
void List_InsertNodeAtHead(INOUT SList * pList, OUT SListNode * pNodeInsert);
void List_InsertNodeAtTail(INOUT SList * pList,	OUT SListNode * pNodeInsert);

void List_InsertNodeSortClbk(INOUT SList * pList, OUT SListNode * pNodeInsert, IN List_PFnSortCallback pfnSortCallback);

void List_DetachNode(INOUT SList * pList, INOUT SListNode * pNode);
VOID List_DetachNode(INOUT SListNode * pNode);
SListNode * List_PDetachNodeFromHead(INOUT SList * pList);
BOOL List_FIsNodeChildren(IN const SList * pList, IN const SListNode * pNodeSearch);
UINT List_UCalculateLength(IN const SList * pList);



//	The class CList is the same as structure SList, except the variables are initialized by the constructor
///	and contains useful wrappers
class CList : public SList
{
public:
	inline CList() { pHead = pTail = NULL; }
	inline SListNode * PDetachNodeFromHead() { return List_PDetachNodeFromHead(INOUT this); }
	inline void InsertNodeAtTail(OUT SListNode * pNodeInsert) { List_InsertNodeAtTail(INOUT this, pNodeInsert); }
	inline UINT ULength() const { return List_UCalculateLength(IN this); }
};



/////////////////////////////////////////////////////////////////////
#ifdef DEBUG_WANT_ASSERT

void List_AssertValidNode(IN const SListNode * pNode);
void List_AssertValidList(IN const SList * pList);

#else

	#define List_AssertValidNode(pNode)
	#define List_AssertValidList(pList)

#endif // ~DEBUG

#endif // LIST_H
