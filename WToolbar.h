//	WToolbar.h
//
//	Classes to draw the main toolbar of SocietyPro.
//	The toolbar is divided into two sections: tabs and buttons.
//
#ifndef WTOOLBAR_H
#define WTOOLBAR_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QToolBar>

//	The total height of the toolbar is the sum of the following value
#define d_cyHeightToolbarTabs		(25+2)			// Height of the widget containing the tabs
#define d_cyHeightToolbarButtons	(40+1)			// Height of the widget containing the toolbar buttons

class CTab
{
public:
	CStr m_strName;				// Name of the tab
	ITreeItem * m_pTreeItem;	// Tree Item associated with the tab (typically a TProfile or TContact, but it can be any ITreeItem)
	int m_cxWidth;				// Width of the tab
public:
	CTab(PSZUC pszName, ITreeItem * pTreeItem);
};

class CArrayPtrTabs : public CArray
{
public:
	inline CTab ** PrgpGetTabsStop(OUT CTab *** pppTabStop) const { return (CTab **)PrgpvGetElementsStop(OUT (void ***)pppTabStop); }
	void DeleteAllTabs();
};


class WTabs : public QWidget
{
protected:
	CArrayPtrTabs m_arraypaTabs;
	CTab * m_pdTabSelected;	// Tab which is selected
	CTab * m_pdTabHover;		// Tab where the mouse is
	enum EHitTest
		{
		eHitTest_zNone				= 0x0000,
		eHitTest_kfButtonClose		= 0x0001,	// The close button (to remove the tab)
		eHitTest_kfButtonNewTab		= 0x0010,	// The button to add a new tab

		// These are used only with m_uFlagsHitTest
		eHitTest_kfMouseHovering	= 0x0020,
		eHitTest_kfMousePressed		= 0x0040,

		eHitTest_kmButtonNewTabHovering = eHitTest_kfButtonNewTab | eHitTest_kfMouseHovering,

		};
	UINT m_uFlagsHitTest;			// Various combination of EHitTest

public:
	WTabs();
	~WTabs();
	CTab * TabAddP(PSZAC pszName, ITreeItem * pTreeItem = NULL);
	CTab * TabAddUniqueP(PSZAC pszName, ITreeItem * pTreeItem);
	void TabAddAndSelect(PSZAC pszName, ITreeItem * pTreeItem = NULL);
	void TabAddUniqueAndSelect(PSZAC pszName, ITreeItem * pTreeItem);
	void TabRemove(ITreeItem * pTreeItem);
	void TabsRemmoveAll();
	void TabSelect(ITreeItem * pTreeItem);
	void TabRepaint(ITreeItem * pTreeItem);
	void OnTabNew();
	void OnTabSelected(ITreeItem * pTreeItemSelected);
	void OnTabClosing(ITreeItem * pTreeItemClosing);

protected:
	virtual QSize sizeHint() const;
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent * pEventMouse);
	virtual void mouseReleaseEvent(QMouseEvent * pEventMouse);
	virtual void leaveEvent(QEvent *);

	void TabDelete(PA_DELETING CTab * paTab);
	CTab * _PFindTabMatchingTreeItem(ITreeItem * pTreeItem) const;

	void _SetSelectedTab(CTab * pTab);
	void _SetFlagsHitTest(UINT uFlagsHitTest);
	CTab * _PGetHitTestInfo(QMouseEvent * pEventMouse, OUT EHitTest * peHitTest) const;
	CTab * _PGetHitTestInfo(int xPos, int yPos, OUT EHitTest * peHitTest) const;

	void _DrawTab(CPainterCell * pPainter, CTab * pTab);
	void _Redraw() { update(); }
}; // WTabs

//	Display tabs a the top of the toolbar
class WToolbarTabs : public QWidget
{
	Q_OBJECT
public:
	WToolbarTabs();
	virtual void paintEvent(QPaintEvent *);

public slots:
	void SL_MenuProfilesShow();
	void SL_MenuProfileSelected(QAction * pAction);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WButtonToolbarIcon : public WButtonIconForToolbar
{
public:
	WButtonToolbarIcon(PA_PARENT QBoxLayout * poParentLayout, EMenuIcon eMenuIcon, PSZAC pszIconHover = NULL);
};


class WButtonToolbarSeparator : public WWidget
{
	enum { c_cxWidth = 10 };
public:
	WButtonToolbarSeparator() : WWidget(NULL) { }
	virtual QSize sizeHint() const { return QSize(c_cxWidth, d_zNA); }
	virtual void paintEvent(QPaintEvent *);
};


class WToolbarButtons : public QWidget
{
	Q_OBJECT
public:
	WToolbarButtons();
	virtual void paintEvent(QPaintEvent *);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class WToolbar : public QToolBar
{
	QWidget * m_pwContent;	// Widget covering the whole area of the toolbar
public:
	WToolbar();
	virtual void resizeEvent(QResizeEvent *pEventResize);
};

#endif // WTOOLBAR_H
