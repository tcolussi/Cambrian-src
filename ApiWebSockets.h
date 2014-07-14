//	ApiWebSockets.h
#ifndef APIWEBSOCKETS_H
#define APIWEBSOCKETS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class OSocketServer : public QTcpServer
{
	Q_OBJECT

public:
	virtual ~OSocketServer() { }
public slots:
	void SL_NewConnection();


};

#endif // APIWEBSOCKETS_H
