#ifndef SOCKETTASKS_H
#define SOCKETTASKS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	The enumeration ETaskClass is used to serialize and unserialize tasks and determine what type of task it is.
enum ETaskClass
	{
	eTaskClass_CTaskSend		= _USZU1('s'),
	eTaskClass_CTaskReceive		= _USZU1('r')
	};


class ITask : public SListNode	// (task)
{
public:
	TIMESTAMP m_tsTaskID;			// Identifier of the task
	/*
	TContact * m_pContact;			// Which contact the task is for
	CBin m_binData;					// Data to send to the contact
	int m_cbTransmitted;
	*/

public:
	ITask(const TIMESTAMP * ptsTaskID = d_ts_pNULL_AssignToNow);
	virtual ~ITask();
	virtual ETaskClass EGetTaskClass() const  = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Linked list of ITask.
//	Since ITask inherits SListNode, there is no need to remove (or delete) nodes from CListTasks as the destructor of ITask will take care of removing nodes from the CListTasks.
//	Also, each CListTasks is mutually exclusive, as each ITask may belong only to one list.
class CListTasks : public CList
{
public:
	void DeleteTask(PA_DELETING ITask * paTask);
	void DeleteAllTasks();
	ITask * PFindTaskByID(TIMESTAMP tsTaskID, ETaskClass eTaskClass) const;
};

class CTaskSend : public ITask
{
public:
	CBin m_binData;					// Data to transmit
public:
	CTaskSend(const TIMESTAMP * ptsTaskID = d_ts_pNULL_AssignToNow);
	virtual ETaskClass EGetTaskClass() const { return eTaskClass_CTaskSend; }
};

//	Receiving data is similar to sending data, except we need to know what is the total data available
class CTaskReceive : public CTaskSend
{
public:
	int m_cbTotal;	// Total expected amount of data to receive

public:
	CTaskReceive(const TIMESTAMP * ptsTaskID);
	virtual ETaskClass EGetTaskClass() const { return eTaskClass_CTaskReceive; }
};

#endif // SOCKETTASKS_H
