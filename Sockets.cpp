///////////////////////////////////////////////////////////////////////////////////////////////////
//	Sockets.cpp
//
//	Classes for network sockets, including XMPP messages.
//	So far, all socket classes implemented in this file are NOT tread safe because they use global/static variables.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

QMutex g_oMutex;	// We need a global lock because the slot SL_SocketDataAvailableForReading() is reentrant, despite the Qt documentation.
CStr g_strScratchBufferSocket;		// (_Gso) Scratch buffer to optimize the performance of socket operations (at the moment, this buffer is safe because the application is running in a single thread).

ISocketUI::ISocketUI()
	{
	m_pwEditSocketMessageLog_YZ = NULL;
	m_pSocket_YZ = NULL;
	}
ISocketUI::ISocketUI(CSocketXmpp * pSocket)
	{
	m_pwEditSocketMessageLog_YZ = NULL;
	m_pSocket_YZ = pSocket;
	}
ISocketUI::~ISocketUI()
	{
	if (m_pSocket_YZ != NULL)
		m_pSocket_YZ->SocketUI_Remove();
	}

TAccountXmpp *
ISocketUI::PGetAccount_YZ() const
	{
	Assert(m_pSocket_YZ != NULL);
	return m_pSocket_YZ->m_pAccount;
	}

void
ISocketUI::SocketUI_OnSuccess()
	{
	Assert(m_pSocket_YZ != NULL);
	}

void
ISocketUI::SocketUI_OnError()
	{
	Assert(m_pSocket_YZ != NULL);
	}

void
ISocketUI::SocketUI_DisplayMessage(ESeverity eSeverity, PSZUC pszuTextMessage)
	{
	Endorse(m_pSocket_YZ == NULL);	// Some messages are displayed to the UI before the socket is created
	FLAGS_SOCKET_STATE uFlagsSocketState = (m_pSocket_YZ != NULL) ? m_pSocket_YZ->m_uFlagsSocketState : 0;
	PSZAC pszErrorContext = (uFlagsSocketState & FSS_kfXmppUserAuthenticated) ? "Socket error" : "Unable to login to chat account";
	if (m_pwEditSocketMessageLog_YZ != NULL)
		{
		CString sMessage;
		m_pwEditSocketMessageLog_YZ->setTextColor(CoFromSeverity(eSeverity));
		if (eSeverity >= eSeverityErrorWarning)
			{
			if (uFlagsSocketState & FSS_kfOptionCreateAccount)
				pszErrorContext = "Unable to create chat account";
			sMessage = (PSZUC)pszErrorContext;
			m_pwEditSocketMessageLog_YZ->setFontWeight(QFont::Bold);
			}
		m_pwEditSocketMessageLog_YZ->setText(sMessage);
		sMessage = pszuTextMessage;
		m_pwEditSocketMessageLog_YZ->setFontWeight(QFont::Normal);
		m_pwEditSocketMessageLog_YZ->append(sMessage);
		m_pwEditSocketMessageLog_YZ->show();		// Make sure the message log is visible
		m_pwEditSocketMessageLog_YZ->repaint();		// Force the widget to redraw itself
		}
	else if (eSeverity > eSeverityNoise)
		StatusBar_DisplayFunctionDefault(eSeverity, pszuTextMessage);
	if (eSeverity >= eSeverityErrorWarning)
		{
		SetCursorRestoreDefault();	// When there is an error, remove the wait cursor otherwise the user will be waiting for the UI to complete
		ErrorLog_AddNewMessage(pszErrorContext, pszuTextMessage);
		}
	} // SocketUI_DisplayMessage()

void
CSocketXmpp::Socket_WriteScratchBuffer()
	{
	return Socket_WriteBin(g_strScratchBufferSocket);
	}

void
CSocketXmpp::Socket_WriteBin(const CBin & binData)
	{
	Socket_WriteData(binData.PszaGetUtf8NZ(), binData.CbGetData());
	}

//	Core routine to send data to the socket
void
CSocketXmpp::Socket_WriteData(PCVOID pvData, int cbData)
	{
	Assert(pvData != NULL);
	Assert(cbData > 0);
	if (((BYTE *)pvData)[cbData - 1] == '\0')
		cbData--;	// Just as a safety measure, remove the tailing null-terminator, otherwise the XMPP server will reject the data as invalid XML and close the stream.
	MessageLog_AppendTextFormatCo(d_coSocketWriteHeader, "[$@] Socket_WriteData() - Sending $I bytes of data to socket 0x$p (uState=0x$x, $s):\n", cbData, this, m_uFlagsSocketState, PszaGetSocketState(state()));
	//MessageLog_AppendTextFormatCo(d_coSocketWriteContent, "{pn}\n", pvData, cbData);
	DebugDisplayDataToWriteMessageLog(pvData, cbData);
	if (write(IN (const char *)pvData, cbData) != cbData)
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to write data to socket! (number of bytes written does not match number of bytes to write)\n");
	}

//	Reformat a raw XML message
//	Sending raw XML messages is useful for debugging.
//
//	FORMATS SUPPORTED
//		^a	- Append the bare JID of the account
//		^A	- Append the full JID of the account
//		^c	- Append the bare JID of the contact
//		^C	- Append the full JID of the contact
//		^s	- Append the name of the server of the socket
void
CBinXcpStanza::BinXmlInitStanzaWithXmlRaw(PSZUC pszMessageXml)
	{
	Assert(pszMessageXml != NULL);
	Assert(pszMessageXml[0] == '<');
	Assert(m_pContact != NULL);
	if (m_pContact == NULL)
		return;	// Just in case
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Empty();

	// We have a "<" for a raw message, so reformat it
	while (TRUE)
		{
		UINT ch = *pszMessageXml++;
		if (ch != '^')
			{
			BinAppendByte(ch);
			if (ch == '\0')
				break;
			continue;
			}
		UINT chTemplate = *pszMessageXml++;
		switch (chTemplate)
			{
		case 'a':
		case 'A':
			BinAppendXmlTextStr((chTemplate == 'a') ? pAccount->m_strJID : pAccount->m_strJIDwithResource);
			break;
		case 'c':
		case 'C':
			BinAppendXmlTextStr(m_pContact->m_strJidBare);
			if (chTemplate == 'C')
				BinAppendXmlTextStr(m_pContact->m_strRessource);
			break;
		case 's':
			BinAppendXmlTextStr(pAccount->m_strServerName);
			break;
		default:
			Assert(FALSE && "Invalid template");
			BinAppendByte('^');
		case '^':
			BinAppendByte(ch);
			if (ch == '\0')
				return;	// We are done (we include the null-terminator so we also have a string)
			} // switch
		} // while
	} // BinXmlInitStanzaWithXmlRaw()

void
CSocketXmpp::Socket_WriteXmlNodeStanza(const CXmlNode * pXmlNodeStanza)
	{
	Assert(pXmlNodeStanza != NULL);
	g_strScratchBufferSocket.Empty();
	g_strScratchBufferSocket.BinAppendXmlNode(pXmlNodeStanza);
	Socket_WriteBin(g_strScratchBufferSocket);
	}

void
CSocketXmpp::Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	Socket_WriteXmlFormatted_VL(pszFmtTemplate, vlArgs);
	}

void
CSocketXmpp::Socket_WriteXmlFormatted_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	g_strScratchBufferSocket.Format_VL(pszFmtTemplate, vlArgs);
	Socket_WriteBin(g_strScratchBufferSocket);
	}

//	Reply to an <iq> stanza, however the XML is stored in g_strScratchBufferSocket as a binary.
//	This method modifies m_pXmlNodeStanzaCurrent_YZ and g_strScratchBufferStatusBar if pszFmtTemplate is present.
PSZUC
CSocketXmpp::ScratchBuffer_WriteXmlIqResult_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);
	Assert(m_pXmlNodeStanzaCurrent_YZ->FCompareTagName(c_sza_iq));
	Endorse(pszFmtTemplate == NULL);	// Send an empty content
	// A stanza reply is just changing some values for 'type', 'from' and 'to'
	CXmlNode * pXmlNodeStanzaIq = const_cast<CXmlNode *>(m_pXmlNodeStanzaCurrent_YZ);
	pXmlNodeStanzaIq->UpdateAttributeValueToStaticString(c_sza_type, c_sza_result);
	pXmlNodeStanzaIq->UpdateAttributeValuesSwap(c_sza_from, c_sza_to);
	pXmlNodeStanzaIq->m_pElementsList = NULL;	// Remove all elements, as we will replace by a single value
	pXmlNodeStanzaIq->m_pszuTagValue = NULL;
	if (pszFmtTemplate != NULL)
		{
		Assert((CHU)pszFmtTemplate[0] == d_chuXmlAlreadyEncoded);
		pXmlNodeStanzaIq->m_pszuTagValue = (PSZU)g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);	// Use the scratch buffer of the status bar to format the string (this is safe as long as we are not using the status bar at the same time)
		}
	g_strScratchBufferSocket.Empty();
	g_strScratchBufferSocket.BinAppendXmlNode(pXmlNodeStanzaIq);
	return g_strScratchBufferSocket.BinAppendNullTerminatorVirtualSzv();
	}

PSZUC
CSocketXmpp::ScratchBuffer_WriteXmlIqResult_Gsb(PSZAC pszFmtTemplate, ...)
	{
	Endorse(pszFmtTemplate == NULL);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	return ScratchBuffer_WriteXmlIqResult_VL_Gsb(pszFmtTemplate, vlArgs);
	}

//	Socket reply to an <iq> stanza
void
CSocketXmpp::Socket_WriteXmlIqResult_Gsb(PSZAC pszFmtTemplate, ...)
	{
	Endorse(pszFmtTemplate == NULL);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	ScratchBuffer_WriteXmlIqResult_VL_Gsb(pszFmtTemplate, vlArgs);
	Socket_WriteScratchBuffer();
	}

