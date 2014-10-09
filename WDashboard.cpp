#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WDashboard.h"

#define d_cxMarginSectionLeft	16	// Number of pixels for the left margin
#define d_cxWidthNumber			20
#define d_cxWidthMenuOverflow	18
#define d_cyMarginTopHeader		6
#define d_cyHeightSectionItem	24
#define d_yPosFirstItem			(d_cyMarginTopHeader + d_cyHeightSectionItem + d_cyMarginTopHeader + 1)
#define d_cyHeightFooter		26

#define d_cyFontHeightItem		16	// 16 pixels
#define d_cyFontHeightFooter	13	// 13 pixels

QFont g_oFontNormal;
QFont g_oFontBold;
QFont g_oFontBoldThick;		// Super bold font (weight of 900)
QFont g_oFontBoldSmaller;	// Smaller bold font for the footer and the number
QPen g_oPenDot;				// Pen to draw a dot to display a number within
QPen g_oPenTextEmpty;
QPen g_oPenTextNotEmpty;

void
CPainterCell::DrawTextWithinCell(const QString & sText)
	{
	drawText(IN m_rcCell, Qt::AlignVCenter, sText);
	}

void
CPainterCell::DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	DrawTextWithinCell(g_strScratchBufferStatusBar);
	}

void
CPainterCell::DrawTextUnderlinedStyle(const QString & sText, Qt::PenStyle eStyle)
	{
	drawText(m_rcCell, Qt::AlignVCenter, sText);
	QRect rcBounds = boundingRect(m_rcCell, Qt::AlignVCenter, sText);
	//rcBounds.setTop(rcBounds.bottom());
	//drawRect(rcBounds);
	QPen oPenOld = pen();
	QPen oPen = oPenOld;
	oPen.setStyle(eStyle);
	//oPen.setWidth(0);
	//oPen.setColor(d_coWhite);
	setPen(oPen);
	setRenderHint(Antialiasing, false);	// Remove the antialiasing to draw a sharp horizontal line
	DrawLineHorizontal(m_rcCell.left(), rcBounds.right() + 1, rcBounds.bottom() + 1);
	setPen(oPenOld);

	/*
	QFontMetrics oFontMetrics = fontMetrics();
	int cxWidth = oFontMetrics.width(sText);
	QPen oPenOld = pen();
	QPen oPen = oPenOld;
	oPen.setStyle(Qt::DotLine);
	setPen(oPen);
	DrawLineHorizontal(m_rcCell.left() + 2, cxWidth + 8, m_rcCell.top() + oFontMetrics.height() + 2);
	setPen(oPenOld);
	*/
	/*
	QFont oFontFooter = g_oFontNormal;
	oFontFooter.setUnderline(true);
	oPainter.setFont(oFontFooter);
	*/
	}

void
CPainterCell::DrawIconLeft(const QIcon & oIcon)
	{
	QRect rcIcon = m_rcCell;
	rcIcon.setWidth(16);
	oIcon.paint(this, rcIcon);
	m_rcCell.setLeft(m_rcCell.left() + 19);
	}

void
CPainterCell::DrawIconLeft(EMenuAction eMenuIcon)
	{
	DrawIconLeft(PGetMenuAction(eMenuIcon)->icon());
	}

void
CPainterCell::FillRect0(QRGB coBackgroundFill)
	{
	QRect rcItemBackground = m_rcCell;
	rcItemBackground.setLeft(-5);
	rcItemBackground.setRight(m_rcCell.right() - 4);
	setPen(Qt::NoPen);
	setBrush(QBrush(coBackgroundFill));
	drawRoundedRect(rcItemBackground, 6, 6);
	//fillRect(rcItemBackground, coBackgroundFill);
	}

