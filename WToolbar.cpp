#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WToolbar.h"
#include "WNavigationTree.h"

WButtonIconForToolbarWithDropDownMenu * g_pwButtonToolbarSwitchProfile;
WMenu * g_pwMenuToolbarSwitchProfile;

//#define TOOLBAR_DISPLAY_TABS		// At the moment, the tabs are not working

#define d_cyHeightToolbarTabs		25
#define d_cyHeightToolbarButtons	41

#define d_coToolbarBorderTop		0xA9A9A9
#define d_coToolbarBorderBottom		0xAAAAAA
#define d_coToolbarGradientTop
#define d_coToolbarGradientMiddle	0xE6E6E6
#define d_coToolbarGradientBottom	0xDADADA

WToolbarTabs::WToolbarTabs()
	{
	setFixedHeight(d_cyHeightToolbarTabs);
	#if 0
	setStyleSheet("background-color: yellow");
	#else
	setStyleSheet(
		"background: qlineargradient("
			"x1:0, y1:0, x2:0, y2:1,"
			"stop:0 #D4D4D4,"
			"stop:1 #DEDEDE"
			");"
		""
		);
	#endif
	//setSizePolicy(QSizePolicy::QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	OLayoutHorizontalAlignLeft * poLayout = new OLayoutHorizontalAlignLeft0(this);
	//WButtonIconForToolbar * pwButtonBack = new WButtonIconForToolbar(poLayout, eMenuIconGoBack);
	QWidget * pwTest = new QWidget;
	poLayout->addWidget(pwTest, 1);	// Add a widget to stretch the whole width
	}

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
	//setContentsMargins(0, 0, 0, 0);
	//setContentsMargins(-5, -5, -5, -5);
//	setContentsMargins(10, 10, 10, 10);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(this);
	WButtonIconForToolbar * pwButtonBack = new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_GoBack);
	WButtonIconForToolbar * pwButtonForward = new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_GoForward);
	WButtonIconForToolbar * pwButtonReload = new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_Reload);
	WEdit * pwEditSearch = new WEditSearch(poLayout, eMenuIcon_Toolbar_Find, "Search apps, websites, files and chats...");
	poLayout->addWidget(new WButtonToolbarSeparator);

	g_pwButtonToolbarSwitchProfile = new WButtonIconForToolbarWithDropDownMenu(poLayout, eMenuIcon_ClassProfile, NULL);
	g_pwMenuToolbarSwitchProfile = g_pwButtonToolbarSwitchProfile->PwGetMenu();
	connect(g_pwMenuToolbarSwitchProfile, SIGNAL(aboutToShow()), this, SLOT(SL_MenuProfilesShow()));
	connect(g_pwMenuToolbarSwitchProfile, SIGNAL(triggered(QAction*)), this, SLOT(SL_MenuProfileSelected(QAction*)));

	/*
	QComboBox * pwCombo = new QComboBox;
	pwCombo->addItem(OGetIcon(eMenuIcon_ClassProfile), "Switch Role");
	poLayout->addWidget(pwCombo);
	poLayout->addWidget(new QLabel("The Real Plato"));
	*/
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
	new WButtonIconForToolbar(poLayout, eMenuIcon_Toolbar_Notifications);

	WButtonIconForToolbarWithDropDownMenu * pwButtonMenuOverflow = new WButtonIconForToolbarWithDropDownMenuNoArrow(poLayout, eMenuIcon_Toolbar_Overflow);
	pwButtonMenuOverflow->setFixedWidth(8);	// The overflow menu is smaller
	WMenu * pawMenu = (WMenu *) new WMenuWithIndicator(pwButtonMenuOverflow);
	pawMenu->ActionsAdd(c_rgzeActionsMenuOverflow);
	WMenu * pawMenuWiki = pawMenu->PMenuAdd("Developer Apps", eMenuAction_WikiSubMenu);
	pawMenuWiki->ActionsAdd(c_rgzeActionsMenuWiki);
	pawMenu->ActionsAdd(c_rgzeActionsMenuQuit);

	pwButtonMenuOverflow->setMenu(pawMenu);
	pawMenu->_ConnectActionsToMainWindow();
	}

