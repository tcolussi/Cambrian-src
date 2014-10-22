#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WDashboard.h"
#include "MenuIcons.h"

//	Corporations and Coallitions

//#define COLORS_THEME_PURPLE
#ifdef COLORS_THEME_PURPLE
	#define d_szStyleSheetCaption			"background-color:#3e313c; color:white; border-bottom:2px solid #372c36;"
	#define d_szStyleSheetSection			"background-color:#4d394b;"
	//#define d_coBackgroundSection			0x4d394b
	#define d_coTextHeader					d_coWhite
	#define d_coBackgroundItemSelected		0x4C9689
	#define d_coBackgroundItemMouseHover	0x3E313C
	#define d_coTextItemEmpty				0xab9ba9	// Color to draw the text of an empty item
	#define d_coTextItemNotEmpty			d_coWhite

#else
	#define d_szStyleSheetCaption			"background-color:#444A47; color:white;" // border-bottom:2px solid #372c36;"
	#define d_szStyleSheetSection			"background-color:#4D5250;"				// background-color:#A0A0FF;" == Light blue
	//#define d_coBackgroundSection			0x4D5250
	#define d_coTextHeader					d_coBlack
	#define d_coBackgroundItemSelected		0xD39B46
	#define d_coBackgroundItemMouseHover	d_coGrayLight
	#define d_coTextItemEmpty				0xB2B4B3
	#define d_coTextItemNotEmpty			d_coWhite
#endif

#define d_cxMarginHeaderLeft	16
#define d_cxMarginSectionLeft	16	// Number of pixels for the left margin
#define d_cxWidthNumber			20
#define d_cxWidthMenuOverflow	18
#define d_cyMarginTopHeader		6
#define d_cyHeightSectionItem	24
#define d_yPosFirstItem			(d_cyMarginTopHeader + d_cyHeightSectionItem + d_cyMarginTopHeader + 1)
#define d_cyHeightFooter		26

#define d_cyFontHeightItem		16	// 16 pixels
#define d_cyFontHeightFooter	14	// 13 pixels

QFont g_oFontNormal;
QFont g_oFontBold;
QFont g_oFontBoldThick;		// Super bold font (weight of 900)
QFont g_oFontBoldSmaller;	// Smaller bold font for the footer and the number
QPen g_oPenDot;				// Pen to draw a dot to display a number within
QPen g_oPenTextEmpty;
QPen g_oPenTextNotEmpty;

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
		QRect rcText(xLeft - 4, yTop, 20, cyHeight);
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
WDashboardSection_TGroup::WDashboardSection_TGroup(PSZAC pszGroupName, EMenuIcon eMenuIcon) : WDashboardSection(pszGroupName)
	{
	m_eMenuIcon = eMenuIcon;
	}

void
WDashboardSection_TGroup::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvGroup)
	{
	TGroup * pGroup = (TGroup *)pvGroup;
	Assert(pGroup != NULL);
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	pPainter->DrawIconAdjustLeft(m_eMenuIcon);
	pPainter->DrawNumberWithinCircle(pGroup->m_cMessagesUnread);
	pPainter->setFont((pGroup->m_cMessagesUnread > 0) ? g_oFontBoldThick : g_oFontNormal);
	g_strScratchBufferStatusBar = pGroup->TreeItem_PszGetNameDisplay();
	g_strScratchBufferStatusBar.NormalizeStringForChannelName();		// Draw all groups with their names normalized
	pPainter->DrawTextWithinCell(g_strScratchBufferStatusBar);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboardSectionChannels::WDashboardSectionChannels() : WDashboardSection_TGroup("CHANNELS", eMenuIcon_ClassChannel)
	{
	m_strTextFooterIfItemsAvailableIsZero.InitFromStringA("New Channel...");
	}
WDashboardSectionGroups::WDashboardSectionGroups() : WDashboardSection_TGroup("PRIVATE GROUPS", eMenuIcon_ClassGroup)
	{
	//m_strTextFooterIfItemsAvailableIsZero.InitFromStringA("New Group...");
	}
WDashboardSectionCorporations::WDashboardSectionCorporations() : WDashboardSection_TGroup("CORPS and COALITIONS", eMenuIcon_ClassOrganization)
	{
	m_strTextFooterIfItemsAvailableIsZero.InitFromStringA("New Corporation...");
	}

void
WDashboardSectionChannels::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrGroups arraypChannels;
	if (pProfile_YZ != NULL)
		m_cTotalItemsAvailable = pProfile_YZ->GetRecentChannels(OUT &arraypChannels);
	m_arraypaItems.AllocateTreeItems(arraypChannels, 8);
	}
