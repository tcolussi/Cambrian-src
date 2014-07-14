///////////////////////////////////////////////////////////////////////////////////////////////////
//	Sockets.h
//
//	Classes for network sockets.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SOCKETS_H
#define SOCKETS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
extern CStr g_strScratchBufferSocket;
extern QMutex g_oMutex;

//	Interface to allow the socket to provide feedback to the user interface (UI).
//	This interface is used when creating a new XMPP account and reporting an error to the user (such as invalid username or password, or a network failure)
class ISocketUI
{
protected:
	WEditTextArea * m_pwEditSocketMessageLog_YZ;	// Widget displaying the messages
	CSocketXmpp * m_pSocket_YZ;						// Socket assigned with the UI.  This socket is useful to fetch information when an error occurs.
public:
	ISocketUI();
	ISocketUI(CSocketXmpp * pSocket);
	~ISocketUI();
	TAccountXmpp * PGetAccount_YZ() const;
	virtual void SocketUI_OnSuccess();
	virtual void SocketUI_OnError();
	virtual void SocketUI_DisplayMessage(ESeverity eSeverity, PSZUC pszuTextMessage);
	friend class CSocketXmpp;
};

///////////////////////////////////////
//	FLAGS_SOCKET_STATE
//
//	Flags representing the state of a socket.
//	The purpose of this enumeration is to determine the context of a socket error.
//	For instance, the error SocketError::RemoteHostClosedError could mean the server does not support XMPP messages, or the connection was closed
//	because of a long period of network inactivity. Only the socket state may determine which error message is appropriate to display to the user.
#define FSS_kfSocketCoreServerFound					0x000001	// The host (server) was found (resolved)
#define FSS_kfSocketCoreAllowUnsecuredConnection	0x000002	// Allow the socket to connect despite the certificate was not authenticated
#define FSS_kmSocketCoreMask						0x00000F	// Bits reserved for the CSocketCore (when implementing it)
#define FSS_kfXmppStreamOpening						0x000010	// The method DoXmppStreamOpen() has been invoked.  This flag is useful to diagnose an error, or prevent unnecessary calling DoXmppStreamClose().
#define FSS_kfXmppStreamOpened						0x000020	// The XMPP stream was opened successfully.  That is, a <stream> element was successfully received
#define FSS_kfXmppStreamClosing						0x000040	// The method DoXmppStreamClose() has been invoked.  This flag is necessary because the virtual method disconnectFromHost() may be called 100 times when a socket connection is being closed.
#define FSS_kfXmppStreamClosed						0x000080	// The XMPP stream was finally closed
#define FSS_kfXmppStreamSupportsTLS					0x000100	// The XMPP server supports secure TLS/SSL connection.
#define FSS_kfXmppStreamStartedTLS					0x000200	// The socket has sent the stanza <starttls>.  This flag is useful to determine if the error <unsupported-stanza-type> is related to the TLS or something else.
#define FSS_kfXmppStanzaReceived					0x000400	// The socket has received at least one stanza.  This flag is necessary to determine if the server does support XMPP messages and therefore report the proper error to the user.
#define FSS_kfXmppSocketReadyToSendMessages			0x000800	// The socket is ready to send XMPP messages (this flag is set after a successful <bind>
//#define FSS_kfXmppServerCanCreateAccount			0x001000	// The XMPP server is capable of creating a new account, that is, the server supports "In-Band Registration" (XEP-0077).
#define FSS_kfXmppCreatingAccount					0x002000	// The XMPP socket is attempting to create (register) an account
#define FSS_kfXmppUserAuthenticated					0x004000	// The the password for the user is recognized by the server
#define FSS_kmXmppMask								0x00FFF0	// Mask of the flags reserved for CSocketXmpp
#define FSS_kfOptionCreateAccount					0x010000	// The user wishes to create/register a new account on the server