void
CSocketXmpp::Socket_WriteXmlIqReplyAcknowledge()
	{
	Socket_WriteXmlIqResult_Gsb(NULL);
	}

void
CSocketXmpp::Socket_WriteXmlPresence()
	{
	Assert(m_pAccount != NULL);
	/*
	#if 0
	#define d_szVersion			"1"	// Keep the old XCP version 1 until version 2 is fully working
	#else
	#pragma GCC warning			"[Warning] Compiling SocietyPro with XCP version 2!"
	#define d_szVersion			"2"
	#endif
	*/
	#if 1
	//#pragma GCC warning			"[Warning] Compiling SocietyPro with XOSP!"
	Socket_WriteXmlFormatted("<presence id='$S'><show>$s</show><" d_szCambrianProtocol_xcp "/></presence>", &m_pAccount->m_strJID, m_pAccount->PszGetPresenceStatus());
	#else
	Socket_WriteXmlFormatted("<presence id='$S'><show>$s</show></presence>", &m_pAccount->m_strJID, m_pAccount->PszGetPresenceStatus());	// Temporary disable the XOSP protocol
	#endif
	}

void
CSocketXmpp::Socket_WriteXmlPingToServerIfIdle()
	{
	const int cMinutesIdle = g_tsmMinutesSinceApplicationStarted - m_tsmLastStanzaReceived;
	Assert(cMinutesIdle >= 0);
	if (cMinutesIdle >= 4)
		{
		//Socket_WriteXmlFormatted("<iq from='^S' to='^S' type='get'><ping xmlns='urn:xmpp:ping'/></iq>", &m_pAccount->m_strJID, &m_pAccount->m_strServerName);
		Socket_WriteXmlFormatted("<iq to='^S' type='get'><ping xmlns='urn:xmpp:ping'/></iq>", &m_pAccount->m_strServerName);
		if (cMinutesIdle >= 15)
			{
			MessageLog_AppendTextFormatSev(eSeverityWarning, "Socket_WriteXmlPingToServerIfIdle($S) - Closing socket after $i minutes without network response\n", &m_pAccount->m_strServerName, cMinutesIdle);
			m_tsmLastStanzaReceived = g_tsmMinutesSinceApplicationStarted;	// Reset the timer
			// No response, therefore close the connection.  Hopefully the next timetick will reconnect
			Socket_Disconnect();
			}
		}
	}

void
CSocketXmpp::Socket_WriteXmlStartTls()
	{
	m_uFlagsSocketState	|= FSS_kfXmppStreamStartedTLS;
	Socket_WriteXmlFormatted("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
	}

void
CSocketXmpp::Socket_WriteXmlAuth()
	{
	Assert(m_pAccount != NULL);
	if ((m_uFlagsSocketState & FSS_kfSocketCoreAllowUnsecuredConnection) == 0)
		{
		// Before requesting the authentication, make sure the certificate encrypting the communication has been approved by the user
		if (!m_pAccount->Certificate_FIsCertificateAuthenticated())
			{
			DisplayErrorMessageToUser(eDisplayExtraError_None, "The certificate encrypting chat communication with server '$S' has not been authenticated!", &m_pAccount->m_strServerName);
			m_pAccount->NoticeListRoaming_AddNoticeCertificateNotAuthenticated(); // ertificateNotAuthenticatedNoticeListRoaming_AddNotice(PA_DELETING new WNoticeWarningCertificateNotAuthenticated(m_pAccount));
			return;
			}
		}
	if ((m_uFlagsSocketState & FSS_kfXmppUserAuthenticated) == 0)
		Socket_WriteXmlFormatted("<auth ^:ss mechanism='DIGEST-MD5'/>");	// Notify the server the client wishes to login using the authentication method DIGEST-MD5
	}

static FLAGS_SOCKET_STATE s_uFlagsSocketStatePrevious;	// Remember the previous value to avoid dumping too many entries in the Message Log

//	2014-Sep-01: I think this method is obsolete
void
CSocketXmpp::Socket_ProcessAllPendingTasks()
	{
	Assert(m_pAccount != NULL);
	if (m_uFlagsSocketState != s_uFlagsSocketStatePrevious)
		{
		s_uFlagsSocketStatePrevious = m_uFlagsSocketState;
		MessageLog_AppendTextFormatSev(eSeverityNoise, "Socket_ProcessAllPendingTasks($S) uState=0x$x state=$s\n", &m_pAccount->m_strServerName, m_uFlagsSocketState, PszaGetSocketState(state()));
		}
	if ((m_uFlagsSocketState & FSS_kfXmppSocketReadyToSendMessages) == 0)
		return;	// Don't attempt process any task if the socket has not be fully authenticated with a proper binding
	if (state() != ConnectedState)
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Socket_ProcessAllPendingTasks() - socket is NOT connected (state=$i)\n", state());
		return;	// We are not connected, so we have to wait
		}
	Assert(Socket_FuIsReadyToSendMessages());
	// Our socket is ready to send XMPP messages, therefore check if there is any contact to dispatch messages (TBD)
	#if 0
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_pAccount->m_arraypEventsUnsent.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		pEvent->Event_WriteToSocket();
		}
	#endif
	}

/*
void
CSocketXmpp::Socket_WriteRequestForContactAuthorization(TContact * pContactAuthorizing)
	{
	Assert(pContactAuthorizing != NULL);
	Assert(pContactAuthorizing->EGetRuntimeClass() == RTI(TContact));
	Socket_WriteXmlFormatted("<presence ^:jc from='^J' to='^j' type='subscribe'/>", m_pAccount, pContactAuthorizing);
	}
*/

void
CSocketXmpp::Socket_GetIpAddress(OUT CStr * pstrIpAddress) const
	{
	pstrIpAddress->InitFromStringQ(peerAddress().toString());
	}

void
CSocketXmpp::SL_SocketHostFound()
	{
	MessageLog_AppendTextFormatSev(eSeverityComment, "[$@] 0x$p SL_SocketHostFound()\n", this);
	m_uFlagsSocketState |= FSS_kfSocketCoreServerFound;
	if (m_pAccount != NULL)
		DisplayMesageToUI_VE("Establishing communication with server $S...", &m_pAccount->m_strServerName);
	}

PSZAC
PszaGetSocketState(QAbstractSocket::SocketState eState)
	{
	switch (eState)
		{
	case QAbstractSocket::UnconnectedState:
		return "UnconnectedState";
	case QAbstractSocket::HostLookupState:
		return "HostLookupState";
	case QAbstractSocket::ConnectingState:
		return "ConnectingState";
	case QAbstractSocket::ConnectedState:
		return "ConnectedState";
	case QAbstractSocket::BoundState:
		return "BoundState";
	case QAbstractSocket::ListeningState:
		return "ListeningState";
	case QAbstractSocket::ClosingState:
		return "ClosingState";
		} // switch
	Assert(FALSE);
	return "<!!!Unknown Socket State!!!>";
	} // PszaGetSocketState()

void
CSocketXmpp::SL_SocketStateChanged(QAbstractSocket::SocketState eState)
	{
	MessageLog_AppendTextFormatSev(eSeverityComment, "[$@] 0x$p SL_SocketStateChanged($s, uState=0x$x) for account $s\n", this, PszaGetSocketState(eState), m_uFlagsSocketState, (m_pAccount != NULL) ? (PSZUC)m_pAccount->m_strJID : NULL);
	Endorse(m_pAccount == NULL);	// The parent account has been deleted, however the socket is still running.
	if (m_pAccount == NULL)
		return;			// Prevent the application from crashing! In such a situation there is nothing to do because there is no more account to perform any processing.

	switch (eState)
		{
	case UnconnectedState:
		//Report((m_uFlagsSocketState & FSS_kfXmppSocketReadyToSendMessages) == 0 && "UnconnectedState should come after ClosingState");
		m_uFlagsSocketState &= ~FSS_kmXmppMask;	// Remove all XMPP flags
		//m_pAccount->TreeItemAccount_SetIconOffline();
		m_pAccount->TreeItemAccount_SetIconDisconnected();
		m_binBufferIncomingData.Empty();		// When there is no connection, make sure the buffer is empty (typically flushing data from previous session, if any)
		break;
	case HostLookupState:
		break;
	case ConnectingState:
		break;
	case ConnectedState:
		/*
		{
		QHostAddress oAddress = localAddress();
		QString sIP = oAddress.toString();
		//sIP = "abc";
		MessageLog_AppendTextFormatSev(eSeverityComment, "\t IP address = $Q\n", &sIP);
		}
		*/
		DoXmppStreamOpen();
		break;
	case BoundState:
		break;
	case ListeningState:
		break;
	case ClosingState:
		m_uFlagsSocketState &= ~FSS_kfXmppSocketReadyToSendMessages;	// As we are closing the socket, we are no longer ready to send XMPP messages
		break;
		} // switch

	MessageLog_AppendTextFormatSev(eSeverityComment, "\t new uState=0x$x\n", m_uFlagsSocketState);
	} // SL_SocketStateChanged()

