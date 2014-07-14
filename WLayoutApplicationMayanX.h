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