//	Return the number of pixels of the drawing.  This is useful to chain drawing.
int
CPainterCell::DrawNumberWithinCircle(int nNumber)
	{
	//nNumber = qrand() % 99;
	if (nNumber > 0)
		{
		int cyHeight = m_rcCell.height();
		int yTop = m_rcCell.top();
		int xLeft = m_rcCell.right() - 20;
		setPen(g_oPenDot);
		DrawLineHorizontal(xLeft + 5, xLeft + 8, yTop + cyHeight / 2);
		//drawPoint(xLeft + 8, m_rcCell.top() + 9);
		//DrawLineHorizontal(rc.left(), rc.right(), ptCenter.y());	// Draw a longer line for large numbers
		setPen(g_oPenTextNotEmpty);
		setFont(g_oFontBoldSmaller);
		QRect rcText(xLeft - 3, yTop, 20, cyHeight);
		drawText(rcText, Qt::AlignVCenter | Qt::AlignCenter, QString::number(nNumber));
		m_rcCell.setRight(xLeft);
		return 16;
		}
	return 0;
	}


WDashboard * g_pwDashboard;

void
CArrayPtrDashboardSectionItems::DeleteAllItems()
	{
	CDashboardSectionItem ** ppItemStop;
	CDashboardSectionItem ** ppItem = PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		CDashboardSectionItem * pItem = *ppItem++;
		delete pItem;
		}
	Empty();
	}

void
CArrayPtrDashboardSectionItems::AllocateItem(void * pvDataItem)
	{
	Add(PA_CHILD new CDashboardSectionItem(pvDataItem));
	}

/*
void
CArrayPtrDashboardSectionItems::AllocateItems(const CArray & arraypDataItems, int cDataItemsMax)
	{
	void ** ppvDataItemStop;
	void ** ppvDataItem = arraypDataItems.PrgpvGetElementsStopMax(OUT &ppvDataItemStop, cDataItemsMax);
	while (ppvDataItem != ppvDataItemStop)
		AllocateItem(*ppvDataItem++);
	}
*/

void
CArrayPtrDashboardSectionItems::AllocateTreeItems(const CArrayPtrTreeItems & arraypTreeItems, int cDataItemsMax)
	{
	ITreeItem ** ppTreeItemStop;
	ITreeItem ** ppTreeItem = arraypTreeItems.PrgpGetTreeItemsStopMax(OUT &ppTreeItemStop, cDataItemsMax);
	while (ppTreeItem != ppTreeItemStop)
		{
		ITreeItem * pTreeItem = *ppTreeItem++;
		CDashboardSectionItem * paItem = new CDashboardSectionItem(pTreeItem);
		paItem->m_uFlagsItem |= CDashboardSectionItem::FI_keDataType_ITreeItem;
		Add(PA_CHILD paItem);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WDashboardSectionChannels::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrGroups arraypChannels;
	if (pProfile_YZ != NULL)
		m_cChannelsTotal = pProfile_YZ->GetRecentChannels(OUT &arraypChannels);
	else
		m_cChannelsTotal = 0;
	m_arraypaItems.AllocateTreeItems(arraypChannels, 8);
	}

void
WDashboardSectionGroups::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrGroups arraypGroups;
	if (pProfile_YZ != NULL)
		pProfile_YZ->GetRecentGroups(OUT &arraypGroups);
	m_arraypaItems.AllocateTreeItems(arraypGroups, 8);
	}

void
WDashboardSectionContacts::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrContacts arraypContacts;
	if (pProfile_YZ != NULL)
		pProfile_YZ->GetRecentContacts(OUT &arraypContacts);
	m_arraypaItems.AllocateTreeItems(arraypContacts, 8);
	}

void
WDashboardSectionBallots::Init(TProfile * /*pProfile_YZ*/)
	{
	hide();
	/*
	IEvent ** ppEventStop;
	IEvent ** ppEvent = pProfile->m_arraypEventsRecentBallots.PrgpGetEventsStopLast(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		m_arraypaItems.AddItem(new CDashboardSectionItem_IEventBallot((IEventBallot *)pEvent));
		}
	*/
	}

void
WDashboardSectionChannels::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvGroupChannel)
	{
	TGroup * pGroup = (TGroup *)pvGroupChannel;
	//pPainter->DrawIconLeft(eMenuAction_GroupChannel);
	pPainter->DrawNumberWithinCircle(pGroup->m_cMessagesUnread);
	pPainter->setFont((pGroup->m_cMessagesUnread > 0) ? g_oFontBoldThick : g_oFontNormal);
	pPainter->DrawTextWithinCell("# " + pGroup->TreeItem_SGetNameDisplay().toLower());
	}

