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
	nNumber = qrand() % 10;
	if (nNumber > 0)
		{
		QRect rc = m_rcCell;
		rc.setLeft(m_rcCell.right() - 16);
		QPoint ptCenter = rc.center();
		setPen(g_oPenDot);
		ptCenter.setY(ptCenter.y() + 2);
		drawPoint(ptCenter);
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

class WDashboardSectionGroups : public WDashboardSection
{
public:
	WDashboardSectionGroups(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
};

void
WDashboardSectionGroups::InitItems(TProfile * pProfile)
	{
	CArrayPtrGroups arraypGroups;
	pProfile->GetRecentGroups(OUT &arraypGroups);
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = arraypGroups.PrgpGetGroupsStopMax(OUT &ppGroupStop, 5);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		m_arraypaItems.AddItem(new CDashboardSectionItem_TGroup(pGroup));
		}
	}

class WDashboardSectionContacts : public WDashboardSection
{
public:
	WDashboardSectionContacts(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
};

void
WDashboardSectionContacts::InitItems(TProfile * pProfile)
	{
	CArrayPtrContacts arraypContacts;
	pProfile->GetRecentContacts(OUT &arraypContacts);
	TContact ** ppContactStop;
	TContact ** ppContact = arraypContacts.PrgpGetContactsStopMax(OUT &ppContactStop, 5);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		m_arraypaItems.AddItem(new CDashboardSectionItem_TContact(pContact));
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
	setObjectName("Dashboard");
	/*
	setWindowTitle("Central Services Inc.");
	setStyleSheet("background-color:#8080FF;");
	*/
	g_oFontBold = g_oFontNormal = font();
	g_oFontBold.setWeight(QFont::Bold);
	//setFont(g_oFontBold);
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
	AddSection(new WDashboardSectionGroups("Ballots"));
	AddSection(new WDashboardSectionGroups("Channels"));
	AddSection(new WDashboardSectionContacts("Peers"));
	AddSection(new WDashboardSectionGroups("Private Groups"));
	}

void
WDashboard::AddSection(PA_CHILD WDashboardSection * pawSection)
	{
	Assert(pawSection != NULL);
	m_arraypSections.Add(pawSection);
	m_poLayoutVertial->addWidget(pawSection);
	}

void
WDashboard::ProfileSelectedChanged(TProfile * pProfile)
	{
	m_pwLabelCaption->Label_SetTextPlain((pProfile != NULL) ? pProfile->m_strNameProfile : c_strEmpty);
	// Notify each section the selected profile changed
	WDashboardSection ** ppSectionStop;
	WDashboardSection ** ppSection = m_arraypSections.PrgpGetSectionsStop(OUT &ppSectionStop);
	while (ppSection != ppSectionStop)
		{
		WDashboardSection * pSection = *ppSection++;
		pSection->m_arraypaItems.DeleteAllItems();
		if (pProfile != NULL)
			pSection->InitItems(pProfile);
		pSection->updateGeometry();
		}
	//m_poLayoutVertial->invalidate();
	//updateGeometry();
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

	#if 0
	CStr strName;
	TProfile * pProfile = g_oConfiguration.m_pProfileSelected;
	if (pProfile != NULL)
		{
		TAccountXmpp * pAccount = pProfile->m_arraypaAccountsXmpp.PGetAccountFirst_YZ();	// Temporary fix until we have real OT IDs
		if (pAccount != NULL)
			{
			int cGroups = 0;
			TGroup ** ppGroupStop;
			TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
			while (ppGroup != ppGroupStop)
				{
				TGroup * pGroup = *ppGroup++;
				strName.Format("#$s", pGroup->TreeItem_PszGetNameDisplay());
				//p.drawRect(rcItem);
				oPainter.drawText(IN rcItemText, Qt::AlignVCenter, strName);
				strName.Format("$i", pGroup->m_arraypaMembers.GetSize());

				oPainter.setPen(oPenDot);
				//oPainter.drawEllipse(rcItemNumber);
				oPainter.drawPoint(xDot, yTop + d_cyHeightSectionItem / 2 + 1);
				oPainter.setPen(oPenDefault);
				//oPainter.drawText(xDot, yTop + d_cyHeightSectionItem / 2, strName);
				oPainter.drawText(rcItemNumber, Qt::AlignVCenter | Qt::AlignCenter, strName);


				//p.drawText(4, yTop, strName);
				/*
				rcItem.setTop(yTop);
				yTop += d_cyHeightSectionItem;
				rcItem.setBottom(yTop);
				*/
				yTop += d_cyHeightSectionItem;
				rcItemText.moveTop(yTop);
				rcItemNumber.moveTop(yTop);
				//rcItem.moveCenter(d_cyHeightSectionItem
				if (cGroups++ > 2)
					break;
				}
			}
	//p.drawText(3, 10, Qt::AlignBottom, m_sName);
		}
	#endif
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