void
WDashboardSectionGroups::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrGroups arraypGroups;
	if (pProfile_YZ != NULL)
		m_cTotalItemsAvailable = pProfile_YZ->GetRecentGroups(OUT &arraypGroups);
	m_arraypaItems.AllocateTreeItems(arraypGroups, 8);
	}
void
WDashboardSectionCorporations::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrGroups arraypGroups;
	if (pProfile_YZ != NULL)
		m_cTotalItemsAvailable = pProfile_YZ->GetRecentCorporations(OUT &arraypGroups);
	m_arraypaItems.AllocateTreeItems(arraypGroups, 8);
	}


void
WDashboardSectionContacts::Init(TProfile * pProfile_YZ)
	{
	CArrayPtrContacts arraypContacts;
	if (pProfile_YZ != NULL)
		m_cTotalItemsAvailable = pProfile_YZ->GetRecentContacts(OUT &arraypContacts);
	m_arraypaItems.AllocateTreeItems(arraypContacts, 8);
	}

void
WDashboardSectionBallots::Init(TProfile * /*pProfile_YZ*/)
	{
	//hide();
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
WDashboardSectionContacts::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvContact)
	{
	TContact * pContact = (TContact *)pvContact;
	//EMenuAction eMenuIconPresence = pContact->Contact_EGetMenuActionPresence();
	pPainter->DrawIconAdjustLeft(pContact->Contact_FuIsOnline() ? eMenuIcon_ClassContactOnline : eMenuIcon_ClassContactOffline);
	pPainter->DrawNumberWithinCircle(pContact->m_cMessagesUnread);
	QFont oFont = (pContact->m_cMessagesUnread > 0) ? g_oFontBoldThick : g_oFontNormal;
	if (!pContact->Contact_FuIsOnline())
		oFont.setItalic(true);
	pPainter->setFont(oFont);
	pPainter->DrawTextWithinCell_VE("$s", pContact->TreeItem_PszGetNameDisplay());
	}

const EMenuActionByte c_rgzeActionsMenuContact[] =
	{
	eMenuAction_ContactRemove,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuChannel[] =
	{
	eMenuAction_GroupChannelInviteOthers,
	eMenuAction_GroupChannelLeave,
	eMenuAction_GroupLaunchBallot,
	eMenuAction_GroupSetPurpose,
	eMenuAction_GroupUpgradeService,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuGroup[] =
	{
	eMenuAction_GroupAddContacts,
	eMenuAction_GroupDelete,
	eMenuAction_GroupLaunchBallot,
	eMenuAction_GroupSetPurpose,
	eMenuAction_GroupUpgradeService,
	eMenuAction_GroupUpgradeToCorporation,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuCorporations[] =
	{
	eMenuAction_GroupLaunchBallot,
	ezMenuActionNone
	};


void
WDashboardSectionChannels::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "WDashboardSectionChannels::OnItemClicked(0x$p, 0x$x)\n", oHitTestInfo.pItem, oHitTestInfo.uFlagsHitTest);
	if (oHitTestInfo.uFlagsHitTest & (FHT_kfHeader | FHT_kfFooter))
		{
		LaunchApplication_Channels(NULL);
		return;
		}
	_OnItemClicked_ITreeItem(oHitTestInfo, c_rgzeActionsMenuChannel);
	}

void
WDashboardSectionGroups::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	if (oHitTestInfo.uFlagsHitTest & (FHT_kfHeader | FHT_kfFooter))
		{
		LaunchApplication_GroupManager(NULL);
		return;
		}
	_OnItemClicked_ITreeItem(oHitTestInfo, c_rgzeActionsMenuGroup);
	}
void
WDashboardSectionCorporations::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	if (oHitTestInfo.uFlagsHitTest & (FHT_kfHeader | FHT_kfFooter))
		{
		LaunchApplication_Corporations(NULL);
		return;
		}
	_OnItemClicked_ITreeItem(oHitTestInfo, c_rgzeActionsMenuCorporations);
	}

void
WDashboardSectionContacts::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	if (oHitTestInfo.uFlagsHitTest & (FHT_kfHeader | FHT_kfFooter))
		{
		LaunchApplication_PeerManager(NULL);
		return;
		}
	_OnItemClicked_ITreeItem(oHitTestInfo, c_rgzeActionsMenuContact);
	}

