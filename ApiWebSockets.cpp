//	ApiWebSockets.cpp
//
//	Collection of objects and functions to expose the Cambrian APIs via WebSockets.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "ApiWebSockets.h"

OSocketWeb * g_poSocketWeb;

OSocketServer * g_poServer;



void
ApiWebSocket_Init(UINT uPort)
	{
	Assert(g_poSocketWeb == NULL);
	if (uPort == 0)
		return;
	MessageLog_AppendTextFormatCo(d_coBlack, "WebSocket API: Listening to port $i\n", uPort);

	/*
	g_poSocketWeb = new OSocketWeb;
	g_poSocketWeb->bind(uPort);
	QObject::connect(g_poSocketWeb, SIGNAL(SI_MessageAvailable(CBin &)), g_pwMainWindow, SLOT(SL_WebSocketMessage(CBin &)));
	*/
	g_poServer = new OSocketServer;
	g_poServer->listen(QHostAddress::Any, uPort);
	QObject::connect(g_poServer, SIGNAL(newConnection()), g_poServer, SLOT(SL_NewConnection()));
	}


void
OSocketServer::SL_NewConnection()
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "SL_NewConnection()\n");
	QTcpSocket *socket = nextPendingConnection();	// Get the socket for the connection
	socket->write("Hello client\r\n");
	socket->flush();
	socket->waitForBytesWritten(3000);
	socket->close();
	}

void
WMainWindow::SL_WebSocketMessage(CBin & binszvFrameData)
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "SL_WebSocketMessage() - $B\n", &binszvFrameData);
	}