#pragma GCC diagnostic ignored "-Wswitch"
void
CSocketXmpp::SL_SocketError(QAbstractSocket::SocketError eError)
	{
	const QString sError = errorString();
	MessageLog_AppendTextFormatSev(eSeverityWarning, "[$@] 0x$p SL_SocketError(eError=$i, uState=0x$x) for account ^j:\n$Q\n", this, eError, m_uFlagsSocketState, m_pAccount, &sError);
	const UINT uPort = m_pAccount->m_uServerPort;
	FLAGS_SOCKET_STATE uFlagsSocketState = m_uFlagsSocketState;
	//m_uFlagsSocketState &= ~FSS_kmXmppMask;	// Remove any previous XMPP states
	CStr strHelp;
	PSZAC pszHelp = NULL;
	switch (eError)
		{
	case ConnectionRefusedError:
		pszHelp = (PSZAC)strHelp.Format("Please make sure server has the ability to process XMPP chat messages and its firewall is not blocking port $i.", uPort);
		break;
	case RemoteHostClosedError:
		if (uFlagsSocketState & FSS_kfXmppStreamClosing)
			{
			//uFlagsSocketState &= ~FSS_kfXmppStreamClosing;		// Remove this flag, so we can get a different error next time
			MessageLog_AppendTextFormatSev(eSeverityComment, "\t Ignoring error because the socket was closed programmatically\n");
			return;
			}
		if (uFlagsSocketState & FSS_kfXmppStreamOpened)
			{
			// The stream was opened, whicm means there was a valid connection with the server
			DisplayErrorMessageToUser(eDisplayExtraError_None, "You were disconnected from the server / $Q", &sError);
			return;
			}
		if (uFlagsSocketState & FSS_kfXmppUserAuthenticated)
			{
			// The user was authenticated, therefore the disconnection is probably due to some kind of network error.
			// This is not a severe error, as when the network will become available, the chat client will reconnect.
			//m_pAccount->TreeItemAccount_SetIconDisconnected();
			return;
			}
		if ((uFlagsSocketState & FSS_kfXmppStanzaReceived) == 0)
			pszHelp = "The server terminated the connection probably because there is no software processing XMPP chat messages.";	// The XMPP stream is not opened, so the most probable error is the server does not support XMPP messages
		break;
	case SslHandshakeFailedError:
		// This error occurs when the certificate has not been validated
		m_piSocketUI->SocketUI_OnSuccess();	// The user wanted to add a new account, so ignore the error and accept as valid.  Let the user manually authenticate the certificate
		//DDialogAccountAdd::S_Success(this);
		return;
		} // switch
	DisplayErrorMessageToUser(eDisplayExtraError_None, "Network connection error with server '$S' (port $i): $Q\n$s", &m_pAccount->m_strServerName, uPort, &sError, pszHelp);
	} // SL_SocketError()


//	This signal is received when the certificate is not recognized by the operating system.
//	Before displaying an error to the user, we check if the certificate was already approved/authenticated by the user.
void
CSocketXmpp::SL_SocketErrorsSsl(const QList<QSslError> & listErrorsSsl)
	{
	MessageLog_AppendTextFormatSev(eSeverityComment, "[$@] 0x$p SL_SocketErrorsSsl(^j) with $i errors:\n", this, m_pAccount, listErrorsSsl.size());
	if ((m_uFlagsSocketState & FSS_kfSocketCoreAllowUnsecuredConnection) || m_pAccount->Certificate_FRegisterCertificateChainFromSocketAndAcceptAllErrors(IN listErrorsSsl))
		ignoreSslErrors();
	}

//	SL_SocketEntersEncryptedMode(), slot of signal QSslSocket::encrypted()
void
CSocketXmpp::SL_SocketEntersEncryptedMode()
	{
	const QString sCipher = sessionCipher().protocolString();
	MessageLog_AppendTextFormatSev(eSeverityComment, "[$@] 0x$p SL_SocketEntersEncryptedMode(^j)  $Q\n", this, m_pAccount, &sCipher);
	m_pAccount->Certificate_RegisterCertificateChainFromSocket();
	Socket_WriteXmlAuth();
	}

CSocketXmpp::CSocketXmpp(TAccountXmpp * pAccount) : ISocketUI(this), m_piSocketUI(this)
	{
	Assert(pAccount != NULL);
	m_pAccount = pAccount;
	m_uFlagsSocketState = 0;
	m_pXmlNodeStanzaCurrent_YZ = NULL;	// This is important to initialize this pointer because some errors (such as SL_SocketError) occurs before receiving any stanza
	connect(this, SIGNAL(hostFound()), this, SLOT(SL_SocketHostFound()));
	connect(this, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(SL_SocketStateChanged(QAbstractSocket::SocketState)));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(SL_SocketError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(SL_SocketErrorsSsl(QList<QSslError>)));
	connect(this, SIGNAL(encrypted()), this, SLOT(SL_SocketEntersEncryptedMode()));
	connect(this, SIGNAL(readyRead()), this, SLOT(SL_SocketDataAvailableForReading()));
	}

//	This virtual destructor is protected to prevent accidental deletion using the delete operator rather than the method Socket_Destroy().
CSocketXmpp::~CSocketXmpp()
	{
	Assert(m_pAccount == NULL && "Please use method Socket_Destroy() to destroy the socket");
//	m_arraypaTasksDownloading.DeleteAllRuntimeObjects();
	}

void
CSocketXmpp::SocketUI_Init(ISocketUI * piSocketUI, BOOL fCreateAccount)
	{
	if (piSocketUI == NULL)
		piSocketUI = this;
	m_piSocketUI = piSocketUI;
	m_piSocketUI->m_pSocket_YZ = this;
	m_uFlagsSocketState = fCreateAccount ? FSS_kfOptionCreateAccount : 0;
	}

void
CSocketXmpp::SocketUI_Remove()
	{
	m_piSocketUI = this;
	Assert(m_piSocketUI->m_pSocket_YZ == this);
	}

/*
void
CSocketXmpp::Socket_Disconnect()
	{
	//DoXmppStreamClose();
	}
*/

//	This is the equivalent of the destructor of the socket.
//	The socket mmay continue to receive a few network messages until its complete destruction.
void
CSocketXmpp::Socket_Destroy(PA_DELETING)
	{
	abort();					// Aborts the current connection and resets the socket. Unlike disconnectFromHost(), this function immediately closes the socket, clearing any pending data in the write buffer.
	deleteLater(PA_DELETING);	// This will delete (call the destructor of CSocketXmpp) when the socket object is no longer needed.
	m_pAccount = NULL;			// Indicate the corresponding account no longer exists.  This is important to prevent the socket to forward messages to a deleted object (which will crash the application).
	}

//	Return TRUE if the socket needs to be (re)connected.
BOOL
CSocketXmpp::Socket_FNeedsReconnection() const
	{
	const SocketState eState = state();
	return (eState != ConnectedState && eState != HostLookupState && eState != ConnectingState);
	}

//	Return TRUE if the GUI should display a special icon regarding a failure of the socket
BOOL
CSocketXmpp::Socket_FDisplayIconFailure() const
	{
	return FALSE;	// Code need to be implemented
	}

void
CSocketXmpp::Socket_Connect()
	{
//	MessageLog_AppendTextFormatCo(d_coOrange, "Socket_Connect($S) m_uFlagsSocketState=0x$x state=$s\n", &m_pAccount->m_strServerName, m_uFlagsSocketState, PszaGetSocketState(state()));
	Assert(m_pAccount != NULL);
	if (isOpen())
		{
		// The socket was already opened, so attempt to disconnect (this is important in case the server name or port changed)
		disconnectFromHost();
		if (state() != UnconnectedState)
			{
			//MessageLog_AppendTextFormatCo(d_coOrange, "Socket_Connect() calling waitForDisconnected() with state=$s\n", PszaGetSocketState(state()));
			DisplayMesageToUI_VE("Disconnecting from server... please wait...");
			waitForDisconnected();	// Wait for the socket to be really disconnected before attempting to reconnect
			}
		}
	if (m_pAccount->m_uServerPort == 0)
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Socket_Connect($S) - No port specified!\n", &m_pAccount->m_strServerName);

	if (m_pAccount->m_uServerPort != 443)
		connectToHost(m_pAccount->m_strServerName, m_pAccount->m_uServerPort);
	else
		connectToHostEncrypted(m_pAccount->m_strServerName, m_pAccount->m_uServerPort);
	}

void
CSocketXmpp::Socket_ConnectUnsecured()
	{
	m_uFlagsSocketState |= FSS_kfSocketCoreAllowUnsecuredConnection;
	Socket_Connect();
	}

void
CSocketXmpp::Socket_Disconnect()
	{
	DoXmppStreamClose();
	}

//	CSocketXmpp::QAbstractSocket::connectToHost()
void
CSocketXmpp::connectToHost(const QString & sHostName, quint16 uPort, OpenMode eOpenMode, NetworkLayerProtocol eProtocol)
	{
	m_uFlagsSocketState &= ~FSS_kmXmppMask;	// Remove any previous XMPP states since we are connecting
	MessageLog_AppendTextFormatSev(eSeverityInfoTextBlue, "CSocketXmpp::connectToHost($Q:$i) state=$s\n", &sHostName, uPort, PszaGetSocketState(state()));
	#ifdef DEBUG
	MessageLog_Show();
	#endif
	m_piSocketUI->SocketUI_DisplayMessage(eSeverityInfoTextBlack, m_pAccount->TreeItemAccount_SetIconConnectingToServer_Gsb());
	QSslSocket::connectToHost(sHostName, uPort, eOpenMode, eProtocol);
	}

//	CSocketXmpp::QAbstractSocket::disconnectFromHost()
void
CSocketXmpp::disconnectFromHost()
	{
	MessageLog_AppendTextFormatSev(eSeverityInfoTextBlue, "CSocketXmpp::disconnectFromHost() state=$s\n", PszaGetSocketState(state()));
	DoXmppStreamClose();	// Attempt to gracefully close the XMPP stream
	QSslSocket::disconnectFromHost();
	}

