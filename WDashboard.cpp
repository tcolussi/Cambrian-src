#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WDashboard.h"

QFont g_oFontNormal;
QFont g_oFontBold;
QPen g_oPenDot;		// Pen to draw a dot to display a number within
QPen g_oPenDefault;


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

//	Return the number of pixels of the drawing.  This is useful to chain drawing.
int
CPainterCell::DrawNumberWithinCircle(int nNumber)
	{
//	nNumber = qrand() % 100;
	if (nNumber > 0)
		{
		QRect rc = m_rcCell;
		rc.setLeft(m_rcCell.right() - 16);
		QPoint ptCenter = rc.center();
		setPen(g_oPenDot);
		ptCenter.setY(ptCenter.y() + 2);
		drawPoint(ptCenter);
		//DrawLineHorizontal(rc.left(), rc.right(), ptCenter.y());	// Draw a longer line for large numbers
		setPen(g_oPenDefault);
		drawText(rc, Qt::AlignVCenter | Qt::AlignCenter, QString::number(nNumber));
		return 16;
		}
	return 0;
	}


WDashboard * g_pwDashboard;

void
CArrayPtrDashboardSectionItems::DeleteAllItems()
	{
	IDashboardSectionItem ** ppItemStop;
	IDashboardSectionItem ** ppItem = PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		IDashboardSectionItem * pItem = *ppItem++;
		delete pItem;
		}
	RemoveAllElements();
	}

void
CArrayPtrDashboardSectionItems::AddItem(IDashboardSectionItem * pItem)
	{
	Assert(pItem != NULL);
	Add(pItem);
	}

void
CArrayPtrDashboardSectionItems::AllocateItemForTreeItem(ITreeItem * piTreeItem)
	{
	Assert(piTreeItem != NULL);
	Add(PA_CHILD new CDashboardSectionItem_ITreeItem(piTreeItem));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WDashboardSectionGroups::InitItems(TProfile * pProfile)
	{
	CArrayPtrGroups arraypGroups;
	pProfile->GetRecentGroups(OUT &arraypGroups);
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = arraypGroups.PrgpGetGroupsStopMax(OUT &ppGroupStop, 10);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		m_arraypaItems.AddItem(new CDashboardSectionItem_TGroup(pGroup));
		}
	}

void
WDashboardSectionContacts::InitItems(TProfile * pProfile)
	{
	CArrayPtrContacts arraypContacts;
	pProfile->GetRecentContacts(OUT &arraypContacts);
	TContact ** ppContactStop;
	TContact ** ppContact = arraypContacts.PrgpGetContactsStopMax(OUT &ppContactStop, 10);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		m_arraypaItems.AddItem(new CDashboardSectionItem_TContact(pContact));
		}
	}

void
WDashboardSectionBallots::InitItems(TProfile * pProfile)
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = pProfile->m_arraypEventsRecentBallots.PrgpGetEventsStopLast(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		m_arraypaItems.AddItem(new CDashboardSectionItem_IEventBallot((IEventBallot *)pEvent));
		}
	}

void
CDashboardSectionItem_ITreeItem::DrawItemText(CPainterCell * pPainter)
	{
	pPainter->DrawTextWithinCell_VE("? $s", m_piTreeItem->TreeItem_PszGetNameDisplay());
	}

int
CDashboardSectionItem_ITreeItem::DrawItemIcons(CPainterCell * pPainter)
	{
	return pPainter->DrawNumberWithinCircle(qrand() % 10);	// Display a random number for demo purpose
	}

void
CDashboardSectionItem_TGroup::DrawItemText(CPainterCell * pPainter)
	{
	pPainter->DrawTextWithinCell_VE("# $s", m_pGroup->TreeItem_PszGetNameDisplay());
	}
int
CDashboardSectionItem_TGroup::DrawItemIcons(CPainterCell * pPainter)
	{
	Assert(m_pGroup != NULL);
	Assert(m_pGroup->EGetRuntimeClass() == RTI(TGroup));
	return pPainter->DrawNumberWithinCircle(m_pGroup->m_cMessagesUnread);
	}

