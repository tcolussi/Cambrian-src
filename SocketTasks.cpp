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
#include "XcpApi.h"

CTaskSendReceive::CTaskSendReceive()
	{
	m_pNext = NULL;
	m_cbTotal = d_zNA;
	}

void
CTaskSendReceive::InitTaskSend()
	{
	m_tsTaskID = Timestamp_GetCurrentDateTime();
	m_cbTotal = c_cbTotal_TaskSendAndRetry;		// By default a task will be retried to be sent
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	To keep memory allocation low, the same list is used to hold tasks to send and receive.
//	For efficiency, the tasks are stored in chronological order, starting with the tasks to be sent.

CTaskSendReceive *
CListTasksSendReceive::PAllocateTaskSend_YZ(BOOL fuAllocateTaskContainingSynchronization)
	{
	if (fuAllocateTaskContainingSynchronization)
		{
		// Search if there is already a task with synchronization
		const CTaskSendReceive * pTask = m_plistTasks;
		while (pTask != NULL)
			{
			if (pTask->m_cbTotal == CTaskSendReceive::c_cbTotal_TaskSendAndRetrySync)
				return NULL;
			pTask = pTask->m_pNext;
			}
		}
	CTaskSendReceive * paTaskSend = new CTaskSendReceive;
	paTaskSend->InitTaskSend();
	if (fuAllocateTaskContainingSynchronization)
		paTaskSend->m_cbTotal = CTaskSendReceive::c_cbTotal_TaskSendAndRetrySync;

	if (m_plistTasks == NULL)
		{
		// This is the typical case where there is nothing in the list
		m_plistTasks = paTaskSend;
		}
	else
		{
		// Append the new task to send task at the end of the list, however before any task to receive
		TIMESTAMP tsPrevious = d_ts_zNA;
		CTaskSendReceive ** ppTaskInsert = &m_plistTasks;
		CTaskSendReceive * pTask = m_plistTasks;
		while (TRUE)
			{
			if (!pTask->FIsTaskSend())
				break;
			Assert(pTask->m_tsTaskID > tsPrevious);
			tsPrevious = pTask->m_tsTaskID;
			ppTaskInsert = &pTask->m_pNext;
			pTask = pTask->m_pNext;
			if (pTask == NULL)
				break;
			} // while
		paTaskSend->m_pNext = *ppTaskInsert;
		*ppTaskInsert = paTaskSend;
		}
	return paTaskSend;
	}

CTaskSendReceive *
CListTasksSendReceive::PFindOrAllocateTaskDownload_NZ(TIMESTAMP tsTaskID, const CXmlNode * pXmlNodeTaskDownload)
	{
	Assert(pXmlNodeTaskDownload != NULL);

	// First, search if the task is there
	TIMESTAMP tsPrevious = d_ts_zNA;
	CTaskSendReceive ** ppTaskInsert = &m_plistTasks;
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		if (!pTask->FIsTaskSend())
			{
			if (pTask->m_tsTaskID == tsTaskID)
				return pTask;
			if (pTask->m_tsTaskID > tsTaskID)
				break;
			}
		else
			{
			Assert(pTask->m_tsTaskID > tsPrevious);
			tsPrevious = pTask->m_tsTaskID;
			}
		ppTaskInsert = &pTask->m_pNext;
		pTask = pTask->m_pNext;
		} // while

	// Th task is not found, therefore allocate it
	pTask = new CTaskSendReceive;
	pTask->m_tsTaskID = tsTaskID;
	pTask->m_cbTotal = pXmlNodeTaskDownload->UFindAttributeValueDecimal_ZZR(d_chXa_TaskDataSizeTotal);
	MessageLog_AppendTextFormatCo(d_coPurple, "Downloading Task ID $t of $I bytes\n", tsTaskID, pTask->m_cbTotal);

	// And insert tot he list
	pTask->m_pNext = *ppTaskInsert;
	*ppTaskInsert = pTask;
	return pTask;
	}