//	Open a new XMPP stream
void
CSocketXmpp::DoXmppStreamOpen()
	{
	setSocketOption(QAbstractSocket::KeepAliveOption, 1);
	setSocketOption(QAbstractSocket::LowDelayOption, QVariant (1));
	QVariant var = socketOption(QAbstractSocket::KeepAliveOption);
	MessageLog_AppendTextFormatSev(eSeverityInfoTextBlue, "DoXmppStreamOpen($i)\n", var.toInt());
	m_uFlagsSocketState |= FSS_kfXmppStreamOpening;
	//Socket_WriteXmlFormatted("<?xml version='1.0'?><stream:stream ^:jc xmlns:stream='http://etherx.jabber.org/streams' xmlns:tls='http://www.ietf.org/rfc/rfc2595.txt' to='$S' version='1.0'>", &m_pAccount->m_strServerName);
	//Socket_WriteXmlFormatted("<?xml version='1.0'?><stream:stream ^:jc xmlns:stream='http://etherx.jabber.org/streams' to='$S' version='1.0'>", &m_pAccount->m_strServerName);
	Socket_WriteXmlFormatted("<stream:stream ^:jc xmlns:stream='http://etherx.jabber.org/streams' to='^S' version='1.0'>", &m_pAccount->m_strServerName);
	}

void
CSocketXmpp::DoXmppStreamClose()
	{
	if ((m_uFlagsSocketState & (FSS_kfXmppStreamOpening | FSS_kfXmppStreamClosing)) == FSS_kfXmppStreamOpening)
		{
		// We have attempted to open a stream, so let's attempt to close the XMPP stream
		m_uFlagsSocketState |= FSS_kfXmppStreamClosing;
		MessageLog_AppendTextFormatSev(eSeverityInfoTextBlue, "DoXmppStreamClose()\n");
		Assert(strlenU(c_szaXmlStreamClose) == 16);
		(void)write(IN c_szaXmlStreamClose, 16);
		flush();	// This line is very important, otherwise the server will keep the stream open, and any attempt by the chat client to close the socket which will make the user to wait because the method waitForDisconnected() will eventually timeout.
		}
	}

#define d_coStanza	d_coYellowDirty

//	The XMPP stream just opened.
//	Check if there is any error, otherwise initiate a chat session, or create an account if DDialogAccountAdd is present.
void
CSocketXmpp::OnEventXmppStanzaStream()
	{
	Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);

	const CXmlNode * pXmlNodeError = m_pXmlNodeStanzaCurrent_YZ->PFindElementErrorStanza();
	if (pXmlNodeError == NULL)
		{
		m_uFlagsSocketState |= FSS_kfXmppStreamOpened;	// If we do not have an error, we assume the stream has been opened
		const CXmlNode * pXmlNodeStreamFeatures = m_pXmlNodeStanzaCurrent_YZ->PFindElement("stream:features");
		if (pXmlNodeStreamFeatures == NULL)
			goto Error;
		if ((m_uFlagsSocketState & (FSS_kfXmppStreamSupportsTLS | FSS_kfXmppStreamStartedTLS)) == 0)
			{
			if (pXmlNodeStreamFeatures->PFindElement("starttls"))
				{
				m_uFlagsSocketState |= FSS_kfXmppStreamSupportsTLS;
				Socket_WriteXmlStartTls();
				return;
				}
			}
		if ((m_uFlagsSocketState & FSS_kfOptionCreateAccount) == 0)
			{
			// The user does not wishes to create a new account, so attempt to login
			Login:
			if ((m_uFlagsSocketState & FSS_kfXmppUserAuthenticated) == 0)
				Socket_WriteXmlAuth();
			else
				Socket_WriteXmlFormatted("<iq type='set'><bind ^:xb><resource>Cambrian</resource></bind></iq>");	// The user is already authenticated, so bind to a resource.
			}
		else
			{
			if (m_uFlagsSocketState & FSS_kfXmppCreatingAccount)
				goto Login;	// The new account was created, therefore it is time to login
			// The user wants to create (regsiter) an account
			m_uFlagsSocketState |= FSS_kfXmppCreatingAccount;
			DisplayMesageToUI_VE("Registering account '$S' on server $S...", &m_pAccount->m_strJID, &m_pAccount->m_strServerName);
			// The XMPP stream is opened, so send a request to register (create) an account
			Socket_WriteXmlFormatted("<iq type='get' id='$p' to='$S'><query xmlns='jabber:iq:register'/></iq>", m_pAccount, &m_pAccount->m_strServerName);
			}
		return;
		} // if

	if (pXmlNodeError->PFindElement("unsupported-stanza-type"))
		{
		if (m_uFlagsSocketState & FSS_kfXmppStreamStartedTLS)
			{
			DisplayErrorMessageToUser(eDisplayExtraError_TextWithIntro, "This server does not support secure communications.");
			return;
			}
		}
	Error:
	DisplayErrorMessageToUser(eDisplayExtraError_DescriptionAndText, "Communication fatal error with server: ");
	} // OnEventXmppStanzaStream()

#if 0
/// Accepts a subscription request.
/// You can call this method in reply to the subscriptionRequest() signal.
bool QXmppRosterManager::acceptSubscription(const QString &bareJid, const QString &reason)
{
	QXmppPresence presence;
	presence.setTo(bareJid);
	presence.setType(QXmppPresence::Subscribed);
	presence.setStatusText(reason);
	return client()->sendPacket(presence);
}

/// Requests a subscription to the given contact.
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.
bool QXmppRosterManager::subscribe(const QString &bareJid, const QString &reason)
{
	QXmppPresence packet;
	packet.setTo(QXmppUtils::jidToBareJid(bareJid));
	packet.setType(QXmppPresence::Subscribe);
	packet.setStatusText(reason);
	return client()->sendPacket(packet);
}
#endif

TContact *
CSocketXmpp::PFindContactFromStanza() const
	{
	if (m_pXmlNodeStanzaCurrent_YZ == NULL)
		return NULL;		// Sometimes an error occurs (such as host not found) without any stanza
	return m_pAccount->Contact_PFindByJID(m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueFrom_NZ(), eFindContact_zDefault);
	}

//	Return TRUE if the stanza was processed by a task, and therefore no more processing is necessary.
//	Return FALSE if the caller should continue processing the stanza.
BOOL
CSocketXmpp::FStanzaProcessedByTaskMatchingEventID()
	{
	Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);
	if (m_pXmlNodeStanzaCurrent_YZ == NULL)
		return FALSE;		// Just in case, otherwise the application will crash
	TIMESTAMP tsEventID = m_pXmlNodeStanzaCurrent_YZ->LFindAttributeValueIdTimestamp_ZZR();
	if (tsEventID > d_ts_zNULL)
		{
		TContact * pContact = PFindContactFromStanza();
		if (pContact != NULL)
			{
			IEvent * pEvent = pContact->Vault_PFindEventByID(tsEventID);
			if (pEvent != NULL)
				{
				// Since we are processing XMPP stanzas, the handling is done on a case-by-case basis.
				const EEventClass eEventClass = pEvent->EGetEventClass();
				switch (eEventClass)
					{
				case CEventPing::c_eEventClass:
					pEvent->Event_SetCompletedAndUpdateWidgetWithinParentChatLog();	// A ping is very simple, as the event completes as soon as we get a reply
					return TRUE;
				case CEventVersion::c_eEventClass:
					((CEventVersion *)pEvent)->XmppProcessStanzaFromContact(m_pXmlNodeStanzaCurrent_YZ);
					return TRUE;
				case CEventFileSent::c_eEventClass:
					((CEventFileSent *)pEvent)->XmppProcessStanzaFromContact(m_pXmlNodeStanzaCurrent_YZ, pContact);
					return TRUE;
				default:
					MessageLog_AppendTextFormatSev(eSeverityWarning, "FStanzaProcessedByTaskMatchingEventID() - Ignoring tsEventID $t\n", pEvent->m_tsEventID);
					} // switch
				}
			}
		}
	return FALSE;
	} // FStanzaProcessedByTaskMatchingEventID()

BOOL
CSocketXmpp::FStanzaProcessedByTaskMatchingDownloadSID(PSZAC pszaVerbContext)
	{
	Assert(pszaVerbContext != NULL);
	Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);
	const CXmlNode * pXmlNodeVerb = m_pXmlNodeStanzaCurrent_YZ->PFindElement(pszaVerbContext);
	if (pXmlNodeVerb != NULL)
		{
		PSZUC pszSessionIdentifier = pXmlNodeVerb->PszFindAttributeValueSid_NZ();
		TIMESTAMP tsEventID = Timestamp_FromString_ML(pszSessionIdentifier);
		if (tsEventID > d_ts_zNULL)
			{
			TContact * pContact = PFindContactFromStanza();
			if (pContact != NULL)
				{
				IEvent * pEvent = pContact->Vault_PFindEventByID(tsEventID);
				if (pEvent != NULL && pEvent->EGetEventClass() == CEventFileSent::c_eEventClass)
					{
					((CEventFileSent *)pEvent)->XmppProcessStanzaVerb(m_pXmlNodeStanzaCurrent_YZ, pszaVerbContext, pXmlNodeVerb);
					return TRUE;
					}
				}
			}
		MessageLog_AppendTextFormatSev(eSeverityWarning, "Ignoring element <$s> from stanza because of missing/invalid Session Identifier '$s'.\n", pszaVerbContext, pszSessionIdentifier);
		}
	return FALSE;
	} // FStanzaProcessedByTaskMatchingDownloadSID()