void
WDashboardSectionChannels::DrawFooter(CPainterCell * pPainter, UINT uFlagsHitTest)
	{
	int cChannelsRemaining = m_cChannelsTotal - m_arraypaItems.GetSize();
	if (cChannelsRemaining > 0)
		g_strScratchBufferStatusBar.Format("+$I more...", cChannelsRemaining);
	else
		g_strScratchBufferStatusBar.Format("New Channel...");
	pPainter->DrawTextUnderlinedStyle(g_strScratchBufferStatusBar, (uFlagsHitTest & FHT_kfFooter) ? Qt::SolidLine : Qt::DotLine);
	}

void
WDashboardSectionGroups::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvGroup)
	{
	TGroup * pGroup = (TGroup *)pvGroup;
	pPainter->DrawNumberWithinCircle(pGroup->m_cMessagesUnread);
	pPainter->DrawTextWithinCell(pGroup->TreeItem_SGetNameDisplay());
	}

void
WDashboardSectionContacts::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvContact)
	{
	TContact * pContact = (TContact *)pvContact;
	//EMenuAction eMenuIconPresence = pContact->Contact_EGetMenuActionPresence();
	pPainter->DrawIconLeft(pContact->Contact_FuIsOnline() ? eMenuAction_PresenceAccountOnline : eMenuAction_PresenceAccountOffline);
	pPainter->DrawNumberWithinCircle(pContact->m_cMessagesUnread);
	pPainter->DrawTextWithinCell_VE("$s", pContact->TreeItem_PszGetNameDisplay());
	}

void
WDashboardSectionContacts::DrawFooter(CPainterCell * pPainter, UINT uFlagsHitTest)
	{
	g_strScratchBufferStatusBar.Format("+ $I more peers...", 123);
	pPainter->DrawTextUnderlinedStyle(g_strScratchBufferStatusBar, (uFlagsHitTest & FHT_kfFooter) ? Qt::SolidLine : Qt::DotLine);
	}

void
WDashboardSectionChannels::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "WDashboardSectionChannels::OnItemClicked(0x$p, 0x$x)\n", oHitTestInfo.pItem, oHitTestInfo.uFlagsHitTest);
	if (oHitTestInfo.uFlagsHitTest & FHT_kfFooter)
		{
		void DisplayDialogChannelsBrowse(TProfile * pProfile);
		DisplayDialogChannelsBrowse(m_pParent->PGetProfile());
		return;
		}
	if (oHitTestInfo.uFlagsHitTest & FHT_kfMenuOverflow)
		{
		WMenu oMenu;
		oMenu.ActionAdd(eMenuAction_GroupChannelInvite);
		oMenu.ActionAdd(eMenuAction_GroupChannelLeave);
		EMenuAction eMenuAction = oMenu.EDisplayContextMenu();
		switch (eMenuAction)
			{
		default:
			break;
			}

		return;
		}
	WDashboardSection::OnItemClicked(oHitTestInfo);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
singleton WDashboardCaption : public QWidget
{
public:
	WDashboardCaption();
};

WDashboardCaption::WDashboardCaption()
	{
	QToolButton * pwButtonUndock = new QToolButton(this);
	pwButtonUndock->setToolTip("Float / Unfloat");
	pwButtonUndock->setStyleSheet("QToolButton { border: none; padding: 3px; }");
	QPixmap oPixmap = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
	pwButtonUndock->setIcon(oPixmap);
	pwButtonUndock->setCursor(Qt::ArrowCursor);
	pwButtonUndock->setFocusPolicy(Qt::ClickFocus);

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addStretch();
	layout->addWidget(pwButtonUndock);
	setLayout(layout);
	setCursor(Qt::OpenHandCursor);		// This cursor shows to the user he/she may drag the widget to undock the Navigation Tree
	}

