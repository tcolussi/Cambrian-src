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

#if 0
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
	#if 0 // For testing
	DetachNode(INOUT paTask);
	delete paTask;
	#endif
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

const char c_szTasks[] = "T";

void
CListTasks::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	if (pXmlExchanger->m_fSerializing)
		{
		if (pHead == NULL)
			return;	// There is no task to serialize
		CBin * pbinTemp = &pXmlExchanger->m_binXmlFileData;	// Use the temporary buffer
		pbinTemp->BinInitFromByte(d_chuXmlAlreadyEncoded);
		ITask * pTask = (ITask *)pHead;
		while (TRUE)
			{
			// An object is always serialized as an element
			MessageLog_AppendTextFormatCo(d_coRed, "Serializing task 0x$p\n", pTask);
			pbinTemp->BinAppendText_VE("<");
			pTask = (ITask *)pTask->pNext;
			if (pTask == NULL)
				break;
			} // while
		pXmlExchanger->XmlExchangeElementEnd(c_szTasks);


		(void)pXmlExchanger->PAllocateElementFromCBinString('T', IN_MOD_TMP *pbinTemp);
		/*

		*/
		return;
		}
	}
#endif

#if 0
void
TContact::XcpApiContact_TaskQueue(PA_TASK ITask * paTask)
	{
	m_listTasksSocket.InsertNodeAtHead(OUT paTask);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ISocketTask::Socket_WriteStanzaIqSet_Gso(PSZAC pszFmtTemplate, ...)
	{
	g_strScratchBufferSocket.Empty();
	g_strScratchBufferSocket.BinAppendText_VE("<iq type='set' from='^J' to='^J' id='$p'>", m_pAccount, m_pContact, UGetStanzaId());
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferSocket.BinAppendTextSzv_VL(pszFmtTemplate, vlArgs);
	g_strScratchBufferSocket.BinAppendBinaryData("</iq>", 5);
	PGetSocket()->Socket_WriteBin(g_strScratchBufferSocket);
	}
#endif

/*
CTaskSend::CTaskSend(const TIMESTAMP * ptsTaskID) : ITask(ptsTaskID)
	{
	}

CTaskReceive::CTaskReceive(const TIMESTAMP * ptsTaskID) : CTaskSend(ptsTaskID)
	{
	m_cbTotal = 0;
	}
*/

CTaskSendReceive::CTaskSendReceive()
	{
	m_pNext = NULL;
	m_cbTotal = d_zNA;
	}

void
CTaskSendReceive::InitTaskSend()
	{
	m_tsTaskID = Timestamp_GetCurrentDateTime();
	m_cbTotal = c_cbTotal_Send;
	}

//	All task to be sent are appended at the end of the list, so the order is preserved
void
CListTasksSendReceive::AddTaskSend(INOUT CTaskSendReceive * pTaskSent)
	{
	Assert(pTaskSent != NULL);
	Assert(pTaskSent->m_pNext == NULL);
	pTaskSent->m_pNext = m_plistTasks;	// Need to fix this (to append at the end)
	m_plistTasks = pTaskSent;
	}

//	A new task to be downloaded is inserted at the beginning of the list so it may be accessed quickly
void
CListTasksSendReceive::AddTaskReceive(INOUT CTaskSendReceive * pTaskReceive)
	{
	Assert(pTaskReceive != NULL);
	Assert(pTaskReceive->m_pNext == NULL);
	pTaskReceive->m_pNext = m_plistTasks;
	m_plistTasks = pTaskReceive;
	}

void
CListTasksSendReceive::DeleteTask(PA_DELETING CTaskSendReceive * paTaskDelete)
	{
	Assert(paTaskDelete != NULL);
	return;	// Just for debugging

	if (paTaskDelete == m_plistTasks)
		m_plistTasks = m_plistTasks->m_pNext;
	else
		{
		CTaskSendReceive * pTask = m_plistTasks;
		while (TRUE)
			{
			CTaskSendReceive * pTaskNext = pTask->m_pNext;
			if (paTaskDelete == pTaskNext)
				{
				//MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "[Debug] Found task $t\n", paTaskDelete->m_tsTaskID);	// Remove this line once executed
				pTask->m_pNext = pTaskNext->m_pNext;	// Remove the task from the linked list
				break;
				}
			pTask = pTaskNext;
			if (pTask == NULL)
				{
				Assert(FALSE && "Task not found");
				return;	// Don't delete the task; it is better to have a memory leak than a crash
				}
			} // while
		}
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Deleting Task ID $t\n", paTaskDelete->m_tsTaskID);
	delete paTaskDelete;
	}

void
CListTasksSendReceive::DeleteAllTasks()
	{
	while (m_plistTasks != NULL)
		{
		CTaskSendReceive * pTaskNext = m_plistTasks->m_pNext;
		delete m_plistTasks;
		m_plistTasks = pTaskNext;
		}
	}

CTaskSendReceive *
CListTasksSendReceive::PFindTaskSend(TIMESTAMP tsTaskID) const
	{
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		if (pTask->m_tsTaskID == tsTaskID && pTask->FIsTaskSend())
			return pTask;
		pTask = pTask->m_pNext;
		}
	return NULL;
	}