//	Process the <iq> stanza.
void
CSocketXmpp::OnEventXmppStanzaIq()
	{
	Assert(strcmpU(m_pXmlNodeStanzaCurrent_YZ->m_pszuTagName, "iq") == 0);

	// The first thing with an <iq> stanza is determining if the result was a success or a failure
	PSZUC pszuType = m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueType_NZ();
	if (strcmpU(pszuType, c_sza_result) == 0)
		{
		// The server returned a valid result (success)
		if (FStanzaProcessedByTaskMatchingEventID())
			return;

		const CXmlNode * pXmlNodeQuery = m_pXmlNodeStanzaCurrent_YZ->PFindElementQuery();
		const CXmlNode * pXmlNodeBind = m_pXmlNodeStanzaCurrent_YZ->PFindElement("bind");
		if (pXmlNodeBind != NULL)
			{
			// The binding was successful, so check if there are any messages to dispatch
			m_pAccount->SetJIDwithResource(pXmlNodeBind->PszuFindElementValue(c_sza_jid));	// The full JID is stored in an element similar as "<jid>user@server/resource</jid>"
//			m_uFlagsSocketState |= FSS_kfXmppSocketReadyToSendMessages;
			m_pAccount->TreeItemAccount_SetIconOnline();
			DisplayMesageToUI_VE(c_szaEmpty);	// Clear the text
			Socket_WriteXmlPresence();	// Write the presence first, then query for the roster
			Socket_WriteXmlFormatted("<iq type='get' id='$p'><query xmlns='jabber:iq:roster'/></iq>", this);
//			Socket_ProcessAllPendingTasks();
//			NoticeListRoaming_RefreshDisplay();	// After a successful connection, there may be notices no longer relevant
			return;
			} // if (bind)

		PSZUC pszuStanzaQueryXmlns = m_pXmlNodeStanzaCurrent_YZ->PszFindStanzaValueQueryXmlns_NZ();
		if (strcmpU(pszuStanzaQueryXmlns, "jabber:iq:roster") == 0)
			{
			m_pAccount->Contacts_RosterUpdate(pXmlNodeQuery);
			m_uFlagsSocketState |= FSS_kfXmppSocketReadyToSendMessages;
			Socket_ProcessAllPendingTasks();
			m_pAccount->Contact_AllocateNewFromGlobalInvitation();	// Add any new invitation
			NoticeListRoaming_RefreshDisplay();	// After a successful connection, there may be notices no longer relevant
			m_pAccount->TreeItemAccount_UpdateIcon();	// Make sure the icon is updated
			return;
			} // if (roster)

		if (strcmpU(pszuStanzaQueryXmlns, "jabber:iq:register") == 0)
			{
			// The server allows "In-Band Registration" (XEP-0077) and is likely to offer a form as in the XML sample below:
			Assert(m_uFlagsSocketState & FSS_kfOptionCreateAccount);
			#if XML_SAMPLE
			<iq id="reg1" type="result" from="chat.cambrian.org">
				<query xmlns="jabber:iq:register">
					<instructions>Choose a username and password for use with this service.</instructions>
					<username/>
					<password/>
					<x type="form" xmlns="jabber:x:data">
						<title>Creating a new account</title>
						<instructions>Choose a username and password for use with this service.</instructions>
						<field type="text-single" label="Username" var="username">
							<required/>
						</field>
						<field type="text-private" label="Password" var="password">
							<required/>
						</field>
					</x>
				</query>
			</iq>
			#endif
			// Send the request to the server to register (create) a new user
			Socket_WriteXmlFormatted("<iq type='set' id='$p'><query xmlns='jabber:iq:register'><username>^S</username><password>^S</password></query></iq>", m_pAccount, &m_pAccount->m_strUsername, &m_pAccount->m_strPassword);
			return;
			} // if (In-Band Registration)

		if (m_uFlagsSocketState & FSS_kfOptionCreateAccount)
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Account '$S' created successfully!\n", &m_pAccount->m_strUsername);
			//m_uFlagsSocketState |= FSS_kfXmppSocketReadyToSendMessages;
			m_piSocketUI->SocketUI_OnSuccess();	// The user wanted to register (create) a new account, so it is time to close the dialog and add it to the GUI
			//DDialogAccountAdd::S_Success(this);
			Socket_WriteXmlAuth();		// Since the account is created, attempt to login with your new username and password
			return;
			}

		if (FCompareStrings(pszuStanzaQueryXmlns, "jabber:iq:version"))
			{
			// Need to handle this for non-Cambrian clients
			/*
			TContact * pContact = PFindContactFromStanza();
			if (pContact != NULL)
				pContact->Vault_AddEventToChatLogAndSendToContacts(new CEventVersion(pXmlNodeQuery));
			*/
			return;
			}

		return;
		} // if ("result")

	CXmlNode * pXmlNodeQuery = m_pXmlNodeStanzaCurrent_YZ->PFindElementQuery();
	PSZUC pszuQueryXmlns = (pXmlNodeQuery != NULL) ? pXmlNodeQuery->PszFindAttributeValueXmlns_NZ() : c_szuEmpty;
	if (FCompareStrings(pszuType, "get"))
		{
		if (FCompareStrings(pszuQueryXmlns, "jabber:iq:version"))
			{
			Socket_WriteXmlIqResult_Gsb(d_szuXmlAlreadyEncoded "<query><name>" d_szApplicationName "</name><version>" d_szApplicationVersion "</version><os>" d_szOS "</os></query>");
			return;
			}
		if (m_pXmlNodeStanzaCurrent_YZ->PFindElement(c_sza_ping))
			{
			Socket_WriteXmlIqReplyAcknowledge();
			return;
			}
		if (FCompareStrings(pszuQueryXmlns, "http://jabber.org/protocol/disco#info"))
			{
			/*
			<iq id="purple8ab4072e" type="result" to="jon@chat.cambrian.org/abc" from="peter@jabber.example.com/worker">
			<query xmlns="http://jabber.org/protocol/disco#info">
				<identity type="pc" name="Pidgin" category="client"/>
				<feature var="jabber:iq:last"/>
				<feature var="jabber:iq:oob"/>
				<feature var="urn:xmpp:time"/>
				<feature var="jabber:iq:version"/>
				<feature var="jabber:x:conference"/>
				<feature var="http://jabber.org/protocol/bytestreams"/>
				<feature var="http://jabber.org/protocol/caps"/>
				<feature var="http://jabber.org/protocol/chatstates"/>
				<feature var="http://jabber.org/protocol/disco#info"/>
				<feature var="http://jabber.org/protocol/disco#items"/>
				<feature var="http://jabber.org/protocol/muc"/>
				<feature var="http://jabber.org/protocol/muc#user"/>
				<feature var="http://jabber.org/protocol/si"/>
				<feature var="http://jabber.org/protocol/si/profile/file-transfer"/>
				<feature var="http://jabber.org/protocol/xhtml-im"/>
				<feature var="urn:xmpp:ping"/>
				<feature var="urn:xmpp:attention:0"/>
				<feature var="urn:xmpp:bob"/>
				<feature var="urn:xmpp:jingle:1"/>
				<feature var="urn:xmpp:avatar:metadata+notify"/>
				<feature var="http://jabber.org/protocol/mood+notify"/>
				<feature var="http://jabber.org/protocol/tune+notify"/>
				<feature var="http://jabber.org/protocol/nick+notify"/>
				<feature var="http://jabber.org/protocol/ibb"/>
			</query>
			</iq>
			*/
			Socket_WriteXmlIqResult_Gsb(d_szuXmlAlreadyEncoded
				"<query xmlns='http://jabber.org/protocol/disco#info'>"
					"<identity type='pc' name='Cambrian' category='client'/>"
					"<feature var='http://jabber.org/protocol/ibb'/>"
					"<feature var='http://jabber.org/protocol/si'/>"
					"<feature var='http://jabber.org/protocol/si/profile/file-transfer'/>"
					"<feature var='urn:xmpp:ping'/>"
					"<feature var='jabber:iq:version'/>"
				"</query>");
			return;
			}

		return;
		} // if ("get")

	if (strcmpU(pszuType, "set") == 0)
		{
		// The server returned "set".  This situation happen when querying for the roster
		if (pXmlNodeQuery != NULL)
			{
			PSZUC pszuXmlns = pXmlNodeQuery->PszFindAttributeValueXmlns_NZ();
			if (strcmpU(pszuXmlns, "jabber:iq:roster") == 0)
				{
				m_pAccount->Contacts_RosterUpdate(pXmlNodeQuery);
				}
			return;
			}
		CXmlNode * pXmlNodeStreamInitiation = m_pXmlNodeStanzaCurrent_YZ->PFindElementSi();
		if (pXmlNodeStreamInitiation != NULL)
			{
			// We have received an offer for a file, so create a task to accept it
			/*
			TContact * pContact = PFindContactFromStanza();
			if (pContact != NULL)
				{
				// This is somewhat a 'hack' to determine if the file was already offered.  Cambrian uses the same id for the stanza <iq> and the steam initialization <si>
				PSZUC pszSID = pXmlNodeStreamInitiation->PszFindAttributeValueId_NZ();
				if (FCompareStrings(m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueId_NZ(), pszSID))
					{
					// Search if there is already a task for downloading
					//TIMESTAMP tsEventID = Timestamp_FromString_ZZR(pszSID);
					CVaultEvents * pVault = pContact->m_paVaultEvents;
					if (pVault != NULL)
						{
						CEventFileReceived ** ppEventStop;
						CEventFileReceived ** ppEvent = (CEventFileReceived **)pVault->m_arraypaEvents.PrgpvGetElementsStop(OUT (void ***)&ppEventStop);
						while (ppEvent != ppEventStop)
							{
							CEventFileReceived * pEvent = *--ppEventStop;
							AssertValidEvent(pEvent);
							if (pEvent->EGetEventClass() == CEventFileReceived::c_eEventClass)
								{
								if (pEvent->m_strxSID.FCompareStringsExactCase(pszSID))
									{
									MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "There is already an open offer to download file '$S' (tsEventID=$t, SID=$S)\n", &pEvent->m_strFileName, pEvent->m_tsEventID, &pEvent->m_strxSID);
									return;
									}
								}
							} // while
						} // if
					} // if
				pContact->Vault_PAllocateEventFileReceived(pXmlNodeStreamInitiation);
				return;
				}
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to accept file because of missing parameters!");
			return;
			*/
			}
		if (FStanzaProcessedByTaskMatchingDownloadSID(c_sza_open) ||
			FStanzaProcessedByTaskMatchingDownloadSID(c_sza_data) ||
			FStanzaProcessedByTaskMatchingDownloadSID(c_sza_close))
			return;
		return;
		} // if ("set")

	// At this point, assume the stanza is an error.  We attempt to forward the stanza to the task and if there is no task processing the error, we display the error to the user.
	if (!FStanzaProcessedByTaskMatchingEventID())
		DisplayStanzaErrorToUser();
	} // OnEventXmppStanzaIq()