#include "WNavigationTree.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboard::WDashboard()
	{
	Assert(g_pwDashboard == NULL);
	g_pwDashboard = this;
	m_pProfile = NULL;
	m_pItemSelected = NULL;
	setObjectName("Dashboard");

	//g_oFontNormal = font();
	g_oFontNormal.setFamily("Lato, sans-serif");
	//g_oFontNormal.setFamily("Times New Roman");
	g_oFontNormal.setPixelSize(d_cyFontHeightItem);
	g_oFontBoldThick = g_oFontBold = g_oFontNormal;
	g_oFontBold.setWeight(QFont::Bold);
	g_oFontBoldThick.setWeight(99);
	g_oFontBoldSmaller = g_oFontBold;
	g_oFontBoldSmaller.setPixelSize(d_cyFontHeightFooter);

	m_pwLabelCaption = new WLabel;
	//m_pwLabelCaption->setStyleSheet("background-color:#8080FF");
	m_pwLabelCaption->setStyleSheet("background-color:#3e313c; color:white; font-size:16px; font-weight:900; border-bottom:2px solid #372c36; padding:16px; padding-left:11px");
	m_pwLabelCaption->setMargin(0);
	//m_pwLabelCaption->setFont(g_oFontBold);
	#if 1
	setTitleBarWidget(m_pwLabelCaption);
	#else
	setTitleBarWidget(new WDashboardCaption);
	#endif

	QWidget * pwWidgetDashboard = new QWidget;	// Main widget for the dashboard
	#if 0
	pwWidgetDashboard->setStyleSheet("background-color:#A0A0FF;");	// Light blue
	#else
	pwWidgetDashboard->setStyleSheet("background-color:#4d394b;");
	#endif
	setWidget(PA_CHILD pwWidgetDashboard);
	m_poLayoutVertial = new OLayoutVerticalAlignTop(pwWidgetDashboard);
	Layout_MarginsClear(INOUT m_poLayoutVertial);
	m_poLayoutVertial->setSpacing(0);

	InitToGarbage(OUT &m_sections, sizeof(m_sections));
	m_sections.pwSectionBalots = new WDashboardSectionBallots("BALLOTS");
	m_sections.pwSectionChannels = new WDashboardSectionChannels("CHANNELS");
	m_sections.pwSectionContacts = new WDashboardSectionContacts("PEERS");
	m_sections.pwSectionGroups = new WDashboardSectionGroups("PRIVATE GROUPS");

	// Add each section to the vertical layout
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		m_poLayoutVertial->addWidget(*ppwSection);
	}

void
WDashboard::ProfileSelectedChanged(TProfile * pProfile)
	{
	m_pProfile = pProfile;
	m_pwLabelCaption->Label_SetTextPlain((pProfile != NULL) ? pProfile->m_strNameProfile : c_strEmpty);
	m_pItemSelected = NULL;
	// Notify each section the selected profile changed
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		{
		WDashboardSection * pwSection = *ppwSection;
		pwSection->SetParent(this);
		pwSection->Init(pProfile);
		pwSection->updateGeometry();
		}
	//m_poLayoutVertial->invalidate();
	//updateGeometry();
	update();
	}

BOOL
WDashboard::FSelectItem(CDashboardSectionItem * pItem)
	{
	if (pItem == m_pItemSelected)
		return FALSE;
	if (m_pItemSelected != NULL)
		{
		if (m_pItemSelected->m_uFlagsItem & CDashboardSectionItem::FI_kfSelected)
			{
			m_pItemSelected->m_uFlagsItem &= ~CDashboardSectionItem::FI_kfSelected;
			}
		}
	if (pItem != NULL)
		{
		pItem->m_uFlagsItem |= CDashboardSectionItem::FI_kfSelected;
		}
	m_pItemSelected = pItem;
	update();	// Redraw the whole dashboard. This is not efficient, however it works.  Ideally there should be a mechanism to redraw individual items
	return TRUE;
	}

void
WDashboard::NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	Assert(pContactOrGroup_NZ != NULL);
	if (pContactOrGroup_NZ->PGetProfile() == m_pProfile)
		ProfileSelectedChanged(m_pProfile);	// At the moment, refresh the entire thing.  This is not efficient, but it works
	}

void
WDashboard::NewEventRelatedToBallot(IEventBallot * /*pEventBallot*/)
	{
	// Search if the ballot is already there, and if not, add it
	//m_sections.pwSectionBalots->m_arraypaItems
	}

void
WDashboard::RefreshContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (pContact->PGetProfile() == m_pProfile)
		m_sections.pwSectionContacts->update();
	}

