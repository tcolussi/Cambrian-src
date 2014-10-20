#ifndef WTOOLBAR_H
#define WTOOLBAR_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QToolBar>

class CTab
{
public:
	CStr m_strName;		// Name of the tab
	PVPARAM m_pvParam;	// User-defined data
	int m_cxWidth;		// Width of the tab
public:
	CTab(PSZUC pszName, PVPARAM pvParam);
};

class CArrayPtrTabs : public CArray
{
public:
	inline CTab ** PrgpGetTabsStop(OUT CTab *** pppTabStop) const { return (CTab **)PrgpvGetElementsStop(OUT (void ***)pppTabStop); }
	void DeleteAllTabs();
};


class WTabs : public QWidget
{
	Q_OBJECT
	CArrayPtrTabs m_arraypaTabs;
	CTab * m_pTabSelected;	// Tab which is selected
	CTab * m_pTabHover;		// Tab where the mouse is
	enum EHitTest
		{
		eHitTest_zNone,
		eHitTest_ButtonNewTab
		};
	EHitTest m_eHitTest;

public:
	WTabs();
	~WTabs();
	void TabAdd(PSZAC pszName, PVPARAM pvParam = NULL);
	void TabDelete(PA_DELETING CTab * paTab);

protected:
	virtual QSize sizeHint() const;
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent * pEventMouse);
	virtual void mouseReleaseEvent(QMouseEvent * pEventMouse);
	virtual void leaveEvent(QEvent *);

	CTab * _PUpdateHitTest(QMouseEvent * pEventMouse);
	CTab * _PUpdateHitTest(int xPos);

	void _DrawTab(CPainterCell * pPainter, CTab * pTab);
	void _Redraw() { update(); }
};

//	Display tabs a the top of the toolbar
class WToolbarTabs : public QWidget
{
	Q_OBJECT
public:
	WToolbarTabs();
	virtual void paintEvent(QPaintEvent *);

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

public slots:
	void SL_MenuProfilesShow();
	void SL_MenuProfileSelected(QAction * pAction);
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