void
CDashboardSectionItem_TContact::DrawItemText(CPainterCell * pPainter)
	{
	pPainter->DrawTextWithinCell_VE("$s", m_pContact->TreeItem_PszGetNameDisplay());
	}
int
CDashboardSectionItem_TContact::DrawItemIcons(CPainterCell * pPainter)
	{
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	return pPainter->DrawNumberWithinCircle(m_pContact->m_cMessagesUnread);
	}

void
CDashboardSectionItem_IEventBallot::DrawItemText(CPainterCell * pPainter)
	{
	pPainter->DrawTextWithinCell_VE("$S", &m_pBallot->m_strTitle);
	}
int
CDashboardSectionItem_IEventBallot::DrawItemIcons(CPainterCell * pPainter)
	{
	return 0;
	}


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
	setObjectName("Dashboard");
	g_oFontBold = g_oFontNormal = font();
	g_oFontBold.setWeight(QFont::Bold);

	m_pwLabelCaption = new WLabel;
	m_pwLabelCaption->setStyleSheet("background-color:#8080FF;");
	m_pwLabelCaption->setMargin(3);
	m_pwLabelCaption->setFont(g_oFontBold);
	#if 1
	setTitleBarWidget(m_pwLabelCaption);
	#else
	setTitleBarWidget(new WDashboardCaption);
	#endif

	QWidget * pwWidgetDashboard = new QWidget;	// Main widget for the dashboard
	pwWidgetDashboard->setStyleSheet("background-color:#A0A0FF;");
	setWidget(PA_CHILD pwWidgetDashboard);
	m_poLayoutVertial = new OLayoutVerticalAlignTop(pwWidgetDashboard);
	Layout_MarginsClear(INOUT m_poLayoutVertial);
	m_poLayoutVertial->setSpacing(0);

	InitToGarbage(OUT &m_sections, sizeof(m_sections));
	m_sections.pwSectionBalots = new WDashboardSectionBallots("Ballots");
	m_sections.pwSectionGroups = new WDashboardSectionGroups("Channels");
	m_sections.pwSectionContacts = new WDashboardSectionContacts("Peers");

	// Add each section to the vertical layout
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		m_poLayoutVertial->addWidget(*ppwSection);

	/*
	AddSection(new WDashboardSectionGroups("Ballots"));
	AddSection(new WDashboardSectionGroups("Channels"));
	AddSection(new WDashboardSectionContacts("Peers"));
	AddSection(new WDashboardSectionGroups("Private Groups"));
	*/
	}
/*
void
WDashboard::AddSection(PA_CHILD WDashboardSection * pawSection)
	{
	Assert(pawSection != NULL);
	m_arraypSections.Add(pawSection);
	m_poLayoutVertial->addWidget(pawSection);
	}
*/

void
WDashboard::ProfileSelectedChanged(TProfile * pProfile)
	{
	m_pProfile = pProfile;
	m_pwLabelCaption->Label_SetTextPlain((pProfile != NULL) ? pProfile->m_strNameProfile : c_strEmpty);
	// Notify each section the selected profile changed
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		{
		WDashboardSection * pwSection = *ppwSection;
		pwSection->m_arraypaItems.DeleteAllItems();
		if (pProfile != NULL)
			pwSection->InitItems(pProfile);
		pwSection->updateGeometry();
		}
	//m_poLayoutVertial->invalidate();
	//updateGeometry();
	repaint();
	}

void
WDashboard::NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	Assert(pContactOrGroup_NZ != NULL);
	if (pContactOrGroup_NZ->PGetProfile() == m_pProfile)
		ProfileSelectedChanged(m_pProfile);	// At the moment, refresh the entire thing.  This is not efficient, but it works
	}

void
WDashboard::NewEventRelatedToBallot(IEventBallot * pEventBallot)
	{
	// Search if the ballot is already there, and if not, add it
	//m_sections.pwSectionBalots->m_arraypaItems
	}

void
WDashboard::RefreshContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (pContact->PGetProfile() == m_pProfile)
		m_sections.pwSectionContacts->repaint();
	}