//	Find the element representing the stanza error.
//	Typically this is the element named <error> however it may be one of the following: <failure>, <conflict> or <stream:error>.
//	Return NULL if there is no such element.
CXmlNode *
CXmlNode::PFindElementErrorStanza() const
	{
	if (FCompareTagName(c_sza_error) ||
		FCompareTagName(c_sza_failure) ||
		FCompareTagName(c_sza_conflict) ||
		FCompareTagName(c_szaXmlStreamError))
		return const_cast<CXmlNode *>(this);
	CXmlNode * pXmlElementError = PFindElementError();
	if (pXmlElementError != NULL)
		return pXmlElementError;
	// So far, we have not found any element <error> so attempt to search for <stream:error> as the last resort
	return PFindElement(c_szaXmlStreamError);
	}

PSZUC
CXmlNode::PszFindElementStanzaErrorDescription() const
	{
	CXmlNode * pXmlNodeError = PFindElementErrorStanza();
	if (pXmlNodeError != NULL)
		return (PSZUC)pXmlNodeError->PszFindStanzaErrorDescription();
	return NULL;
	}

PSZAC
CXmlNode::PszFindStanzaErrorDescription() const
	{
	if (PFindElement("not-authorized"))
		return "Not Authorized - The authentication failed because the Username or Password is incorrect. Since Usernames are case sensitives, try typing your Username all in lowercase, or the same way as you created it.";
	if (PFindElement("not-acceptable"))
		return "Not Acceptable -  The request was rejected by the server because it does not meet criteria defined by policies of the server or recipient.";
	if (PFindElement("malformed-request"))
		return "Malformed Request - The data sent to the server violates the syntax for the specified SASL mechanism.  Please contact the developers.";
	if (PFindElement("not-well-formed"))
		return "SocietyPro has sent XML that violates the well-formedness rules of XML.";
	if (PFindElement("system-shutdown"))
		return "The XMPP server is shutting down and consequently disconnecting you.";	// <stream:error><system-shutdown xmlns="urn:ietf:params:xml:ns:xmpp-streams"/><text xmlns="urn:ietf:params:xml:ns:xmpp-streams">Received SIGTERM</text></stream:error>

	// At this point, we have no idea what is the error description, so attempt to use the name of the element (XMPP often uses element names to report errors)
	const CXmlNode * pElementError = m_pElementsList;
	//pElementError = NULL;
	if (pElementError != NULL)
		{
		if (pElementError->FCompareTagName(c_sza_text))
			pElementError = pElementError->m_pNextSibling;	// We have the <text> element, so skip the next element and hopefully the error is the next element
		if (pElementError != NULL)
			{
			PSZAC pszErrorName = (PSZAC)pElementError->m_pszuTagName;
			//pszErrorName = NULL;	// For testing the code case below
			if (pszErrorName != NULL && pszErrorName[0] != '\0')
				return pszErrorName;
			/*
			if (pszErrorName != NULL)
				{
				if (FCompareStrings(pszErrorName, c_sza_error))
					{
					// The error name is "error" which is pretty useless, so attempt to fetch the element from the child node
					const CXmlNode * pElementErrorDescription = pElementError->m_pElementsList;
					if (pElementErrorDescription != NULL)
						pszErrorName = (PSZAC)pElementErrorDescription->m_pszuTagName;
					}
				if (pszErrorName[0] != '\0')
					return pszErrorName;
				}
			*/
			}
		}
	// We were not able to find a non-empty element name, so we have to use a more drastic solution: return the entire stanza!
	CBin binStanza;
	binStanza.BinAppendXmlNode(PFindRootNodeXmlTreeParent());	// Find the root element of the stanza and serialize its content into a bin
	// Since we cannot return a string from the stack, use the allocator of the object CXmlTree to to store the content of the bin.  This trick will not lead to an increase of memory because the XML tree is destroyed (or its memory is recycled) after each stanza is processed.
	return (PSZAC)PFindMemoryAccumulatorOfXmlTreeParent()->PszuAllocateCopyCBinString(binStanza);
	} // PszFindStanzaErrorDescription()


//	Find the value of the element <text> from <error> (if any), or return the value of the element <text>.
PSZUC
CXmlNode::PszFindElementValueStanzaErrorText() const
	{
	// First, search if there is a <error> element
	const CXmlNode * pXmlElementError = PFindElementErrorStanza();
	if (pXmlElementError == NULL)
		pXmlElementError = this;	// Otherwise return the value of <text> (if any)
	return pXmlElementError->PszFindElementValueText();
	}

void
CSocketXmpp::DisplayMesageToUI_VE(PSZAC pszFmtTemplate, ...)
	{
	CStr strMessage;
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	m_piSocketUI->SocketUI_DisplayMessage(eSeverityInfoTextBlack, strMessage.Format_VL(pszFmtTemplate, vlArgs));
	}

void
CSocketXmpp::DisplayStanzaErrorToUser()
	{
	const CXmlNode * pXmlElementError = NULL;
	if (m_pXmlNodeStanzaCurrent_YZ != NULL)
		pXmlElementError = m_pXmlNodeStanzaCurrent_YZ->PFindElementErrorStanza();

	if (pXmlElementError != NULL)
		{
		const CXmlNode * pXmlElementErrorServiceUnavailable = pXmlElementError->PFindElement("service-unavailable");
		if (m_uFlagsSocketState & FSS_kfOptionCreateAccount)
			{
			// We are attempting to create an account
			if (pXmlElementError->PFindElement(c_sza_conflict) != NULL)
				{
				// A 'conflict' means the username is already in use
				DisplayMesageToUI_VE("The requested username '$S' is already in use, trying to login instead.", &m_pAccount->m_strUsername);
				Socket_WriteXmlAuth();
				return;
				/*
				DisplayErrorMessageToUser(eDisplayExtraError_None, "The requested username '$S' is already used by someone else. Please try another username.", &m_pAccount->m_strUsername);
				DDialogAccountAdd::S_SetFocusToUsername(this);
				return;
				*/
				}
			if (pXmlElementErrorServiceUnavailable != NULL)
				{
				DisplayErrorMessageToUser(eDisplayExtraError_TextWithIntro, "The server does not allow the creation of a new account from the software.  To create an account on this server, please contact the administrator and ask him/her to create your account.");
				return;
				}
			/*
			if (pXmlElementError->PFindElement("not-acceptable") != NULL)
				{
				// <error type="modify"><not-acceptable/><text>The requested username is invalid.</text></error>
				DisplayErrorMessageToUser(pXmlElementError, "Not Acceptable -  The request was rejected by the server because it does not meet criteria defined by policies of the server or recipient.");
				return;
				}
			if (pXmlElementError->PFindElement("malformed-request") != NULL)
				{
				Report(strcmpU(pXmlElementError->PszuFindAttributeValueXmlns(), c_szXmlns_urn_ietf_params_xml_ns_xmpp_sasl) == 0);
				// According to http://xmpp.org/rfcs/rfc6120.html, this error is sent in reply to an <abort/>, <auth/>, <challenge/>, or <response/> element.
				DisplayErrorMessageToUser(pXmlElementError, "Malformed Request - The data sent to the server violates the syntax for the specified SASL mechanism.  Please contact the developers.");
				return;
				}
			*/
			DisplayErrorMessageToUser(eDisplayExtraError_DescriptionAndText, NULL);	// There is no need for a formatting template because the dialog creating the account will display "Unable to create account" before the error description.
			return;
			} // if (creating account)

		if (pXmlElementError->FCompareTagName(c_szaXmlStreamError))
			{
			DisplayErrorMessageToUser(eDisplayExtraError_DescriptionAndText, "Network stream error: ");
			return;
			}
		if (pXmlElementError->FCompareTagName(c_sza_failure))
			{
			// The <failure> stanza is typically a failed login.  Example: <failure xmlns="urn:ietf:params:xml:ns:xmpp-sasl"><not-authorized/></failure>
			Report(strcmpU(m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueXmlns_NZ(), c_szXmlns_urn_ietf_params_xml_ns_xmpp_sasl) == 0);

			if (m_uFlagsSocketState & FSS_kfOptionCreateAccount)
				{
				// We are attempting to create an account and we got this error because we were trying to login instead
				DisplayErrorMessageToUser(eDisplayExtraError_None, "The requested username '$S' is already used by someone else. Please try another username.", &m_pAccount->m_strUsername);
				m_piSocketUI->SocketUI_OnError();
				//DDialogAccountAdd::S_SetFocusToUsername(this);
				return;
				}
			/*
			if (m_pXmlNodeStanzaCurrent_YZ->PFindElement("not-authorized") != NULL)
				{
				DisplayErrorMessageToUser(eDisplayExtraError_TextWithIntro, "Not Authorized - The authentication failed because the Username or Password is incorrect. "
					"Since Usernames are case sensitives, try typing your Username all in lowercase, or the same way as you created it.");
				return;
				}
			*/
			DisplayErrorMessageToUser(eDisplayExtraError_DescriptionAndText, "Authentication failure: ");
			return;
			}
		DisplayErrorMessageToUser((pXmlElementErrorServiceUnavailable != NULL) ? eDisplayExtraError_DescriptionAndTextIconOffline : eDisplayExtraError_DescriptionAndText, "Network Error: ");	// The Service Unavailable is a soft error, typically indicating the user is no longer connected. Such a situation occurs quite often when a device/computer enters in sleep mode while pretending being online.
		return;
		} // if (error)

	// We are unable to find any <error> element, so just skip it.  After all, it is probably not an error but rather a stanza unknown to the program
	MessageLog_AppendTextFormatSev(eSeverityWarning, "No processing done for the following unrecognized stanza received from the server '$S':\n^N", &m_pAccount->m_strServerName, m_pXmlNodeStanzaCurrent_YZ);
	} // DisplayStanzaErrorToUser()