void
WDashboardSectionBallots::OnItemClicked(SHitTestInfo oHitTestInfo)
	{
	if (oHitTestInfo.uFlagsHitTest & (FHT_kfHeader | FHT_kfFooter))
		{
		LaunchApplication_Ballotmaster(NULL);
		return;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WDashboard_TProfiles::IDashboard_AddSections(INOUT OLayoutVertical * /*poLayout*/)
	{
	hide();
	}
void
WDashboard_TProfiles::IDashboard_Notify(UINT /*uFlagsDashboardNotify*/, POBJECT /*pObjectModified*/)
	{
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboard_TProfile::WDashboard_TProfile(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	m_pProfile_NZ = pProfile;

	}

QString
WDashboard_TProfile::IDashboard_SGetCaption()
	{
	return c_sEmpty;
	}

void
WDashboard_TProfile::IDashboard_AddSections(INOUT OLayoutVertical * poLayout)
	{
	InitToGarbage(OUT &m_sections, sizeof(m_sections));
	m_sections.pwSectionChannels = new WDashboardSectionChannels;
	m_sections.pwSectionGroups = new WDashboardSectionGroups();
	m_sections.pwSectionCorporations = new WDashboardSectionCorporations();
	m_sections.pwSectionContacts = new WDashboardSectionContacts("PEERS");
	m_sections.pwSectionBalots = new WDashboardSectionBallots("BALLOTS");
	poLayout->Layout_AddWidgets((QWidget **)&m_sections, sizeof(m_sections) / sizeof(QWidget *));	// Add each section to the vertical layout
	}

void
WDashboard_TProfile::IDashboard_Notify(UINT /*uFlagsDashboardNotify*/, POBJECT /*pObjectModified*/)
	{

	}

///////////////////////////////////////////////////////////////////////////////////////////////////
singleton WDashboardCaption : public QWidget
{
public:
	WDashboardCaption();
	virtual QSize sizeHint() const { return QSize(0, 58); }
	virtual QSize minimumSizeHint() const { return QSize(20, 0); }
	virtual void paintEvent(QPaintEvent *);
};

WDashboardCaption::WDashboardCaption()
	{
	setStyleSheet(d_szStyleSheetCaption);
	}

void
WDashboardCaption::paintEvent(QPaintEvent *)
	{
	CPainter oPainter(this);
	oPainter.setFont(g_oFontBoldThick);
	QRect rcCaption = rect();
	rcCaption.setLeft(d_cxMarginHeaderLeft);
	//oPainter.drawText(rcCaption, Qt::AlignVCenter, parentWidget()->windowTitle());
	WDashboard * pwParent = (WDashboard *)parentWidget();
	Assert(pwParent != NULL);
	#ifdef SIMPLE_DASHBOARD
	TProfile * pProfile = pwParent->PGetProfile_YZ();
	if (pProfile != NULL)
		oPainter.drawText(rcCaption, Qt::AlignVCenter, pProfile->m_strNameProfile);
	#else
	if (pwParent->m_paiwDashboard_NZ != NULL)
		oPainter.drawText(rcCaption, Qt::AlignVCenter, pwParent->m_paiwDashboard_NZ->IDashboard_SGetCaption());
	#endif
	}

#include "WNavigationTree.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

IDashboard * WDashboard::s_paiwDashboard_NZ;

WDashboard::WDashboard() : QDockWidget("Comm Panel")
	{
	Assert(g_pwDashboard == NULL);
	g_pwDashboard = this;
	#ifdef SIMPLE_DASHBOARD
	s_paiwDashboard_NZ = new WDashboard_TProfiles();	// Make sure the pointer is never NULL
	#else
	m_paiwDashboard_NZ = NULL;
	#endif
	m_pItemSelected_YZ = NULL;
	setObjectName("Dashboard");

	//g_oFontNormal = font();
	g_oFontNormal.setFamily("Lato [sans-serif]");
	//g_oFontNormal.setFamily("Times New Roman");
	g_oFontNormal.setPixelSize(d_cyFontHeightItem);
	g_oFontNormal.setWeight(QFont::Bold);	// Make everything bold
	g_oFontBoldThick = g_oFontBold = g_oFontNormal;
	g_oFontBold.setWeight(QFont::Bold);
	g_oFontBoldThick.setWeight(99);
	g_oFontBoldSmaller = g_oFontBold;
	g_oFontBoldSmaller.setPixelSize(d_cyFontHeightFooter);

	setTitleBarWidget(new WDashboardCaption);
	QWidget * pwWidgetDashboard = new QWidget;	// Main widget for the dashboard
	m_poLayoutVertial = new OLayoutVerticalAlignTop0(pwWidgetDashboard);

	#define WANT_SCROLL_BAR
	#ifdef WANT_SCROLL_BAR

	QScrollArea * pwScrollArea = new QScrollArea;
	pwScrollArea->setFrameShape(QFrame::NoFrame);
	pwScrollArea->setWidgetResizable(true);
	pwScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	pwWidgetDashboard->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);	// This is necessary, otherwise the layout will proportionally clip th ewidgets
	pwScrollArea->setStyleSheet(
	/*
	" QScrollBar {"
	" border: 2px solid green;"
	" background: yellow;"
	" }"
	*/
	"* { background-color:#4D5250; }"
	);
	pwScrollArea->setWidget(pwWidgetDashboard);
	setWidget(pwScrollArea);

	#else
	QWidget * pwWidgetDashboard = new QWidget;	// Main widget for the dashboard
	pwWidgetDashboard->setStyleSheet(d_szStyleSheetSection);
	setWidget(PA_CHILD pwWidgetDashboard);
	m_poLayoutVertial = new OLayoutVerticalAlignTop0(pwWidgetDashboard);
	#endif

	#ifdef SIMPLE_DASHBOARD
	InitToGarbage(OUT &m_sections, sizeof(m_sections));
	m_sections.pwSectionChannels = new WDashboardSectionChannels;
	m_sections.pwSectionGroups = new WDashboardSectionGroups();
	m_sections.pwSectionCorporations = new WDashboardSectionCorporations();
	m_sections.pwSectionContacts = new WDashboardSectionContacts("PEERS");
	m_sections.pwSectionBalots = new WDashboardSectionBallots("BALLOTS");

	// Add each section to the vertical layout
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		m_poLayoutVertial->addWidget(*ppwSection);
	#endif
	}

#ifdef SIMPLE_DASHBOARD
void
WDashboard::ProfileSelectedChanged(TProfile * pProfile)
	{
	m_pProfile_YZ = pProfile;
	//setWindowTitle((pProfile != NULL) ? pProfile->m_strNameProfile : c_strEmpty);
	//m_pwLabelCaption->Label_SetTextPlain((pProfile != NULL) ? pProfile->m_strNameProfile : c_strEmpty);
	RefreshAll();
	}

void
WDashboard::RefreshAll()
	{
	m_pItemSelected_YZ = NULL;
	// Notify each section the selected profile changed
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		{
		WDashboardSection * pwSection = *ppwSection;
		pwSection->SetParent(this);
		pwSection->Init(m_pProfile_YZ);
		pwSection->updateGeometry();
		}
	//updateGeometry();
	update();
	}

BOOL
WDashboard::FSelectItem(CDashboardSectionItem * pItem)
	{
	if (pItem == m_pItemSelected_YZ)
		return FALSE;
	if (m_pItemSelected_YZ != NULL)
		{
		if (m_pItemSelected_YZ->m_uFlagsItem & CDashboardSectionItem::FI_kfSelected)
			m_pItemSelected_YZ->m_uFlagsItem &= ~CDashboardSectionItem::FI_kfSelected;
		}
	if (pItem != NULL)
		pItem->m_uFlagsItem |= CDashboardSectionItem::FI_kfSelected;
	m_pItemSelected_YZ = pItem;
	update();	// Redraw the whole dashboard. This is not efficient, however it works.  Ideally there should be a mechanism to redraw individual items
	return TRUE;
	}

void
WDashboard::RefreshContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (pContact->PGetProfile() == m_pProfile_YZ)
		m_sections.pwSectionContacts->update();
	}

void
WDashboard::RefreshGroup(TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	if (pGroup->PGetProfile() == m_pProfile_YZ)
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
	m_sections.pwSectionChannels->Refresh();
	}
void
WDashboard::RefreshGroups()
	{
	m_sections.pwSectionGroups->Refresh();
	}
void
WDashboard::RefreshGroupsOrChannel(TGroup * pGroupOrChannel)
	{
	if (pGroupOrChannel->Group_FuIsChannel())
		RefreshChannels();
	else
		RefreshGroups();
	}

void
WDashboard::RefreshContacts()
	{
	m_sections.pwSectionContacts->Refresh();
	}

void
WDashboard::RedrawContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	// At the moment, redraw the entire widget
	m_sections.pwSectionContacts->WidgetRedraw();
	}

