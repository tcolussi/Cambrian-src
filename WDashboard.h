#ifndef WDASHBOARD_H
#define WDASHBOARD_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Class having a 'boundary rectangle' where the painter is allowed to draw.
//	A typical use of this class is to paint a cell in a grid.
class CPainterCell : public OPainter
{
public:
	QRect m_rcCell;
	int m_yCenter;		// Center point of the cell.

public:
	CPainterCell(QPaintDevice * poPaintDevice): OPainter(poPaintDevice) { }

	void DrawTextWithinCell(const QString & sText);
	void DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...);
	int DrawNumberWithinCircle(int nNumber);
	void DrawIconLeft(const QIcon & oIcon);
};


//	Interface to draw one item on the dashboard
class CDashboardSectionItem	// (item)
{
public:
	enum
		{
		FI_kfDrawBold		= 0x0001,	// Draw the item in bold
		FI_kfSelected		= 0x0002

		};
	UINT m_uFlagsItem;
	union
		{
		void * pvDataItem;			// Generic pointer for the data related to the item

		ITreeItem * piTreeItem;
		TContact * pContact;
		TGroup * pGroup;

		IEvent * pEvent;
		IEventBallot * pEventBallot;
		} m_data;

public:
	CDashboardSectionItem(void * pvDataItem) { m_uFlagsItem = 0; m_data.pvDataItem = pvDataItem; }
};

class CArrayPtrDashboardSectionItems : private CArray
{
public:
	inline CDashboardSectionItem ** PrgpGetItemsStop(OUT CDashboardSectionItem *** pppItemStop) const { return (CDashboardSectionItem **)PrgpvGetElementsStop(OUT (void ***)pppItemStop); }
	inline int GetSize() const { return CArray::GetSize(); }

	void DeleteAllItems();
	void AllocateItem(void * pvDataItem);
	void AllocateItems(const CArray & arraypDataItems, int cDataItemsMax);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WDashboardSection : public WWidget
{
public:
	CString m_sName;		// Name of the section
	CArrayPtrDashboardSectionItems m_arraypaItems;	// Items to draw

public:
	WDashboardSection(PSZAC pszSectionName);
	~WDashboardSection();
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvDataItem);

	virtual void paintEvent(QPaintEvent *);			// From QWidget
	virtual QSize sizeHint() const;					// From QWidget
	virtual int heightForWidth(int cxWidth) const;	// From QWidget
};

class CArrayPtrDashboardSections : public CArray
{
public:
	inline WDashboardSection ** PrgpGetSectionsStop(OUT WDashboardSection *** pppSectionStop) const { return (WDashboardSection **)PrgpvGetElementsStop(OUT (void ***)pppSectionStop); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WDashboardSectionGroups : public WDashboardSection
{
public:
	WDashboardSectionGroups(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvGroup);
};

class WDashboardSectionContacts : public WDashboardSection
{
public:
	WDashboardSectionContacts(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvContact);
};

class WDashboardSectionBallots : public WDashboardSection
{
public:
	WDashboardSectionBallots(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	The dashboard is a widget showing recent events related to a profile
singleton WDashboard : public QDockWidget
{
	Q_OBJECT
protected:
	TProfile * m_pProfile;							// Pointer of the profile the dashboard is displaying
	WLabel * m_pwLabelCaption;
	OLayoutVerticalAlignTop * m_poLayoutVertial;	// Stack the sections vertically
//	CArrayPtrDashboardSections m_arraypSections;
	struct	// Contain one pointer per section.  Those pointers are for a quick access to a section
		{
		WDashboardSectionBallots * pwSectionBalots;
		WDashboardSectionGroups * pwSectionGroups;
		WDashboardSectionContacts * pwSectionContacts;
		} m_sections;

public:
	WDashboard();
	void ProfileSelectedChanged(TProfile * pProfile);
	void NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ);
	void NewEventRelatedToBallot(IEventBallot * pEventBallot);
	void RefreshContact(TContact * pContact);
	void RefreshGroup(TGroup * pGroup);
}; // WDashboard

#endif // WDASHBOARD_H
