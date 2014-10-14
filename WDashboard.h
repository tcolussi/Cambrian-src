#ifndef WDASHBOARD_H
#define WDASHBOARD_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WDashboard;

//	Class having a 'boundary rectangle' where the painter is allowed to draw.
//	A typical use of this class is to paint a cell in a grid.
class CPainterCell : public OPainter
{
public:
	QRect m_rcCell;

public:
	CPainterCell(QPaintDevice * poPaintDevice): OPainter(poPaintDevice) { }

	void DrawTextWithinCell(const QString & sText);
	void DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...);
	void DrawTextUnderlinedStyle(const QString & sText, Qt::PenStyle eStyle);
	int DrawNumberWithinCircle(int nNumber);
	void DrawIconLeft(const QIcon & oIcon);
	void DrawIconLeft(EMenuIcon eMenuIcon);
	void FillRect0(QRGB coBackgroundFill);
};

//	Interface to draw one item on the dashboard
class CDashboardSectionItem	// (item)
{
public:
	enum
		{
		FI_keDataType_zUnknown		= 0x0000,
		FI_keDataType_ITreeItem		= 0x0001,	// The content of m_data contains a pointer to ITreeItem
		FI_keDataType_IEvent		= 0x0002,	// The content of m_data contains a pointer to IEvent
		FI_kmDataTypeMask			= 0x000F,

		FI_kfMouseHover				= 0x0010,	// The mouse is hovering over the item
		FI_kfSelected				= 0x0020,
		FI_kfDrawBold				= 0x0100,	// Draw the item in bold
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
		} mu_data;

public:
	CDashboardSectionItem(void * pvDataItem) { m_uFlagsItem = 0; mu_data.pvDataItem = pvDataItem; }
};

class CArrayPtrDashboardSectionItems : private CArray
{
public:
	inline CDashboardSectionItem ** PrgpGetItemsStop(OUT CDashboardSectionItem *** pppItemStop) const { return (CDashboardSectionItem **)PrgpvGetElementsStop(OUT (void ***)pppItemStop); }
	inline int GetSize() const { return CArray::GetSize(); }

	void DeleteAllItems();
	void AllocateItem(void * pvDataItem);
	void AllocateTreeItems(const CArrayPtrTreeItems & arraypTreeItems, int cDataItemsMax);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class WDashboardSection : public WWidget
{
protected:
	WDashboard * m_pParent;	// This pointer is necessary when an item is clicked, the other item(s) selected from other section(s) are unselected.
	CString m_sName;		// Name of the section
	CArrayPtrDashboardSectionItems m_arraypaItems;	// Items to draw
	int m_cTotalItemsAvailable;						// Total number of items (the section will display "$i more..." if the total available is larger than the number of items to draw
	CStr m_strTextFooterIfItemsAvailableIsZero;		// Text to display in the footer if m_cTotalItemsAvailable <= 0.  Typically this will display "New ..."

	//	Extra information regarding the hit-testing
	enum
		{
		FHT_zNone			= 0x0000,
		FHT_kfHeader		= 0x0001,
		FHT_kfFooter		= 0x0002,
		FHT_kfMenuOverflow	= 0x0004
		};
	struct SHitTestInfo		//	Structure holding information about hit testing for a WDashboardSection
		{
		UINT uFlagsHitTest;
		CDashboardSectionItem * pItem;	// Which item was selected (if any)
		};
	SHitTestInfo m_oHitTestInfo;						// Information about the last known hit testing, such as which item is under the mouse.  This structure is necessary to draw the section.
public:
	WDashboardSection(PSZAC pszSectionName);
	~WDashboardSection();
	void SetParent(WDashboard * pParent);
	void Refresh();
	void WidgetRedraw() { update(); }
	virtual void Init(TProfile * pProfile_YZ);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvDataItem);
	virtual void DrawFooter(CPainterCell * pPainter, UINT uFlagsHitTest);
	virtual void OnItemClicked(SHitTestInfo oHitTestInfo);
	//virtual void OnMenuClicked(SHitTestInfo oHitTestInfo);

protected:
	virtual QSize sizeHint() const;					// From QWidget
	//virtual QSize minimumSizeHint();
	virtual int heightForWidth(int cxWidth) const;	// From QWidget
	virtual void paintEvent(QPaintEvent *);			// From QWidget
	virtual void mouseMoveEvent(QMouseEvent * pEventMouse);
	virtual void mouseReleaseEvent(QMouseEvent * pEventMouse);
	virtual void leaveEvent(QEvent *);