CTaskSendReceive *
CListTasksSendReceive::PFindTaskReceive(TIMESTAMP tsTaskID) const
	{
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		if (pTask->m_tsTaskID == tsTaskID && !pTask->FIsTaskSend())
			return pTask;
		pTask = pTask->m_pNext;
		}
	return NULL;
	}


void
CListTasksSendReceive::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	if (pXmlExchanger->m_fSerializing)
		{
		if (m_plistTasks == NULL)
			return;	// Nothing to serialize
		CBin * pbinTemp = pXmlExchanger->PGetBinTemporaryDuringSerializationInitAlreadyEncoded();	// Use the temporary buffer to serializa all the tasks
		SerializeToXml(IOUT pbinTemp);
		(void)pXmlExchanger->XmlExchange_PAllocateElementFromCBinString('T', IN_MOD_TMP *pbinTemp);
		}
	else
		{
		const CXmlNode * pXmlTasks = pXmlExchanger->XmlExchange_PFindElement('T');
		if (pXmlTasks != NULL)
			UnserializeFromXml(pXmlTasks->m_pElementsList);
		}
	}

void
CListTasksSendReceive::SerializeToXml(IOUT CBin * pbinXmlTasks)
	{
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		MessageLog_AppendTextFormatCo(d_coRed, "Serializing Task ID $t\n", pTask->m_tsTaskID);
		pbinXmlTasks->BinAppendText_VE("\n<t i='$t' c='$i' b='{B|}'/>", pTask->m_tsTaskID, pTask->m_cbTotal, &pTask->m_binData);
		pTask = pTask->m_pNext;
		}
	}

void
CListTasksSendReceive::UnserializeFromXml(const CXmlNode * pXmlNodeElementTask)
	{
	Assert(m_plistTasks == NULL);
	CTaskSendReceive ** ppaTask = &m_plistTasks;
	while (pXmlNodeElementTask != NULL)
		{
		//MessageLog_AppendTextFormatCo(d_coRed, "Unserializing Task ^N", pXmlNodeElementTask);
		CTaskSendReceive * paTask = new CTaskSendReceive;
		*ppaTask = paTask;
		ppaTask = &paTask->m_pNext;	// Pointer to append the next task at the end of the linked list
		paTask->m_tsTaskID = pXmlNodeElementTask->TsGetAttributeValueTimestamp_ML('i');
		paTask->m_cbTotal = pXmlNodeElementTask->UFindAttributeValueDecimal_ZZR('c');
		paTask->m_binData.BinAppendBinaryDataFromBase85SCb_ML(pXmlNodeElementTask->PszuFindAttributeValue('b'));
		pXmlNodeElementTask = pXmlNodeElementTask->m_pNextSibling;
		}

	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGreen), "CListTasksSendReceive::UnserializeFromXml():\n");
	DisplayTasksToMessageLog();
	}

//	For debugging
void
CListTasksSendReceive::DisplayTasksToMessageLog()
	{
	TIMESTAMP tsNow = Timestamp_GetCurrentDateTime();
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		MessageLog_AppendTextFormatCo(d_coGreen, "\t [$T] Task ID $t: cbReceived = $I, cbData = $I\n", tsNow - pTask->m_tsTaskID, pTask->m_tsTaskID, pTask->m_cbTotal, pTask->m_binData.CbGetData());
		MessageLog_AppendTextFormatCo(d_coGray, "$B\n", &pTask->m_binData);
		pTask = pTask->m_pNext;
		}
	}
