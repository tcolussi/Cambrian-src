#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WDashboard.h"

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

QFont g_oFontNormal;
QFont g_oFontBold;
QPen g_oPenDot;		// Pen to draw a dot to display a number within
QPen g_oPenDefault;

class WDashboardSectionGroups : public WDashboardSection
{
public:
	WDashboardSectionGroups(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
};

void
WDashboardSectionGroups::InitItems(TProfile * pProfile)
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
			m_arraypaItems.AllocateItemForTreeItem(pGroup);
			if (cGroups++ > 5)
				break;
			}
		}
	}

void
CDashboardSectionItem_ITreeItem::DrawItemText(CPainterCell * pPainter)
	{
	g_strScratchBufferStatusBar.Format("# $s", m_piTreeItem->TreeItem_PszGetNameDisplay());
	pPainter->drawText(IN pPainter->m_rcCell, Qt::AlignVCenter, g_strScratchBufferStatusBar);
	}

int
CDashboardSectionItem_ITreeItem::DrawItemIcons(CPainterCell * pPainter)
	{
	int cItems = 10; // pGroup->m_arraypaMembers.GetSize();
	if (cItems > 0)
		{
		pPainter->m_rcCell.setLeft(pPainter->m_rcCell.right() - 16);
		pPainter->setPen(g_oPenDot);
		QPoint ptCenter = pPainter->m_rcCell.center();
		ptCenter.setY(ptCenter.y() + 2);
		pPainter->drawPoint(ptCenter);
		pPainter->setPen(g_oPenDefault);
		pPainter->drawText(pPainter->m_rcCell, Qt::AlignVCenter | Qt::AlignCenter, QString::number(cItems));
		return 16;
		}
	return 0;
	}

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
	QLabel * pwLabel = new QLabel("Central Services Inc.");
	pwLabel->setStyleSheet("background-color:#8080FF;");
	pwLabel->setMargin(3);
	pwLabel->setFont(g_oFontBold);;
	setTitleBarWidget(pwLabel);

	QWidget * pwWidgetDashboard = new QWidget;	// Main widget for the dashboard
	pwWidgetDashboard->setStyleSheet("background-color:#A0A0FF;");
	setWidget(PA_CHILD pwWidgetDashboard);

	m_poLayoutVertial = new OLayoutVerticalAlignTop(pwWidgetDashboard);
	Layout_MarginsClear(INOUT m_poLayoutVertial);
	m_poLayoutVertial->setSpacing(0);
	AddSection(new WDashboardSectionGroups("Ballots"));
	AddSection(new WDashboardSectionGroups("Channels"));
	AddSection(new WDashboardSectionGroups("Peers"));
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
	// Notify each section the selected profile changed
	WDashboardSection ** ppSectionStop;
	WDashboardSection ** ppSection = m_arraypSections.PrgpGetSectionsStop(OUT &ppSectionStop);
	while (ppSection != ppSectionStop)
		{
		WDashboardSection * pSection = *ppSection++;
		pSection->hide();
		pSection->show();	// Temporary hack to update the geometry
		pSection->m_arraypaItems.DeleteAllItems();
		if (pProfile != NULL)
			pSection->InitItems(pProfile);
		}
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
	return QSize(0, m_arraypaItems.GetSize() * d_cyHeightSectionItem + 24);
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
	//oPainter.drawRect(0, 0, width() - 1, height() - 1);
	//oPainter.drawPoint(10, 10);

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