void
WDashboard::RefreshGroup(TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	if (pGroup->PGetProfile() == m_pProfile)
		{
		if (pGroup->Group_FuIsChannel())
			m_sections.pwSectionChannels->update();
		else
			m_sections.pwSectionGroups->update();
		}
	}

void
WDashboard::RefreshChannels()
	{
	m_sections.pwSectionChannels->SetParent(this);
	m_sections.pwSectionChannels->Init(m_pProfile);
	m_sections.pwSectionChannels->updateGeometry();
	m_sections.pwSectionChannels->update();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboardSection::WDashboardSection(PSZAC pszSectionName) : WWidget(NULL)
	{
	m_sName = pszSectionName;
	m_oHitTestInfo.pItem = NULL;
	setMouseTracking(true);
	}

WDashboardSection::~WDashboardSection()
	{
	//m_arraypaItems.DeleteAllItems();
	}

WDashboardSection::SHitTestInfo WDashboardSection::c_oHitTestInfoEmpty;

void
WDashboardSection::SetParent(WDashboard * pParent)
	{
	Assert(pParent != NULL);
	m_pParent = pParent;
	m_arraypaItems.DeleteAllItems();
	m_oHitTestInfo = c_oHitTestInfoEmpty;
	}

void
WDashboardSection::Init(TProfile * pProfile_YZ)
	{
	Endorse(pProfile_YZ == NULL);
	}

void
WDashboardSection::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * /*pvDataItem*/)
	{
	Assert(pPainter != NULL);
	}

void
WDashboardSection::DrawFooter(CPainterCell * pPainter, UINT /*uFlagsItem*/)
	{
	Assert(pPainter != NULL);
	}

void
WDashboardSection::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	if (oHitTestInfo.pItem != NULL)
		{
		UINT ukmDataType = (oHitTestInfo.pItem->m_uFlagsItem & CDashboardSectionItem::FI_kmDataTypeMask);
		if (ukmDataType == CDashboardSectionItem::FI_keDataType_ITreeItem)
			{
			m_pParent->FSelectItem(oHitTestInfo.pItem);
			NavigationTree_SelectTreeItem(oHitTestInfo.pItem->m_data.piTreeItem);
			return;
			}
		}
	}
/*
void
WDashboardSection::OnMenuClicked(SHitTestInfo oHitTestInfo)
	{
	}
*/


QSize
WDashboardSection::sizeHint() const
	{
	return QSize(150, d_yPosFirstItem + d_cyHeightFooter + 14 + m_arraypaItems.GetSize() * d_cyHeightSectionItem);
	}

//	WDashboardSection::QWidget::heightForWidth()
int
WDashboardSection::heightForWidth(int cxWidth) const
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "WDashboardSection::heightForWidth($i)\n", cxWidth);
	Assert(FALSE && "Is this method ever called?");
	return QWidget::heightForWidth(cxWidth);
	}

#define d_coBackgroundItemSelected		0x4C9689
#define d_coBackgroundItemMouseHover	0x3E313C
#define d_coTextItemEmpty				0xab9ba9	// Color to draw the text of an empty item
#define d_coTextItemNotEmpty			d_coWhite
#define d_coTextHeader					d_coWhite;

