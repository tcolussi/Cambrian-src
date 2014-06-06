#ifndef SOCKETTASKS_H
#define SOCKETTASKS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_chXmlPrefixSocketRawData	'~'		// Debugging prefix to send raw data directly to the socket instead of sending the text to the contact.  Example: "~<presence/>"


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Special enumeration for the task completion.
//	The motivation of having a special enumeration is to make the code easier to read.
//	For isntance, a task may fail to perform its duty, however the method EWriteDataToSocket() must return eTaskCompleted because there is no more processing to be done.  Returning errSuccess would be a bit confusing.
//	Likewise returning eTaskCompletionNeedsMoreProcessing is more descriptive than returning an error code from the enum EError.
enum ETaskCompletion
{
	eTaskCompleted = 0,			// There is need to queue the task to the socket anymore (typically when the task completed, or when the task aborted).  Whatever the task was deleted upon returning eTaskCompleted is at the discretion of the event which created the task.
	eTaskCompletionNeedsMoreProcessing	// The task needs to be put to the queue of the socket waiting for completion.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Linked list of ITask.
//	Since ITask inherits SListNode, there is no need to remove (or delete) nodes from CListTasks as the destructor of ITask will take care of removing nodes from the CListTasks.
//	Also, each CListTasks is mutually exclusive, as each ITask may belong only to one list.
class CListTasks : public CList
{
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Interface ITask
//
//	Interface for a task related to an event.
//	Since Cambrian is about sending and receiving messages, most tasks will interact with sockets to transmit data and will display something in the Chat Log.
//
class ITask : public SListNode	// (task)
{
public:
	union
		{
		IEvent * pEvent;	// Generic event
		CEventMessageTextSent * pEventMessageSent;
		CEventGroupMessageSent * pEventMessageSentToGroup;
		CEventFileSent * pEventFileSent;
		CEventFileReceived * pEventFileReceived;
		CEventPing * pEventPing;
		} mu_parent;	// Parent of the task
	TAccountXmpp * m_pAccount;			// Cached version of mu_parent.pEvent->mu_parentowner.pTreeItem->m_pAccount.  Since tasks use intensively the object CSocketXmpp from the account, it makes sense to cache this value.
	QTextBlock m_oTextBlockMessageLog;	// Text block to update in the Chat Log

public:
	ITask(IEvent * pEventParent, const OCursor * poCursorTextBlock);
	virtual ~ITask();
	inline TIMESTAMP GetEventID() const { return mu_parent.pEvent->m_tsEventID; }
	inline TAccountXmpp * PGetAccount() const { return m_pAccount; }
	CSocketXmpp * PGetSocket_YZ() const;
	void Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlIqSet_VE_Gso(PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlIqReplyAcknowledge();
	void ChatLog_UpdateEventWithinWidget(INOUT OCursor * poCursorTextBlock);
	void ChatLog_UpdateEventWithinWidget();
	void TaskSentOnce();
	void TaskCompleted(PA_DELETING);

	// New virtual methods for the task
	virtual ETaskCompletion EWriteDataToSocket();
	virtual void ProcessStanza(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext = NULL, const CXmlNode * pXmlNodeVerb = NULL);
}; // ITask



///////////////////////////////////////////////////////////////////////////////////////////////////
class ITaskFile : public ITask
{
protected:
	QFile m_oFile;					// File to save or load the data.
	UINT m_uSequence;				// Sequence number to send chunks of data, starting at 0

public:
	ITaskFile(IEvent * pEventParent, const OCursor * poCursorTextBlock);
};

//	Task to send (upload) a file to a contact
class CTaskFileUpload : public ITaskFile
{
public:
	CTaskFileUpload(CEventFileSent * pEventParent, const OCursor * poCursorTextBlock);
	virtual ETaskCompletion EWriteDataToSocket();
	virtual void ProcessStanza(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext, const CXmlNode * pXmlNodeVerb);
	void OnEventTransferCompleted();
};

//	Task to receive (download) a file from a contact
class CTaskFileDownload : public ITaskFile
{
public:
	CTaskFileDownload(CEventFileReceived * pEventParent, const OCursor * poCursorTextBlock);
	~CTaskFileDownload();
	virtual ETaskCompletion EWriteDataToSocket();
	virtual void ProcessStanza(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext, const CXmlNode * pXmlNodeVerb);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Array holding all the 'tasks' downloading a file
//	For performance reasons, the implementation of this class is rather unique: A pointer to an event is added to the array, however a pointer to a task is returned.
class CArrayPtrTasksDownloading : private CArray
{
public:
	inline void TaskEventAdd(CEventFileReceived * pEventFileReceived) { Add(pEventFileReceived); }
	inline void TaskEventRemove(CEventFileReceived * pEventFileReceived) { RemoveElementFastAssertF(pEventFileReceived); }
	CTaskFileDownload * PFindTaskMatchingSessionIdentifier(PSZUC pszSessionIdentifier) const;
};

#endif // SOCKETTASKS_H
