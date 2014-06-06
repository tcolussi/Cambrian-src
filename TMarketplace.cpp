//	Tree Item representing the root node of the 'Marketplace'
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TMarketplace.h"

void
NavigationTree_NewBrowser()
	{
	TBrowser * pBrowser = new TBrowser(&g_oConfiguration);
	g_oConfiguration.m_arraypaBrowsers.Add(PA_CHILD pBrowser);
	pBrowser->SetIconNameAndUrl(eMenuAction_DisplaySecureWebBrowsing, "Web Browsing");
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItem_SelectWithinNavigationTree();
	}

void
NavigationTree_NewBrowserMarketplace()
	{
	TBrowser * pBrowser = new TBrowser(&g_oConfiguration);
	g_oConfiguration.m_arraypaBrowsers.Add(PA_CHILD pBrowser);
	pBrowser->SetIconNameAndUrl(eMenuIconMarketplace, "Marketplace", "http://marketplace.metropipe.net:9001");
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItem_SelectWithinNavigationTree();
	}

TTreeItemDemo * g_pBanking;
TTreeItemDemo * g_pSecurity;

TMarketplace::TMarketplace()
	{
	m_pawMarketplace = NULL;

	// This code is not part of the 'Marketplace', however was added for demo purpose
	new TTreeItemDemo(NULL, "Reputation", eMenuIconReputation);

	TTreeItemDemo * pRegistry = new TTreeItemDemo(NULL, "Registry", eMenuIconCertificate);
	new TTreeItemDemo(pRegistry, "Incorporate a Company", eMenuIconCorporations, "Placeholder to enter information to register a corporation");
	//new TBrowser(pRegistry, "Incorporate a Company", eMenuIconCorporations, "http://registry.cambrian.org/company.aspx");
	new TTreeItemDemo(pRegistry, "Register as a Mediator", eMenuIconMarketplaceMediation);
	new TTreeItemDemo(pRegistry, "Register as an Arbitrator", eMenuIconMarketplaceArbitration);
	new TTreeItemDemo(pRegistry, "Start a new Jurisdiction", eMenuIconJurisdiction);

	g_pBanking = new TTreeItemDemo(NULL, "Banking", eMenuIconBanking);
	new TTreeItemDemo(g_pBanking, "Open a bank account", eMenuIconBankNew);
	g_oConfiguration.NavigationTree_DisplayWallet();

	TTreeItemDemo * pOT = new TTreeItemDemo(NULL, "Open Transactions Banking", eMenuIconBanking);
		TTreeItemDemo * pBanks = new TTreeItemDemo(pOT, "Featured Banks", eMenuIconBank);
			new TTreeItemDemo(pBanks, "HSBC", eMenuIconBank);
			new TTreeItemDemo(pBanks, "CityBank", eMenuIconBank);
			new TTreeItemDemo(pBanks, "<Register Existing Bank...>", eMenuIconBankNew);
		TTreeItemDemo * pIssue = new TTreeItemDemo(pOT, "Issue", eMenuIconIssue);
			new TTreeItemDemo(pIssue, "Asset", eMenuIconIssueAsset);
			new TTreeItemDemo(pIssue, "Currency", eMenuIconIssueCurrency);
			new TTreeItemDemo(pIssue, "Future", eMenuIconIssueFuture);
		TTreeItemDemo * pCorporations = new TTreeItemDemo(pOT, "Corporations Dashboard", eMenuIconCorporations);
			TTreeItemDemo * pMonetas = new TTreeItemDemo(pCorporations, "Monetas", eMenuIconCorporations);
				new TTreeItemDemo(pMonetas, "Issue Dividends", eMenuIconIssueDividends);
				new TTreeItemDemo(pMonetas, "Notify Shareholders of Vote", eMenuIconVote);
			new TTreeItemDemo(pCorporations, "<Create New Corporation...>", eMenuIconCorporations);


	TTreeItemDemo * pExchange = new TTreeItemDemo(NULL, "Exchange", eMenuIconExchange);
		new TTreeItemDemo(pExchange, "Buy Bitcoins", eMenuIconBitcoin);
		new TTreeItemDemo(pExchange, "Sell Bitcoins", eMenuIconSell);
		TTreeItemDemo * pOtMarkets = new TTreeItemDemo(pExchange, "OT Markets Dashboard", eMenuIconMarket);
			new TTreeItemDemo(pOtMarkets, "OT://1  OrlinUSD <-> BTC", eMenuIconExchange);
			new TTreeItemDemo(pOtMarkets, "OT://1  Monetas Shares <-> BTC", eMenuIconExchange);
			new TTreeItemDemo(pOtMarkets, "OT://2  Cambrian Shares <-> OrlinUSD", eMenuIconExchange);
		TTreeItemDemo * pLocalAssets = new TTreeItemDemo(pExchange, "LocalAssets Dashboard", eMenuIconIssueAsset);
			new TTreeItemDemo(pLocalAssets, "Place Ad", eMenuIconBroadcast);
			new TTreeItemDemo(pLocalAssets, "Search City", eMenuAction_FindText);
		TTreeItemDemo * pOtRipple = new TTreeItemDemo(pExchange, "OT Ripple Dashboard", eMenuIconRipple);
			new TTreeItemDemo(pOtRipple, "Broadcast Buy/Sell Offer", eMenuIconBroadcast);
			new TTreeItemDemo(pOtRipple, "Listen for Buy/Sell Offers", eMenuIconListen);

		#if 0
		TTreeItemDemo * pExchangeCoffee = new TTreeItemDemo(pExchange, "MayanX Coffee Exchange", eMenuIconCoffeeExchange, "Placeholder to display a summary of a coffee exchange, as well as promotions and/or featured deals");
			/*
			TTreeItemDemo * pNew = new TTreeItemDemo(pExchangeCoffee, "New Account", eMenuIconCoffeeBuy);
				new TTreeItemDemo(pNew, "Cell Alpha", eMenuIconCoffeeBuy);
				new TTreeItemDemo(pNew, "Cell Beta", eMenuIconCoffeeBuy);
			*/
			new TTreeItemDemo(pExchangeCoffee, "Buy Coffee", eMenuIconCoffeeBuy, "Placeholder to display asks by coffee farmers", "Search Offers");
			new TTreeItemDemo(pExchangeCoffee, "Sell Coffee", eMenuIconCoffeeSell, "Placeholder to display bids by potential coffee buyers", "Search Bids");
			new TTreeItemDemo(pExchangeCoffee, "Trade History", eMenuIconExchange, "Display the trade history of the user", "Search Trade History");
		#endif

	TTreeItemDemo * pMarketplace = new TTreeItemDemo(NULL, "Marketplace", eMenuIconMarketplace, "Placeholder to display premium products and services on the marketplace", "Search Marketplace");
		new TTreeItemDemo(pMarketplace, "For Sale", eMenuIconSell, "Placeholder to display products and services for sale", "Search");		
//		new TTreeItemDemo(pMarketplace, "Personals", eMenuIconPersonals, "People searching for love", "Search for Love");
		TTreeItemDemo * pServices = new TTreeItemDemo(pMarketplace, "Services", eMenuIconServices);
			new TTreeItemDemo(pServices, "Healthcare", eMenuIconHealthcare, "Placeholder to display healthcare providers and insurers", "Search healthcare provider and/or insurance companies");
			new TTreeItemDemo(pServices, "Security", eMenuIconSecurityGuard, "Private security");
			new TTreeItemDemo(pServices, "Lawyers", eMenuIconMarketplaceLawyers, "Placeholder to display premium legal services", "Search Legal Services");
			new TTreeItemDemo(pServices, "Mediators", eMenuIconMarketplaceMediation);
			new TTreeItemDemo(pServices, "Arbitrators", eMenuIconMarketplaceArbitration);
		new TTreeItemDemo(pMarketplace, "Jobs", eMenuIconJobs, "Placeholder to display jobs availables", "Search Jobs");

	TTreeItemDemo * pJurisdictions = new TTreeItemDemo(NULL, "Jurisdictions", eMenuIconJurisdiction);
	new TTreeItemDemo(pJurisdictions, "Seasteading", eMenuIconJurisdiction);
	TTreeItemDemo * pStartupCities = new TTreeItemDemo(pJurisdictions, "Startup Cities", eMenuIconJurisdiction);
		new TTreeItemDemo(pStartupCities, "Elevator", eMenuIconJurisdiction);
	pJurisdictions->TreeItem_AllocateChildren_VEZ(eMenuIconJurisdiction, "Intentional Communities", "Personality Based", "Interest Based", "Traditional Cities", "Traditional Countries", NULL);

	g_pSecurity = new TTreeItemDemo(NULL, "Security", eMenuIconSecurityOptions, "Placeholder for various options to enhance the security of the application");
	g_oConfiguration.NavigationTree_DisplayAllCertificates();
	// Password Vault
	// CryptoWill

	}