void
WDashboard::RedrawGroup(TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	WDashboardSection * pwSection = pGroup->Group_FuIsChannel() ? (WDashboardSection *)m_sections.pwSectionChannels : (WDashboardSection *)m_sections.pwSectionGroups;
	pwSection->WidgetRedraw();
	}

void
WDashboard::BumpContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	RefreshContacts();	// Need to be optimized
	}

void
WDashboard::BumpTreeItem(ITreeItem * pTreeItem)
	{
	Assert(pTreeItem != NULL);
	if (pTreeItem->EGetRuntimeClass() == RTI(TGroup))
		{
		RefreshGroupsOrChannel((TGroup *)pTreeItem);
		}
	}


void
WDashboard::NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	Assert(pContactOrGroup_NZ != NULL);
	if (pContactOrGroup_NZ->PGetProfile() == m_pProfile_YZ)
		ProfileSelectedChanged(m_pProfile_YZ);	// At the moment, refresh the entire dashboard.  This is not efficient, but it works
	}

void
WDashboard::NewEventRelatedToBallot(IEventBallot * /*pEventBallot*/)
	{
	// Search if the ballot is already there, and if not, add it
	//m_sections.pwSectionBalots->m_arraypaItems
	}

#else

#endif // ~SIMPLE_DASHBOARD

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboardSection::SHitTestInfo WDashboardSection::c_oHitTestInfoEmpty;

