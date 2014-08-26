#ifndef WLAYOUTTABBEDBROWSER_H
#define WLAYOUTTABBEDBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QWebFrame>
#include <qtabwidget.h>
#include "TBrowserTab.h"


class WWebViewTabbed;
class TBrowserTabs;
class TBrowserTab;
class WaTabWidget;


class CArrayPtrCWebViews : public CArray
{
public:
	inline WWebViewTabbed ** PrgpGetWebViewsStop(OUT WWebViewTabbed *** pppWebViewStop) const { return (WWebViewTabbed **)  PrgpvGetElementsStop(OUT (void***) pppWebViewStop); }
};






// WLayoutBrowserTabs
class WLayoutTabbedBrowser : public WLayout
{
	Q_OBJECT

	CArrayPtrCWebViews  m_arraypaWebViews;
	TProfile * m_pProfile;
	TBrowserTabs *m_pBrowserTabs;

public:
	WaTabWidget * m_pTabWidget;

	WLayoutTabbedBrowser(TBrowserTabs *pBrowserTabs, TProfile * pProfile);
	~WLayoutTabbedBrowser();

	int AddTab(TBrowserTab *pTBrowserTab);
	int GetTabsCount();
	QWebView* getTab(int i);

public slots:
	void SL_WebViewTitleChanged(const QString & title);
	void SL_AddTab(bool checked);
	void SL_TabCloseRequested(int index);
	void SL_CurrentChanged(int index);

};

// Custom web view to reference its tree item
class WWebViewTabbed : public QSplitter
{
	Q_OBJECT

protected:
	TProfile * m_pProfile;
	WEdit * m_pwEdit;
	WButtonIconForToolbar * m_pwButtonBack;
	WButtonIconForToolbar * m_pwButtonForward;
	OJapiCambrian * m_paCambrian;
	QWebFrame * m_poFrame;


public:
	QWebView *m_pwWebView;
	TBrowserTab *m_pTab;

	WWebViewTabbed(TBrowserTab *pTab, TProfile *pProfile);
	void NavigateToAddress(const CStr & strAddress);

public slots:
	void SL_NavigateToAddress();
	void SL_UrlChanged(QUrl url);
	void SL_GoBack();
	void SL_GoForward();
	void SL_WebViewTitleChanged(const QString & title);
	void SL_InitJavaScript();
	void SL_Loaded(bool ok);
	void SL_IconChanged();


Q_SIGNALS:
	void titleChanged(const QString & title);
};

class WaTabWidget : public QTabWidget
{
protected:
	void addPlusButtonTab();
	void keyPressEvent ( QKeyEvent * event);

public:
	WaTabWidget(QWidget *parent = 0 );

	int addTab(QWidget *widget, const QString & label);
	void removeTab(int index);
	void setTabsClosable ( bool closeable );
	int count();


};

class WaTabBar : public QTabBar
{
	QPushButton *plusButton;
	void movePlusButton();
	void reinitializePlusButton();

/*protected:
	void tabLayoutChange();
	void resizeEvent(QResizeEvent *event);
	QSize tabSizeHint(int index);
	*/

public:
	WaTabBar(QWidget *parent = 0);
	/*
	void setTabsClosable(bool closable);
	int	addTab ( const QString & text);
	int	addTab ( const QIcon & icon, const QString & text );
	*/
};


#endif // WLAYOUTTABBEDBROWSER_H
