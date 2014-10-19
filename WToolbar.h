#ifndef WTOOLBAR_H
#define WTOOLBAR_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QToolBar>

//	Display tabs a the top of the toolbar
class WToolbarTabs : public QWidget
{
	Q_OBJECT
public:
	WToolbarTabs();

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


class WToolbar : public QToolBar
{
	QWidget * m_pwContent;	// Widget covering the whole area of the toolbar
public:
	WToolbar();
	virtual void resizeEvent(QResizeEvent *pEventResize);
};

#endif // WTOOLBAR_H