//	Method to display socket errors to the user.
void
CSocketXmpp::DisplayErrorMessageToUser(EDisplayExtraError eDisplayExtraError, PSZAC pszFmtTemplate, ...)
	{
	Endorse(pszFmtTemplate == NULL);			// The entire error message will come from eDisplayExtraError
	Endorse(m_pXmlNodeStanzaCurrent_YZ == NULL);	// The error may occur at the time the socket is connecting and therefore there is no stanza to process
//	MessageLog_AppendTextFormatCo(d_coOrangeRed, "DisplayErrorMessageToUser(eDisplayExtraError=$i, pszFmtTemplate=\"$s\")\n^N", eDisplayExtraError, pszFmtTemplate, m_pXmlNodeStanzaCurrent_YZ);

	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	CStr strError;
	strError.Format_VL((pszFmtTemplate != NULL) ? pszFmtTemplate : c_szaEmpty, vlArgs);
	if (eDisplayExtraError != eDisplayExtraError_None && m_pXmlNodeStanzaCurrent_YZ != NULL)
		{
		CXmlNode * pXmlElementError = m_pXmlNodeStanzaCurrent_YZ->PFindElementErrorStanza();
		if (pXmlElementError != NULL)
			{
			if (eDisplayExtraError & eDisplayExtraError_kfStanzaDescription)
				strError.AppendTextU((PSZUC)pXmlElementError->PszFindStanzaErrorDescription());
			if (eDisplayExtraError & eDisplayExtraError_kfTextValue)
				strError.AppendSeparatorAndTextU((eDisplayExtraError & eDisplayExtraError_kfTextIntroduction) ? "\n\tError text returned by the server: " : NULL, pXmlElementError->PszFindElementValueStanzaErrorText());
			}
		}

	PSZUC pszMessageError = strError.TrimTailingWhiteSpacesNZ();
	m_piSocketUI->SocketUI_DisplayMessage((eDisplayExtraError & eDisplayExtraError_kfSetIconOffline) ? eSeverityNoise : eSeverityErrorWarning, pszMessageError);
//	StatusBar_SetTextErrorU(pszMessageError);
	ErrorLog_AddNewMessage((PSZAC)pszMessageError, IN m_pXmlNodeStanzaCurrent_YZ); // Add the error to the Error Log with the entire stanza

	ITreeItem * pTreeItemError = PFindContactFromStanza();	// Try to determine if the error is related to a contact or related to the account
	if (pTreeItemError == NULL)
		pTreeItemError = m_pAccount;
	pTreeItemError->TreeItemW_SetIconError(pszMessageError, (eDisplayExtraError & eDisplayExtraError_kfSetIconOffline) ? eMenuAction_PresenceAccountOffline : eMenuIconFailure);
	NoticeListRoaming_RefreshDisplay();	// If an error occurs, perhaps there may be notices which must be updated and/or no longer relevant
	} // DisplayErrorMessageToUser()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	OnEventXmppStanzaArrived()
//
//	This is the main routine processing all incoming XML packets from the network socket.
//
//	In the XMPP documentation, an XML stanza is a discrete semantic unit (XML element) of structured information that is sent from one entity to another over an XML stream.
//	In this project, an XML stanza is any valid XML element coming from the socket; not limited to <message/>, <presence/>, and <iq/> elements.
//
void
CSocketXmpp::OnEventXmppStanzaArrived()
	{
	Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);
	MessageLog_AppendTextFormatCo(d_coGoldenRod, "CSocketXmpp::OnEventXmppStanzaArrived(0x$p)  md5={Nf}\n", m_pXmlNodeStanzaCurrent_YZ, m_pXmlNodeStanzaCurrent_YZ);
	DebugDisplayStanzaToMessageLog();
	//m_pAccount->DebugDumpContacts();

	// Before searching for any XMPP specific method, check if we are using the Cambrian Protocol.
	CXmlNode * pXmlNodeXCP = m_pXmlNodeStanzaCurrent_YZ->PFindElement(c_sza_xcp);
	if (pXmlNodeXCP != NULL)
		{
		TContact * pContact = m_pAccount->Contact_PFindByJID(IN m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueFrom_NZ(), eFindContact_kfCreateNew);	// Find the contact matching the the stanza
		if (pXmlNodeXCP->m_pszuTagValue != NULL)
			{
			if (pContact != NULL)
				pContact->XmppXcp_ProcessStanza(pXmlNodeXCP);
			else
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "XMPP Stanza contains an invalid contact:\n^N", m_pXmlNodeStanzaCurrent_YZ);	// This happens when the stanza is incomplete and the JID is not adequate to create a new contact
			return;
			}
		}

	PSZUC pszuNameStanza = m_pXmlNodeStanzaCurrent_YZ->m_pszuTagName;
	Assert(pszuNameStanza != NULL);
	if (pszuNameStanza[0] == 'i' && pszuNameStanza[1] == 'q' && pszuNameStanza[2] == '\0')
		{
		OnEventXmppStanzaIq();
		return;
		}
	if (FCompareStrings(pszuNameStanza, "presence"))
		{
		if (m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueTypeNoError() != NULL)
			{
			m_pAccount->Contact_PresenceUpdate(m_pXmlNodeStanzaCurrent_YZ);
			return;
			}
		DisplayErrorMessageToUser(eDisplayExtraError_DescriptionAndText, "Stanza Presence Error: ");
		return;
		}
	if (FCompareStrings(pszuNameStanza, "message"))
		{
		if (m_pXmlNodeStanzaCurrent_YZ->PszFindAttributeValueTypeNoError() != NULL)
			{
			m_pAccount->ChatLog_DisplayStanza(m_pXmlNodeStanzaCurrent_YZ);
			return;
			}
		// We have an error with the message
		#ifdef XML_SAMPLE
		<message type="error" to="abc@example.com" from="def@example.com">
			<error type="cancel">
				<service-unavailable xmlns="urn:ietf:params:xml:ns:xmpp-stanzas"/>
			</error>
		</message>
		#endif // XML_SAMPLE
		DisplayErrorMessageToUser(eDisplayExtraError_DescriptionAndText, "Stanza Message Error: ");
		return;
		}
	if (FCompareStrings(pszuNameStanza, "success"))
		{
		// The connection is now authenticated, so re-open a stream to fetch the new stream features
		m_uFlagsSocketState |= FSS_kfXmppUserAuthenticated;
		DoXmppStreamOpen();
		m_piSocketUI->SocketUI_OnSuccess();		// The user wanted to add a new account, so it is time to close the dialog (if any) and continue with the GUI
		//DDialogAccountAdd::S_Success(this);	// The user wanted to add a new account, so it is time to close the dialog and add it to the GUI
		return;
		} // if (success)
	if (FCompareStrings(pszuNameStanza, "challenge"))
		{
		// This is the challenge necessary for a login
		CStr strResponse;
		PSZUC pszuChallengeDecoded = strResponse.BinAppendBinaryDataFromBase64Szv(m_pXmlNodeStanzaCurrent_YZ->m_pszuTagValue);
		//MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "Challenge Decoded: $s\n", pszuChallengeDecoded);	// Challenge Decoded: realm="chat.cambrian.org",nonce="a8389990-c66e-4ded-b8cc-e34d5beddb69",qop="auth",charset=utf-8,algorithm=md5-sess
		CStr strNonce;
		PSZUC pszuNonce = strNonce.InitFromValueOfCommaSeparatedStringPsz(IN pszuChallengeDecoded, "nonce");
		QUuid guidClientNonce = QUuid::createUuid();
//		InitToZeroes(OUT &guidClientNonce, sizeof(guidClientNonce));	// For debugging

		PSZUC pszuServer = m_pAccount->m_strServerName;
		SHashMd5 hashA, hashB;
		HashMd5_CalculateFromStringFormatted_Gsb(OUT &hashA, "$S:$s:$S", &m_pAccount->m_strUsername, pszuServer, &m_pAccount->m_strPassword);	// Get the hash value for the "secret"
		HashMd5_CalculateFromStringFormatted_Gsb(OUT &hashA, "{gn}:$s:{gf}", IN &hashA, pszuNonce, &guidClientNonce);
		HashMd5_CalculateFromStringFormatted_Gsb(OUT &hashB, "AUTHENTICATE:xmpp/$s", pszuServer);
		HashMd5_CalculateFromStringFormatted_Gsb(OUT &hashB, "{gf}:$s:00000001:{gf}:auth:{gf}", &hashA, pszuNonce, &guidClientNonce, &hashB);

		strResponse.Format("username=\"$S\",realm=\"$s\",nonce=\"$s\",cnonce=\"{gf}\",nc=00000001,qop=auth,digest-uri=\"xmpp/$s\",response=\"{gf}\",charset=utf-8", &m_pAccount->m_strUsername, pszuServer, pszuNonce, &guidClientNonce, pszuServer, &hashB);
		HashMd5_CalculateFromCStr(OUT &hashA, IN strResponse);
		//MessageLog_AppendTextFormatCo(d_coGrayLight, "Responding to Challenge (md5={gf}): $S\n", &hashA, &strResponse);
		Socket_WriteXmlFormatted("<response ^:ss>{S/}</response>", &strResponse);
		return;
		} // if (challenge)

	if (memcmp(pszuNameStanza, "stream:", 7) == 0)
		{
		// We just initiated a stream
		OnEventXmppStanzaStream();
		return;
		}
	if (FCompareStrings(pszuNameStanza, "proceed"))
		{
		// MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGreen), "Stanza <proceed> received ... what is next?\n");
		startClientEncryption();
		DoXmppStreamOpen();
		return;
		}

	DisplayStanzaErrorToUser();
	} // OnEventXmppStanzaArrived()

