#include "WLayoutTabbedBrowser.h"

WLayoutTabbedBrowser::WLayoutTabbedBrowser(TBrowserTabs *pBrowserTabs)
	{
	Assert(pBrowserTabs != NULL);
	m_pBrowserTabs = pBrowserTabs;

	// tab widget
	m_pTabWidget = new QTabWidget(this);
	m_pTabWidget->setTabsClosable(true);
	m_pTabWidget->setTabShape(QTabWidget::Triangular);
	m_pTabWidget->setMovable(true);

	// add tab button
	QIcon pIco(":/ico/IconHaven");
	QToolButton *newTabButton= new QToolButton(this);
	m_pTabWidget->setCornerWidget(newTabButton, Qt::TopLeftCorner);
	newTabButton->setCursor(Qt::ArrowCursor);
	newTabButton->setAutoRaise(true);
	newTabButton->setIcon(pIco);
	newTabButton->setToolTip(tr("Add tab"));
	QObject::connect(newTabButton, &QToolButton::clicked, this, &WLayoutTabbedBrowser::SL_AddTab);
	}

WLayoutTabbedBrowser::~WLayoutTabbedBrowser()
	{
	}

int
WLayoutTabbedBrowser::AddTab(TBrowserTab *pTBrowserTab)
	{
	Assert(pTBrowserTab != NULL);
	WWebViewTabbed *paWebView = new WWebViewTabbed(pTBrowserTab);
	m_pTabWidget->addTab(paWebView, "New tab");

	if ( !pTBrowserTab->m_url.FIsEmptyString() )
		paWebView->load(pTBrowserTab->m_url.ToQString() );

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
		m_pTabWidget->setTabText(index, title);
	}

void WLayoutTabbedBrowser::SL_AddTab(bool checked = false)
	{
	// add the new tab from the TBrowserTabs object
	m_pBrowserTabs->AddTab();
	MessageLog_AppendTextFormatCo(d_coBlack, "AddTab()\n");
	}



/////////////////////////////////////////////////////////


WWebViewTabbed::WWebViewTabbed(TBrowserTab *pTab)
	{
	m_pTab = pTab;
	}

void WWebViewTabbed::load(const QUrl &url)
	{
	QWebView::load(url);
	}

