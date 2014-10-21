//	WToolbar.cpp
//
//	Code to draw the main toolbar.  To add or remove actions, visit WToolbarActions.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WToolbar.h"
#include "WToolbarActions.h"

#define d_Toolbar_coText							0x6C6C6C // Color to draw the text on the toolbar.  At the moment there is only one color, however in the future there may be more for selected items and/or disabled items

#define d_ToolbarTabs_coBorderBottom				0xA9A9A9
#define d_ToolbarTabs_coGradientTop					0xD4D4D4
#define d_ToolbarTabs_coGradientBottom				0xDEDEDE

#define d_ToolbarTabs_Tab_cyHeightTop				2
#define d_ToolbarTabs_Tab_cxOverlap					10	// How many pixels to overlap the tabs
#define d_ToolbarTabs_Tab_cxWidthGradient			15	// How many pixels we need to adjust to draw the gradient (the edges are icons of 16 pixels, and we draw the gradient one pixel more)
#define d_ToolbarTabs_Tab_coBorder					d_ToolbarTabs_coBorderBottom
#define d_ToolbarTabs_Tab_coGradientTop				0xD2D2D2
#define d_ToolbarTabs_Tab_coGradientBottom			0xBEBEBE
#define d_ToolbarTabs_Tab_coGradientTopSelected		0xF6F6F6
#define d_ToolbarTabs_Tab_coGradientBottomSelected	0xE5E5E5

#define d_ToolbarButtons_coBorderTop
#define d_ToolbarButtons_coBorderBottom			0xAAAAAA
#define d_ToolbarButtons_coGradientTop			0xE6E6E6
#define d_ToolbarButtons_coGradientBottom		0xDADADA

#define d_Tab_coSeparator			d_ToolbarButtons_coBorderBottom	// To draw the separators (dividers between the toolbar buttons), use the same color as the bottom of the toolbar
#define d_Tab_cxWidthDefault		160
#define d_Tab_cxWidthIconClose		10		// Allow 10 pixels to draw the X icon on the right of the text
#define d_Tab_cxWidthIconNewTab		30		// Special icon to create a new tab

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
	m_uFlagsHitTest = eHitTest_zNone;	// So far, nothing living under the mouse
	setMouseTracking(true);
	}

WTabs::~WTabs()
	{
	m_arraypaTabs.DeleteAllTabs();
	}

CTab *
WTabs::TabAddP(PSZAC pszName, PVPARAM pvParam)
	{
	CTab * paTab = new CTab((PSZUC)pszName, pvParam);
	if (m_pTabSelected == NULL)
		m_pTabSelected = paTab;
	m_arraypaTabs.Add(PA_CHILD paTab);
	updateGeometry();
	_Redraw();
	return paTab;
	}

void
WTabs::TabAddAndSelect(PSZAC pszName, PVPARAM pvParam)
	{
	m_pTabSelected = TabAddP(pszName, pvParam);
	}

void
WTabs::TabSelect(PVPARAM pvParamTabToSelect)
	{
	_SetSelectedTab(_PFindTabByParam(pvParamTabToSelect));
	}

//	Private method to delete a tab
void
WTabs::TabDelete(PA_DELETING CTab * paTab)
	{
	if (paTab == NULL)
		return;
	if (paTab == m_pTabSelected)
		m_pTabSelected = NULL;
	m_arraypaTabs.RemoveElementAssertI(paTab);
	delete paTab;
	_Redraw();
	}

CTab *
WTabs::_PFindTabByParam(PVPARAM pvParamTab) const
	{
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		if (pTab->m_pvParam == pvParamTab)
			return pTab;
		}
	return NULL;
	}
//	Method to use to remove a tab.
//	This method will remove the first tab matching pvParam of TabAdd()
void
WTabs::TabRemove(PVPARAM pvParamTabToRemove)
	{
	TabDelete(_PFindTabByParam(pvParamTabToRemove));
	}

void
WTabs::TabsRemmoveAll()
	{
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		delete *ppTab++;
		}
	m_arraypaTabs.Empty();
	m_pTabSelected = NULL;
	_Redraw();
	}