TMarketplace::~TMarketplace()
	{
	delete m_pawMarketplace;
	}

//	TMarketplace::ITreeItem::TreeItem_GotFocus()
void
TMarketplace::TreeItem_GotFocus()
	{
	if (m_pawMarketplace == NULL)
		m_pawMarketplace = new WMarketplace;
	MainWindow_SetCurrentLayout(IN m_pawMarketplace);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
CMarketplaceArbitrators::CMarketplaceArbitrators()
	{
	m_pawMarketplace = NULL;
	}

CMarketplaceArbitrators::~CMarketplaceArbitrators()
	{
	delete m_pawMarketplace;
	}

//	CMarketplaceArbitrators::ITreeItem::TreeItem_GotFocus()
void
CMarketplaceArbitrators::TreeItem_GotFocus()
	{
	if (m_pawMarketplace == NULL)
		m_pawMarketplace = new WMarketplaceArbitrators;
	MainWindow_SetCurrentLayout(IN m_pawMarketplace);
	}



WLayoutWallet::WLayoutWallet(ITreeItem * pTreeItemFilterBy, EWalletViewFlags eWalletViewFlags)
	{
	Assert(pTreeItemFilterBy != NULL);
	CStr strSearch;
	if (pTreeItemFilterBy->EGetRuntimeClass() != RTI(TContact))
		strSearch.Format("Search transactions related to $s", pTreeItemFilterBy->TreeItem_PszGetNameDisplay());
	else
		strSearch.Format("Search transactions within $s", pTreeItemFilterBy->TreeItem_PszGetNameDisplay());

	WEdit * pwEdit = new WEdit;
	pwEdit->setMaximumHeight(20);
	pwEdit->Edit_SetWatermark(strSearch.PszaGetUtf8NZ());
	pwEdit->setParent(this);

	WTable * pwTable = new WTable(this);
	pwTable->SetColumns_VEZ("Date", "Contact", "Amount mBTC", "Value", "Comment", NULL);

	CArrayPtrEventsWalletTransactions arraypaTransactions;
	arraypaTransactions.Wallets_AppendEventsTransactionsFor(pTreeItemFilterBy, eWalletViewFlags);
	CEventWalletTransaction ** ppTransactionStop;
	CEventWalletTransaction ** ppTransaction = arraypaTransactions.PrgpGetTransactionsStop(OUT &ppTransactionStop);
	while (ppTransaction != ppTransactionStop)
		{
		CEventWalletTransaction * pTransaction = *ppTransaction++;
//		Assert(pTransaction->EGetEventType() == eEventType_SendBitcoins);
		OTableRow oRow;
		oRow.AddDateTime(pTransaction->m_tsEventID);
		oRow.AddData(pTransaction->mu_parentowner.pTreeItem->TreeItem_PszGetNameDisplay());
		oRow.AddAmount_mBTC(pTransaction->m_amtQuantity);
		oRow.AddData(pTransaction->m_strValue);
		oRow.AddData(pTransaction->m_strComment);
		pwTable->AppendRow(oRow);
		}
	pwTable->horizontalHeader()->setStretchLastSection(true);
	pwTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	connect(pwTable, SIGNAL(clicked(QModelIndex)), this, SLOT(SL_TableCell_clicked(QModelIndex)));
	}

void
WLayoutWallet::SL_TableCell_clicked(const QModelIndex & index)
	{
	MessageLog_AppendTextFormatCo(d_coGray, "Clicked: row=$i, column=$i\n", index.row(), index.column());
	}

const EMenuActionByte c_rgzeActionsMenuWallet[] =
	{
	eMenuAction_WalletGenerateDummyTransactions,
	eMenuActionSeparator,
	eMenuAction_WalletClose,
	ezMenuActionNone
	};


//	TWallet::ITreeItem::TreeItem_MenuAppendActions()
void
TWallet::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuWallet);
	}


//	TWallet::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TWallet::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_WalletGenerateDummyTransactions:
		GenerateDummyTransactions();
		return ezMenuActionNone;
	case eMenuAction_WalletClose:
		SerializeXmlToDisk();	// Save the wallet before deleting it
		delete this;
		return ezMenuActionNone;
	case eMenuSpecialAction_ITreeItemRenamed:
		return ezMenuActionNone;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

//	TWallet::ITreeItem::TreeItem_GotFocus()
void
TWallet::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(new WLayoutWalletGrid(this, eWalletViewFlag_kmDisplayTransactionsAll));
	}

//	TWalletView::ITreeItem::TreeItem_GotFocus()
void
TWalletView::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(new WLayoutWallet(m_pTreeItemFilterBy, m_eWalletViewFlags));
	}



///////////////////////////////////////////////////////////////////////////////////////////////////