typedef UINT FLAGS_SOCKET_STATE;

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Socket handling XMPP messages for all contacts of a specific TAccountXmpp.
//	Given the XMPP design architecture, there is a one-to-one relationship between a TAccountXmpp and CSocketXmpp.  In other words, each XMPP account requires its own CSocketXmpp.
//	For performance reasons, all XMPP 'stanzas' are handled by CSocketXmpp as opposed to create classes for each type of XMPP stanza. Such a design may not be the most elegant from an academic point of view, however this is what will lead to the best performances.
class CSocketXmpp : public QSslSocket, protected ISocketUI
{
	Q_OBJECT
protected:
	TAccountXmpp * m_pAccount;				// Pointer to the object containing the account info for the socket connection (username, password, server).  This pointer is never NULL unless the account has been deleted while the socket is still running after SocketDestroy().
	FLAGS_SOCKET_STATE m_uFlagsSocketState;
	CBin m_binBufferIncomingData;					// Buffer to store incoming data from the socket.  This is necessary because sometimes a stanza may arrive from multiple network packets.
	const CXmlNode * m_pXmlNodeStanzaCurrent_YZ;	// Current stanza being processed by the method OnEventXmppStanzaArrived() and related OnEventXmppStanza*().  The motivation for using a member variable instead of a parameter to the method is avoiding pushing this parameter to every stack, including the error processing.
//	CListTasks m_listTasksWaitingForCompletion;
	ISocketUI * m_piSocketUI;						// Pointer to the interface for the socket to provide feedback to the user.
	TIMESTAMP_MINUTES m_tsmLastStanzaReceived;		// Timestamp where the last network packet was received. This is useful to determine if a socket timed out and needs to be reconnected.
public:
	CSocketXmpp(TAccountXmpp * pAccount);
protected:
	virtual ~CSocketXmpp();
public:
//	inline void SocketOption_EnableAccountCreation() { m_uFlagsSocketState = FSS_kfOptionCreateAccount; }
	void SocketUI_Init(ISocketUI * piSocketUI, BOOL fCreateAccount);
	void SocketUI_Remove();
	void Socket_Connect();
	void Socket_ConnectUnsecured();
	void Socket_Disconnect();
	void Socket_Destroy(PA_DELETING);
	BOOL Socket_FNeedsReconnection() const;
	BOOL Socket_FDisplayIconFailure() const;
	BOOL Socket_FuIsReadyToSendMessages() const { return (m_uFlagsSocketState & FSS_kfXmppSocketReadyToSendMessages); }
	PSZUC ScratchBuffer_WriteXmlIqResult_VL_Gsb(PSZAC pszFmtTemplate, va_list vlArgs);
	PSZUC ScratchBuffer_WriteXmlIqResult_Gsb(PSZAC pszFmtTemplate, ...);
	void Socket_WriteScratchBuffer();
	void Socket_WriteBin(const CBin & binData);
	void Socket_WriteData(PCVOID pvData, int cbData);
	void Socket_WriteXmlRaw(ITreeItemChatLogEvents * pContactOrGroup, PSZUC pszMessageXml);
	void Socket_WriteXmlNodeStanza(const CXmlNode * pXmlNodeStanza);
	void Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlFormatted_VL(PSZAC pszFmtTemplate, va_list vlArgs);
	void Socket_WriteXmlIqResult_Gsb(PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlIqReplyAcknowledge();
	void Socket_WriteXmlPresence();
	void Socket_WriteXmlPingToServerIfIdle();
	void Socket_WriteXmlPingToContact(TContact * pContact);
	void Socket_WriteRequestForContactAuthorization(TContact * pContactAuthorizing);
	void Socket_WriteXmlStartTls();
	void Socket_WriteXmlAuth();
	void Socket_ProcessAllPendingTasks();
	void Socket_GetIpAddress(OUT CStr * pstrIpAddress) const;
	inline QSslCipher Socket_GetSessionCipher() const { return sessionCipher(); }

	TContact * PFindContactFromStanza() const;

protected:
	void DoXmppStreamOpen();
	void DoXmppStreamClose();
	void OnEventXmppStanzaArrived();
	void OnEventXmppStanzaStream();
	void OnEventXmppStanzaIq();
	BOOL FStanzaProcessedByTaskMatchingEventID();
	BOOL FStanzaProcessedByTaskMatchingDownloadSID(PSZAC pszaVerbContex);

	void DisplayMesageToUI_VE(PSZAC pszFmtTemplate, ...);
	void DisplayStanzaErrorToUser();
	enum EDisplayExtraError
		{
		eDisplayExtraError_None,							// Do not append any extra text to the error message.  The error text is good as is.
		eDisplayExtraError_kfStanzaDescription				= 0x01,	// Append the error description from the stanza
		eDisplayExtraError_kfTextIntroduction				= 0x02,	// Append an introduction of the text returned by the server
		eDisplayExtraError_kfTextValue						= 0x04,	// Append the content of element <text> (if available of course)
		eDisplayExtraError_kfSetIconOffline					= 0x10,	// Use the offline icon rather than the error icon.  This is used when an error is not worth the attention to the user because the contact is no longer online.

		eDisplayExtraError_Description						= eDisplayExtraError_kfStanzaDescription,
		eDisplayExtraError_TextWithIntro					= eDisplayExtraError_kfTextValue | eDisplayExtraError_kfTextIntroduction,
		eDisplayExtraError_TextOnly							= eDisplayExtraError_kfTextValue,
		eDisplayExtraError_DescriptionAndText				= eDisplayExtraError_kfStanzaDescription | eDisplayExtraError_TextWithIntro,
		eDisplayExtraError_DescriptionAndTextIconOffline	= eDisplayExtraError_DescriptionAndText | eDisplayExtraError_kfSetIconOffline
		};
	void DisplayErrorMessageToUser(EDisplayExtraError eDisplayExtraError, PSZAC pszFmtTemplate, ...);

	void DebugDisplayStanzaToMessageLog();
	void DebugDisplayPendingDataToMessageLog();
	void DebugDisplayDataToWriteMessageLog(PCVOID pvData, int cbData);
protected:
	// Virtual methods from QAbstractSocket
	virtual void connectToHost(const QString & sHostName, quint16 uPort, OpenMode eOpenMode = ReadWrite, NetworkLayerProtocol eProtocol = AnyIPProtocol);
	virtual void disconnectFromHost();

protected slots:
	void SL_SocketHostFound();
	void SL_SocketStateChanged(QAbstractSocket::SocketState eState);
	void SL_SocketError(QAbstractSocket::SocketError eError);
	void SL_SocketErrorsSsl(const QList<QSslError> &lErrorsSsl);
	void SL_SocketEntersEncryptedMode();
	void SL_SocketDataAvailableForReading();
	friend class ISocketUI;
}; // CSocketXmpp

#define d_uPortXmppDefault			5222	// Default XMPP port (this port becomes secure after sending the stanza <starttls>)
#define d_uPortXmppDefaultSecure	5223	// Default 'secure' XMPP port

#define d_coSocketWriteHeader			d_coThistle
#define d_coSocketWriteContent			d_coViolet

PSZAC PszaGetSocketState(QAbstractSocket::SocketState eState);

///////////////////////////////////////////////////////////////////////////////////////////////////
enum ESocketState
{
	eSocketState_WaitingForHandshake,
	eSocketState_WaitingForFrameHeader,
	eSocketState_WaitingForFrameHeaderPayload16,
	eSocketState_WaitingForFrameHeaderPayload64,
	eSocketState_WaitingForFrameHeaderMask,
	eSocketState_WaitingForFrameDataPayload
};

enum EOpcode
{
	eOpcode_zContinue	= 0x0,
	eOpcode_DataText	= 0x1,
	eOpcode_DataBinary	= 0x2,
	eOpcode_Reserved3	= 0x3,
	eOpcode_Reserved4	= 0x4,
	eOpcode_Reserved5	= 0x5,
	eOpcode_Reserved6	= 0x6,
	eOpcode_Reserved7	= 0x7,
	eOpcode_Close		= 0x8,
	eOpcode_Ping		= 0x9,
	eOpcode_Pong		= 0xA,
	eOpcode_ReservedB	= 0xB,
	eOpcode_ReservedC	= 0xC,
	eOpcode_ReservedD	= 0xD,
	eOpcode_ReservedE	= 0xE,
	eOpcode_ReservedF	= 0xF,
};

#define d_bFrameHeader0_kmFragmentOpcode	0x0F	// Mask to extract the opcode from the header
#define d_bFrameHeader0_kfFragmentFinal		0x80	// Bit indicating that this is the final fragment in a message
#define d_bFrameHeader1_kmPayloadLength		0x7F
#define d_bFrameHeader1_kePayloadLength64	0x7F
#define d_bFrameHeader1_kePayloadLength16	0x7E
#define d_bFrameHeader1_kfPayloadMasked		0x80	// Bit indicating that the payload is masked

class OSocketWeb : public QTcpSocket
{
	Q_OBJECT
public:
	ESocketState m_eSocketState;
	BYTE m_bFrameHeader0_eOpcode;
	BYTE m_bFrameHeader0_kfFragmentFinal;
	BYTE m_bFrameHeader1_kfPayloadMasked;
	BYTE m_rgbFrameMask[4];	// Mask (if any) of the frame
	qint64 m_cblFrameData;	// Payload (in bytes) of the frame
	CBin m_binFrameData;	// Actual data of the payload

public:
	OSocketWeb();
	EOpcode EGetOpcode() const { return (EOpcode)m_bFrameHeader0_eOpcode; }
	void DataWrite(IN_MOD_INV void * pvData, int cbData, EOpcode eOpcode = eOpcode_DataText);
	void DataWrite(IN_MOD_INV CBin & binData, EOpcode eOpcode = eOpcode_DataText);

protected:
	//inline BOOL _FuIsPayloadMasked() { return (m_rgbFrameHeader[1] & d_bFrameHeader1_kfPayloadMasked); }
	void _DataMask(INOUT BYTE * prgbData, int cbData) const;

protected slots:
	void SL_Connected();
	void SL_DataAvailable();

signals:
	void SI_MessageAvailable(CBin &);
}; // OSocketWeb

#endif // SOCKETS_H