//	Delete the task matching the ID, and if not found, do nothing.
void
CListTasksSendReceive::DeleteTaskMatchingID(TIMESTAMP tsTaskID, BOOL fDeleteTaskToSend)
	{
	CTaskSendReceive ** ppTaskPrevious = &m_plistTasks;
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		if ((pTask->m_tsTaskID == tsTaskID) && (pTask->FIsTaskSend() == fDeleteTaskToSend))
			{
			MessageLog_AppendTextFormatCo(d_coPurple, "DeleteTaskMatchingID() - Task ID $t is now deleted ($I bytes)\n", tsTaskID, pTask->m_binXmlData.CbGetData());
			*ppTaskPrevious = pTask->m_pNext;	// Remove the task to delete from the linked list
			delete pTask;
			return;
			}
		ppTaskPrevious = &pTask->m_pNext;
		pTask = pTask->m_pNext;
		}
	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coPurple), "DeleteTaskMatchingID() - Cannot delete Task ID $t because its ID is no cannot be found!\n", tsTaskID);
	}

//	Delete a task from the list.
//	Since Task IDs are broadcasted repeatedly, we need to keep the last Task ID so there is no attempt to download the same task after it was previously downloaded and executed.
void
CListTasksSendReceive::DeleteTask(PA_DELETING CTaskSendReceive * paTaskDelete)
	{
	Assert(paTaskDelete != NULL);

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
	MessageLog_AppendTextFormatCo(d_coPurple, "Deleting Task ID $t\n", paTaskDelete->m_tsTaskID);
	delete paTaskDelete;
	DisplayTasksToMessageLog();
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
		if (!pTask->FIsTaskSend())
			break;
		if (pTask->m_tsTaskID >= tsTaskID)
			{
			if (pTask->m_tsTaskID == tsTaskID)
				return pTask;
			break;
			}
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
		pbinXmlTasks->BinAppendText_VE("\n<t i='$t' s='$i' _debug_sofar='$i' d='{B|}'/>", pTask->m_tsTaskID, pTask->m_cbTotal, pTask->m_binXmlData.CbGetData(), &pTask->m_binXmlData);
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
		paTask->m_cbTotal = pXmlNodeElementTask->UFindAttributeValueDecimal_ZZR('s');
		paTask->m_binXmlData.BinAppendBinaryDataFromBase85SCb_ML(pXmlNodeElementTask->PszuFindAttributeValue('d'));
		pXmlNodeElementTask = pXmlNodeElementTask->m_pNextSibling;
		}

	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGreen), "CListTasksSendReceive::UnserializeFromXml():\n");
	DisplayTasksToMessageLog();
	}

//	For debugging
void
CListTasksSendReceive::DisplayTasksToMessageLog()
	{
	int cTasks = 0;
	TIMESTAMP tsNow = Timestamp_GetCurrentDateTime();
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		TIMESTAMP_DELTA dts = tsNow - pTask->m_tsTaskID;
		const int cbXmlData = pTask->m_binXmlData.CbGetData();
		if (pTask->FIsTaskSend())
			MessageLog_AppendTextFormatCo(d_coPurple, "\t [$T] Sending Task ID $t: $I bytes\n", dts, pTask->m_tsTaskID, cbXmlData);
		else
			MessageLog_AppendTextFormatCo(d_coPurple, "\t [$T] Downloading Task ID $t: cbTotalToDownload = $I, cbDownloadedSoFar = $I\n", dts, pTask->m_tsTaskID, pTask->m_cbTotal, cbXmlData);
		if (cbXmlData > 0) // && cbXmlData < 200)
			MessageLog_AppendTextFormatCo(d_coGray, "\t\t {Bm}\n", &pTask->m_binXmlData);
		pTask = pTask->m_pNext;
		cTasks++;
		} // while
	if (cTasks >= 3)
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coPurple), "\t\t Total of $i tasks\n", cTasks);
	}
