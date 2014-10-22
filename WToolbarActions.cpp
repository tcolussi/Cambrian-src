//	WToolbarActions.cpp
//
//	Code to handle events (actions) on the toolbar.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WToolbar.h"
#include "WToolbarActions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WTabs::OnTabNew()
	{
	const PSZAC c_szNamesProfiles[] = { "The Real Plato", "The Fake Plato", "NYC OTC Dealer", "Superman", "Spiderman" };
	TProfile * paProfile = new TProfile(&g_oConfiguration);	// This will cause a memory leak, but it is for the demo to test the pvParam
	paProfile->m_strNameProfile.InitFromStringA(c_szNamesProfiles[qrand() % LENGTH(c_szNamesProfiles)]);
	TabAddAndSelect(paProfile->m_strNameProfile, paProfile);
	}

void
WTabs::OnTabSelected(ITreeItem * pTreeItemSelected)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "OnTabSelected(0x$p)\n", pTreeItemSelected);
	if (pTreeItemSelected == NULL)
		return;
	RTI_ENUM rti = pTreeItemSelected->EGetRuntimeClass();
	if (rti == RTI(TProfile))
		{
		NavigationTree_SelectProfile((TProfile *)pTreeItemSelected);
		return;
		}
	ITreeItemChatLogEvents * pContactOrGroup = PGetRuntimeInterfaceOf_ITreeItemChatLogEvents(pTreeItemSelected);
	if (pContactOrGroup != NULL)
		{
		NavigationTree_SelectProfile(pContactOrGroup->PGetProfile());
		NavigationTree_SelectTreeItem(pContactOrGroup);
		}
	}

void
WTabs::OnTabClosing(ITreeItem * pTreeItemClosing)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "OnTabClosing(0x$p)\n", pTreeItemClosing);
	TabRemove(pTreeItemClosing);
	}

WTabs * g_pwTabs;

void
Toolbar_PopulateTabs()
	{
	if (g_pwTabs == NULL)
		return;	// Temporary: if the toolbar is not compiled
	Assert(g_pwTabs != NULL);
	g_pwTabs->TabsRemmoveAll();	// Remove any previous tab
	TProfile ** ppProfilesStop;
	TProfile ** ppProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfilesStop(OUT &ppProfilesStop);
	while (ppProfiles != ppProfilesStop)
		{
		TProfile * pProfile = *ppProfiles++;
		g_pwTabs->TabAddP(pProfile->m_strNameProfile, pProfile);
		}
	g_pwTabs->TabSelect(g_oConfiguration.m_pProfileSelected);
	}

void
Toolbar_TabAdd(ITreeItem * pTreeItem)
	{
	Assert(pTreeItem != NULL);
	if (g_pwTabs == NULL)
		return;	// Temporary: if the toolbar is not compiled
	g_pwTabs->TabAddUniqueAndSelect((PSZAC)pTreeItem->TreeItem_PszGetNameDisplay(), pTreeItem);
	}

void
Toolbar_TabSelect(ITreeItem * pTreeItem)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "Toolbar_TabSelect($s)\n", (pTreeItem != NULL) ? pTreeItem->TreeItem_PszGetNameDisplay() : NULL);
	if (g_pwTabs == NULL)
		return;	// Temporary: if the toolbar is not compiled
	Assert(g_pwTabs != NULL);
	g_pwTabs->TabSelect(pTreeItem);
	}

