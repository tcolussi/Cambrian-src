//	WToolbarActions.cpp
//
//	Code to handle events (actions) on the toolbar.
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WToolbarActions.h"
#include "WToolbar.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WTabs::OnTabNew()
	{
	QWidget * pawParam = new QWidget;	// This will cause a memory leak, but it is for the demo to test the pvParam
	#if 0
	TabAddAndSelect("New Tab", pawParam);
	#else
	TabAddP("New Tab", pawParam);
	TabSelect(pawParam);
	#endif
	}

void
WTabs::OnTabSelected(PVPARAM pvParamTabSelected)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "OnTabSelected(0x$p)\n", pvParamTabSelected);
	}

void
WTabs::OnTabClosing(PVPARAM pvParamTabClosing)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "OnTabClosing(0x$p)\n", pvParamTabClosing);
	TabRemove(pvParamTabClosing);
	}



///////////////////////////////////////////////////////////////////////////////////////////////////
WButtonIconForToolbarWithDropDownMenu * g_pwButtonToolbarSwitchProfile;
WMenu * g_pwMenuToolbarSwitchProfile;


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
	poLayout->setSpacing(10);
	/*WButtonToolbarIcon * pwButtonBack =*/ new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_GoBack, ":/images/ui/prevPageHoveredIcon.png");
	/*WButtonToolbarIcon * pwButtonForward =*/ new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_GoForward, ":/images/ui/nextPageHoveredIcon.png");
	/*WButtonToolbarIcon * pwButtonReload =*/ new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_Reload, ":/images/ui/refreshPageHoveredIcon.png");
	/*WEdit * pwEditSearch =*/ new WEditSearch(poLayout, eMenuIcon_Toolbar_Find, "Search apps, websites, files and chats...");
	poLayout->addWidget(new WButtonToolbarSeparator);

	g_pwButtonToolbarSwitchProfile = new WButtonIconForToolbarWithDropDownMenu(poLayout, eMenuIcon_ClassProfile, NULL);
	g_pwMenuToolbarSwitchProfile = g_pwButtonToolbarSwitchProfile->PwGetMenu();
	connect(g_pwMenuToolbarSwitchProfile, SIGNAL(aboutToShow()), this, SLOT(SL_MenuProfilesShow()));
	connect(g_pwMenuToolbarSwitchProfile, SIGNAL(triggered(QAction*)), this, SLOT(SL_MenuProfileSelected(QAction*)));

	QLabel * pwLabelPicture = new QLabel;
	pwLabelPicture->setPixmap(QPixmap(":/images/ui/testThumbImage.png"));
	poLayout->addWidget(pwLabelPicture);
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
WToolbarButtons::SL_MenuProfilesShow()
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
WToolbarButtons::SL_MenuProfileSelected(QAction * pAction)
	{
	const int iProfile = pAction->data().toInt();
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile((TProfile *)g_oConfiguration.m_arraypaProfiles.PvGetElementAtSafe_YZ(iProfile));
	}


