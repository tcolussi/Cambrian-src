#ifndef WLAYOUTTABBEDBROWSER_H
#define WLAYOUTTABBEDBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


class CArrayPtrWebViews : public CArray
{
public:
	inline QWebView ** PrgpGetWebViewsStop(OUT QWebView *** pppWebViewStop) const { return (QWebView **)  PrgpvGetElementsStop(OUT (void***) pppWebViewStop); }
};



class WLayoutTabbedBrowser : public WLayout
{
	QTabWidget * m_pTabWidget;
	CArrayPtrWebViews m_arraypaWebViews;

public:
	WLayoutTabbedBrowser();
	~WLayoutTabbedBrowser();

	int AddTab(CStr & sUrl);
	int GetTabsCount();
	QWebView* getTab(int i);

public slots:
	void SL_WebViewTitleChanged(const QString & title);


};





#endif // WLAYOUTTABBEDBROWSER_H
