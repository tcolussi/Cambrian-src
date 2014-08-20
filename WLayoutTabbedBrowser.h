#ifndef WLAYOUTTABBEDBROWSER_H
#define WLAYOUTTABBEDBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TBrowserTab.h"

// Custom web view to reference its tree item
class WWebViewTabbed : public QWebView
{
public:
	TBrowserTab *m_pTab;

	WWebViewTabbed(TBrowserTab *pTab);
	void load ( const QUrl & url);
};


class CArrayPtrCWebViews : public CArray
{
public:
	inline WWebViewTabbed ** PrgpGetWebViewsStop(OUT WWebViewTabbed *** pppWebViewStop) const { return (WWebViewTabbed **)  PrgpvGetElementsStop(OUT (void***) pppWebViewStop); }
};


// WLayoutBrowserTabs
class WLayoutTabbedBrowser : public WLayout
{
	QTabWidget * m_pTabWidget;
	CArrayPtrCWebViews  m_arraypaWebViews;
	TBrowserTabs *m_pBrowserTabs;

public:
	WLayoutTabbedBrowser(TBrowserTabs *pBrowserTabs);
	~WLayoutTabbedBrowser();

	int AddTab(TBrowserTab *pTBrowserTab);
	int GetTabsCount();
	QWebView* getTab(int i);

public slots:
	void SL_WebViewTitleChanged(const QString & title);
	void SL_AddTab(bool checked);
	void SL_TabCloseRequested(int index);


};





#endif // WLAYOUTTABBEDBROWSER_H
