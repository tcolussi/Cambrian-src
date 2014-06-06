#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutProfile.h"

//	TMyProfiles::ITreeItem::TreeItem_GotFocus()
void
TMyProfiles::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutMyProfiles);
	}

//	TProfile::ITreeItem::TreeItem_GotFocus()
void
TProfile::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutProfile(this));
	}

WLayoutMyProfiles::WLayoutMyProfiles()
	{
	m_pAccountNew = NULL;
	OLayoutVerticalAlignTop * poLayoutVertical = Splitter_PoAddGroupBoxAndLayoutVertical_VE("Welcome to Cambrian");
	WLabelSelectableWrap * pwLabel = new WLabelSelectableWrap(g_oConfiguration.m_arraypaProfiles.FIsEmpty() ?
		"To get started, you need to create a profile.\n"
		"Your profile contains information about you or your business for communicating with others.\n\n"
		"Creating a profile is easy; you pick a name you wish other people will recognize you." :
		"You already have a profile, however you are welcome to have multiple profiles.");
	poLayoutVertical->addWidget(pwLabel);
	m_pwEditProfile = new WEdit;
	m_pwEditProfile->Edit_SetWatermark("Enter name, nickname or business name of your new profile");
	m_pwEditProfile->Edit_SetToolTip((PSZUC)"Examples:\n\tJoe Smith\n\tSuperman\n\tACME Widgets, Inc.");
//	m_pwEditProfile->setFocus();

	OLayoutHorizontalAlignLeft * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	//poLayout->Layout_AddLabelAndWidgetH_PA("ID:", m_pwEditProfile);
	poLayout->addWidget(m_pwEditProfile);
	WButtonTextWithIcon * pwButtonCreateNewID = new WButtonTextWithIcon("Create Profile ", eMenuIconIdentities);
	poLayout->addWidget(pwButtonCreateNewID);

	m_pwCheckboxAutomatic = new QCheckBox("Automatically create an XMPP account for my new profile");
	m_pwCheckboxAutomatic->setToolTip("Uncheck this option if you wish to manually assign an XMPP account to your new profile");
	m_pwCheckboxAutomatic->setChecked(true);
	poLayoutVertical->addWidget(m_pwCheckboxAutomatic);

	m_pwEditSocketMessageLog_YZ = new WEditMessageLogHidden;
	poLayoutVertical->addWidget(m_pwEditSocketMessageLog_YZ);

	connect(pwButtonCreateNewID, SIGNAL(clicked()), this, SLOT(SL_ButtonCreateNewProfile()));
	connect(m_pwEditProfile, SIGNAL(returnPressed()), this, SLOT(SL_ButtonCreateNewProfile()));
	}

WLayoutMyProfiles::~WLayoutMyProfiles()
	{
	/*
	if (m_pAccountNew != NULL)
		m_pAccountNew->Socket_DisconnectUI();
	*/
	}

void
WLayoutMyProfiles::SL_ButtonCreateNewProfile()
	{
	CStr strProfileName = *m_pwEditProfile;
	if (strProfileName.FIsEmptyString())
		{
		EMessageBoxWarning("Please enter a valid name for your profile.");
		m_pwEditProfile->setFocus();
		return;
		}
	// Search if the profile is not already there
	TProfile ** ppProfileStop;
	TProfile ** ppProfile = g_oConfiguration.m_arraypaProfiles.PrgpGetProfilesStop(OUT &ppProfileStop);
	while (ppProfile != ppProfileStop)
		{
		TProfile * pProfile = *ppProfile++;
		Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
		if (pProfile->m_strNameProfile.FCompareStringsNoCase(strProfileName))
			{
			EMessageBoxWarning("There is already a profile named '$S'.  Please pick another profile name.", &strProfileName);
			m_pwEditProfile->setFocus();
			return;
			}
		}
	TProfile * pProfile = new TProfile(&g_oConfiguration);	// At the moment, there is only one configuration object, so use the global variable
	g_oConfiguration.m_arraypaProfiles.Add(PA_CHILD pProfile);
	pProfile->m_strNameProfile = strProfileName;
	pProfile->GenerateKeys();
	pProfile->TreeItemProfile_DisplayWithinNavigationTree();
	//pProfile->m_paTreeWidgetItem->setExpanded(true);

	#ifdef DEBUG_IMPORT_OLD_CONFIG_XML
	if (g_arraypAccounts.FIsEmpty())
		{
		// There is no account yet, so attempt to import the accounts from the old configuration
		pProfile->UnserializeContactsFromOldConfigXml();
		if (!g_arraypAccounts.FIsEmpty())
			{
			// We imported some old XMPP accounts, therefore there is no need to create a new one
			pProfile->TreeItemLayout_SetFocus();
			return;
			}
		}
	#endif
	if (m_pwCheckboxAutomatic->isChecked())
		pProfile->PAllocateAccountAutomaticCreationUI(this);
	else
		pProfile->TreeItemLayout_SetFocus();
	} // SL_ButtonCreateNewProfile()