void
WToolbarButtons::paintEvent(QPaintEvent *)
	{
	OPainter oPainter(this);
	QRect rc = rect();
	oPainter.FillRectWithGradientVertical(rc, d_coToolbarGradientMiddle, d_coToolbarGradientBottom);
	oPainter.setPen(d_coToolbarBorderBottom);
	oPainter.DrawLineHorizontal(0, rc.width(), rc.bottom());
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
		g_pwMenuToolbarSwitchProfile->ActionAddFromText(pProfile->m_strNameProfile, iProfile, eMenuIconProfile);
		}
	}

void
WToolbarButtons::SL_MenuProfileSelected(QAction * pAction)
	{
	const int iProfile = pAction->data().toInt();
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile((TProfile *)g_oConfiguration.m_arraypaProfiles.PvGetElementAtSafe_YZ(iProfile));
	}

void
WButtonToolbarSeparator::paintEvent(QPaintEvent *)
	{
	OPainter oPainter(this);
	oPainter.setPen(d_coToolbarBorderBottom);
	QRect rc = rect();
	oPainter.DrawLineVertical(c_cxWidth / 2, 0, rc.height());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WToolbar::WToolbar() : QToolBar("Toolbar")
	{
	setObjectName("Toolbar3");
	setFixedHeight(d_cyHeightToolbarButtons);
	setMovable(false);		// Hide the gripper (handle to move the toolbar)
//	setStyleSheet("background-color: yellow");

	#if 1
	m_pwContent = new QWidget(this);	// Widget covering the whole toolbar area
//	m_pwContent->setFixedHeight(d_cyHeightToolbarButtons);
//	m_pwContent->setMinimumWidth(300);
//	m_pwContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//	m_pwContent->setStyleSheet("background-color: red");

	OLayoutVertical * poLayoutVertical = new OLayoutVerticalAlignTop0(m_pwContent);
	poLayoutVertical->addWidget(new WToolbarButtons);
	/*
	OLayoutHorizontal * poLayoutButtons = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	new WButtonIconForToolbar(poLayoutButtons, eMenuIcon_GoBack);
	new WButtonIconForToolbar(poLayoutButtons, eMenuIcon_GoForward);
	WEdit * pwEditSearch = new WEditSearch(poLayoutButtons, eMenuIcon_Toolbar_Find, "Search apps, websites, files and chats...");
	poLayoutButtons->addWidget(new WEdit, 1);
	poLayoutButtons->addWidget(new WButtonToolbarSeparator);
	*/

	//poLayoutVertical->addWidget(new WToolbarButtons);
	//setLayout(poLayoutVertical);
	#else
	OLayoutVertical * poLayoutVertical = new OLayoutVerticalAlignTop0(this);
	poLayoutVertical->addWidget(new WToolbarButtons);
	setLayout(poLayoutVertical);
	setMinimumWidth(1000);
	#endif


	/*
	//setContentsMargins(10, 10, 10, 10);
	setStyleSheet("background-color: yellow");


	QWidget * pwContent = new QWidget(this);	// Widget covering the whole toolbar area
	pwContent->setSizePolicy(QSizePolicy::QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

//	addWidget(PA_CHILD pwContent);
	pwContent->setStyleSheet("background-color: red");

	OLayoutVertical * poLayoutVertical = new OLayoutVerticalAlignTop0(this);
	pwContent->setLayout(poLayoutVertical);
	poLayoutVertical->addWidget(new WToolbarButtons);
	*/

//addWidget(new WToolbarButtons);
	//new WButtonIconForToolbar(poLayoutVertical, eMenuIcon_GoBack);

	/*
	OLayoutHorizontal * poLayoutVertical = new OLayoutHorizontal(this);
	poLayoutVertical->addWidget(new QLabel("Test"));
	//WButtonIconForToolbar(poLayoutVertical, eMenuIcon_GoBack);
	setLayout(poLayoutVertical);
	*/

	/*
	pwContent->setStyleSheet("background-color: red");
	OLayoutVertical * poLayoutVertical = new OLayoutVerticalAlignTop0(pwContent);
	WButtonIconForToolbar(poLayoutVertical, eMenuIcon_GoBack);
	pwContent->setLayout(poLayoutVertical);
	*/
	/*
	poLayoutVertical->addWidget(new WToolbarButtons);
	*/

	//addWidget(new WToolbarButtons);
	//addWidget(new QLabel("test"));
	//new WButtonIconForToolbar(this, eMenuIcon_Bitcoin);
	}

void
WToolbar::resizeEvent(QResizeEvent * pEventResize)
	{
	m_pwContent->resize(pEventResize->size());
	}

