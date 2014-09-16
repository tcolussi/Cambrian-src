//	WLayoutApplicationMayanX.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutApplicationMayanX.h"

TApplicationMayanX::~TApplicationMayanX()
	{
	delete m_pawLayout;
	}

//	TApplicationMayanX::ITreeItem::TreeItem_GotFocus()
void
TApplicationMayanX::TreeItem_GotFocus()
	{
	if (m_pawLayout == NULL)
		m_pawLayout = new WLayoutApplicationMayanX(this);
	MainWindow_SetCurrentLayout(IN m_pawLayout);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
CExchangeCoffee::CExchangeCoffee()
	{
	m_pawLayout = NULL;
	TreeItem_DisplayWithinNavigationTree(NULL, "MayanX Coffee Exchange", eMenuIconCoffeeExchange);
		new TTreeItemDemo(this, "Buy Coffee", eMenuIconCoffeeBuy, "Placeholder to display asks by coffee farmers", "Search Offers");
		new TTreeItemDemo(this, "Sell Coffee", eMenuIconCoffeeSell, "Placeholder to display bids by potential coffee buyers", "Search Bids");
		new TTreeItemDemo(this, "Trade History", eMenuIconExchange, "Display the trade history of the user", "Search Trade History");
	m_strUrlServer.BinInitFromStringWithNullTerminator("http://mayanx.webaweb.net/api.asmx");

	#if 0
	if (m_binKeyPublic.FIsEmptyBinary())
		m_binKeyPublic.BinAppendBinaryData(g_oConfiguration.PGetSalt(), CChatConfiguration::c_cbSalt);
	#else
	SHashSha1 hash;
	HashSha1_InitRandom(OUT &hash);
	m_binKeyPublic.BinAppendBinaryData(&hash, sizeof(hash));
	#endif
	m_strxUserID = g_oConfiguration.m_strxUserIdCoffeeExchange;
	}

CExchangeCoffee::~CExchangeCoffee()
	{
	delete m_pawLayout;
	}

//	CExchangeCoffee::ITreeItem::TreeItem_GotFocus()
void
CExchangeCoffee::TreeItem_GotFocus()
	{
	if (m_pawLayout == NULL)
		m_pawLayout = new WLayoutApplicationMayanX(this);
	MainWindow_SetCurrentLayout(IN m_pawLayout);
	}
*/



///////////////////////////////////////////////////////////////////////////////////////////////////
/*
CExchangeCoffeeTrade::CExchangeCoffeeTrade(CExchangeCoffee * pParent)
	{
	m_pParent = pParent;
	m_pawLayout = NULL;
	}

CExchangeCoffeeTrade::~CExchangeCoffeeTrade()
	{
	delete m_pawLayout;
	}

//	CExchangeCoffeeTrade::ITreeItem::TreeItem_GotFocus()
void
CExchangeCoffeeTrade::TreeItem_GotFocus()
	{
	if (m_pawLayout == NULL)
		m_pawLayout = new WLayoutExchangeCoffeeTrade(this);
	MainWindow_SetCurrentLayout(IN m_pawLayout);
	}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutApplicationMayanX::WLayoutApplicationMayanX(TApplicationMayanX * pApplication)
	{
	m_pApplication = pApplication;
    m_pwTableMarketBids = NULL;
    m_pwTableOrders = NULL;
    m_pwTableUserBalances= NULL;
    m_pwTableUsersDocuments=NULL;
    m_pwTableUsers=NULL;
	m_poLayoutButtons = Splitter_PoAddGroupBoxAndLayoutForm_VE("Welcome to the MayanX Exchange");
	m_pwLabelWelcome = new WLabelSelectableWrap("If you wish to trade on the MayanX Exchange, you need to register your profile.  A profile is necessary to identify you when you login as well as giving you the opportunity to build a reputation.  If you wish to use a different profile, such as a business profile, feel free to create a new one.");
	m_poLayoutButtons->addRow(m_pwLabelWelcome);
	m_pwButtonRegister = new WButtonTextWithIcon(m_pApplication->m_strxUserID.FIsEmptyString() ?
		"Register...|Register my profile on the MayanX Exchange" :
		"Login...|Login to the MayanX Exchange", eMenuIconCoffeeBuy);
	m_poLayoutButtons->addRow(m_pwButtonRegister);
	m_nPriceMin = 0;
	m_nPriceMax = 0;
	m_pwLabelPriceLast = new WLabel;
	m_poLayoutButtons->addRow(m_pwLabelPriceLast);
	m_pwButtonPing = new WButtonTextWithIcon("Socket Ping", eMenuAction_ContactPing);
	m_poLayoutButtons->addRow(m_pwButtonPing);
	connect(m_pwButtonPing, SIGNAL(clicked()), this, SLOT(SL_ButtonPing()));
	connect(m_pwButtonRegister, SIGNAL(clicked()), this, SLOT(SL_ExchangeLogin()));
	connect(&m_oInternetServer, SIGNAL(finished(QNetworkReply*)), this, SLOT(SL_InternetRequestCompleted(QNetworkReply*)));

	connect(&m_oSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(SL_SocketError(QAbstractSocket::SocketError)));
	connect(&m_oSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(SL_SocketStateChanged(QAbstractSocket::SocketState)));
	connect(&m_oSocket, SIGNAL(SI_MessageAvailable(CBin &)), this, SLOT(SL_SocketMessage(CBin &)));
//	connect(&m_oSocket, SIGNAL(connected()), this, SLOT(SL_SocketConnected()));
//	connect(&m_oSocket, SIGNAL(readyRead()), this, SLOT(SL_SocketDataAvailableForReading()));
	m_oSocket.connectToHost("mayan.cambrian.org", 8080);
	//m_oSocket.connectToHost("echo.websocket.org", 80);
	}

void
WLayoutApplicationMayanX::SL_SocketError(QAbstractSocket::SocketError e)
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "WebSocket error $i\n", e);
	}

void
WLayoutApplicationMayanX::SL_SocketStateChanged(QAbstractSocket::SocketState eState)
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "WebSocket state changed: $i $s\n", eState, PszaGetSocketState(eState));
	}

void
WLayoutApplicationMayanX::SL_SocketMessage(CBin & binszvFrameData)
	{
	PSZUC pszFrameData = binszvFrameData.PbGetData();
	PSZR pszrEvent = PszrStringContainsSubString(pszFrameData, "Event\":");
	if (pszrEvent == NULL)
		return;
	//MessageLog_AppendTextFormatCo(d_coBlue, "Transaction message: $s\n", pszFrameData);
	int eEvent = atoi((const char *)pszrEvent);
	if (eEvent != 5)
		return;	// If the event is not a transaction, ignore it
	PSZR pszrSubstring = PszrStringContainsSubString(pszFrameData, "\\\"P\\\":");
	if (pszrSubstring != NULL)
		{
		SStringToNumber stn;
		InitToGarbage(OUT &stn, sizeof(stn));
		stn.uFlags = STN_mskfNoLeadingSpaces | STN_mskfNoSkipTailingSpaces | STN_mskfAllowRandomTail | STN_mskfAllowHexPrefix | STN_mskfMustHaveDigits | STN_mskfUnsignedInteger;
		stn.pszuSrc = pszrSubstring;
		if (FStringToNumber(INOUT &stn))
			{
			if (stn.u.nData > m_nPriceMax)
				{
				m_nPriceMax = stn.u.nData;
				if (m_nPriceMin == 0)
					m_nPriceMin = m_nPriceMax;
				}
			if (stn.u.nData < m_nPriceMin)
				m_nPriceMin = stn.u.nData;
			m_pwLabelPriceLast->Label_SetTextFormat_VE_Gsb("Last price: $I  (min=$I, max=$I)", stn.u.nData, m_nPriceMin, m_nPriceMax);
			if (stn.u.nData > m_nPriceLast)
				m_pwLabelPriceLast->setStyleSheet("color: green");
			else
				m_pwLabelPriceLast->Label_SetTextColorError();
			m_nPriceLast = stn.u.nData;
			//MessageLog_AppendTextFormatCo(d_coBlue, "WebSocket Price: $I\n", stn.u.nData);

			}
		}
	}

/*
void
WLayoutApplicationMayanX::SL_SocketConnected()
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "WebSocket connected $s\n", m_oSocket.isValid() ? "valid" : "INVALID");
	SHashSha1 hashNonce;
	HashSha1_InitRandom(OUT &hashNonce);
	CBin bin;
	bin.BinAppendText_VE(
		"GET / HTTP/1.1\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: {h/}\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n", &hashNonce);
	MessageLog_AppendTextFormatCo(d_coGoldenRod, "Writing:\n$B", &bin);
	m_oSocket.write(bin.PszaGetUtf8NZ(), bin.CbGetData());
	}
*/
OSocketWeb::OSocketWeb()
	{
	m_eSocketState = eSocketState_WaitingForFrameHeader;
	connect(this, SIGNAL(connected()), this, SLOT(SL_Connected()));
	connect(this, SIGNAL(readyRead()), this, SLOT(SL_DataAvailable()));
	}

void
OSocketWeb::DataWrite(IN_MOD_INV void * pvData, int cbData, EOpcode eOpcode)
	{
	int cbHeader;
	BYTE rgbHeader[2 + sizeof(quint64) + sizeof(m_rgbFrameMask)];	// The largest header
	rgbHeader[0] = d_bFrameHeader0_kfFragmentFinal | eOpcode;
	if (cbData <= 0xFFFF)
		{
		if (cbData < d_bFrameHeader1_kePayloadLength16)
			{
			rgbHeader[1] = d_bFrameHeader1_kfPayloadMasked | cbData;	// The whole payload fits within 7 bits
			cbHeader = 2;
			}
		else
			{
			*(quint16 *)(rgbHeader + 1) = cbData;
			rgbHeader[1] = d_bFrameHeader1_kfPayloadMasked | d_bFrameHeader1_kePayloadLength16;
			cbHeader = 2 + sizeof(quint16);
			}
		}
	else
		{
		// The payload needs more than 16 bits, so use 64 bits
		*(quint64 *)(rgbHeader + 1) = cbData;
		rgbHeader[1] = d_bFrameHeader1_kfPayloadMasked | d_bFrameHeader1_kePayloadLength64;
		cbHeader = 2 + sizeof(quint64);
		}
	#pragma GCC diagnostic ignored "-Wstrict-aliasing"	// Ignore the warning when casting to *quint32
	*(quint32 *)m_rgbFrameMask = *(quint32 *)&rgbHeader[cbHeader] = qrand() << 16 | qrand();	// Generate a random mask and store it into m_rgbFrameMask[] for _DataMask()
	write((const char *)rgbHeader, cbHeader + sizeof(m_rgbFrameMask));
	_DataMask(INOUT (BYTE *)pvData, cbData);
	write((const char *)pvData, cbData);
	} // DataWrite()

void
OSocketWeb::DataWrite(IN_MOD_INV CBin & binData, EOpcode eOpcode)
	{
	DataWrite(binData.PbGetData(), binData.CbGetData(), eOpcode);
	}

void
WLayoutApplicationMayanX::SL_ButtonPing()
	{
	char szData[] = "{\"Event\":\"TransactionLastPrice\"}";
	m_oSocket.DataWrite(szData, sizeof(szData) - 1);
	}

//	Routine to mask (or unmask data)
void
OSocketWeb::_DataMask(INOUT BYTE * prgbData, int cbData) const
	{
	for (int ibData = 0; ibData < cbData; ibData++)
		prgbData[ibData] ^= m_rgbFrameMask[ibData & 0x03];
	}

void
OSocketWeb::SL_Connected()
	{
	m_eSocketState = eSocketState_WaitingForHandshake;
	MessageLog_AppendTextFormatCo(d_coBlue, "WebSocket connected $s\n", isValid() ? "valid" : "INVALID");
	SHashSha1 hashNonce;
	HashSha1_InitRandom(OUT &hashNonce);
	CBin bin;
	bin.BinAppendText_VE(
		"GET / HTTP/1.1\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: {h/}\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n", &hashNonce);
	MessageLog_AppendTextFormatCo(d_coGoldenRod, "Writing:\n$B", &bin);
	write(bin.PszaGetUtf8NZ(), bin.CbGetData());
	}

void
OSocketWeb::SL_DataAvailable()
	{
	g_oMutex.lock();	// Make sure all signals SI_MessageAvailable() are synchronized
	int cbDataAvailable = bytesAvailable();		// Read only once the number of bytes available. This is important to prevent 'thread starvation' in the case the socket data arrives faster than the web socket can handle it.  By readin the bytes available once, we are guaranteed this 'state machine' method will eventually exit.
	switch (m_eSocketState)
		{
	case eSocketState_WaitingForHandshake:
		readAll();	// Flush the handshake
		m_eSocketState = eSocketState_WaitingForFrameHeader;
		// Fall Through //
	case eSocketState_WaitingForFrameHeader:
		WaitingForFrameHeader:
		cbDataAvailable -= 2;
		if (cbDataAvailable >= 0)
			{
			BYTE rgbFrameHeader[2];
			read(OUT (char *)rgbFrameHeader, sizeof(rgbFrameHeader));
			m_bFrameHeader0_kfFragmentFinal = (rgbFrameHeader[0] & d_bFrameHeader0_kfFragmentFinal);
			const BYTE bOpcode = (rgbFrameHeader[0] & d_bFrameHeader0_kmFragmentOpcode);
			if (bOpcode != eOpcode_zContinue)
				m_bFrameHeader0_eOpcode = bOpcode;
			m_bFrameHeader1_kfPayloadMasked = (rgbFrameHeader[1] & d_bFrameHeader1_kfPayloadMasked);
			m_cblFrameData = (rgbFrameHeader[1] & d_bFrameHeader1_kmPayloadLength);
//			MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "WebSocket Header: [0]=0x$x [1]=0x$x  (Frame = $L bytes, $L bytes remaining)\n", rgbFrameHeader[0], rgbFrameHeader[1], m_cblFrameData, bytesAvailable());
			switch (m_cblFrameData)
				{
			case d_bFrameHeader1_kePayloadLength16:
				m_eSocketState = eSocketState_WaitingForFrameHeaderPayload16;
				goto WaitingForFrameHeaderPayload16;
			case d_bFrameHeader1_kePayloadLength64:
				m_eSocketState = eSocketState_WaitingForFrameHeaderPayload64;
				goto WaitingForFrameHeaderPayload64;
				} // switch
			m_eSocketState = eSocketState_WaitingForFrameHeaderMask;
			goto WaitingForFrameHeaderMask;
			}
		break;
	case eSocketState_WaitingForFrameHeaderPayload16:
		WaitingForFrameHeaderPayload16:
		cbDataAvailable -= sizeof(quint16);
		if (cbDataAvailable >= 0)
			{
			quint16 cbwFrameData;
			read(OUT (char *)&cbwFrameData, sizeof(cbwFrameData));
			m_cblFrameData = qFromBigEndian(cbwFrameData);
			m_eSocketState = eSocketState_WaitingForFrameHeaderMask;
			//MessageLog_AppendTextFormatCo(COX_MakeBold(d_coRed), "WebSocket Payload16: $L bytes\n", m_cblFrameData);
			goto WaitingForFrameHeaderMask;
			}
		break;
	case eSocketState_WaitingForFrameHeaderPayload64:
		WaitingForFrameHeaderPayload64:
		cbDataAvailable -= sizeof(quint64);
		if (cbDataAvailable >= 0)
			{
			quint64 cblFrameData;
			read(OUT (char *)&cblFrameData, sizeof(cblFrameData));
			m_cblFrameData = qFromBigEndian(cblFrameData);
			MessageLog_AppendTextFormatCo(COX_MakeBold(d_coRed), "WebSocket Payload64: $L bytes\n", m_cblFrameData);
			m_eSocketState = eSocketState_WaitingForFrameHeaderMask;
			goto WaitingForFrameHeaderMask;
			}
		break;
	case eSocketState_WaitingForFrameHeaderMask:
		WaitingForFrameHeaderMask:
		if (m_bFrameHeader1_kfPayloadMasked)
			{
			MessageLog_AppendTextFormatCo(d_coRed, "Frame is masked\n");
			cbDataAvailable -= sizeof(m_rgbFrameMask);
			if (cbDataAvailable < 0)
				break;
			read(OUT (char *)m_rgbFrameMask, sizeof(m_rgbFrameMask));
			}
		m_eSocketState = eSocketState_WaitingForFrameDataPayload;
		// Fall Through //
	case eSocketState_WaitingForFrameDataPayload:
		cbDataAvailable -= m_cblFrameData;
		if (cbDataAvailable >= 0)
			{
			BYTE * pbData = m_binFrameData.PbeAllocateExtraDataWithVirtualNullTerminator(m_cblFrameData);
			const int cbDataRead = read(OUT (char *)pbData, m_cblFrameData);	// Read the data from the socket
			if (cbDataRead != m_cblFrameData)
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "WebSocket: cbDataRead=$I, m_cblFrameData=$L\n", cbDataRead, m_cblFrameData);
//			MessageLog_AppendTextFormatCo(d_coBlueDark, "$s\n", pbData);
			if (m_bFrameHeader0_kfFragmentFinal)
				{
				// Check if there are any special opcodes the socket can handle automatically
				switch (m_bFrameHeader0_eOpcode)
					{
				case eOpcode_DataText:
				case eOpcode_DataBinary:
					emit SI_MessageAvailable(INOUT m_binFrameData);	// Both text and binary are treated the same, as Cambrian uses UTF-8 for text
					break;
				case eOpcode_Ping:
					DataWrite(INOUT m_binFrameData, eOpcode_Pong);	// Respond to the ping request
					break;
				case eOpcode_Close:
					// Close the socket
					break;
					} // switch
				m_binFrameData.Empty();
				} // if
			m_eSocketState = eSocketState_WaitingForFrameHeader;
			goto WaitingForFrameHeader;
			}
		break;
		} // switch
	g_oMutex.unlock();
	} // SL_DataAvailable()