//	WLayoutMyProfiles::WLayout::Layout_SetFocus()
void
WLayoutMyProfiles::Layout_SetFocus()
	{
	m_pwEditProfile->setFocus();
	}

//	WLayoutMyProfiles::ISocketUI::SocketUI_OnSuccess()
void
WLayoutMyProfiles::SocketUI_OnSuccess()
	{
	Assert(m_pSocket_YZ != NULL);
	PGetAccount_YZ()->m_pProfileParent->TreeItemLayout_SetFocus();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutProfile::WLayoutProfile(TProfile * pProfile)
	{
	m_pProfile = pProfile;
	m_pAccountNew = NULL;
	m_pwGroupBoxAccountNew = NULL;
	m_poLayoutApplications = NULL;
	m_pwEditSearchApplications = NULL;
	if (pProfile->m_arraypaAccountsXmpp.FIsEmpty())
		{
		m_pwGroupBoxAccountNew = Splitter_PwAddGroupBox_VE("Please assign an XMPP account for your profile '$S'", &pProfile->m_strNameProfile);
		OLayoutVerticalAlignTop * poLayout = new OLayoutVerticalAlignTop(m_pwGroupBoxAccountNew);
		poLayout->Layout_PwAddRowLabelWrap("Cambrian needs an XMPP account to send and receive messages with others:");
		WButtonTextWithIcon * pwButtonAccountNewAutomatic = poLayout->Layout_PwAddRowButtonAndLabel("Automatic!", eMenuIconXmpp, "Automatically and instantly create an XMPP account");
		m_pwEditSocketMessageLog_YZ = new WEditMessageLogHidden;
		poLayout->addWidget(m_pwEditSocketMessageLog_YZ);
		WButtonTextWithIcon * pwButtonAccountNewManual = poLayout->Layout_PwAddRowButtonAndLabel("Sign Up...", eMenuIconXmpp, "Manually create an XMPP account by specifying a username and password");
		WButtonTextWithIcon * pwButtonAccountLogin = poLayout->Layout_PwAddRowButtonAndLabel("Login...", eMenuIconXmpp, "Login to an existing XMPP account using a username and password");
		poLayout->Layout_AddWidgetSpacer();
		connect(pwButtonAccountNewAutomatic, SIGNAL(clicked()), this, SLOT(SL_ButtonAccountNewInstant()));
		connect(pwButtonAccountNewManual, SIGNAL(clicked()), this, SLOT(SL_ButtonAccountNewManual()));
		connect(pwButtonAccountLogin, SIGNAL(clicked()), this, SLOT(SL_ButtonAccountLogin()));
		}
	else
		DisplayApplications();
	//Splitter_AddWidgetSpacer();
	}

WLayoutProfile::~WLayoutProfile()
	{
	/*
	if (m_pAccountNew != NULL)
		m_pAccountNew->Socket_DisconnectUI();	// Disconnect the socket from the UI, otherwise the application will crash as the socket will attempt to invoke virtual methods of a non-existent interface ISocketUI.
	*/
	}

void
CStr::InitWithRandomPassword(const void * pvRandomValue)
	{
	Assert(CbGetData() == 0);
	BinAppendStringBase16FromBinaryData(IN pvRandomValue, 6);	// Generate a password of 12 hexadecimal characters
	m_paData->cbData++;	// Include the null-terminator
	}

void
CStr::InitWithRandomUsername(const CStr & strUsernameBase, UINT uRandomValue)
	{
	PSZUC pszUsernameBase = strUsernameBase;
	PSZU pszUsernameRandom = (PSZU)PvSizeAlloc(strUsernameBase.CbGetData() + 32);
	CHU * pchUsernameRandom = pszUsernameRandom;
	while (TRUE)
		{
		CHS ch = Ch_GetCharLowercase(*pszUsernameBase++);
		if (ch == '\0')
			break;
		if (ch < 'a' || ch > 'z')
			{
			if (ch != '.')
				ch = '-';	// Replace everything else with a dash
			}
		*pchUsernameRandom++ = ch;
		} // while
	pchUsernameRandom = PchNumberToString32(OUT pchUsernameRandom, uRandomValue & 0x0FFFFFF);	// Append a random value up to 16 million.
	*pchUsernameRandom = '\0';
	m_paData->cbData = pchUsernameRandom - pszUsernameRandom + 1;
	} // InitWithRandomUsername()

TAccountXmpp *
TProfile::PAllocateAccount()
	{
	TAccountXmpp * pAccount = new TAccountXmpp(this);
	m_arraypaAccountsXmpp.Add(PA_CHILD pAccount);
	return pAccount;
	}

TAccountXmpp *
TProfile::PAllocateAccountAutomaticCreationUI(ISocketUI * piSocketUI)
	{
	Assert(piSocketUI != NULL);
	TAccountXmpp * pAccount = PAllocateAccount();
	SHashSha1 hashSha1;
	HashSha1_InitRandom(OUT &hashSha1);	// Generate a truly random username and password
	pAccount->m_strUsername.InitWithRandomUsername(m_strNameProfile, *(UINT *)&hashSha1);
	pAccount->m_strPassword.InitWithRandomPassword(IN hashSha1.rgbData + 4);
	pAccount->TreeItemAccount_DisplayWithinNavigationTreeInit(c_szXmppServerDefault, d_uPortXmppDefault);
	pAccount->Socket_ConnectUI(piSocketUI, TRUE);
	return pAccount;
	}


void
WLayoutProfile::SL_ButtonAccountNewInstant()
	{
	/*
	if (m_pAccountNew != NULL)
		m_pAccountNew->Socket_DisconnectUI();	// Disconnect the previous socket from the UI
	*/
	m_pAccountNew = m_pProfile->PAllocateAccountAutomaticCreationUI(this);
	}

void
WLayoutProfile::SL_ButtonAccountNewManual()
	{
	DisplayDialogAccountNew(eMenuAction_AccountRegister);
	}

void
WLayoutProfile::SL_ButtonAccountLogin()
	{
	DisplayDialogAccountNew(eMenuAction_AccountLogin);
	}


//	WLayoutProfile::WLayout::Layout_SetFocus()
void
WLayoutProfile::Layout_SetFocus()
	{
	//MessageLog_AppendTextFormatCo(d_coGreen, "WLayoutProfile::Layout_SetFocus()\n");
	if (m_pwEditSearchApplications != NULL)
		m_pwEditSearchApplications->setFocus();
	}

//	WLayoutProfile::ISocketUI::SocketUI_OnSuccess()
void
WLayoutProfile::SocketUI_OnSuccess()
	{
	Assert(m_pSocket_YZ != NULL);
	DisplayApplications();
	}

void
WLayoutProfile::SL_SearchTextChanged(const QString & sSearch)
	{
	QTreeWidgetItemIterator oIterator(m_pwTreeViewApplications);
	if (sSearch.isEmpty())
		{
		// Show the entire tree
		while (TRUE)
			{
			CTreeWidgetItem * pTreeWidgetItem = (CTreeWidgetItem *)*oIterator++;
			if (pTreeWidgetItem == NULL)
				return;
			pTreeWidgetItem->setHidden(false);
			}
		}
	else
		{
		while (TRUE)
			{
			CTreeWidgetItem * pTreeWidgetItem = (CTreeWidgetItem *)*oIterator++;
			if (pTreeWidgetItem == NULL)
				return;
			BOOL fShowRow = (pTreeWidgetItem->text(0).contains(sSearch, Qt::CaseInsensitive) ||
							 pTreeWidgetItem->text(1).contains(sSearch, Qt::CaseInsensitive));
			pTreeWidgetItem->SetItemVisibleAlongWithItsParents(fShowRow);
			}
		}
	} // SL_SearchTextChanged()

void
WLayoutProfile::SL_ActionDoubleClicked(QTreeWidgetItem * pItemClicked, int iColumn)
	{
	UNUSED_PARAMETER(iColumn);
	IApplication * pApplication = NULL;
	QString sSelected = pItemClicked->text(0);
	if (sSelected.contains("MayanX",  Qt::CaseInsensitive))
		pApplication = (IApplication *)m_pProfile->PAllocateApplicationMayanX();	// Temporary hack

	if (pApplication != NULL)
		pApplication->TreeItemLayout_SetFocus();
	else
		//EMessageBoxInformation("At the moment, only the Bitcoin wallet and MayanX Exchange are implemented");
		EMessageBoxInformation("At the moment, only the MayanX Exchange is implemented");
	}

void
WLayoutProfile::DisplayApplications()
	{
	m_pwEditSocketMessageLog_YZ = NULL;
	delete m_pwGroupBoxAccountNew;	// Hide the widgets to create an account
	m_pwGroupBoxAccountNew = NULL;
	if (m_poLayoutApplications != NULL)
		return;	// We are already displaying the actions
	m_poLayoutApplications = Splitter_PoAddGroupBoxAndLayoutVertical_VE("Double-click on the aplication you would like to use with your profile '$S'", &m_pProfile->m_strNameProfile);

	m_pwEditSearchApplications = new WEdit;
	//pwEdit->setMaximumHeight(20);
	m_pwEditSearchApplications->Edit_SetWatermark("Search Application");
	m_poLayoutApplications->addWidget(m_pwEditSearchApplications);
	m_pwEditSearchApplications->setFocus();
	connect(m_pwEditSearchApplications, SIGNAL(textEdited(QString)), this, SLOT(SL_SearchTextChanged(QString)));

	m_pwTreeViewApplications = new QTreeWidget(this);
	m_poLayoutApplications->addWidget(m_pwTreeViewApplications, 100);
	m_pwTreeViewApplications->setColumnCount(2);
	QHeaderView * pwHeader = m_pwTreeViewApplications->header();
	pwHeader->hide();
	pwHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	pwHeader->setSectionResizeMode(1, QHeaderView::Stretch);
	connect(m_pwTreeViewApplications, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(SL_ActionDoubleClicked(QTreeWidgetItem*,int)));

//	PAddOption(eMenuIconXmpp, "Create an XMPP account", "Associate an additional XMPP account with my profile");
	QTreeWidgetItem * pContacts = PAddOption(eMenuIconXmpp, "Import Contacts", "");
	PAddOption(eMenuIconFacebook, "Import Facebook contacts", "Communicate securely with my Facebook contacts using Cambrian", pContacts);
	PAddOption(eMenuIconGoogleHangout, "Import Google+ contacts", "Communicate securely with my Google contacts using Cambrian", pContacts);
	PAddOption(eMenuIconBitcoin, "Create a Bitcoin wallet", "Use Cambrian to send and receive Bitcoin via Blockchain.info");
//	PAddOption(eMenuIconCorporations, "Incorporate a Company", "Register a business. Use Cambrian to issue dividends and communicate with directors and shareholders to vote");

	QTreeWidgetItem * pRegistry = PAddOption(eMenuIconCorporations, "Cambrian Registry", "");
			PAddOption(eMenuIconCorporations, "Incorporate", "Incorporate a Cambrian company", pRegistry);
//			PAddOption(eMenuIconMarketplaceMediation, "Mediator", "Register as a mediator", pRegistry);
			PAddOption(eMenuIconMarketplaceArbitration, "Arbitrate", "Register as an arbitrator", pRegistry);

	/*
	QTreeWidgetItem * pBank = PAddOption(eMenuIconBank, "Open a bank account", "You may open a personal or business bank account using Cambrian");
		PAddOption(eMenuIconBank, "CityBank", "Open an account at CityBank", pBank);
		PAddOption(eMenuIconBank, "Wells Fargo", "Open an account at Wells Fargo", pBank);
	*/

	QTreeWidgetItem * pTrade = PAddOption(eMenuIconExchange, "Exchange", "Buy and sell products at an exchange");
		PAddOption(eMenuIconCoffeeExchange, "MayanX", "Buy and sell coffee on the MayanX Exchange", pTrade);
		QTreeWidgetItem * pBTC = PAddOption(eMenuIconBitcoin, "Bitcoin", "Buy and sell Bitcoin at an exchange", pTrade);
			PAddOption(eMenuIconBitstamp, "Bitstamp", "Open an account at Bitstamp.net", pBTC);
			PAddOption(eMenuIconBitcoin, "CaVirtex", "Open an account at CaVirtex.com", pBTC);
			PAddOption(eMenuIconBitcoin, "Kraken", "Open an account at Kraken.com", pBTC);
//			PAddOption(eMenuIconCoinbase, "Coinbase", "Open an account at Coinbase.com", pBTC);
//		PAddOption(eMenuIconBitcoin, "Crypto Trust Point", "Join a Bitcoin decentralized P2P exchange with an ATM debit card", pTrade);
/*
	QTreeWidgetItem * pOT = PAddOption(eMenuIconMarketplace, "Open Transaction", "Join an Open Transaction server to trade");
		PAddOption(eMenuIconIssueAsset, "Issue Asset", "Issue my own assets on an OT server", pOT);
		PAddOption(eMenuIconIssueFuture, "Issue Futures", "Create my own future contracts on an OT server", pOT);
		PAddOption(eMenuIconIssueCurrency, "Issue Currency", "Create my own currency on an OT server", pOT);

		QTreeWidgetItem * pOtMarkets = PAddOption(eMenuIconMarket, "OT Markets Dashboard", "", pOT);
			PAddOption(eMenuIconMarket, "OT://1  OrlinUSD <-> BTC", "", pOtMarkets);
			PAddOption(eMenuIconMarket, "OT://1  Monetas Shares <-> BTC", "", pOtMarkets);
			PAddOption(eMenuIconMarket, "OT://2  Cambrian Shares <-> OrlinUSD", "", pOtMarkets);
		QTreeWidgetItem * pOtRipple = PAddOption(eMenuIconRipple, "OT Ripple Dashboard", "", pOT);
			PAddOption(eMenuIconBroadcast, "Broadcast Buy/Sell Offer", "Broadcast my offer(s) through the OT network", pOtRipple);
			PAddOption(eMenuIconListen, "Listen for Buy/Sell Offers", "Listen to all offers within the OT network", pOtRipple);
*/
	QTreeWidgetItem * pMarketplace = PAddOption(eMenuIconMarketplace, "Marketplace", "");
	QTreeWidgetItem * pLocalAssets = PAddOption(eMenuIconIssueAsset, "Local", "", pMarketplace);
		PAddOption(eMenuIconBroadcast, "Place Ad", "Place an advertisement to a server", pLocalAssets);
		PAddOption(eMenuAction_FindText, "Search City", "Search for an advertisement within my city", pLocalAssets);


//	PAddOption(eMenuIconJobs, "Jobs Offers", "Search available jobs and/or place a job offer", pMarketplace);
	//PAddOption(eMenuIconSell, "Earn Money", "Earn money by answering questions bearing a monetary reward");
		PAddOption(eMenuIconSell, "For Sale", "Search products available for sale and/or sell my product(s) online", pMarketplace);
		QTreeWidgetItem * pServices = PAddOption(eMenuIconServices, "Services", "Available professional services", pMarketplace);
			PAddOption(eMenuIconHealthcare, "Healthcare", "Find a healthcare provider or a health insurance company", pServices);
			PAddOption(eMenuIconSecurityGuard, "Security", "Hire private security", pServices);
//			PAddOption(eMenuIconMarketplaceLawyers, "Lawyers", "Hire a lawyer", pServices);
//			PAddOption(eMenuIconMarketplaceMediation, "Mediators", "Hire a mediator", pServices);
			PAddOption(eMenuIconMarketplaceArbitration, "Arbitrators", "Hire an arbitrator", pServices);

	QTreeWidgetItem * pJ = PAddOption(eMenuIconJurisdiction, "Jurisdictions", "");
	PAddOption(eMenuIconJurisdiction, "Join a Jurisdiction", "Join a group of people sharing similar values as you do", pJ);
	PAddOption(eMenuIconJurisdiction, "Start a new Jurisdiction", "", pJ);
//		PAddOption(eMenuIconMarketplaceMediation, "Register as a Mediator", "", pJ);
//		PAddOption(eMenuIconMarketplaceArbitration, "Register as an Arbitrator", "", pJ);
#if 0
//		PAddOption(eMenuIconJurisdiction, "Seasteading", "", pJ);
		QTreeWidgetItem * pS = PAddOption(eMenuIconJurisdiction, "Startup Cities", "", pJ);
			PAddOption(eMenuIconJurisdiction, "Elevator", "", pS);
		PAddOption(eMenuIconJurisdiction, "Intentional Communities", "", pJ);
		PAddOption(eMenuIconJurisdiction, "Interest Based", "", pJ);
		PAddOption(eMenuIconJurisdiction, "Personality Based", "", pJ);
		/*
		QTreeWidgetItem * pCountries = PAddOption(eMenuIconJurisdiction, "Traditional Countries", "", pJ);
			PAddOption(eMenuIconJurisdiction, "Traditional Cities", "", pCountries);
		*/
#endif

	m_pwTreeViewApplications->expandAll();
	}

QTreeWidgetItem *
WLayoutProfile::PAddOption(EMenuAction eMenuIcon, PSZAC pszName, PSZAC pszDescription, QTreeWidgetItem * pParent)
	{
	QTreeWidgetItem * poTreeItem = new QTreeWidgetItem;
	poTreeItem->setIcon(0, PGetMenuAction(eMenuIcon)->icon());
	poTreeItem->setText(0, pszName);
	poTreeItem->setText(1, pszDescription);
	if (pParent != NULL)
		pParent->addChild(poTreeItem);
	else
		m_pwTreeViewApplications->addTopLevelItem(poTreeItem);
	return poTreeItem;
	}

