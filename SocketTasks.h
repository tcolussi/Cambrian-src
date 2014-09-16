#ifndef SOCKETTASKS_H
#define SOCKETTASKS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	The same class is used to send and receive data.
//	The only difference is the task to receive has a positive m_cbTotal
class CTaskSendReceive
{
public:
	TIMESTAMP m_tsTaskID;		// Identifier of the task
	CTaskSendReceive * m_pNext;	// Next task in the queue
	CBin m_binXmlData;			// XML data to transmit (a task always sends an XML element)
	int m_cbTotal;				// Total size of the data to receive.  If this value is zero or negative, it means it is a task to send.
	enum	// This enum indicates possible values of m_cbTotal
		{
		c_cbTotal_TaskSentOnce			= -1,	// The task was sent once, and in the future, just send the header.
		c_cbTotal_TaskSendTryOnlyOnce	= -2,	// Use the <iq> instead of <message> to attempt to send the task once, and delete afterwards
		c_cbTotal_TaskSendAndRetry		= -3,	// Retry as long as it is necessary to send the task.  This is the default behevior.
		c_cbTotal_TaskSendAndRetrySync	= -4,	// Same as above, except the task contains synchronization data.
		};

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
	CTaskSendReceive * PAllocateTaskSend_YZ(BOOL fuAllocateTaskContainingSynchronization = FALSE);
	CTaskSendReceive * PAllocateTaskSend() { return PAllocateTaskSend_YZ(); }
	CTaskSendReceive * PFindOrAllocateTaskDownload_NZ(TIMESTAMP tsTaskID, const CXmlNode * pXmlNodeTaskDownload);
	void DeleteTaskMatchingID(TIMESTAMP tsTaskID, BOOL fDeleteTaskToSend);
	void DeleteTask(PA_DELETING CTaskSendReceive * paTask);
	void DeleteAllTasks();
	CTaskSendReceive * PFindTaskSend(TIMESTAMP tsTaskID) const;
	void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
	void SerializeToXml(IOUT CBin * pbinXmlTasks);
	void UnserializeFromXml(const CXmlNode * pXmlNodeElementTask);
	void DisplayTasksToMessageLog();
};

#endif // SOCKETTASKS_H
