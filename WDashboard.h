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

public:
	CPainterCell(QPaintDevice * poPaintDevice): OPainter(poPaintDevice) { }

	void DrawTextWithinCell(const QString & sText);
	void DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...);
	int DrawNumberWithinCircle(int nNumber);
};


//	Interface to draw one item on the dashboard
class IDashboardSectionItem	// (item)
{
public:
	UINT m_uFlags;

public:
	IDashboardSectionItem() { m_uFlags = 0; }
	virtual ~IDashboardSectionItem() { }
	virtual void DrawItemText(CPainterCell * pPainter) = 0;
	virtual int DrawItemIcons(CPainterCell * pPainter) = 0;
};

class CDashboardSectionItem_ITreeItem : public IDashboardSectionItem
{
public:
	ITreeItem * m_piTreeItem;
public:
	CDashboardSectionItem_ITreeItem(ITreeItem * piTreeItem) { m_piTreeItem = piTreeItem; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};

class CDashboardSectionItem_TGroup : public IDashboardSectionItem
{
public:
	TGroup * m_pGroup;

public:
	CDashboardSectionItem_TGroup(TGroup * pGroup) { m_pGroup = pGroup; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};

class CDashboardSectionItem_TContact : public IDashboardSectionItem
{
public:
	TContact * m_pContact;

public:
	CDashboardSectionItem_TContact(TContact * pContact) { m_pContact = pContact; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};


class CArrayPtrDashboardSectionItems : private CArray
{
public:
	inline IDashboardSectionItem ** PrgpGetItemsStop(OUT IDashboardSectionItem *** pppItemStop) const { return (IDashboardSectionItem **)PrgpvGetElementsStop(OUT (void ***)pppItemStop); }
	inline int GetSize() const { return CArray::GetSize(); }
	void DeleteAllItems();
	void AddItem(IDashboardSectionItem * pItem);
	void AllocateItemForTreeItem(ITreeItem * piTreeItem);

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

	// From QWidget
	virtual QSize sizeHint() const;
	virtual int heightForWidth(int cxWidth) const;
	virtual void paintEvent(QPaintEvent *);
};

class CArrayPtrDashboardSections : public CArray
{
public:
	inline WDashboardSection ** PrgpGetSectionsStop(OUT WDashboardSection *** pppSectionStop) const { return (WDashboardSection **)PrgpvGetElementsStop(OUT (void ***)pppSectionStop); }
};

//	The dashboard is a widget showing recent events related to a profile
singleton WDashboard : public QDockWidget
{
	Q_OBJECT
protected:
	WLabel * m_pwLabelCaption;
	OLayoutVerticalAlignTop * m_poLayoutVertial;	// Stack the sections vertically
	CArrayPtrDashboardSections m_arraypSections;

public:
	WDashboard();
	void AddSection(PA_CHILD WDashboardSection * pawSection);
	void ProfileSelectedChanged(TProfile * pProfile);
};

#endif // WDASHBOARD_H