WLayoutApplicationMayanX::~WLayoutApplicationMayanX()
	{
	}

void
WLayoutApplicationMayanX::SL_ExchangeLogin()
	{
	if (m_pApplication->m_strxUserID.FIsEmptyString())
		{
		TProfile * pProfile = m_pApplication->m_pProfileParent;
		// We do not have a UserID, so register one on the exchange
		CInternetRequestWebMethodXmlMayanX oRequest("UserRegister");
		oRequest.BinAppendXmlElementBinaryBase64("bIdentity", pProfile->m_binKeyPublic);
		oRequest.BinAppendXmlElementText("sName", pProfile->m_strNameProfile);
		oRequest.BinAppendXmlElementText("sJID", (PSZUC)"jon@chat.cambrian.org");
		m_oInternetServer.RequestSend(&oRequest);
		return;
		}
	if (m_oInternetServer.m_strxSessionID.FIsEmptyString())
		{
		// If there is no SessionID, then we have to login
		CInternetRequestWebMethodXmlMayanX oRequest("UserLogin");
		oRequest.BinAppendXmlElementText("xUserID", m_pApplication->m_strxUserID);
		m_oInternetServer.RequestSend(&oRequest);
		}
	} // SL_ExchangeLogin()

enum ECodeMayanX
	{
	eCodeUserNotFound = 1,
	eCodeUserAlreadyExist = 6,
	eCodeMax
	};

