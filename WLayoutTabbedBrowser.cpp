#include "WLayoutTabbedBrowser.h"

WLayoutTabbedBrowser::WLayoutTabbedBrowser()
	{
	m_pTabWidget = new QTabWidget(this);
	}

WLayoutTabbedBrowser::~WLayoutTabbedBrowser()
	{
	delete m_pTabWidget;
	// TODO: delete all browsers in m_arraypaWebViews
	}

int
WLayoutTabbedBrowser::AddTab(CStr &sUrl)
	{
	QUrl url(sUrl);
	if (url.scheme().isEmpty())
		url.setScheme("http");

	QWebView *paWebView = new QWebView();
	paWebView->load(url);
	m_pTabWidget->addTab(paWebView, "New tab");

	QObject::connect(paWebView, &QWebView::titleChanged,
					 this, &WLayoutTabbedBrowser::SL_WebViewTitleChanged);


	return m_arraypaWebViews.Add(paWebView);
}

int
WLayoutTabbedBrowser::GetTabsCount()
	{
	return m_arraypaWebViews.GetSize();
	}

QWebView*
WLayoutTabbedBrowser::getTab(int i)
	{
	return (QWebView*) m_arraypaWebViews.PvGetElementAtSafe_YZ(i);
	}


void
WLayoutTabbedBrowser::SL_WebViewTitleChanged(const QString &title)
	{
	QWebView *pWebView = (QWebView *) QObject::sender();
	int index = m_arraypaWebViews.FindElementI(pWebView);
	if ( index != -1 && !title.isEmpty() )
		{
		m_pTabWidget->setTabText(index, title);
		}
	}