void
WDashboard::RefreshGroup(TGroup * pGroup)
	{
	if (pGroup->PGetProfile() == m_pProfile)
		m_sections.pwSectionGroups->repaint();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboardSection::WDashboardSection(PSZAC pszSectionName) : WWidget(NULL)
	{
	m_sName = pszSectionName;
	}

WDashboardSection::~WDashboardSection()
	{
	//m_arraypaItems.DeleteAllItems();
	}

void
WDashboardSection::InitItems(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	}

#define d_cxMarginSection		4
#define d_cxWidthNumber			20
#define d_cyHeightSectionItem	16

QSize
WDashboardSection::sizeHint() const
	{
	return QSize(150, m_arraypaItems.GetSize() * d_cyHeightSectionItem + 24);
	}

//	WDashboardSection::QWidget::heightForWidth()
int
WDashboardSection::heightForWidth(int cxWidth) const
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "WDashboardSection::heightForWidth($i)\n", cxWidth);
	return QWidget::heightForWidth(cxWidth);
	}

//	WDashboardSection::QWidget::paintEvent()
void
WDashboardSection::paintEvent(QPaintEvent *)
	{
	CPainterCell oPainter(this);
//	oPainter.drawRect(0, 0, width() - 1, height() - 1);
	g_oPenDefault = oPainter.pen();
	#if 1
	oPainter.setRenderHint(QPainter::Antialiasing,true);
	oPainter.setRenderHint(QPainter::SmoothPixmapTransform,true);
	#endif
	g_oPenDot.setColor(d_coOrange);
	g_oPenDot.setWidth(16);
	g_oPenDot.setCapStyle(Qt::RoundCap);
//	oPainter.setOpacity(0.7);

	QRect rcSection = rect();
	qreal yTop = 3 + d_cyHeightSectionItem;
	qreal xRight = rcSection.width();
	oPainter.m_rcCell.setLeft(d_cxMarginSection);
	oPainter.m_rcCell.setRight(xRight);
	oPainter.m_rcCell.setTop(3 + d_cyHeightSectionItem);
	oPainter.m_rcCell.setHeight(d_cyHeightSectionItem);
	IDashboardSectionItem ** ppItemStop;
	IDashboardSectionItem ** ppItem = m_arraypaItems.PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		IDashboardSectionItem * pItem = *ppItem++;
		oPainter.m_rcCell.setRight(xRight);
		//oPainter.drawRect(oPainter.m_rcCell);
		int cxWidthIcons = pItem->DrawItemIcons(&oPainter);
		oPainter.m_rcCell.setLeft(d_cxMarginSection);
		oPainter.m_rcCell.setRight(xRight - cxWidthIcons);
		pItem->DrawItemText(&oPainter);

		yTop += d_cyHeightSectionItem;
		oPainter.m_rcCell.moveTop(yTop);
		}

	// Draw the caption last using the bold font
	oPainter.setFont(g_oFontBold);
	/*
	rcSection.setHeight(d_cyHeightSectionItem);
	p.drawText(d_cxMarginSection, IN rcSection, Qt::AlignVCenter, m_sName);
	*/
	oPainter.drawText(d_cxMarginSection, d_cyHeightSectionItem, m_sName);
	oPainter.drawLine(rcSection.bottomLeft(), rcSection.bottomRight());
	/*
	oPainter.drawRect( 10, 10, 85, 35 );
	oPainter.drawRoundRect( 10, 55, 85, 35 );
	QRect rect( 105, 10, 85, 35 );
	oPainter.drawRoundRect( rect );
	oPainter.drawRect( rect.translated( 0, 45 ) );
	*/
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
	if (g_pwDashboard != NULL)
		g_pwDashboard->RefreshGroup(pGroup);
	}

void
Dashboard_NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->NewEventsFromContactOrGroup(pContactOrGroup_NZ);
	}

void
Dashboard_NewEventRelatedToBallot(IEventBallot * pEventBallot)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->NewEventRelatedToBallot(pEventBallot);
	}