WDashboardSection::WDashboardSection(PSZAC pszSectionName) : WWidget(NULL)
	{
	m_sName = pszSectionName;
	m_oHitTestInfo.pItem = NULL;
	m_cTotalItemsAvailable = d_zNA;
	setMouseTracking(true);
	}

WDashboardSection::~WDashboardSection()
	{
	//m_arraypaItems.DeleteAllItems();
	}

void
WDashboardSection::SetParent(WDashboard * pParent)
	{
	Assert(pParent != NULL);
	m_pParent = pParent;
	m_arraypaItems.DeleteAllItems();
	m_cTotalItemsAvailable = d_zNA;
	m_oHitTestInfo = c_oHitTestInfoEmpty;
	}

//	Re-initialize the section and redraw eve
void
WDashboardSection::Refresh()
	{
	SetParent(m_pParent);
	Init(m_pParent->PGetProfile_YZ());
	updateGeometry();
	update();
	}

void
WDashboardSection::Init(TProfile * UNUSED_PARAMETER(pProfile_YZ))
	{
	}

void
WDashboardSection::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * /*pvDataItem*/)
	{
	Assert(pPainter != NULL);
	}

void
WDashboardSection::DrawFooter(CPainterCell * pPainter, UINT uFlagsHitTest)
	{
	Assert(pPainter != NULL);
	int cMore = m_cTotalItemsAvailable - m_arraypaItems.GetSize();
	if (cMore > 0)
		g_strScratchBufferStatusBar.Format("+$I more...", cMore);
	else
		{
		if (m_strTextFooterIfItemsAvailableIsZero.FIsEmptyString())
			return;
		g_strScratchBufferStatusBar = m_strTextFooterIfItemsAvailableIsZero;
		}
	pPainter->DrawTextUnderlinedStyle(g_strScratchBufferStatusBar, (uFlagsHitTest & FHT_kfFooter) ? Qt::SolidLine : Qt::DotLine);
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
			NavigationTree_SelectTreeItem(oHitTestInfo.pItem->mu_data.piTreeItem);
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
	int cyHeight = 14 + d_yPosFirstItem + (m_arraypaItems.GetSize() * d_cyHeightSectionItem);
	if (m_cTotalItemsAvailable > m_arraypaItems.GetSize() || !m_strTextFooterIfItemsAvailableIsZero.FIsEmptyString())
		cyHeight += d_cyHeightFooter;
	return QSize(d_zNA, cyHeight);
	}
/*
QSize
WDashboardSection::minimumSizeHint()
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "WDashboardSection::minimumSizeHint()\n");
	return QSize(500, 500);
	}
*/
//	WDashboardSection::QWidget::heightForWidth()
int
WDashboardSection::heightForWidth(int cxWidth) const
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "WDashboardSection::heightForWidth($i)\n", cxWidth);
	Assert(FALSE && "Is this method ever called?");
	return QWidget::heightForWidth(cxWidth);
	}