void
WLayoutApplicationMayanX::SL_InternetRequestCompleted(QNetworkReply * poNetworkReply)
	{
	CInternetResponseWebMethodXmlSoap oInternetResponse(poNetworkReply);

	// First of all, check if there is an error from the server
	CXmlNode * pXmlNodeError = oInternetResponse.PGetXmlNodeError();
	if (pXmlNodeError != NULL)
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "SL_InternetRequestCompleted() with the following error:\n^N", pXmlNodeError);
		int nCode = pXmlNodeError->NFindElementOrAttributeValueNumeric("nCode");
		if (nCode == eCodeUserNotFound)
			{
			// The user is not found or does not exist, therefore clear the m_strxUserID so we may retry
			m_pApplication->m_strxUserID.Empty();
			}
		if (nCode == eCodeUserAlreadyExist)
			{
			// The user already exists, so fetch the xUserID from the
			m_pApplication->m_strxUserID = pXmlNodeError->PszuFindElementValue("xDescriptionParam");
			SL_ExchangeLogin();	// Attempt to login again
			}
		return;
		}

	CXmlNode * pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserRegister");
	if (pXmlNode != NULL)
		{
		m_pApplication->m_strxUserID = pXmlNode->m_pszuTagValue;
		if (!m_pApplication->m_strxUserID.FIsEmptyString())
			SL_ExchangeLogin();		// Attempt to login with the new UserID.
		else
			EMessageBoxWarning("The API 'UserRegister' returned an invalid UserID of value '$S'", &m_pApplication->m_strxUserID);
		return;
		}
	pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserLogin");
	if (pXmlNode != NULL)
		{
		CBin binChallenge;
		binChallenge.BinAppendBinaryDataFromBase64Szv(pXmlNode->PszuFindElementValue("bChallenge"));
		if (!binChallenge.FIsEmptyBinary())
			{
			// We received a challenge, so respond to it now
			CBin binResponseData = m_pApplication->m_pProfileParent->m_binKeyPublic;
			binResponseData.BinAppendCBin(binChallenge);
			//MessageLog_AppendTextFormatCo(d_coBlue, " bIdentity: {B/}\n bChallenge: {B/}\n bIdentity + bChallenge: {B/}\n", &m_pTreeItemParent->m_binKeyPublic, &binChallenge, &binResponseData);
			binResponseData.BinHashToMd5();

			CInternetRequestWebMethodXmlMayanX oRequest("UserLogin");
			oRequest.BinAppendXmlElementText("xUserID", m_pApplication->m_strxUserID);
			oRequest.BinAppendXmlElementBinaryBase64("bResponse", binResponseData);
			m_oInternetServer.RequestSend(&oRequest);
			return;
			}
		if (m_oInternetServer.m_strxSessionID.FIsEmptyString())
			{
			m_oInternetServer.m_strxSessionID = pXmlNode->PszuFindElementValue("xSessionID");
			//m_oInternetServer.m_strxSessionID = (PSZUC)"as8323jas932e";
			if (!m_oInternetServer.m_strxSessionID.FIsEmptyString())
				{
				MessageLog_AppendTextFormatCo(d_coBlue, "Login successful: xSessionID = $S\n", &m_oInternetServer.m_strxSessionID);
				OLayoutHorizontal * poLayoutOrder = new OLayoutHorizontal((QWidget *)NULL);
				m_poLayoutButtons->addRow(poLayoutOrder);
				m_pwEditQuantity = new WEditNumber;
				poLayoutOrder->Layout_AddLabelAndWidgetH_PA("Quantity:", m_pwEditQuantity);
				m_pwEditPrice = new WEditNumber;
				poLayoutOrder->Layout_AddLabelAndWidgetH_PA("Price:", m_pwEditPrice);
				WButtonTextWithIcon * pwButton = new WButtonTextWithIcon("Place Order", eMenuIconCoffeeBuy);
				poLayoutOrder->addWidget(pwButton);
				poLayoutOrder->addWidget(PA_CHILD new QWidget, 1);
				connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUserOrderAdd()));

				//Updating user balance
				OLayoutHorizontal * poLayoutUserBalance = new OLayoutHorizontal((QWidget *)NULL);
				m_poLayoutButtons->addRow(poLayoutUserBalance);
                m_pwEditUpdateBalanceQuantity = new WEditNumber;
                m_pwEditUpdateBalanceUserId = new WEditNumber;
                poLayoutUserBalance->Layout_AddLabelAndWidgetH_PA("User Id:", m_pwEditUpdateBalanceUserId);
                poLayoutUserBalance->Layout_AddLabelAndWidgetH_PA("Quantity:", m_pwEditUpdateBalanceQuantity);
                m_pwLabelBalance=new WLabel;
                pwButton = new WButtonTextWithIcon("Update Balance", eMenuIconCoffeeBuy);
                poLayoutUserBalance->addWidget(pwButton);
                poLayoutUserBalance->addWidget(m_pwLabelBalance);
				poLayoutUserBalance->addWidget(PA_CHILD new QWidget, 1);
                connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUserBalanceUpdate()));

                //Cancel an Order
                OLayoutHorizontal * poLayoutCancelAnOrder = new OLayoutHorizontal((QWidget *)NULL);
                m_poLayoutButtons->addRow(poLayoutCancelAnOrder);
                m_pwEditIdToCancelOrder = new WEditNumber;
                poLayoutCancelAnOrder->Layout_AddLabelAndWidgetH_PA("Id:", m_pwEditIdToCancelOrder);
                pwButton = new WButtonTextWithIcon("Cancel Order", eMenuIconCoffeeBuy);
                poLayoutCancelAnOrder->addWidget(pwButton);
                poLayoutCancelAnOrder->addWidget(PA_CHILD new QWidget, 1);
                connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUserOrderCancel()));

                //Update an Order
                OLayoutHorizontal * poLayoutUpdateAnOrder = new OLayoutHorizontal((QWidget *)NULL);
                m_poLayoutButtons->addRow(poLayoutUpdateAnOrder);
                m_pwEditIdToUpdateOrder = new WEditNumber;
                poLayoutUpdateAnOrder->Layout_AddLabelAndWidgetH_PA("Id:", m_pwEditIdToUpdateOrder);
                m_pwEditPriceToUpdateOrder = new WEditNumber;
                poLayoutUpdateAnOrder->Layout_AddLabelAndWidgetH_PA("Price:", m_pwEditPriceToUpdateOrder);
                m_pwEditQuantityToUpdateOrder = new WEditNumber;
                poLayoutUpdateAnOrder->Layout_AddLabelAndWidgetH_PA("Quantity:", m_pwEditQuantityToUpdateOrder);
                pwButton = new WButtonTextWithIcon("Update Order", eMenuIconCoffeeBuy);
                poLayoutUpdateAnOrder->addWidget(pwButton);
                poLayoutUpdateAnOrder->addWidget(PA_CHILD new QWidget, 1);
                connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUserOrderUpdate()));

                //Update User's Account
                OLayoutHorizontal* poLayoutUpdateUsersAccount = new OLayoutHorizontal((QWidget *)NULL);
                m_poLayoutButtons->addRow(poLayoutUpdateUsersAccount);
                m_pwEditUpdateAccountId=new WEditNumber;
                m_pwEditUpdateAccountName = new WEdit;
                m_pwEditUpdateAccountRole = new WEditNumber;
                poLayoutUpdateUsersAccount->Layout_AddLabelAndWidgetH_PA("User Id:", m_pwEditUpdateAccountId);
                poLayoutUpdateUsersAccount->Layout_AddLabelAndWidgetH_PA("User Name:", m_pwEditUpdateAccountName);
                poLayoutUpdateUsersAccount->Layout_AddLabelAndWidgetH_PA("User Role:", m_pwEditUpdateAccountRole);
                pwButton = new WButtonTextWithIcon("Update User Account", eMenuIconCoffeeBuy);
                poLayoutUpdateUsersAccount->addWidget(pwButton);
                poLayoutUpdateUsersAccount->addWidget(PA_CHILD new QWidget, 1);
                connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUserAccountUpdate()));

                //Get User's Document
                OLayoutHorizontal* poLayoutUserDocuments = new OLayoutHorizontal((QWidget *)NULL);
                m_poLayoutButtons->addRow(poLayoutUserDocuments);
                m_pwEditGetUserDocumentId=new WEditNumber;
                poLayoutUserDocuments->Layout_AddLabelAndWidgetH_PA("Document Id:", m_pwEditGetUserDocumentId);
                pwButton = new WButtonTextWithIcon("Get Document", eMenuIconCoffeeBuy);
                poLayoutUserDocuments->addWidget(pwButton);
                poLayoutUserDocuments->addWidget(PA_CHILD new QWidget, 1);
                connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUserDocumentGet()));

                //Buttons to Fetch Data
                OLayoutHorizontal* poLayoutFetchData = new OLayoutHorizontal((QWidget *)NULL);
                m_poLayoutButtons->addRow(poLayoutFetchData);
                    //Fetching User's Balances

                    pwButton = new WButtonTextWithIcon("Fetch User's' Balances",eMenuIconCoffeeBuy);
                    poLayoutFetchData->addWidget(pwButton);
                    connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonFetchUserBalances()));

                    //Fetching Users
                    pwButton = new WButtonTextWithIcon("Fetch Users",eMenuIconCoffeeBuy);
                    poLayoutFetchData->addWidget(pwButton);
                    connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonFetchUsers()));

                    //Fetching User's Documents
                    pwButton = new WButtonTextWithIcon("Fetch User's Documents",eMenuIconCoffeeBuy);
                    poLayoutFetchData->addWidget(pwButton);
                    connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonFetchUserDocuments()));

                    //Upload a Document
                    pwButton = new WButtonTextWithIcon("Upload a Document...",eMenuIconCoffeeBuy);
                    poLayoutFetchData->addWidget(pwButton);
                    connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonUploadADocument()));

                poLayoutFetchData->addWidget(PA_CHILD new QWidget, 1);

				/*
				// We have a valid Session ID, so we may download the bids.  For this, we will display a new button
				WButtonTextWithIcon * pwButton = new WButtonTextWithIcon("Download Market Bids", eMenuIconCoffeeBuy);
				m_poLayoutButtons->addRow(pwButton);
				connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonDownloadMarketBids()));
				*/
				m_pwLabelWelcome->hide();
				m_pwButtonRegister->hide();
				DisplayOrders(0);
                DisplayUserBalances(0);
				}
			}
		return;
		} // if (UserLogin)

	pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserOrderAdd");
	if (pXmlNode != NULL)
		{
		DisplayOrders(pXmlNode);
		DisplayMarketOrders(pXmlNode);	// Also add the new order to the Market Orders
		return;
		}
	pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserOrders");
	if (pXmlNode != NULL)
		{
		DisplayOrders(pXmlNode);
		return;
		}

	pXmlNode = oInternetResponse.PGetXmlNodeResponse("MarketOrders");
	if (pXmlNode != NULL)
		{
		DisplayMarketOrders(pXmlNode);
		return;
		}

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserBalanceUpdate");
    if (pXmlNode != NULL)
        {
        DisplayUserBalanceUpdated(pXmlNode);
        return;
        }

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("FetchUserBalances");
    if (pXmlNode != NULL)
        {
        MessageLog_AppendTextFormatCo(d_coRed, "Fetching user balances\n\n");
        DisplayUserBalances(pXmlNode);
        return;
        }

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("FetchUsers");
    if (pXmlNode != NULL)
        {
        MessageLog_AppendTextFormatCo(d_coRed, "Fetching all users\n\n");
        DisplayUsers(pXmlNode);
        return;
        }

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserAccountUpdate");
    if (pXmlNode != NULL)
        {
        MessageLog_AppendTextFormatCo(d_coRed, "updating user account\n\n");
        DisplayUserAccountUpdate(pXmlNode);
        return;
        }

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserDocuments");
    if (pXmlNode != NULL)
        {
        MessageLog_AppendTextFormatCo(d_coRed, "getting all user's documents\n\n");
        DisplayAllUsersDocuments(pXmlNode);
        return;
        }


    pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserDocumentGet");
    if (pXmlNode != NULL)
        {
        MessageLog_AppendTextFormatCo(d_coRed, "getting all user's documents\n\n");
        SaveUsersDocument(pXmlNode);
        return;
            }

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserOrderUpdate");
    if (pXmlNode != NULL)
        {
        UserOrderUpdate(pXmlNode);
        return;
            }

    pXmlNode = oInternetResponse.PGetXmlNodeResponse("UserOrderCancel");
    if (pXmlNode != NULL)
        {
        UserOrderCancel(pXmlNode);
        return;
            }

	} // SL_InternetRequestCompleted()

