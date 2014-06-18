
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WMarketplace.h"

WMarketplace::WMarketplace()
	{
	//setOrientation(Qt::Vertical);
	QLineEdit * pLineEdit = new QLineEdit(this);
	pLineEdit->setMaximumHeight(20);
	pLineEdit->setPlaceholderText("Search Marketplace");

	QLabel * pLabel = new QLabel("Placeholder to display premium products and services", this);
	pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pLabel->setFrameShape(QFrame::Box);

	/*
	QTableWidget * pTable = new QTableWidget(this);
	pTable->setRowCount(2);
	pTable->setColumnCount(2);

	// Call the server to fetch the data
	m_pSocket = new QSslSocket(this);
	m_pSocket->connectToHost("localhost", 8888);
	connect(m_pSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(SL_SocketStateChanged(QAbstractSocket::SocketState)));
	connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(SL_SocketDataAvailableForReading()));

	pTable->setItem(0, 0, new QTableWidgetItem("Bid"));
	pTable->setItem(1, 0, new QTableWidgetItem("Ask"));
	*/
//	addWidget(pLineEdit);
//	addWidget(pTable);
	/*
	QHBoxLayout * pLayout = new QHBoxLayout(this);
	pLayout->addWidget(pLineEdit);
	pLayout->addWidget(pTable);
	setLayout(pLayout);
	*/
//	view->setModel(model);
//	view->resizeColumnsToContents();

//	QLineEdit * paLineEdit = new QLineEdit;
//	addWidget(PA_CHILD paLineEdit);
	}

void
WMarketplace::SL_SocketStateChanged(QAbstractSocket::SocketState eState)
	{
	MessageLog_AppendTextFormatCo(d_coGreenDark, "[$@] SL_SocketStateChanged($s)\n", PszaGetSocketState(eState));
	switch (eState)
		{
	case QAbstractSocket::UnconnectedState:
		return;
	case QAbstractSocket::HostLookupState:
		break;
	case QAbstractSocket::ConnectingState:
		break;
	case QAbstractSocket::ConnectedState:
		m_pSocket->write("Hello");
		return;
	case QAbstractSocket::BoundState:
		break;
	case QAbstractSocket::ListeningState:
		break;
	case QAbstractSocket::ClosingState:
		break;
		} // switch
	}

void
WMarketplace::SL_SocketDataAvailableForReading()
	{
	const int cbDataAvailable = m_pSocket->bytesAvailable();	// Get how many bytes are available from the socket

	CBin binData;
	char * pbData = (char *)binData.PbAllocateExtraDataWithVirtualNullTerminator(cbDataAvailable);
	const int cbDataRead = m_pSocket->read(OUT pbData, cbDataAvailable);	// Read the data from the socket
	Assert((cbDataRead == cbDataAvailable) && "All data should be read!");
	Assert((int)strlen(pbData) == cbDataRead);

	MessageLog_AppendTextFormatCo(d_coBlack, "Socket Data: $B\n", &binData);
	}

//"Jurisdictions"
//"Corporations"
//"Legal Systems"

///////////////////////////////////////////////////////////////////////////////////////////////////
WMarketplaceArbitrators::WMarketplaceArbitrators()
	{
	QLineEdit * pLineEdit = new QLineEdit(this);
	pLineEdit->setMaximumHeight(20);
	pLineEdit->setPlaceholderText("Search Arbitrator");

	WTable * pTable = new WTable(this);
	pTable->SetColumns_VEZ("Arbitrator Name", "Speciality", "Website", "# References", "# Cases", "% Appeal", NULL);

	pTable->AppendRow_VEZ("Haven", "", "www.haven.hn", "4563", "216", "1.2%", NULL);
	pTable->AppendRow_VEZ("Judge Judy", "small claims", "www.judgejudy.com", "327", "22", "5%", NULL);
	pTable->AppendRow_VEZ("Susanne", "immigration", "www.tarkowskitempelhof.com", "63", "8", "", NULL);
	addWidget(new WButtonText(" Add Arbitrator... "));
	Splitter_AddWidgetSpacer();
	}

#include "WGrid.h"

const SGridColumn c_rgzColumns[] =
{
	{ "Date", GCF_Width(170) },
	{ "Contact", GCF_Width(120) },
	{ "Amount", GCF_Width(90) },
	{ "Value", GCF_Width(80) },
	{ "Comment", GCF_WidthStretch(200) },
	{ d_zNA, d_zNA }
};

enum
	{
	iColumn_Date,
	iColumn_Contact,
	iColumn_Amount,
	iColumn_Value,
	iColumn_Comment
	};

//	Must have the same interface as PFn_GridCellGetText()
CString
GridCellGetTextTransaction(IEventWalletTransaction * pTransaction, const CGridColumn * pColumn, EGridCellText)
	{
//	Assert(pTransaction->EGetEventType() == eEventType_SendBitcoins);
	switch (pColumn->m_iColumn)
		{
	case iColumn_Date:
		return QDateTime::fromMSecsSinceEpoch(pTransaction->m_tsEventID).toString();
	case iColumn_Contact:
		return pTransaction->ChatLog_PszGetNickNameOfContact();
	case iColumn_Amount:
		return Amount_SFormat(pTransaction->m_amtQuantity, d_chEncodingAmountMilliBitcoin);
	case iColumn_Value:
		return pTransaction->m_strValue;
	case iColumn_Comment:
		return pTransaction->m_strComment;
		}
	return c_sEmpty;
	}

WLayoutWalletGrid::WLayoutWalletGrid(ITreeItem * pTreeItemFilterBy, EWalletViewFlags eWalletViewFlags)
	{
	WGrid * pwGrid = new WGrid(this);
	pwGrid->ColumnsAdd(c_rgzColumns, (PFn_GridCellGetText)GridCellGetTextTransaction);
	CArrayPtrEvents arraypaTransactions;
	arraypaTransactions.Wallets_AppendEventsTransactionsFor(pTreeItemFilterBy, eWalletViewFlags);
	pwGrid->RowsAdd(arraypaTransactions);
	}
