#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WToolbar.h"
#include "WNavigationTree.h"

WButtonIconForToolbarWithDropDownMenu * g_pwButtonToolbarSwitchProfile;
WMenu * g_pwMenuToolbarSwitchProfile;

#define TOOLBAR_DISPLAY_TABS		// At the moment, the tabs are not working

//	The total height of the toolbar is the sum of the following value

#define d_cyHeightToolbarTabs		(25+2)			// Height of the widget containing the tabs
#define d_cyHeightToolbarButtons	(40+1)			// Height of the widget containing the toolbar buttons

#define d_Toolbar_coText						// Text color

#define d_ToolbarTabs_coBorderBottom				0xA9A9A9
#define d_ToolbarTabs_coGradientTop					0xD4D4D4
#define d_ToolbarTabs_coGradientBottom				0xDEDEDE

#define d_ToolbarTabs_Tab_cyHeightTop				2
#define d_ToolbarTabs_Tab_cxOverlap					10	// How many pixels to overlap the tabs
#define d_ToolbarTabs_Tab_coBorder					d_ToolbarTabs_coBorderBottom
#define d_ToolbarTabs_Tab_coGradientTop				0xD2D2D2
#define d_ToolbarTabs_Tab_coGradientBottom			0xBEBEBE
#define d_ToolbarTabs_Tab_coGradientTopSelected		0xF6F6F6
#define d_ToolbarTabs_Tab_coGradientBottomSelected	0xE5E5E5


#define d_ToolbarButtons_coBorderTop
#define d_ToolbarButtons_coBorderBottom			0xAAAAAA
#define d_ToolbarButtons_coGradientTop			0xE6E6E6
#define d_ToolbarButtons_coGradientBottom		0xDADADA

//#define d_coToolbarBorderTop		0xA9A9A9

#define d_Tab_coSeparator		d_ToolbarButtons_coBorderBottom	// To draw the separators (dividers between the toolbar buttons), use the same color as the bottom of the toolbar
#define d_Tab_cxWidthDefault	100
#define d_Tab_cxWidthIconNew	30

CTab::CTab(PSZUC pszName, PVPARAM pvParam)
	{
	m_strName = pszName;
	m_pvParam = pvParam;
	m_cxWidth = d_Tab_cxWidthDefault;
	}

void
CArrayPtrTabs::DeleteAllTabs()
	{
	CTab ** ppTabStop;
	CTab ** ppTab = PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		delete *ppTab++;
	Empty();
	}


WTabs::WTabs()
	{
	m_pTabSelected = NULL;
	m_pTabHover = NULL;
	m_eHitTest = eHitTest_zNone;
	setMouseTracking(true);
	}

WTabs::~WTabs()
	{
	m_arraypaTabs.DeleteAllTabs();
	}

void
WTabs::TabAdd(PSZAC pszName, PVPARAM pvParam)
	{
	CTab * paTab = new CTab((PSZUC)pszName, pvParam);
	if (m_pTabSelected == NULL)
		m_pTabSelected = paTab;
	m_arraypaTabs.Add(PA_CHILD paTab);
	_Redraw();
	}

void
WTabs::TabDelete(PA_DELETING CTab * paTab)
	{
	Assert(paTab != NULL);
	if (paTab == m_pTabSelected)
		m_pTabSelected = NULL;
	m_arraypaTabs.RemoveElementAssertI(paTab);
	delete paTab;
	_Redraw();
	}


QSize
WTabs::sizeHint() const
	{
	int cxWidth = d_Tab_cxWidthIconNew;
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		cxWidth += pTab->m_cxWidth;
		}
	MessageLog_AppendTextFormatCo(d_coRed, "WTabs::sizeHint() returns a width of $I\n", cxWidth);
	return QSize(cxWidth, d_zNA);
	}

