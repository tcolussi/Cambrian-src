#include "WLayoutTabbedBrowser.h"

WLayoutTabbedBrowser::WLayoutTabbedBrowser(TBrowserTabs *pBrowserTabs, TProfile * pProfile)
	{
	Assert(pBrowserTabs != NULL);
	m_pBrowserTabs = pBrowserTabs;
	m_pProfile = pProfile;

	// tab widget
	m_pTabWidget = new QTabWidget(this);
	//m_pTabWidget->setTabsClosable(true);
	m_pTabWidget->setTabShape(QTabWidget::Triangular);
	m_pTabWidget->setMovable(true);
	m_pTabWidget->setStyleSheet("QTabBar::tab { height: 23px; width: 150px; color: rgb(100, 100, 100) }");
	QObject::connect(m_pTabWidget, &QTabWidget::tabCloseRequested, this, &WLayoutTabbedBrowser::SL_TabCloseRequested);

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

	WWebViewTabbed *paWebView = new WWebViewTabbed(pTBrowserTab, m_pProfile);
	if ( !pTBrowserTab->m_url.FIsEmptyString() )
		paWebView->NavigateToAddress(pTBrowserTab->m_url );

	QObject::connect(paWebView, SIGNAL(titleChanged(QString)), this, SLOT(SL_WebViewTitleChanged(QString)) );

	int iTab = m_pTabWidget->addTab(paWebView, "New tab");
	m_pTabWidget->setCurrentIndex(iTab);

	// make it closable only if there are two or more tabs
	if ( m_pTabWidget->count() > 1)
		m_pTabWidget->setTabsClosable(true);

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
	WWebViewTabbed *pTabPage = (WWebViewTabbed *) QObject::sender();
	int index = m_arraypaWebViews.FindElementI(pTabPage);
	//MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "TabbedBrowser::SL_WebTitleChanged : $Q $i\n", &title, index);
	if ( index != -1 && !title.isEmpty() )
		{
		m_pTabWidget->setTabText(index, title);
		pTabPage->m_pTab->m_strNameDisplayTyped = CStr(title);
		//pTabPage->m_pTab->TreeItemW_UpdateText();
		}
	}

void WLayoutTabbedBrowser::SL_AddTab(bool checked = false)
	{
	// add the new tab using the TBrowserTabs object
	m_pBrowserTabs->AddTab();
	MessageLog_AppendTextFormatCo(d_coBlack, "AddTab()\n");
	}

void WLayoutTabbedBrowser::SL_TabCloseRequested(int index)
	{
	WWebViewTabbed *pTabPage = (WWebViewTabbed *) m_pTabWidget->widget(index);
	m_pBrowserTabs->m_arraypaTabs.DeleteTreeItem(pTabPage->m_pTab);

	m_pTabWidget->removeTab(index);
	delete pTabPage;

	if ( m_pTabWidget->count() == 0 )
		SL_AddTab(true);

	if ( m_pTabWidget->count() == 1)
		m_pTabWidget->setTabsClosable(false);
	}



/////////////////////////////////////////////////////////

WWebViewTabbed::WWebViewTabbed(TBrowserTab *pTab, TProfile *pProfile) : QSplitter(Qt::Vertical)
	{
	m_pTab = pTab;
	m_pProfile = pProfile;

	// address bar
	QWidget * pWidgetAddressBar = new QWidget(this);
	pWidgetAddressBar->setContentsMargins(0, 0, 0, 0);
	pWidgetAddressBar->setMaximumHeight(24);
	OLayoutHorizontal * poLayout = new OLayoutHorizontal(pWidgetAddressBar);
	poLayout->setContentsMargins(2, 2, 2, 0);

	// back and forward buttons
	m_pwButtonBack    = new WButtonIconForToolbar(eMenuIconGoBack, "Go Back");
	m_pwButtonForward = new WButtonIconForToolbar(eMenuIconGoForward, "Go Forward");
	poLayout->addWidget(m_pwButtonBack);
	poLayout->addWidget(m_pwButtonForward);

	// address text
	m_pwEdit = new WEdit();
	m_pwEdit->Edit_SetWatermark("Enter web address");
	m_pwEdit->setParent(this);
	poLayout->addWidget(m_pwEdit);

	connect(m_pwEdit,          &WEdit::returnPressed,           this, &WWebViewTabbed::SL_NavigateToAddress );
	connect(m_pwButtonBack,    &WButtonIconForToolbar::clicked, this, &WWebViewTabbed::SL_GoBack);
	connect(m_pwButtonForward, &WButtonIconForToolbar::clicked, this, &WWebViewTabbed::SL_GoForward);

	m_pwWebView = new QWebView(this);
	connect(m_pwWebView, &QWebView::urlChanged,   this, &WWebViewTabbed::SL_UrlChanged);
	connect(m_pwWebView, &QWebView::titleChanged, this, &WWebViewTabbed::SL_WebViewTitleChanged);
	connect(m_pwWebView, &QWebView::loadFinished, this, &WWebViewTabbed::SL_Loaded);

	//	QScriptEngine

	m_paCambrian = new OJapiCambrian(pProfile, this);

	QWebPage * poPage = m_pwWebView->page();
	m_poFrame = poPage->mainFrame();
	SL_InitJavaScript();
	//connect(m_poFrame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(SL_InitJavaScript()));
	connect(m_poFrame, &QWebFrame::javaScriptWindowObjectCleared, this, &WWebViewTabbed::SL_InitJavaScript);
	poPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);	// Enable the context menu item named "Inspect".  This is useful for debugging web pages.
	}

void WWebViewTabbed::NavigateToAddress(const CStr &strAddress)
	{
	Assert(m_pwWebView != NULL);
	Assert(m_pTab != NULL);
	// MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlack), "WWebViewTabbed::NavigateToAddress __ $S - $S\n", &m_pTab->m_url, &strAddress);

	// update tree item
	if ( !m_pTab->m_url.FCompareStringsNoCase(strAddress))
		m_pTab->m_url = strAddress;

	// update address bar
	if ( m_pwEdit->text() != strAddress.ToQString() )
		m_pwEdit->setText(strAddress);

	// navigate
	m_pwWebView->stop();
	QUrl url(strAddress);
	if (url.scheme().isEmpty())
		url.setScheme("http");

	m_pwWebView->load(url);
	m_pwWebView->setFocus();
	}

void WWebViewTabbed::SL_NavigateToAddress()
	{
	CStr strAddress = *m_pwEdit;
	NavigateToAddress(strAddress);
}

void WWebViewTabbed::SL_UrlChanged(QUrl url)
	{
	//MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "WWebViewTabbed::SL_URLChanged\n");
	if (m_pwEdit != NULL)
		{
		QString sUrl = url.toString();
		m_pwEdit->setText(sUrl);
		m_pTab->m_url = sUrl;
	}
}

void WWebViewTabbed::SL_GoBack()
	{
	m_pwWebView->back();
	}

void WWebViewTabbed::SL_GoForward()
	{
	m_pwWebView->forward();
	}

void WWebViewTabbed::SL_WebViewTitleChanged(const QString &title)
	{
	emit titleChanged(title);
	}

void WWebViewTabbed::SL_InitJavaScript()
	{
	m_paCambrian->m_arraypaTemp.DeleteAllRuntimeObjects();// Delete any previous temporary object
	m_poFrame->addToJavaScriptWindowObject("Cambrian", m_paCambrian); // , QWebFrame::ScriptOwnership);
	}

void WWebViewTabbed::SL_Loaded(bool ok)
	{
	if ( !ok )
		m_poFrame->setHtml("The web page doesn't exist");
	}