	SHitTestInfo OGetHitTestInfo(QMouseEvent * pEventMouse) const;
	SHitTestInfo OGetHitTestInfo(int xPos, int yPos) const;
	BOOL FSetHitTestInfo(SHitTestInfo oHitTestInfo);

	void _OnItemClicked_ITreeItem(SHitTestInfo oHitTestInfo, const EMenuActionByte rgzeMenuActions[]);

	static SHitTestInfo c_oHitTestInfoEmpty;
}; // WDashboardSection

class CArrayPtrDashboardSections : public CArray
{
public:
	inline WDashboardSection ** PrgpGetSectionsStop(OUT WDashboardSection *** pppSectionStop) const { return (WDashboardSection **)PrgpvGetElementsStop(OUT (void ***)pppSectionStop); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WDashboardSection_TGroup : public WDashboardSection
{
protected:
	EMenuIcon m_eMenuIcon;

public:
	WDashboardSection_TGroup(PSZAC pszGroupName, EMenuIcon eMenuIcon);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvGroup);
};

class WDashboardSectionChannels : public WDashboardSection_TGroup
{
public:
	WDashboardSectionChannels();
	virtual void Init(TProfile * pProfile_YZ);
	virtual void OnItemClicked(SHitTestInfo oHitTestInfo);
};

class WDashboardSectionGroups : public WDashboardSection_TGroup
{
public:
	WDashboardSectionGroups();
	virtual void Init(TProfile * pProfile_YZ);
	virtual void OnItemClicked(SHitTestInfo oHitTestInfo);
};

class WDashboardSectionCorporations : public WDashboardSection_TGroup
{
public:
	WDashboardSectionCorporations();
	virtual void Init(TProfile * pProfile_YZ);
	virtual void OnItemClicked(SHitTestInfo oHitTestInfo);
};

class WDashboardSectionContacts : public WDashboardSection
{
public:
	WDashboardSectionContacts(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void Init(TProfile * pProfile_YZ);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvContact);
	virtual void OnItemClicked(SHitTestInfo oHitTestInfo);
};

class WDashboardSectionBallots : public WDashboardSection
{
public:
	WDashboardSectionBallots(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void Init(TProfile * pProfile_YZ);
	virtual void OnItemClicked(SHitTestInfo oHitTestInfo);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	The dashboard is a widget showing recent events related to a profile
singleton WDashboard : public QDockWidget
{
	Q_OBJECT
protected:
	TProfile * m_pProfile_YZ;							// Pointer of the profile the dashboard is displaying
	OLayoutVerticalAlignTop * m_poLayoutVertial;	// Stack the sections vertically
	struct	// Contain one pointer per section.  Those pointers are for a quick access to a section.  To change the order of the sections, change their order in this structure.
		{
		WDashboardSectionChannels * pwSectionChannels;
		WDashboardSectionContacts * pwSectionContacts;
		WDashboardSectionGroups * pwSectionGroups;		// Private groups
		WDashboardSectionCorporations * pwSectionCorporations;	// Corporations & Coalitions
		WDashboardSectionBallots * pwSectionBalots;
		} m_sections;
	CDashboardSectionItem * m_pItemSelected_YZ;		// Which item is selected (has the focus)

public:
	WDashboard();
	inline TProfile * PGetProfile_YZ() const { return m_pProfile_YZ; }
	void ProfileSelectedChanged(TProfile * pProfile);
	void RefreshAll();
	void RefreshContact(TContact * pContact);
	void RefreshGroup(TGroup * pGroup);
	void RefreshChannels();
	void RefreshGroups();
	void RefreshGroupsOrChannel(TGroup * pGroupOrChannel);
	void RefreshContacts();
	void RedrawContact(TContact * pContact);
	void RedrawGroup(TGroup * pGroup);
	void BumpContact(TContact * pContact);
	void BumpTreeItem(ITreeItem * pTreeItem);
	void NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ);
	void NewEventRelatedToBallot(IEventBallot * pEventBallot);

	BOOL FSelectItem(CDashboardSectionItem * pItem);

	//virtual QSize sizeHint() const { return QSize(300, 0); }
}; // WDashboard

#endif // WDASHBOARD_H
