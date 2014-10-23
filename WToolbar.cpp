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

QPen g_oPenToolbar_Dot;
QFont g_oFontToolbar_Dot;

CTab::CTab(PSZUC pszName, ITreeItem * pTreeItem)
	{
	m_strName = pszName;
	m_pTreeItem = pTreeItem;
	if (pTreeItem != NULL)
		pTreeItem->m_uFlagsTreeItem |= ITreeItem::FTI_kfTreeItemInToolbarTabs;
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
	m_pdTabSelected = NULL;
	m_pdTabHover = NULL;
	m_uFlagsHitTest = eHitTest_zNone;	// So far, nothing living under the mouse
	setMouseTracking(true);
	g_oPenToolbar_Dot.setColor(0xEB4D5C);	// Orange color
	g_oPenToolbar_Dot.setWidth(14);
	g_oPenToolbar_Dot.setCapStyle(Qt::RoundCap);
	}

WTabs::~WTabs()
	{
	m_arraypaTabs.DeleteAllTabs();
	}

CTab *
WTabs::TabAddP(PSZAC pszName, ITreeItem * pTreeItem)
	{
	CTab * paTab = new CTab((PSZUC)pszName, pTreeItem);
	if (m_pdTabSelected == NULL)
		m_pdTabSelected = paTab;
	m_arraypaTabs.Add(PA_CHILD paTab);
	updateGeometry();
	_Redraw();
	return paTab;
	}

//	Add the tab only if not already present.
//	Return NULL of no tab was added
CTab *
WTabs::TabAddUniqueP(PSZAC pszName, ITreeItem * pTreeItem)
	{
	if (_PFindTabMatchingTreeItem(pTreeItem) == NULL)
		return TabAddP(pszName, pTreeItem);
	return NULL;
	}

void
WTabs::TabAddAndSelect(PSZAC pszName, ITreeItem * pTreeItem)
	{
	m_pdTabSelected = TabAddP(pszName, pTreeItem);
	}

void
WTabs::TabAddUniqueAndSelect(PSZAC pszName, ITreeItem * pTreeItem)
	{
	if (_PFindTabMatchingTreeItem(pTreeItem) == NULL)
		m_pdTabSelected = TabAddP(pszName, pTreeItem);
	}

void
WTabs::TabSelect(ITreeItem * pTreeItem)
	{
	_SetSelectedTab(_PFindTabMatchingTreeItem(pTreeItem));
	}

void
WTabs::TabRepaint(ITreeItem * pTreeItem)
	{
	if (_PFindTabMatchingTreeItem(pTreeItem) != NULL)
		_Redraw();	// This could be optimized to redraw only the portion of the tab rather than all tabs
	}

//	Private method to delete a tab
void
WTabs::TabDelete(PA_DELETING CTab * paTab)
	{
	if (paTab == NULL)
		return;
	if (paTab == m_pdTabSelected)
		m_pdTabSelected = NULL;
	m_arraypaTabs.RemoveElementAssertI(paTab);
	delete paTab;
	_Redraw();
	}

CTab *
WTabs::_PFindTabMatchingTreeItem(ITreeItem * pTreeItem) const
	{
	CTab ** ppTabStop;
	CTab ** ppTab = m_arraypaTabs.PrgpGetTabsStop(OUT &ppTabStop);
	while (ppTab != ppTabStop)
		{
		CTab * pTab = *ppTab++;
		if (pTab->m_pTreeItem == pTreeItem)
			return pTab;
		}
	return NULL;
	}

//	Method to use to remove a tab.
//	This method will remove the first tab matching pvParam of TabAdd()
void
WTabs::TabRemove(ITreeItem * pTreeItem)
	{
	TabDelete(_PFindTabMatchingTreeItem(pTreeItem));
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
	m_pdTabSelected = NULL;
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
	g_oFontToolbar_Dot = oPainter.font();
	g_oFontToolbar_Dot.setPixelSize(g_oFontToolbar_Dot.pixelSize() - 2);	// Make the font a bit smaller
	g_oFontToolbar_Dot.setBold(true);

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
		if (pTab == m_pdTabSelected)
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
	if (m_pdTabSelected != NULL)
		{
		// Always draw the selected tab last, so it appears on top of the others
		oPainter.m_rcCell = rcCellTabSelected;
		oPainter.m_rcCell.adjust(0, -1, 0, -1);	// Draw the selected tab just one pixel above
		_DrawTab(&oPainter, m_pdTabSelected);
		}
	} // paintEvent()

