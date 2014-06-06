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

#define d_cbBufferSizeDefaultTransferFiles		4096	// Default block size to transfer files
#define d_coTurquoise	0x40E0D0
#define d_coTeal		0x008080
#define d_coNavy		0x000080
#define d_coPurple		0x800080

#endif