void
WTabs::paintEvent(QPaintEvent *)
	{
	CPainterCell oPainter(this);
	QRect rcCellTabSelected;
	//oPainter.m_rcCell.setBottom(oPainter.m_rcCell.bottom() - 1);	// Don't draw the divider
	int xTab = 0;
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		oPainter.m_rcCell.setLeft(xTab);
		oPainter.m_rcCell.setWidth(pTab->m_cxWidth);
		if (pTab == m_pTabSelected)
			rcCellTabSelected = oPainter.m_rcCell;	// Make copy of the rectangle, so we can draw it later
		else
			_DrawTab(&oPainter, pTab);
		xTab += pTab->m_cxWidth - d_ToolbarTabs_Tab_cxOverlap;
		}

	oPainter.m_rcCell.setLeft(xTab + d_ToolbarTabs_Tab_cxOverlap);
	oPainter.m_rcCell.setWidth(d_Tab_cxWidthIconNew);
	oPainter.DrawIconAlignment((m_eHitTest == eHitTest_ButtonNewTab) ? eMenuIcon_ToolbarTab_NewHover : eMenuIcon_ToolbarTab_New, Qt::AlignLeft | Qt::AlignVCenter);
	if (m_pTabSelected != NULL)
		{
		// Always draw the selected tab last, so it appears on top of the others
		oPainter.m_rcCell = rcCellTabSelected;
		_DrawTab(&oPainter, m_pTabSelected);
		}
	} // paintEvent()

void
WTabs::mouseMoveEvent(QMouseEvent * pEventMouse)
	{
	CTab * pTab = _PUpdateHitTest(pEventMouse);
	if (m_pTabHover != pTab)
		{
		m_pTabHover = pTab;
		_Redraw();
		}
	}

void
WTabs::mouseReleaseEvent(QMouseEvent * pEventMouse)
	{
	CTab * pTab = _PUpdateHitTest(pEventMouse);
	if (m_pTabSelected != pTab)
		{
		m_pTabSelected = pTab;
		_Redraw();
		}
	if (m_eHitTest == eHitTest_ButtonNewTab)
		{
		TabAdd("New Tab");
		updateGeometry();
		}
	}

void
WTabs::leaveEvent(QEvent *)
	{
	if (m_eHitTest != eHitTest_zNone)
		{
		m_eHitTest = eHitTest_zNone;
		_Redraw();
		}
	}

CTab *
WTabs::_PUpdateHitTest(QMouseEvent * pEventMouse)
	{
	return _PUpdateHitTest(pEventMouse->pos().x());
	}

CTab *
WTabs::_PUpdateHitTest(int xPos)
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "_UpdateHitTest($i)\n", xPos);
	CTab * pTabHitTest = NULL;
	EHitTest eHitTestOld = m_eHitTest;
	m_eHitTest = eHitTest_zNone;
	int xTabRight = 0;
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		xTabRight += pTab->m_cxWidth - d_ToolbarTabs_Tab_cxOverlap;
		if (xPos <= xTabRight)
			{
			pTabHitTest = pTab;
			goto Done;
			}
		} // while
	xTabRight += d_Tab_cxWidthIconNew;
	if (xPos < xTabRight)
		m_eHitTest = eHitTest_ButtonNewTab;
	Done:
	if (m_eHitTest != eHitTestOld)
		_Redraw();
	return pTabHitTest;
	}


void
WTabs::_DrawTab(CPainterCell * pPainter, CTab * pTab)
	{
	Assert(pPainter != NULL);
	Assert(pTab != NULL);
	BOOL fuIsTabSelected = (pTab == m_pTabSelected);

	QRect rcGradient = pPainter->m_rcCell;
	rcGradient.adjust(+15, d_ToolbarTabs_Tab_cyHeightTop, -15, +3);
	pPainter->FillRectWithGradientVertical(rcGradient, fuIsTabSelected ? d_ToolbarTabs_Tab_coGradientTopSelected : d_ToolbarTabs_Tab_coGradientTop, fuIsTabSelected ? d_ToolbarTabs_Tab_coGradientBottomSelected : d_ToolbarTabs_Tab_coGradientBottom);
	pPainter->DrawIconAlignmentLeftBottom(fuIsTabSelected ? eMenuIcon_ToolbarTab_EdgeLeftSelected : eMenuIcon_ToolbarTab_EdgeLeft);
	pPainter->DrawIconAlignmentRightBottom(fuIsTabSelected ? eMenuIcon_ToolbarTab_EdgeRightSelected : eMenuIcon_ToolbarTab_EdgeRight);
	pPainter->setPen(d_ToolbarTabs_Tab_coBorder);
	pPainter->DrawLineHorizontal(rcGradient.left(), rcGradient.right(), d_ToolbarTabs_Tab_cyHeightTop);
	pPainter->DrawLineHorizontal(pPainter->m_rcCell.left(), pPainter->m_rcCell.right(), pPainter->m_rcCell.bottom());

	pPainter->drawText(IN pPainter->m_rcCell, Qt::AlignCenter, pTab->m_strName);
	if (fuIsTabSelected)
		{
		pPainter->setPen(d_ToolbarButtons_coGradientTop);
		pPainter->DrawLineHorizontal(pPainter->m_rcCell.left() + 2, pPainter->m_rcCell.right() - 2, pPainter->m_rcCell.bottom());
		}
	}

