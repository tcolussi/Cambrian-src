//	WLayoutApplicationMayanX.h
#ifndef WLAYOUTAPPLICATIONMAYANX_H
#define WLAYOUTAPPLICATIONMAYANX_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TApplicationMayanX.h"

/*
///////////////////////////////////////////////////////////////////////////////////////////////////
class CExchangeCoffee : public ITreeItem
{
	RTI_IMPLEMENTATION(CExchangeCoffee)
public:
	CStr m_strUrlServer;	// Server to connect to the coffee exchange
	CBin m_binKeyPublic;	// Public key identifying the user
	CStr m_strxUserID;		// UserID to login to the server
	class WLayoutApplicationMayanX * m_pawLayout;

public:
	CExchangeCoffee();
	~CExchangeCoffee();
	virtual void TreeItem_GotFocus();		// From ITreeItem
};

class CExchangeCoffeeTrade : public ITreeItem
{
	RTI_IMPLEMENTATION(CExchangeCoffeeTrade)
public:
	CExchangeCoffee * m_pParent;
	class WLayoutExchangeCoffeeTrade * m_pawLayout;

public:
	CExchangeCoffeeTrade(CExchangeCoffee * pParent);
	~CExchangeCoffeeTrade();
	virtual void TreeItem_GotFocus();		// From ITreeItem
};
*/

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

///////////////////////////////////////////////////////////////////////////////////////////////////
class WLayoutApplicationMayanX  : public WLayout
{
	Q_OBJECT
protected:
	TApplicationMayanX * m_pApplication;
	CInternetServerMayanX m_oInternetServer;
	OSocketWeb m_oSocket;
    CStr m_strxSessionID;
private:
	OLayoutForm * m_poLayoutButtons;
	QLabel * m_pwLabelWelcome;
	WButtonTextWithIcon * m_pwButtonRegister;
	WEdit * m_pwEditQuantity;
	WEdit * m_pwEditPrice;
    WEdit * m_pwEditIdToCancelOrder;
    WEdit * m_pwEditIdToUpdateOrder;
    WEdit * m_pwEditQuantityToUpdateOrder;
    WEdit * m_pwEditPriceToUpdateOrder;
    WEdit * m_pwEditUpdateBalanceQuantity;
    WEdit * m_pwEditUpdateBalanceUserId;

    WLabel * m_pwLabelBalance;
    WTable * m_pwTableMarketBids;
    WTable * m_pwTableOrders;
    WTable * m_pwTableUserBalances;
    WTable * m_pwTableUsers;
    WTable * m_pwTableUsersDocuments;

    WEdit * m_pwEditUpdateAccountId;
    WEdit * m_pwEditUpdateAccountName;
    WEdit * m_pwEditUpdateAccountRole;

    WEdit * m_pwEditGetUserDocumentId;
	WLabel * m_pwLabelPriceLast;	// Last price of the coffee
	int m_nPriceMin;
	int m_nPriceMax;
	int m_nPriceLast;
	WButtonTextWithIcon * m_pwButtonPing;

public:
	WLayoutApplicationMayanX(TApplicationMayanX * pApplication);
	~WLayoutApplicationMayanX();
    void DisplayMarketOrders(const CXmlNode * pXmlNode);
	void DisplayOrders(const CXmlNode * pXmlNodeOrders);
    void DisplayUserBalanceUpdated(const CXmlNode * pXmlNode);
    void DisplayUserBalances(const CXmlNode * pXmlNode);
    void DisplayUsers(const CXmlNode * pXmlNode);
    void DisplayUserAccountUpdate(const CXmlNode * pXmlNode);
    void DisplayAllUsersDocuments(const CXmlNode * pXmlNode);
    void SaveUsersDocument(const CXmlNode * pXmlNode);
    void UserOrderUpdate(const CXmlNode * pXmlNode);
    void UserOrderCancel(const CXmlNode * pXmlNode);

public slots:
	void SL_ExchangeLogin();
	void SL_InternetRequestCompleted(QNetworkReply * poNetworkReply);
    void SL_ButtonUserOrderAdd();
    void SL_ButtonUserOrderCancel();

    void SL_ButtonUserOrderUpdate();
    void SL_ButtonUserBalanceUpdate();
    void SL_ButtonFetchUserBalances();
    void SL_ButtonUserAccountUpdate();
    void SL_ButtonFetchUsers();
    void SL_ButtonUploadADocument();
    void SL_ButtonUserDocumentGet();
    void SL_ButtonFetchUserDocuments();

	void SL_TableMarketOrdersDoubleClicked(const QModelIndex & oIndex);

	void SL_SocketError(QAbstractSocket::SocketError);
	void SL_SocketStateChanged(QAbstractSocket::SocketState eState);
	void SL_SocketMessage(CBin & binszvFrameData);
	void SL_ButtonPing();
//	void SL_SocketConnected();
//	void SL_SocketDataAvailableForReading();
}; // WLayoutApplicationMayanX

#endif // WLAYOUTAPPLICATIONMAYANX_H