void
Toolbar_TabRedraw(ITreeItem * pTreeItem)
	{
	if (g_pwTabs == NULL)
		return;	// Temporary: if the toolbar is not compiled
	Assert(g_pwTabs != NULL);
	g_pwTabs->TabRepaint(pTreeItem);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WButtonIconForToolbarWithDropDownMenu * g_pwButtonToolbarSwitchProfile;
WMenu * g_pwMenuToolbarSwitchProfile;


WToolbarTabs::WToolbarTabs()
	{
	//setFixedHeight(d_cyHeightToolbarTabs);
	OLayoutHorizontalAlignLeft * poLayout = new OLayoutHorizontalAlignLeft0(this);

	g_pwButtonToolbarSwitchProfile = new WButtonIconForToolbarWithDropDownMenu(poLayout, eMenuIcon_ClassProfile, NULL);
	g_pwButtonToolbarSwitchProfile->setFixedHeight(d_cyHeightToolbarTabs);	// This will set the height for the entire widget
	g_pwMenuToolbarSwitchProfile = g_pwButtonToolbarSwitchProfile->PwGetMenu();
	connect(g_pwMenuToolbarSwitchProfile, SIGNAL(aboutToShow()), this, SLOT(SL_MenuProfilesShow()));
	connect(g_pwMenuToolbarSwitchProfile, SIGNAL(triggered(QAction*)), this, SLOT(SL_MenuProfileSelected(QAction*)));

	Assert(g_pwTabs == NULL);
	g_pwTabs = new WTabs;
	poLayout->addWidget(g_pwTabs);

	#if 0
	QWidget * pwButton = new WButtonText("abc");
	poLayout->addWidget(pwButton);
	QGraphicsDropShadowEffect * paShadow = new QGraphicsDropShadowEffect;
	paShadow->setOffset(2);
	paShadow->setBlurRadius(4);
	paShadow->setColor(QColor(63, 63, 63, 180));
	pwButton->setGraphicsEffect(paShadow);
	#endif
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
const EMenuActionByte c_rgzeActionsMenuOverflow[] =
	{
	eMenuAction_PlaySound,
	eMenuAction_FindText,
	eMenuActionSeparator,
	eMenuAction_ShowLogMessages,
	eMenuAction_ShowLogErrors,
	eMenuActionSeparator,
	eMenuAction_ConfigurationSaveAsXmlFile,
	eMenuAction_ConfigurationOpenFromXmlFile,
	eMenuActionSeparator,
	eMenuAction_DisplayProfileInfo,
	eMenuAction_DisplayCertificates,

	eMenuActionSeparator,
	//eMenuAction_WikiSubMenu, // added manually c_rgzeActionsMenuWiki[]
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuQuit[] =
	{
	eMenuActionSeparator,
	eMenuAction_Quit,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuWiki[] =
	{
	eMenuAction_WikiOfficeKingpin,
	eMenuAction_WikiPomodoro,
	eMenuAction_WikiJapiTests,
	eMenuAction_WikiScratch,
	eMenuAction_AppDefaultNewTab,
	eMenuAction_AppUnderconstruction,
	eMenuAction_AppConfiguration,
	ezMenuActionNone
	};


///////////////////////////////////////////////////////////////////////////////////////////////////
WToolbarButtons::WToolbarButtons()
	{
	setFixedHeight(d_cyHeightToolbarButtons);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(this);
	poLayout->setSpacing(15);
	/*WButtonToolbarIcon * pwButtonBack =*/ new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_GoBack, ":/images/ui/prevPageHoveredIcon.png");
	/*WButtonToolbarIcon * pwButtonForward =*/ new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_GoForward, ":/images/ui/nextPageHoveredIcon.png");
	/*WButtonToolbarIcon * pwButtonReload =*/ new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_Reload, ":/images/ui/refreshPageHoveredIcon.png");
	/*WEdit * pwEditSearch =*/ new WEditSearch(poLayout, eMenuIcon_Toolbar_Find, "Search apps, websites, files and chats...");
	poLayout->addWidget(new WButtonToolbarSeparator);

	WButtonIconForToolbarWithDropDownMenu * pwButtonJurisdiction = new WButtonIconForToolbarWithDropDownMenu(poLayout, eMenuIcon_Toolbar_Pantheon, "Pantheon");
	pwButtonJurisdiction->setFixedHeight(d_cyHeightToolbarTabs);	// This will set the height for the entire widget
	WMenu * pwMenuJurisdiction = pwButtonJurisdiction->PwGetMenu();
	pwMenuJurisdiction->ActionAddFromText((PSZUC)"Pantheon", eMenuIcon_Toolbar_Pantheon, eMenuIcon_Toolbar_Pantheon);
	pwMenuJurisdiction->ActionAddFromText((PSZUC)"Central Services", eMenuIcon_Toolbar_Pantheon, eMenuIcon_ClassJuristiction);

	#if 0
	QLabel * pwLabelPicture = new QLabel;
	pwLabelPicture->setPixmap(QPixmap(":/images/ui/testThumbImage.png"));
	poLayout->addWidget(pwLabelPicture);
	#endif
	poLayout->addWidget(new WButtonToolbarSeparator);
	poLayout->addWidget(new QLabel("Home"));
	poLayout->addWidget(new WButtonToolbarSeparator);
	new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_SocietyPro);
	new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_Grid);
	poLayout->addWidget(new WButtonToolbarSeparator);
	new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_Friends);
	new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_Messages);
	#if 0
	new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_Notifications, ":/images/ui/refreshPageIcon.png");
	#else
	new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_Notifications);
	#endif

	WButtonIconForToolbarWithDropDownMenu * pwButtonMenuOverflow = new WButtonIconForToolbarWithDropDownMenuNoArrow(poLayout, eMenuIcon_Toolbar_Overflow);
	pwButtonMenuOverflow->setFixedWidth(8);	// The overflow menu is smaller
	WMenu * pawMenu = (WMenu *) new WMenuWithIndicator(pwButtonMenuOverflow);
	pawMenu->ActionsAdd(c_rgzeActionsMenuOverflow);
	WMenu * pawMenuWiki = pawMenu->PMenuAdd("Developer Apps", eMenuIcon_Browser);
	pawMenuWiki->ActionsAdd(c_rgzeActionsMenuWiki);
	pawMenu->ActionsAdd(c_rgzeActionsMenuQuit);

	pwButtonMenuOverflow->setMenu(pawMenu);
	pawMenu->_ConnectActionsToMainWindow();
	}

void
WToolbarTabs::SL_MenuProfilesShow()
	{
	g_pwMenuToolbarSwitchProfile->clear();
	int cProfiles;
	TProfile ** prgpProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfiles(OUT &cProfiles);
	for (int iProfile = 0; iProfile < cProfiles; iProfile++)
		{
		TProfile * pProfile = prgpProfiles[iProfile];
		Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
		g_pwMenuToolbarSwitchProfile->ActionAddFromText(pProfile->m_strNameProfile, iProfile, eMenuIcon_ClassProfile);
		}
	g_pwMenuToolbarSwitchProfile->raise();
	g_pwMenuToolbarSwitchProfile->sizeHint();
	}

void
WToolbarTabs::SL_MenuProfileSelected(QAction * pAction)
	{
	const int iProfile = pAction->data().toInt();
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile((TProfile *)g_oConfiguration.m_arraypaProfiles.PvGetElementAtSafe_YZ(iProfile));
	}


