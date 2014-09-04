#ifndef SOCKETTASKS_H
#define SOCKETTASKS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#if 0
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

	void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
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
#endif


//	The same class is used to send and receive.
//	The only difference is the task to receive uses m_cbTotal
class CTaskSendReceive
{
public:
	TIMESTAMP m_tsTaskID;		// Identifier of the task
	CTaskSendReceive * m_pNext;	// Next task in the queue
	CBin m_binData;				// Data to transmit
	int m_cbTotal;				// Total expected amount of data to receive
	enum { c_cbTotal_Send = -1 };	// Value to indicate the task is to send

public:
	CTaskSendReceive();
	void InitTaskSend();
	BOOL FIsTaskSend() const { return (m_cbTotal < 0); }

};

class CListTasksSendReceive
{
public:
	CTaskSendReceive * m_plistTasks;	// Linked list of tasks

public:
	CListTasksSendReceive() { m_plistTasks = NULL; }
	void AddTaskSend(INOUT CTaskSendReceive * pTask);
	void AddTaskReceive(INOUT CTaskSendReceive * pTask);
	void DeleteTask(PA_DELETING CTaskSendReceive * paTask);
	void DeleteAllTasks();
	CTaskSendReceive * PFindTaskSend(TIMESTAMP tsTaskID) const;
	CTaskSendReceive * PFindTaskReceive(TIMESTAMP tsTaskID) const;
	void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
	void SerializeToXml(IOUT CBin * pbinXmlTasks);
	void UnserializeFromXml(const CXmlNode * pXmlNodeElementTask);
	void SentTasksToContact(TContact * pContact);
	void DisplayTasksToMessageLog();
};

#endif // SOCKETTASKS_H
