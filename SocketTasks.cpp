///////////////////////////////////////////////////////////////////////////////////////////////////
//	SocketTasks.cpp
//
//	Classes to hold tasks for a socket, such as a sending a chat message or transfrring a file.
//	The purpose of those classes is allowing the queueing of messages to be dispatched on a server
//	as well as a reliable serialization.
//	In the future, messages may include images, notifications and other types of events, some
//	of which may not be serialized.  Therefore this class is vital to determine what ought
//	to be dispatched on the server, what should be serialized as the chat history, and
//	what should be displayed on the chat history however not serialized.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


ITask::ITask(const TIMESTAMP * ptsTaskID)
	{
	if (ptsTaskID == d_ts_pNULL_AssignToNow)
		m_tsTaskID = Timestamp_GetCurrentDateTime();	// We are creating a new task, so use the current date & time as identifier of the task.
	else
		m_tsTaskID = *ptsTaskID;						// We are unserializing an existing task, so use its previous identifier
	}

ITask::~ITask()
	{
	}

void
CListTasks::DeleteTask(PA_DELETING ITask * paTask)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CListTasks::DeleteTask() - Task ID '$t'\n", paTask->m_tsTaskID);
	Assert(paTask != NULL);
	DetachNode(INOUT paTask);
	delete paTask;
	}

void
CListTasks::DeleteAllTasks()
	{

	}

ITask *
CListTasks::PFindTaskByID(TIMESTAMP tsTaskID, ETaskClass eTaskClass) const
	{
	ITask * pTask = (ITask *)pHead;
	while (pTask != NULL)
		{
		if (pTask->m_tsTaskID == tsTaskID && pTask->EGetTaskClass() == eTaskClass)
			break;
		pTask = (ITask *)pTask->pNext;
		} // while
	return pTask;
	}

void
TContact::XcpApiContact_TaskQueue(PA_TASK ITask * paTask)
	{
	m_listTasksSocket.InsertNodeAtHead(OUT paTask);
	}

#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
void
ISocketTask::Socket_WriteStanzaIqSet_Gso(PSZAC pszFmtTemplate, ...)
	{
	g_strScratchBufferSocket.Empty();
	g_strScratchBufferSocket.BinAppendTextSzv_VE("<iq type='set' from='^J' to='^J' id='$p'>", m_pAccount, m_pContact, UGetStanzaId());
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferSocket.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	g_strScratchBufferSocket.BinAppendBinaryData("</iq>", 5);
	PGetSocket()->Socket_WriteBin(g_strScratchBufferSocket);
	}

#endif


CTaskSend::CTaskSend(const TIMESTAMP * ptsTaskID) : ITask(ptsTaskID)
	{
	}

CTaskReceive::CTaskReceive(const TIMESTAMP * ptsTaskID) : CTaskSend(ptsTaskID)
	{
	m_cbTotal = 0;
	}