WToolbarTabs::WToolbarTabs()
	{
	//setFixedHeight(d_cyHeightToolbarTabs);
	OLayoutHorizontalAlignLeft * poLayout = new OLayoutHorizontalAlignLeft0(this);
	WButtonIconForToolbarWithDropDownMenu * pwButtonJurisdiction = new WButtonIconForToolbarWithDropDownMenu(poLayout, eMenuIcon_Toolbar_Pantheon, "Pantheon");
	pwButtonJurisdiction->setFixedHeight(d_cyHeightToolbarTabs);	// This will set the height for the entire widget
	WMenu * pwMenuJurisdiction = pwButtonJurisdiction->PwGetMenu();
	pwMenuJurisdiction->ActionAddFromText((PSZUC)"Pantheon", eMenuIcon_Toolbar_Pantheon, eMenuIcon_Toolbar_Pantheon);
	pwMenuJurisdiction->ActionAddFromText((PSZUC)"Central Services", eMenuIcon_Toolbar_Pantheon, eMenuIcon_ClassJuristiction);

	WTabs * pwTabs = new WTabs;
	pwTabs->TabAdd("New Tab");
	pwTabs->TabAdd("New Tab");
	pwTabs->TabAdd("New Tab");
	poLayout->addWidget(pwTabs);

//	poLayout->addWidget(new QLabel("this is a label"));
	}

void
WToolbarTabs::paintEvent(QPaintEvent *)
	{
	CPainter oPainter(this);
	QRect rc = rect();
	oPainter.FillRectWithGradientVertical(rc, d_ToolbarTabs_coGradientTop, d_ToolbarTabs_coGradientBottom);
	oPainter.setPen(d_ToolbarTabs_coBorderBottom);
	oPainter.DrawLineHorizontal(0, rc.right(), rc.bottom());
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
	new WButtonToolbarIcon(poLayout, eMenuIcon_Toolbar_Notifications, ":/images/ui/refreshPageIcon.png");

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
WToolbarButtons::paintEvent(QPaintEvent *)
	{
	CPainter oPainter(this);
	QRect rc = rect();
	oPainter.FillRectWithGradientVertical(rc, d_ToolbarButtons_coGradientTop, d_ToolbarButtons_coGradientBottom);
	oPainter.setPen(d_ToolbarButtons_coBorderBottom);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
WButtonToolbarIcon::WButtonToolbarIcon(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon, PSZAC pszIconHover) : WButtonIconForToolbar(poParentLayout, eMenuIcon)
	{
	//setStyleSheet("margin-left: 20");
	if (pszIconHover != NULL)
		{
		QString sStylePrevious = styleSheet();
		g_strScratchBufferStatusBar.Format("$Q :hover { image: url($s); }", &sStylePrevious, pszIconHover);
		setStyleSheet(g_strScratchBufferStatusBar);
		}
	}

void
WButtonToolbarSeparator::paintEvent(QPaintEvent *)
	{
	CPainter oPainter(this);
	oPainter.setPen(d_Tab_coSeparator);
	QRect rc = rect();
	oPainter.DrawLineVertical(c_cxWidth / 2, 0, rc.height());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WToolbar::WToolbar() : QToolBar("Toolbar")
	{
	setObjectName("Toolbar3");
	#ifdef TOOLBAR_DISPLAY_TABS
	setFixedHeight(d_cyHeightToolbarTabs + d_cyHeightToolbarButtons);
	#else
	setFixedHeight(d_cyHeightToolbarButtons);
	#endif
	setMovable(false);		// Hide the gripper (handle to move the toolbar)
	#if 0
	setStyleSheet("background-color: yellow; color: gray;");
	#else
	setStyleSheet("font-size: 12px; color: #404040;");
	#endif

	m_pwContent = new QWidget(this);	// Widget covering the whole toolbar area
	OLayoutVertical * poLayoutVertical = new OLayoutVerticalAlignTop0(m_pwContent);
	poLayoutVertical->addWidget(new WToolbarTabs);
	poLayoutVertical->addWidget(new WToolbarButtons);
	}

void
WToolbar::resizeEvent(QResizeEvent * pEventResize)
	{
	m_pwContent->resize(pEventResize->size());
	}