//	SL_SocketDataAvailableForReading(), slot of signal QIODevice::readyRead()
void
CSocketXmpp::SL_SocketDataAvailableForReading()
	{
	Assert(m_pAccount != NULL);
	m_tsmLastStanzaReceived = g_tsmMinutesSinceApplicationStarted;
	g_cMinutesIdleNetworkDataReceived = 0;		// Reset the idle counter

	const int cbDataAvailable = bytesAvailable();	// Get how many bytes are available from the socket
	MessageLog_AppendTextFormatCo(d_coYellowDark, "[$@] 0x$p SL_SocketDataAvailableForReading($I bytes) for account $S\n", this, cbDataAvailable, &m_pAccount->m_strJID);
	//m_binBufferIncomingData.BinAppendText((PSZUC)"yyy");	// Inject some corrupted XML data (to test if our routines detect XML corruption and handle it properly)
	if (m_pXmlNodeStanzaCurrent_YZ != NULL)
		{
		// Although the documentation says the signal readyRead() is NOT reentrant, there are cases where it is.  Better be safe than sorry!
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[$@] 0x$p SL_SocketDataAvailableForReading() is reentrant!  Ignoring $I bytes while waiting for completion of processing previous data...\n", this, cbDataAvailable);
		return;
		}
	Report(m_pAccount != NULL);
	if (m_pAccount == NULL)
		{
		// This is is a very very rare case where the socket was destroyed yet received data
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coRed), "\t Read $I bytes however ignoring them because m_pAccount is NULL.\n", cbDataAvailable);
		return;
		}

	g_oMutex.lock();
	char * pbData = (char *)m_binBufferIncomingData.PbeAllocateExtraDataWithVirtualNullTerminator(cbDataAvailable);
	const int cbDataRead = read(OUT pbData, cbDataAvailable);	// Read the data from the socket
	Assert((cbDataRead == cbDataAvailable) && "All data should be read!");
	Assert((int)strlen(pbData) == cbDataRead);
	//m_binBufferIncomingData.BinAppendText((PSZUC)"zzz");	// Inject some corrupted XML data (to test if our routines detect XML corruption and handle it properly)
	//m_binBufferIncomingData.AppendVirtualNullTerminator();

	// Scan the buffer for any valid XML content.
	PSZUC pszuDataBuffer = m_binBufferIncomingData.PbGetData();
	PSZUC pszuDataBufferXmlStop DEBUG_CODE( = d_pszuGarbageValue );
	PSZUC pszuDataBufferXmlStart = PszuFindValidXmlElements(IN pszuDataBuffer, OUT_F_INV &pszuDataBufferXmlStop);
	if (pszuDataBufferXmlStart != NULL)
		{
		Assert(pszuDataBufferXmlStop != d_pszuGarbageValue);

		// We have enough data to for a complete XML element / stanza
		CErrorMessage error;
		CXmlTree oXmlTree;
		//oXmlTree.DestroyLargeBuffers();
		oXmlTree.SetFileDataCopy(IN pszuDataBufferXmlStart, IN pszuDataBufferXmlStop);
		if (oXmlTree.EParseFileDataToXmlNodes(INOUT_F_VALID &error) == errSuccess)
			{
			// Report any data skipped (typically because the data was corrupted)
			while (pszuDataBuffer < pszuDataBufferXmlStart)
				{
				const UINT ch = *pszuDataBuffer;
				if (ch == '\0')
					break;
				if (!Ch_FIsWhiteSpace(ch))
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Skipping $I bytes of corrupted XML data within socket buffer: {Pn}\n", pszuDataBufferXmlStart - pszuDataBuffer, pszuDataBuffer, pszuDataBufferXmlStart);
					ErrorLog_AddNewMessage("Corrupted XML data", pszuDataBuffer);
					ErrorLog_Show();	// This kind of error is worth showing immediately to the user
					break;
					}
				pszuDataBuffer++;
				} // while

			// Process the stanzas
			m_uFlagsSocketState |= FSS_kfXmppStanzaReceived;
			m_pAccount->TreeItemAccount_SetIconOnline();	// If we are able to receive a valid stanza, it means the connection is good.  If the stanza is an error, then the method DisplayStanzaErrorToUser() will take of reporting the error.
			m_pXmlNodeStanzaCurrent_YZ = &oXmlTree;
			do
				{
				OnEventXmppStanzaArrived();
				Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);		// This happens if the slot SL_SocketDataAvailableForReading() reenters.  It should happen anymore, but it is good to verify
				if (m_pXmlNodeStanzaCurrent_YZ != NULL)
					m_pXmlNodeStanzaCurrent_YZ = m_pXmlNodeStanzaCurrent_YZ->m_pNextSibling;
				}
			while (m_pXmlNodeStanzaCurrent_YZ != NULL);

			if (*pszuDataBufferXmlStop != '\0')
				{
				Assert(strlenU(c_szaXmlStreamClose) == 16);
				if (memcmp(pszuDataBufferXmlStop, c_szaXmlStreamClose, 16) != 0)
					{
					MessageLog_AppendTextFormatCo(COX_MakeBold(d_coOrange), "Remaining $I bytes in buffer: {sm}\n", m_binBufferIncomingData.CbGetData() - (pszuDataBufferXmlStop - pszuDataBuffer), pszuDataBufferXmlStop);
					m_binBufferIncomingData.DataRemoveUntilPointer(pszuDataBufferXmlStop);
					goto EnsureValidString;
					}
				// We received "</stream:stream>, therefore close the stream
				DoXmppStreamClose();
				}
			m_binBufferIncomingData.Empty();	// We no longer need the buffer, since we parsed and processed the XML stanzas.
			goto UnlockMutex;
			} // if (valid XML nodes)

		// The XML is not valid, so display the error
		(void)error.FDisplayToMessageLog(); // TODO: Use DisplaySocketErrorToUser() to report this error
		} // if
	DebugDisplayPendingDataToMessageLog();
	EnsureValidString:
	m_binBufferIncomingData.BinEnsureContentHasNoNullTerminatorAndIsTerminatedWithVirtualNullTerminator();
	UnlockMutex:
	g_oMutex.unlock();
	} // SL_SocketDataAvailableForReading()


//	This methods display the content of a stanza only to what is relevant
void
CSocketXmpp::DebugDisplayStanzaToMessageLog()
	{
	Assert(m_pXmlNodeStanzaCurrent_YZ != NULL);
	PSZUC pszNameStanza = m_pXmlNodeStanzaCurrent_YZ->m_pszuTagName;
	if (FCompareStrings(pszNameStanza, "iq"))
		{
		const CXmlNode * pXmlNodeData = m_pXmlNodeStanzaCurrent_YZ->PFindElement("data");
		if (pXmlNodeData != NULL)
			return;	// Don't display stanzas containg data in base64
		}

	MessageLog_AppendTextFormatCo(d_coChocolate, "{Nm}\n", m_pXmlNodeStanzaCurrent_YZ);
	}

void
CSocketXmpp::DebugDisplayPendingDataToMessageLog()
	{
	BYTE * pbData = m_binBufferIncomingData.PbGetData();
	if (pbData == NULL)
		return;
	if (memcmp(pbData, "<iq ", 4) == 0)
		return;	// Don't display lage <iq>
	MessageLog_AppendTextFormatCo(d_coOrange, "Pending $I bytes of XML data: {Bm}\n", m_binBufferIncomingData.CbGetData(), &m_binBufferIncomingData);
	}


//	Method displaying to the Message Log what is being written/sent to the socket.
//	This method scans the buffer and attempt to be smart by avoiding displaying large blocks of data, so the Message Log displays meaningful messages.
void
CSocketXmpp::DebugDisplayDataToWriteMessageLog(PCVOID pvData, int cbData)
	{
	CBin bin;
	bin.BinInitFromBinaryDataWithVirtualNullTerminator(pvData, cbData);
	int ibElementDataOpen = bin.IbFindDataAfterString("<data");
	if (ibElementDataOpen > 0)
		{
		int ibElementDataClose = bin.IbFindDataAfterString("</data>");
		int cbElementData = ibElementDataClose - ibElementDataOpen;
		if (cbElementData > 100)
			{
			CHA szReplaceBy[40];
			int cbReplaceBy = sprintf(OUT szReplaceBy, " [ %d bytes removed ] ", cbElementData);
			bin.SubstituteData(ibElementDataOpen + 50, cbElementData - 75, szReplaceBy, cbReplaceBy);
			}
		}
	MessageLog_AppendTextFormatCo(d_coSocketWriteContent, "{Bm}\n", &bin);
	}