//	WDashboardSection::QWidget::paintEvent()
void
WDashboardSection::paintEvent(QPaintEvent *)
	{
	CPainterCell oPainter(this);
	#if 1
	oPainter.setRenderHint(QPainter::Antialiasing, true);
	oPainter.setRenderHint(QPainter::TextAntialiasing, true);
	oPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	#endif
//	oPainter.drawRect(0, 0, width() - 1, height() - 1);

	// Initialize the pens and fonts
	g_oPenTextNotEmpty = oPainter.pen();
	g_oPenTextNotEmpty.setColor(d_coTextItemNotEmpty);
	g_oPenTextEmpty = g_oPenTextNotEmpty;
	g_oPenTextEmpty.setColor(d_coTextItemEmpty);
	g_oPenDot.setColor(0xEB4D5C);	// Orange color
	g_oPenDot.setWidth(16);
	g_oPenDot.setCapStyle(Qt::RoundCap);
//	oPainter.setOpacity(0.7);

	// Draw the caption
	oPainter.setFont(g_oFontBoldSmaller);
	oPainter.setPen(g_oPenTextEmpty);
	oPainter.drawText(d_cxMarginSectionLeft, d_cyMarginTopHeader + d_cyHeightSectionItem, m_sName);

	QRect rcSection = rect();
	qreal xRight = rcSection.width();
	qreal yTop = d_yPosFirstItem;

	oPainter.m_rcCell.setTop(d_yPosFirstItem);
	oPainter.m_rcCell.setHeight(d_cyHeightSectionItem);
	CDashboardSectionItem ** ppItemStop;
	CDashboardSectionItem ** ppItem = m_arraypaItems.PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		oPainter.m_rcCell.setLeft(d_cxMarginSectionLeft);
		oPainter.m_rcCell.setRight(xRight);
		//oPainter.drawRect(oPainter.m_rcCell);
		CDashboardSectionItem * pItem = *ppItem++;
		UINT uFlagsItem = pItem->m_uFlagsItem;
		//uFlagsItem = qrand() % 8;
		if (uFlagsItem & (CDashboardSectionItem::FI_kfSelected | CDashboardSectionItem::FI_kfMouseHover))
			{
			oPainter.FillRect0((uFlagsItem & CDashboardSectionItem::FI_kfSelected) ? d_coBackgroundItemSelected : d_coBackgroundItemMouseHover);
			}
		if (uFlagsItem & CDashboardSectionItem::FI_kfMouseHover)
			{
			QRect rcMenuOverflow = oPainter.m_rcCell;
			int xLeft = rcMenuOverflow.right() - d_cxWidthMenuOverflow;
			if (m_oHitTestInfo.uFlagsHitTest & FHT_kfMenuOverflow)
				{
				rcMenuOverflow.setLeft(xLeft);
				oPainter.fillRect(rcMenuOverflow, (m_oHitTestInfo.uFlagsHitTest & FHT_kfMenuOverflow) ? d_coPurpleLight : d_coPurple);
				}
			QIcon oIcon = PGetMenuAction(eMenuIconOverflow)->icon();
			oIcon.paint(&oPainter, xLeft, yTop - 1, 16, 16);
			}
		oPainter.setPen((uFlagsItem & CDashboardSectionItem::FI_kfSelected) ? g_oPenTextNotEmpty : g_oPenTextEmpty);
		oPainter.setFont((uFlagsItem & CDashboardSectionItem::FI_kfDrawBold) ? g_oFontBold : g_oFontNormal);
		DrawItem(&oPainter, uFlagsItem, pItem->m_data.pvDataItem);
		yTop += d_cyHeightSectionItem;
		oPainter.m_rcCell.moveTop(yTop);
		} // while

	// Draw the footer
	oPainter.m_rcCell.setLeft(d_cxMarginSectionLeft);
	oPainter.m_rcCell.setRight(xRight);
	oPainter.setFont(g_oFontBoldSmaller);
	oPainter.setPen((m_oHitTestInfo.uFlagsHitTest & FHT_kfFooter) ? g_oPenTextNotEmpty : g_oPenTextEmpty);
	DrawFooter(&oPainter, m_oHitTestInfo.uFlagsHitTest);
	}

WDashboardSection::SHitTestInfo
WDashboardSection::OGetHitTestInfo(QMouseEvent * pEventMouse) const
	{
	QPoint ptMouse = pEventMouse->pos();
	return OGetHitTestInfo(ptMouse.x(), ptMouse.y());
	}

