#ifndef WMARKETPLACE_H
#define WMARKETPLACE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WMarketplace  : public WLayout
{
	Q_OBJECT
protected:
	QSslSocket * m_pSocket;
public:
	WMarketplace();

public slots:
	void SL_SocketStateChanged(QAbstractSocket::SocketState eState);
	void SL_SocketDataAvailableForReading();
}; // WMarketplace




class WMarketplaceArbitrators  : public WLayout
{
	Q_OBJECT
protected:

public:
	WMarketplaceArbitrators();
}; // WMarketplaceArbitrators




//	Layout to display wallet transactions
class WLayoutWallet : public WLayout
{
	Q_OBJECT
public:

public:
	WLayoutWallet(ITreeItem * pTreeItemFilterBy, EWalletViewFlags eWalletViewFlags);

public slots:
	void SL_TableCell_clicked(const QModelIndex & index);
};

class WLayoutWalletGrid : public WLayout
{
public:
	WLayoutWalletGrid(ITreeItem * pTreeItemFilterBy, EWalletViewFlags eWalletViewFlags);
};



#endif // WMARKETPLACE_H