//	WDashboardSection::QWidget::paintEvent()
void
WDashboardSection::paintEvent(QPaintEvent *)
	{
	CPainterCell oPainter(this);
	QRect rcSection = rect();
	//oPainter.drawRect(rcSection);

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
	g_oFontBoldSmaller.setWeight(99);
	oPainter.setFont(g_oFontBoldSmaller);
	oPainter.setPen((m_oHitTestInfo.uFlagsHitTest & FHT_kfHeader) ? g_oPenTextNotEmpty : g_oPenTextEmpty);
	oPainter.drawText(d_cxMarginSectionLeft, d_cyMarginTopHeader + d_cyHeightSectionItem, m_sName);

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
			OGetIcon((m_oHitTestInfo.uFlagsHitTest & FHT_kfMenuOverflow) ? eMenuIcon_OverflowHovered : eMenuIcon_Overflow).paint(&oPainter, xLeft, yTop, 16, 16);
			}
		oPainter.setPen((uFlagsItem & CDashboardSectionItem::FI_kfSelected) ? g_oPenTextNotEmpty : g_oPenTextEmpty);
		oPainter.setFont((uFlagsItem & CDashboardSectionItem::FI_kfDrawBold) ? g_oFontBold : g_oFontNormal);
		DrawItem(&oPainter, uFlagsItem, pItem->mu_data.pvDataItem);
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
WDashboardSection::_OnItemClicked_ITreeItem(SHitTestInfo oHitTestInfo, const EMenuActionByte rgzeMenuActions[])
	{
	Assert(rgzeMenuActions != NULL);
	if ((oHitTestInfo.uFlagsHitTest & FHT_kfMenuOverflow) && (oHitTestInfo.pItem != NULL))
		{
		ITreeItem * pTreeItem = oHitTestInfo.pItem->mu_data.piTreeItem;
		Assert(pTreeItem != NULL);
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
		pTreeItem->TreeItem_EDisplayContextMenu(rgzeMenuActions);
		return;
		}
	WDashboardSection::OnItemClicked(oHitTestInfo);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
void
Dashboard_RefreshAccordingToSelectedProfile(TProfile * pProfileSelected)
	{
	Endorse(pProfileSelected == NULL);
	Assert(g_pwDashboard != NULL);
	#ifdef SIMPLE_DASHBOARD
	g_pwDashboard->ProfileSelectedChanged(pProfileSelected);
	#else
	Assert(WDashboard::s_paiwDashboard_NZ != NULL);
	WDashboard::s_paiwDashboard_NZ->IDashboard_Notify();
	#endif
	}

//	Refresh the entire dashboard, by repopulating everything
void
Dashboard_RefreshAll()
	{
	g_pwDashboard->RefreshAll();
	}

void
Dashboard_UpdateContact(TContact * pContact)
	{
	Assert(g_pwDashboard != NULL);
	g_pwDashboard->RefreshContact(pContact);
	}

void
Dashboard_UpdateGroup(TGroup * pGroup)
	{
	Assert(g_pwDashboard != NULL);
	g_pwDashboard->RefreshGroup(pGroup);
	}

void
Dashboard_RefreshGroupsAndChannels()
	{
	Dashboard_RefreshGroups();
	Dashboard_RefreshChannels();
	}

void
Dashboard_RefreshGroups()
	{
	g_pwDashboard->RefreshGroups();
	}
void
Dashboard_RefreshChannels()
	{
	g_pwDashboard->RefreshChannels();
	}

void
Dashboard_RefreshContacts()
	{
	g_pwDashboard->RefreshContacts();
	}

void
Dashboard_RedrawContact(TContact * pContact)
	{
	g_pwDashboard->RedrawContact(pContact);
	}

void
Dashboard_RedrawGroup(TGroup * pGroup)
	{
	g_pwDashboard->RedrawGroup(pGroup);
	}

void
Dashboard_BumpContact(TContact * pContact)
	{
	g_pwDashboard->BumpContact(pContact);
	}

void
Dashboard_BumpTreeItem(ITreeItem * pTreeItem)
	{
	g_pwDashboard->BumpTreeItem(pTreeItem);
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
	Assert(g_pwDashboard != NULL);
	g_pwDashboard->NewEventRelatedToBallot(pEventBallot);
	}

void
Dashboard_RemoveSelection()
	{
	Assert(g_pwDashboard != NULL);
	(void)g_pwDashboard->FSelectItem(NULL);
	}