WDashboardSection::SHitTestInfo
WDashboardSection::OGetHitTestInfo(int xPos, int yPos) const
	{
	SHitTestInfo oHitTestInfo = c_oHitTestInfoEmpty;
	const int cxWidth = width();
	if (xPos > cxWidth - d_cxWidthMenuOverflow)
		oHitTestInfo.uFlagsHitTest |= FHT_kfMenuOverflow;
	if (yPos <= d_yPosFirstItem)
		{
		oHitTestInfo.uFlagsHitTest |= FHT_kfHeader;
		goto Done;
		}
	else
		{
		int ySection = d_yPosFirstItem + d_cyHeightSectionItem;
		// Perform some hit testing
		CDashboardSectionItem ** ppItemStop;
		CDashboardSectionItem ** ppItem = m_arraypaItems.PrgpGetItemsStop(OUT &ppItemStop);
		while (ppItem != ppItemStop)
			{
			CDashboardSectionItem * pItem = *ppItem++;
			if (yPos <= ySection)
				{
				oHitTestInfo.pItem = pItem;
				goto Done;
				}
			ySection += d_cyHeightSectionItem;
			}
		}
	// Anything below is the footer
	oHitTestInfo.uFlagsHitTest |= FHT_kfFooter;
	Done:
	return oHitTestInfo;
	}

BOOL
WDashboardSection::FSetHitTestInfo(SHitTestInfo oHitTestInfo)
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "WDashboardSection::FSetHitTestInfo(0x$p, 0x$x)\n", oHitTestInfo.pItem, oHitTestInfo.eHitTest);
	if ((oHitTestInfo.uFlagsHitTest == m_oHitTestInfo.uFlagsHitTest) &&
		(oHitTestInfo.pItem == m_oHitTestInfo.pItem))
		return FALSE;	// Nothing to do
	if (m_oHitTestInfo.pItem != NULL)
		m_oHitTestInfo.pItem->m_uFlagsItem &= ~CDashboardSectionItem::FI_kfMouseHover;
	if (oHitTestInfo.pItem != NULL)
		oHitTestInfo.pItem->m_uFlagsItem |= CDashboardSectionItem::FI_kfMouseHover;
	m_oHitTestInfo = oHitTestInfo;
	update();
	return TRUE;
	}

void
WDashboardSection::mouseMoveEvent(QMouseEvent * pEventMouse)
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] WDashboardSection::mouseMoveEvent() - buttons=$x\n", pEventMouse->buttons());
	if (pEventMouse->buttons() == Qt::NoButton)
		(void)FSetHitTestInfo(OGetHitTestInfo(pEventMouse));
	}


void
WDashboardSection::mouseReleaseEvent(QMouseEvent * pEventMouse)
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "WDashboardSection::mouseReleaseEvent() - buttons=$x\n", pEventMouse->button());
	if (pEventMouse->button() == Qt::LeftButton)
		{
		SHitTestInfo oHitTestInfo = OGetHitTestInfo(pEventMouse);
		if (!FSetHitTestInfo(oHitTestInfo))
			OnItemClicked(oHitTestInfo);	// Send the click event if the mouse was not dragged while clicking
		}
	}

void
WDashboardSection::leaveEvent(QEvent *)
	{
	(void)FSetHitTestInfo(c_oHitTestInfoEmpty);
	}

void
Dashboard_UpdateAccordingToSelectedProfile(TProfile * pProfileSelected)
	{
	Endorse(pProfileSelected == NULL);
	if (g_pwDashboard != NULL)
		g_pwDashboard->ProfileSelectedChanged(pProfileSelected);
	}

void
Dashboard_UpdateContact(TContact * pContact)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->RefreshContact(pContact);
	}

void
Dashboard_UpdateGroup(TGroup * pGroup)
	{
	Assert(g_pwDashboard != NULL);
	g_pwDashboard->RefreshGroup(pGroup);
	}

void
Dashboard_UpdateChannels()
	{
	g_pwDashboard->RefreshChannels();
	}

void
Dashboard_NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	Assert(pContactOrGroup_NZ != NULL);
	Assert(g_pwDashboard != NULL);
	//MessageLog_AppendTextFormatSev(eSeverityComment, "Dashboard_NewEventsFromContactOrGroup($s)\n", pContactOrGroup_NZ->TreeItem_PszGetNameDisplay());
	g_pwDashboard->NewEventsFromContactOrGroup(pContactOrGroup_NZ);
	}

void
Dashboard_NewEventRelatedToBallot(IEventBallot * pEventBallot)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->NewEventRelatedToBallot(pEventBallot);
	}

void
Dashboard_RemoveSelection()
	{
	Assert(g_pwDashboard != NULL);
	(void)g_pwDashboard->FSelectItem(NULL);
	}