QSize
WTabs::sizeHint() const
	{
	int cxWidth = d_Tab_cxWidthIconNewTab;
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		cxWidth += pTab->m_cxWidth;
		}
	//MessageLog_AppendTextFormatCo(d_coRed, "WTabs::sizeHint() returns a width of $I\n", cxWidth);
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

	oPainter.m_rcCell.setTop(d_ToolbarTabs_Tab_cyHeightTop);
	oPainter.m_rcCell.setLeft(xTab + d_ToolbarTabs_Tab_cxOverlap);
	oPainter.m_rcCell.setWidth(d_Tab_cxWidthIconNewTab);
	oPainter.DrawIconAlignment(
		((m_uFlagsHitTest & eHitTest_kmButtonNewTabHovering) == eHitTest_kmButtonNewTabHovering) ?
		eMenuIcon_ToolbarTab_NewHover :
		eMenuIcon_ToolbarTab_New, Qt::AlignLeft | Qt::AlignVCenter);
	if (m_pTabSelected != NULL)
		{
		// Always draw the selected tab last, so it appears on top of the others
		oPainter.m_rcCell = rcCellTabSelected;
		oPainter.m_rcCell.adjust(0, -1, 0, -1);	// Draw the selected tab just one pixel above
		_DrawTab(&oPainter, m_pTabSelected);
		}
	} // paintEvent()

void
WTabs::mouseMoveEvent(QMouseEvent * pEventMouse)
	{
	EHitTest eHitTest;
	CTab * pTab = _PGetHitTestInfo(pEventMouse, OUT &eHitTest);
	_SetFlagsHitTest(eHitTest | eHitTest_kfMouseHovering);
	if (m_pTabHover != pTab)
		{
		m_pTabHover = pTab;
		_Redraw();
		}
	}

void
WTabs::mouseReleaseEvent(QMouseEvent * pEventMouse)
	{
	EHitTest eHitTest;
	CTab * pTab = _PGetHitTestInfo(pEventMouse, OUT &eHitTest);
	_SetFlagsHitTest(eHitTest);
	if (eHitTest & eHitTest_kfButtonNewTab)
		{
		OnTabNew();
		return;
		}
	if (eHitTest & eHitTest_kfButtonClose)
		{
		Assert(pTab != NULL);
		OnTabClosing(pTab->m_pvParam);
		return;
		}
	if (m_pTabSelected != pTab)
		{
		m_pTabSelected = pTab;
		_Redraw();
		if (pTab != NULL)
			OnTabSelected(pTab->m_pvParam);
		}
	}

void
WTabs::leaveEvent(QEvent *)
	{
	_SetFlagsHitTest(eHitTest_zNone);
	}

void
WTabs::_SetSelectedTab(CTab * pTab)
	{
	Endorse(pTab == NULL);	// Don't select anything
	if (m_pTabSelected != pTab)
		{
		m_pTabSelected = pTab;
		_Redraw();
		}
	}


void
WTabs::_SetFlagsHitTest(UINT uFlagsHitTest)
	{
	if (m_uFlagsHitTest == uFlagsHitTest)
		return;
	m_uFlagsHitTest = uFlagsHitTest;
	_Redraw();
	}

CTab *
WTabs::_PGetHitTestInfo(QMouseEvent * pEventMouse, OUT EHitTest * peHitTest) const
	{
	QPoint ptMouse = pEventMouse->pos();
	return _PGetHitTestInfo(ptMouse.x(), ptMouse.y(), OUT peHitTest);
	}

CTab *
WTabs::_PGetHitTestInfo(int xPos, int yPos, OUT EHitTest * peHitTest) const
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "_UpdateHitTest($i)\n", xPos);
	Assert(peHitTest != NULL);
	*peHitTest = eHitTest_zNone;
	int xTabRight = 0;
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		xTabRight += pTab->m_cxWidth - d_ToolbarTabs_Tab_cxOverlap;
		if (xPos <= xTabRight)
			{
			int xPosGradientRight = xTabRight - d_ToolbarTabs_Tab_cxWidthGradient + d_ToolbarTabs_Tab_cxOverlap;
			if ((xPos <= xPosGradientRight) &&
				(xPos >= xPosGradientRight - d_Tab_cxWidthIconClose) &&
				(yPos >= (d_cyHeightToolbarTabs - d_Tab_cxWidthIconClose) / 2) &&	// Make sure the y-coordinate is within the 'box' to draw the close idon
				(yPos <= (d_cyHeightToolbarTabs + d_Tab_cxWidthIconClose) / 2))
				*peHitTest = eHitTest_kfButtonClose;
			return pTab;
			}
		} // while
	xTabRight += d_Tab_cxWidthIconNewTab;
	if (xPos < xTabRight)
		*peHitTest = eHitTest_kfButtonNewTab;
	return NULL;
	}