void
WLayoutApplicationMayanX::SL_ButtonUserOrderAdd()
	{
	CInternetRequestWebMethodXmlMayanX oRequest("UserOrderAdd");
	oRequest.BinAppendXmlElementText("sAsset", (PSZUC)"Coffee");
	oRequest.BinAppendXmlElementText("fAsk", (PSZUC)"true");
	oRequest.BinAppendXmlElementText("nQuantity", m_pwEditQuantity);
	oRequest.BinAppendXmlElementText("nPrice", m_pwEditPrice);
	oRequest.BinAppendXmlElementText("sCurrency", (PSZUC)"USD");
	m_oInternetServer.RequestSend(&oRequest);
	}

void
WLayoutApplicationMayanX::SL_ButtonUserOrderCancel()
    {
    CInternetRequestWebMethodXmlMayanX oRequest("UserOrderCancel");
    oRequest.BinAppendXmlElementText("xOrderID", m_pwEditIdToCancelOrder);
    m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_ButtonUserBalanceUpdate()
    {
    CInternetRequestWebMethodXmlMayanX oRequest("UserBalanceUpdate");
    //oRequest.BinAppendXmlElementText("xSessionId", m_oInternetServer.m_strxSessionID);	// This needs to be fixed within m_oInternetServer
    oRequest.BinAppendXmlElementText("sAsset", (PSZUC)"Coffee");
    oRequest.BinAppendXmlElementText("nQuantity", m_pwEditUpdateBalanceQuantity);
    m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_ButtonUserOrderUpdate()
    {
    CInternetRequestWebMethodXmlMayanX oRequest("UserOrderUpdate");
    oRequest.BinAppendXmlElementText("nQuantity", m_pwEditQuantityToUpdateOrder);
    oRequest.BinAppendXmlElementText("nPrice", m_pwEditPriceToUpdateOrder);
    oRequest.BinAppendXmlElementText("xOrderID", m_pwEditIdToUpdateOrder);
    m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_ButtonFetchUserBalances()
    {
        CInternetRequestWebMethodXmlMayanX oRequest("FetchUserBalances");
        m_oInternetServer.RequestSend(&oRequest);

    }

void
WLayoutApplicationMayanX::SL_ButtonUserAccountUpdate()
    {
    CInternetRequestWebMethodXmlMayanX oRequest("UserAccountUpdate");
    oRequest.BinAppendXmlElementText("xUserId", m_pwEditUpdateAccountId);
    oRequest.BinAppendXmlElementText("sName", m_pwEditUpdateAccountName);
    oRequest.BinAppendXmlElementText("eRole", m_pwEditUpdateAccountRole);
    m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_ButtonFetchUsers()
    {
        CInternetRequestWebMethodXmlMayanX oRequest("FetchUsers");
        m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_ButtonFetchUserDocuments()
    {
        CInternetRequestWebMethodXmlMayanX oRequest("UserDocuments");
        m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_ButtonUploadADocument()
    {
        QFileDialog dialog;
         dialog.setFileMode(QFileDialog::AnyFile);
         if(dialog.exec()) {
             if(!dialog.selectedFiles().isEmpty())
             {
                 QString pathToFile=dialog.selectedFiles().first();
                 QString name= pathToFile.split("/").last();

                 CBin binFile;
                 binFile.BinFileReadE(pathToFile);

                 CInternetRequestWebMethodXmlMayanX oRequest("UploadADocument");
                 oRequest.BinAppendXmlElementText("eType", (PSZUC)"1");
                 oRequest.BinAppendXmlElementText("sDocID", (PSZUC)"F83CK23");
                 oRequest.BinAppendXmlElementText("sDocName", (PSZUC)name.toStdString().c_str());
                 oRequest.BinAppendXmlElementBinaryBase64("bDocData", binFile);
                 m_oInternetServer.RequestSend(&oRequest);
             }
         }
    }



void
WLayoutApplicationMayanX::SL_ButtonUserDocumentGet()
    {
        CInternetRequestWebMethodXmlMayanX oRequest("UserDocumentGet");
        oRequest.BinAppendXmlElementText("xID", m_pwEditGetUserDocumentId);
        m_oInternetServer.RequestSend(&oRequest);
    }

void
WLayoutApplicationMayanX::SL_TableMarketOrdersDoubleClicked(const QModelIndex & oIndex)
	{
	//LPARAM lParamRow = m_pwTableMarketBids->LParamGetFromRowSelected();
	LPARAM lParamRow = m_pwTableMarketBids->LParamGetFromRow(oIndex);

	int iColumn = oIndex.column();
	EMessageBoxInformation("lParamRow = $i, iColumn = $i", lParamRow, iColumn);
	}

void
WLayoutApplicationMayanX::DisplayMarketOrders(const CXmlNode * pXmlNodeMarketOrders)
    {
    if (m_pwTableMarketBids == NULL)
        {
        addWidget(new QLabel("Market Orders"));
        m_pwTableMarketBids = new WTable(this);
        m_pwTableMarketBids->SetColumns_VEZ("Volume", "Price", "Currency", NULL);
        // As soon as the table is visible, call the web service to fetch the list of bids

		#if 1
		OTableRow oRow;
		oRow.AddData((PSZUC)"row #1");
		m_pwTableMarketBids->AppendRow(oRow, 1);
		oRow.clear();
		oRow.AddData((PSZUC)"row #2");
		m_pwTableMarketBids->AppendRow(oRow, 2);
		connect(m_pwTableMarketBids, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(SL_TableMarketOrdersDoubleClicked(QModelIndex)));
		#endif

        CInternetRequestWebMethodXmlMayanX oRequest("MarketOrders");
        m_oInternetServer.RequestSend(&oRequest);
        }
    if (pXmlNodeMarketOrders == NULL)
        return;
    const CXmlNode * pXmlNodeElement;
    if (pXmlNodeMarketOrders->PFindElement("Id") != NULL)
        pXmlNodeElement = pXmlNodeMarketOrders;
    else
        pXmlNodeElement = pXmlNodeMarketOrders->PFindElement("Order");
    while (pXmlNodeElement != NULL)
        {
        OTableRow oRow;
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("nQuantity"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("nPrice"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("sCurrency"));
        m_pwTableMarketBids->AppendRow(oRow);
        pXmlNodeElement = pXmlNodeElement->PFindNextSibling();
        }
    }

void
WLayoutApplicationMayanX::DisplayOrders(const CXmlNode * pXmlNodeOrders)
    {
    if (m_pwTableOrders == NULL)
        {
        addWidget(new QLabel("Open Orders"));
        m_pwTableOrders = new WTable(this);
        m_pwTableOrders->SetColumns_VEZ("   Asset   ", "Volume", "Price", "Currency", "Cancel Order", NULL);

        CInternetRequestWebMethodXmlMayanX oRequest("UserOrders");
        m_oInternetServer.RequestSend(&oRequest);
        DisplayMarketOrders(NULL);
        }
    if (pXmlNodeOrders == NULL)
        return;
    const CXmlNode * pXmlNodeElement;
    if (pXmlNodeOrders->PFindElement("Id") != NULL)
        pXmlNodeElement = pXmlNodeOrders;
    else
        pXmlNodeElement = pXmlNodeOrders->PFindElement("Order");
    while (pXmlNodeElement != NULL)
        {
        OTableRow oRow;
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("sAsset"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("nQuantity"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("nPrice"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("sCurrency"));
        oRow.AddData((PSZUC)"[ x ]");
        m_pwTableOrders->AppendRow(oRow);
        pXmlNodeElement = pXmlNodeElement->PFindNextSibling();
        }
    } // DisplayOrders()


void
WLayoutApplicationMayanX::DisplayUserBalances(const CXmlNode * pXmlNodeUserBalances)
    {
    if (m_pwTableUserBalances == NULL)
        {
        addWidget(new QLabel("User Balances"));
        m_pwTableUserBalances = new WTable(this);
        m_pwTableUserBalances->SetColumns_VEZ("Asset", "Quantity", NULL);
        // As soon as the table is visible, call the web service to fetch the list of user balances
        CInternetRequestWebMethodXmlMayanX oRequest("FetchUserBalances");
        m_oInternetServer.RequestSend(&oRequest);
        }
    else
    {
        m_pwTableUserBalances->clear();
        m_pwTableUserBalances->SetColumns_VEZ("Asset", "Quantity", NULL);
    }
    if (pXmlNodeUserBalances == NULL)
        return;
    const CXmlNode * pXmlNodeElement;
    pXmlNodeElement = pXmlNodeUserBalances->PFindElement("Balance");
    while (pXmlNodeElement != NULL)
        {
        OTableRow oRow;
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("Asset"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("Quantity"));
        m_pwTableUserBalances->AppendRow(oRow);
        pXmlNodeElement = pXmlNodeElement->PFindNextSibling();
        }

    } // DisplayUserBalances()

void
WLayoutApplicationMayanX::DisplayUsers(const CXmlNode * pXmlNodeUsers)
    {
    if (m_pwTableUsers == NULL)
        {
        addWidget(new QLabel("Users"));
        m_pwTableUsers = new WTable(this);
        m_pwTableUsers->SetColumns_VEZ("Id","Name", "Role", NULL);
        // As soon as the table is visible, call the web service to fetch the list of user balances
        CInternetRequestWebMethodXmlMayanX oRequest("FetchUsers");
        m_oInternetServer.RequestSend(&oRequest);
        }
    else
        {
        m_pwTableUsers->clear();
        m_pwTableUsers->SetColumns_VEZ("Id","Name", "Role", NULL);
        }
    if (pXmlNodeUsers == NULL)
        return;
    const CXmlNode * pXmlNodeElement;
    pXmlNodeElement = pXmlNodeUsers->PFindElement("User");
    while (pXmlNodeElement != NULL)
        {
        OTableRow oRow;
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("Id"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("Name"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("eRole"));
        m_pwTableUsers->AppendRow(oRow);
        pXmlNodeElement = pXmlNodeElement->PFindNextSibling();
        }

    } // DisplayUsers()

void
WLayoutApplicationMayanX::DisplayUserAccountUpdate(const CXmlNode * )
    {
    CInternetRequestWebMethodXmlMayanX oRequest("FetchUsers");
    m_oInternetServer.RequestSend(&oRequest);
    } // DisplayUserAccountUpdate()

    void
WLayoutApplicationMayanX::DisplayAllUsersDocuments(const CXmlNode * pXmlNodeUsers)
    {
    if (m_pwTableUsersDocuments == NULL)
        {
        addWidget(new QLabel("Documents"));
        m_pwTableUsersDocuments = new WTable(this);
        m_pwTableUsersDocuments->SetColumns_VEZ("Id","Name", "DocId", "Type", "Status", NULL);
        // As soon as the table is visible, call the web service to fetch the list of user balances
        CInternetRequestWebMethodXmlMayanX oRequest("UserDocuments");
        m_oInternetServer.RequestSend(&oRequest);
        }
    else
    {
        m_pwTableUsersDocuments->clear();
        m_pwTableUsersDocuments->SetColumns_VEZ("Id","Name", "DocId", "Type", "Status", NULL);
    }
    if (pXmlNodeUsers == NULL)
        return;
    const CXmlNode * pXmlNodeElement;
    MessageLog_AppendTextFormatCo(d_coRed, "XML-1: ^N",pXmlNodeUsers);
    pXmlNodeElement = pXmlNodeUsers->PFindElement("Document");
    MessageLog_AppendTextFormatCo(d_coRed, "XML-2: ^N",pXmlNodeElement);


    while (pXmlNodeElement != NULL)
        {
        OTableRow oRow;
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("Id"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("DocName"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("DocId"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("eType"));
        oRow.AddData(pXmlNodeElement->PszuFindElementOrAttributeValue("eStatus"));
        m_pwTableUsersDocuments->AppendRow(oRow);
        pXmlNodeElement = pXmlNodeElement->PFindNextSibling();
        }

    }

void
WLayoutApplicationMayanX::SaveUsersDocument(const CXmlNode * pXmlNodeGetDocument){

    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
      if(dialog.exec()) {
          QString dirname= dialog.selectedFiles().first();
          if( !dirname.isNull() )
          {
            if (pXmlNodeGetDocument == NULL)
                return;

            if (pXmlNodeGetDocument != NULL)
            {
                CBin file;
                file.BinAppendBinaryDataFromBase64Szv(pXmlNodeGetDocument->PszuFindElementOrAttributeValue("DocData"));
                CString sDocName =  pXmlNodeGetDocument->PszuFindElementOrAttributeValue("DocName");
                QString filename=QString(sDocName);
                file.BinFileWriteE(dirname+"/"+filename);
             }
          }
      }
}

void
WLayoutApplicationMayanX::UserOrderUpdate(const CXmlNode * pXmlNode)
{
    MessageLog_AppendTextFormatCo(d_coRed, "XML-1: ^N",pXmlNode);
}

void
WLayoutApplicationMayanX::UserOrderCancel(const CXmlNode * pXmlNode)
{
    MessageLog_AppendTextFormatCo(d_coRed, "XML-1: ^N",pXmlNode);
}

void
WLayoutApplicationMayanX::DisplayUserBalanceUpdated(const CXmlNode * pXmlNodeNewBalance)
    {
	int nBalance=300;
    if(pXmlNodeNewBalance!=NULL)
    {
        MessageLog_AppendTextFormatCo(d_coRed, "is not null\n\n");
        MessageLog_AppendTextFormatCo(d_coRed, "XML: ^N",pXmlNodeNewBalance);
    }

    nBalance=LStringToNumber_ZZR_ML(pXmlNodeNewBalance->m_pszuTagValue);
    m_pwLabelBalance->Label_SetTextFormat_VE_Gsb("Your new balance is $i", nBalance);
    } // DisplayUserBalanceUpdated()