void
WTabs::mouseMoveEvent(QMouseEvent * pEventMouse)
	{
	EHitTest eHitTest;
	CTab * pTab = _PGetHitTestInfo(pEventMouse, OUT &eHitTest);
	_SetFlagsHitTest(eHitTest | eHitTest_kfMouseHovering);
	if (m_pdTabHover != pTab)
		{
		m_pdTabHover = pTab;
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
		OnTabClosing(pTab->m_pTreeItem);
		return;
		}
	if (m_pdTabSelected != pTab)
		{
		m_pdTabSelected = pTab;
		_Redraw();
		if (pTab != NULL)
			OnTabSelected(pTab->m_pTreeItem);
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
	if (m_pdTabSelected != pTab)
		{
		m_pdTabSelected = pTab;
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
	BOOL fuIsTabSelected = (pTab == m_pdTabSelected);

	QRect rcGradient = pPainter->m_rcCell;
	rcGradient.adjust(+d_ToolbarTabs_Tab_cxWidthGradient, d_ToolbarTabs_Tab_cyHeightTop, -d_ToolbarTabs_Tab_cxWidthGradient, +3);	// Remove 15 pixels from each side to draw the gradient (this is because we are already drawing the icons on both edges)
	pPainter->FillRectWithGradientVertical(rcGradient, fuIsTabSelected ? d_ToolbarTabs_Tab_coGradientTopSelected : d_ToolbarTabs_Tab_coGradientTop, fuIsTabSelected ? d_ToolbarTabs_Tab_coGradientBottomSelected : d_ToolbarTabs_Tab_coGradientBottom);
	pPainter->DrawIconAlignmentLeftBottom(fuIsTabSelected ? eMenuIcon_ToolbarTab_EdgeLeftSelected : eMenuIcon_ToolbarTab_EdgeLeft);
	pPainter->DrawIconAlignmentRightBottom(fuIsTabSelected ? eMenuIcon_ToolbarTab_EdgeRightSelected : eMenuIcon_ToolbarTab_EdgeRight);
	QRect rcText = rcGradient;
	OGetIcon(eMenuIcon_ToolbarTab_Close).paint(pPainter, rcText, Qt::AlignRight | Qt::AlignVCenter, (pTab == m_pdTabHover) && (m_uFlagsHitTest & eHitTest_kfButtonClose) ? QIcon::Normal : QIcon::Disabled);
	rcText.adjust(0, -2, -d_Tab_cxWidthIconClose, 0);	// Adjust the rectangle to exclude the close (X) icon
	int cMessagesUnread = 0;
	TContact * pContact = (TContact *)pTab->m_pTreeItem;
	if (pContact != NULL)
		{
		EMenuIcon eMenuIcon = pContact->TreeItemFlags_FIsComposingText() ? eMenuIcon_Pencil_10x10 : eMenuIcon_zNull;
		RTI_ENUM rti = pContact->EGetRuntimeClass();
		if (rti == RTI(TContact))
			{
			// Draw the online status of the contact
			cMessagesUnread = pContact->m_cMessagesUnread;
			if (eMenuIcon == eMenuIcon_zNull)
				eMenuIcon = pContact->Contact_EGetMenuIconPresence();
			}
		else if (rti == RTI(TGroup))
			{
			cMessagesUnread = pContact->m_cMessagesUnread;
			}
		else if (rti == RTI(TProfile))
			{
			eMenuIcon = eMenuIcon_ClassProfile;
			}
		if (eMenuIcon != eMenuIcon_zNull)
			OGetIcon(eMenuIcon).paint(pPainter, rcText, Qt::AlignLeft | Qt::AlignVCenter);
		}

//	cMessagesUnread = qrand() % 99;
	if (cMessagesUnread > 0)
		{
		QFont oFontPrevious = pPainter->font();
		QRect rcBoundary = pPainter->boundingRect(IN rcText, Qt::AlignCenter, pTab->m_strName);
		int xLeft = rcBoundary.right() + 8;
		int xRight = xLeft + 15;
		rcBoundary.setLeft(xLeft);
		rcBoundary.setRight(xRight);
		pPainter->setRenderHint(QPainter::Antialiasing, true);
		pPainter->setPen(g_oPenToolbar_Dot);
		pPainter->setFont(g_oFontToolbar_Dot);
		pPainter->DrawLineHorizontal(xLeft + 5, xRight - 5, 2 + (rcBoundary.top() + rcBoundary.bottom()) / 2);

		pPainter->setPen(d_coWhite);
		pPainter->drawText(rcBoundary, Qt::AlignVCenter | Qt::AlignCenter, QString::number(cMessagesUnread));
		pPainter->setFont(oFontPrevious);
		pPainter->setRenderHint(QPainter::Antialiasing, false);
		}

	pPainter->setPen(d_Toolbar_coText);
	pPainter->drawText(IN rcText, Qt::AlignCenter, pTab->m_strName);
	pPainter->DrawLineHorizontalCo(rcGradient.left(), rcGradient.right(), d_ToolbarTabs_Tab_cyHeightTop - fuIsTabSelected, d_ToolbarTabs_Tab_coBorder);
	pPainter->DrawLineHorizontalCo(pPainter->m_rcCell.left() + 1, pPainter->m_rcCell.right() - 1, d_cyHeightToolbarTabs - 1, fuIsTabSelected ? d_ToolbarButtons_coGradientTop : d_ToolbarTabs_Tab_coBorder);
	} // _DrawTab()

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