void
WTabs::_DrawTab(CPainterCell * pPainter, CTab * pTab)
	{
	Assert(pPainter != NULL);
	Assert(pTab != NULL);
	BOOL fuIsTabSelected = (pTab == m_pTabSelected);

	QRect rcGradient = pPainter->m_rcCell;
	rcGradient.adjust(+d_ToolbarTabs_Tab_cxWidthGradient, d_ToolbarTabs_Tab_cyHeightTop, -d_ToolbarTabs_Tab_cxWidthGradient, +3);	// Remove 15 pixels from each side to draw the gradient (this is because we are already drawing the icons on both edges)
	pPainter->FillRectWithGradientVertical(rcGradient, fuIsTabSelected ? d_ToolbarTabs_Tab_coGradientTopSelected : d_ToolbarTabs_Tab_coGradientTop, fuIsTabSelected ? d_ToolbarTabs_Tab_coGradientBottomSelected : d_ToolbarTabs_Tab_coGradientBottom);
	pPainter->DrawIconAlignmentLeftBottom(fuIsTabSelected ? eMenuIcon_ToolbarTab_EdgeLeftSelected : eMenuIcon_ToolbarTab_EdgeLeft);
	pPainter->DrawIconAlignmentRightBottom(fuIsTabSelected ? eMenuIcon_ToolbarTab_EdgeRightSelected : eMenuIcon_ToolbarTab_EdgeRight);
	/*
	pPainter->DrawLineHorizontal(pPainter->m_rcCell.left(), pPainter->m_rcCell.right(), d_cyHeightToolbarTabs - 1); //d_ToolbarpPainter->m_rcCell.bottom());
	if (fuIsTabSelected)
		{
		pPainter->setPen(d_ToolbarButtons_coGradientTop);
		pPainter->DrawLineHorizontal(pPainter->m_rcCell.left() + 1, pPainter->m_rcCell.right() - 1, d_cyHeightToolbarTabs - 1); // pPainter->m_rcCell.bottom());
		}
	*/
	QRect rcText = rcGradient;
	OGetIcon(eMenuIcon_ToolbarTab_Close).paint(pPainter, rcText, Qt::AlignRight | Qt::AlignVCenter, (pTab == m_pTabHover) && (m_uFlagsHitTest & eHitTest_kfButtonClose) ? QIcon::Normal : QIcon::Disabled);
	rcText.adjust(0, -2, -d_Tab_cxWidthIconClose, 0);	// Allow 10 pixels for the close icon
	pPainter->setPen(d_Toolbar_coText);
	pPainter->drawText(IN rcText, Qt::AlignCenter, pTab->m_strName);
	pPainter->DrawLineHorizontalCo(rcGradient.left(), rcGradient.right(), d_ToolbarTabs_Tab_cyHeightTop - fuIsTabSelected, d_ToolbarTabs_Tab_coBorder);
	pPainter->DrawLineHorizontalCo(pPainter->m_rcCell.left() + 1, pPainter->m_rcCell.right() - 1, d_cyHeightToolbarTabs - 1, fuIsTabSelected ? d_ToolbarButtons_coGradientTop : d_ToolbarTabs_Tab_coBorder);
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

	Assert(g_pwTabs == NULL);
	g_pwTabs = new WTabs;
	poLayout->addWidget(g_pwTabs);

//	poLayout->addWidget(new QLabel("this is a label"));
	}

void
WToolbarTabs::paintEvent(QPaintEvent *)
	{
	CPainter oPainter(this);
	QRect rc = rect();
	oPainter.FillRectWithGradientVertical(rc, d_ToolbarTabs_coGradientTop, d_ToolbarTabs_coGradientBottom);
	oPainter.DrawLineHorizontalCo(0, rc.right(), rc.bottom(), d_ToolbarTabs_coBorderBottom);
	}


void
WToolbarButtons::paintEvent(QPaintEvent *)
	{
	CPainter oPainter(this);
	QRect rc = rect();
	oPainter.FillRectWithGradientVertical(rc, d_ToolbarButtons_coGradientTop, d_ToolbarButtons_coGradientBottom);
	oPainter.DrawLineHorizontalCo(0, rc.right(), rc.bottom(), d_ToolbarButtons_coBorderBottom);
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
	setFixedHeight(d_cyHeightToolbarTabs + d_cyHeightToolbarButtons);
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

