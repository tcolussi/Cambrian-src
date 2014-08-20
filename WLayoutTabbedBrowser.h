#ifndef WLAYOUTTABBEDBROWSER_H
#define WLAYOUTTABBEDBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QWebFrame>
#include "TBrowserTab.h"
#include "ApiJavaScript.h"


class WWebViewTabbed;


class CArrayPtrCWebViews : public CArray
{
public:
	inline WWebViewTabbed ** PrgpGetWebViewsStop(OUT WWebViewTabbed *** pppWebViewStop) const { return (WWebViewTabbed **)  PrgpvGetElementsStop(OUT (void***) pppWebViewStop); }
};



// WLayoutBrowserTabs
class WLayoutTabbedBrowser : public WLayout
{
	Q_OBJECT

	QTabWidget * m_pTabWidget;
	CArrayPtrCWebViews  m_arraypaWebViews;
	TBrowserTabs *m_pBrowserTabs;
	TProfile * m_pProfile;

public:
	WLayoutTabbedBrowser(TBrowserTabs *pBrowserTabs, TProfile * pProfile);
	~WLayoutTabbedBrowser();

	int AddTab(TBrowserTab *pTBrowserTab);
	int GetTabsCount();
	QWebView* getTab(int i);

public slots:
	void SL_WebViewTitleChanged(const QString & title);
	void SL_AddTab(bool checked);
	void SL_TabCloseRequested(int index);

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


Q_SIGNALS:
	void titleChanged(const QString & title);
};




#endif // WLAYOUTTABBEDBROWSER_H
