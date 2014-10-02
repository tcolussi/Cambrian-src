#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutTabbedBrowser.h"
#include "TBrowserTabs.h"
#include "TBrowserTab.h"

WLayoutTabbedBrowser::WLayoutTabbedBrowser(TBrowserTabs *pBrowserTabs, TProfile * pProfile)
	{
	Assert(pBrowserTabs != NULL);
	m_pBrowserTabs = pBrowserTabs;
	m_pProfile = pProfile;

	// tab widget
	m_pTabWidget = new WaTabWidget((QWidget*) this);
	m_pTabWidget->setTabsClosable(true);
	m_pTabWidget->setTabShape(QTabWidget::Triangular);
	//m_pTabWidget->setMovable(true);
	m_pTabWidget->setStyleSheet("QTabBar::tab { height: 23px; width: 150px; color: rgb(100, 100, 100) } QTabBar::tab:last {width: 40px}");
	//m_pTabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0,0);
	QObject::connect(m_pTabWidget, &QTabWidget::tabCloseRequested, this, &WLayoutTabbedBrowser::SL_TabCloseRequested);
	QObject::connect(m_pTabWidget, &QTabWidget::currentChanged, this, &WLayoutTabbedBrowser::SL_CurrentChanged);

	// add tab button
	/*
	QIcon iconAdd(":/ico/Add");
	QToolButton *newTabButton= new QToolButton(this);
	m_pTabWidget->setCornerWidget(newTabButton, Qt::TopLeftCorner);
	newTabButton->setCursor(Qt::ArrowCursor);
	newTabButton->setAutoRaise(true);
	newTabButton->setIcon(iconAdd);
	newTabButton->setToolTip(tr("Add tab"));
	QObject::connect(newTabButton, &QToolButton::clicked, this, &WLayoutTabbedBrowser::SL_AddTab);
	*/
	}

WLayoutTabbedBrowser::~WLayoutTabbedBrowser()
	{
	}

int
WLayoutTabbedBrowser::AddTab(TBrowserTab *pTBrowserTab)
	{
	Assert(pTBrowserTab != NULL);
	Assert(pTBrowserTab->m_pwWebViewTab == NULL); // new TreeItem shouldn't already have a tab view

	WWebViewTabbed *paWebView = new WWebViewTabbed(pTBrowserTab, m_pProfile);
	pTBrowserTab->m_pwWebViewTab = paWebView;

	if ( !pTBrowserTab->m_url.FIsEmptyString() )
		paWebView->NavigateToAddress(pTBrowserTab->m_url );
	else
		{
		// open default page
		const SApplicationHtmlInfo *pInfo = PGetApplicationHtmlInfo("Default NewTab");
		Assert(pInfo != NULL && "Application 'Default NewTab' doesn't exist");
		if ( pInfo )
			{
			CStr strUrl = MainWindow_SGetPathOfApplication(pInfo->pszLocation);
			MessageLog_AppendTextFormatCo(d_coRed, "WLayoutTabbedBrowser::AddTab $S\n", &strUrl);
			pTBrowserTab->SetUrl(strUrl);// this in turn calls to NavigateToAddress
			}
		}


	QObject::connect(paWebView, SIGNAL(titleChanged(QString)), this, SLOT(SL_WebViewTitleChanged(QString)) );

	int iTab = m_pTabWidget->addTab(paWebView, "New tab");
	m_pTabWidget->setCurrentIndex(iTab);
	//m_pTabWidget->setTabsClosable(true);

	return m_arraypaWebViews.Add(paWebView);
	}

void WLayoutTabbedBrowser::RemoveTab ( int index )
	{
	WWebViewTabbed *pWebView = (WWebViewTabbed *)m_arraypaWebViews.PvGetElementAtSafe_YZ(index);
	if ( pWebView )
		{
		m_arraypaWebViews.RemoveElementAt(index);	// remove from array
		m_pTabWidget->removeTab(index);				// remove from tab widget
		delete pWebView;

		if ( m_pTabWidget->count() == 0 )
			m_pBrowserTabs->AddTab();
		}
	}

int
WLayoutTabbedBrowser::GetTabsCount()
	{
	return m_arraypaWebViews.GetSize();
	}

int WLayoutTabbedBrowser::CurrentTabIndex()
	{
	// returns -1 if there are no tabs
	if ( m_pTabWidget )
		return m_pTabWidget->currentIndex();
	return -1;
	}

WWebViewTabbed*
WLayoutTabbedBrowser::getTab(int i)
	{
	return (WWebViewTabbed*) m_arraypaWebViews.PvGetElementAtSafe_YZ(i);
	}


void
WLayoutTabbedBrowser::SL_WebViewTitleChanged(const QString &title)
	{
	WWebViewTabbed *pTabPage = (WWebViewTabbed *) QObject::sender();
	//int index = m_arraypaWebViews.FindElementI(pTabPage);
	int index = m_pTabWidget->indexOf(pTabPage);
	MessageLog_AppendTextFormatCo(d_coBlue, "WLayoutTabbedBrowser::SL_WebViewTitleChanged($i) : $Q\n", index, &title);
	if ( index != -1 && !title.isEmpty() )
		{
		m_pTabWidget->setTabText(index, title);
		pTabPage->m_pTab->m_strNameDisplayTyped = CStr(title);
		//pTabPage->m_pTab->TreeItemW_UpdateText();
		}
	}

void
WLayoutTabbedBrowser::SL_AddTab(bool checked = false)
	{
	MessageLog_AppendTextFormatCo(d_coBluePastel, "WLayoutBrowser::SL_AddTab($i)\n", checked);
	// add the new tab using the TBrowserTabs object /*???*/
	m_pBrowserTabs->AddTab();
	}

void
WLayoutTabbedBrowser::SL_TabCloseRequested(int index)
	{
	MessageLog_AppendTextFormatCo(d_coBlueDark, "WLayoutBrowser::SL_TabCloseRequested($i)\n", index);
	m_pBrowserTabs->DeleteTab(index);
}

void WLayoutTabbedBrowser::SL_CurrentChanged(int index)
	{
	//MessageLog_AppendTextFormatCo(d_coBlue, "WLayoutTabbedBrowser::SL_CurrentChanged($i, $i)\n", index, (index==m_pTabWidget->count() ) );
	// add the new tab using the TBrowserTabs object
	//if ( index == m_pTabWidget->count() - 1 && m_pTabWidget->count() > 1)
	if ( index == m_pTabWidget->count() && m_pTabWidget->count() > 0)
		{
		MessageLog_AppendTextFormatCo(d_coBlue, "WLayoutTabbedBrowser::SL_CurrentChanged-($i/$i) => addTab()\n", index, m_pTabWidget->count() );
		m_pBrowserTabs->AddTab();
		}
	}



////////////////////////////////////////////////////////////////////////////////////



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
	connect(m_poFrame, &QWebFrame::iconChanged,                   this, &WWebViewTabbed::SL_IconChanged);
	poPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);	// Enable the context menu item named "Inspect".  This is useful for debugging web pages.
	}

void
WWebViewTabbed::NavigateToAddress(const CStr &strAddress)
	{
	Assert(m_pwWebView != NULL);
	Assert(m_pTab != NULL);
	//MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlack), "WWebViewTabbed::NavigateToAddress __ $S\n", &strAddress);

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

void
WWebViewTabbed::SL_NavigateToAddress()
	{
	CStr strAddress = *m_pwEdit;
	NavigateToAddress(strAddress);
}

void
WWebViewTabbed::SL_UrlChanged(QUrl url)
	{
	//MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "WWebViewTabbed::SL_URLChanged\n");
	if (m_pwEdit != NULL)
		{
		QString sUrl = url.toString();
		m_pwEdit->setText(sUrl);
		m_pTab->m_url = sUrl;
	}
}

void
WWebViewTabbed::SL_GoBack()
	{
	m_pwWebView->back();
	}

void
WWebViewTabbed::SL_GoForward()
	{
	m_pwWebView->forward();
	}

void
WWebViewTabbed::SL_WebViewTitleChanged(const QString &title)
	{
	emit titleChanged(title);
	}

void
WWebViewTabbed::SL_InitJavaScript()
	{
	m_paCambrian->m_arraypaTemp.DeleteAllRuntimeObjects();// Delete any previous temporary object
	m_poFrame->addToJavaScriptWindowObject("Cambrian", m_paCambrian); // , QWebFrame::ScriptOwnership);
	}

void
WWebViewTabbed::SL_Loaded(bool ok)
	{
	if ( !ok )
		m_poFrame->setHtml("The web page doesn't exist");
	}

void
WWebViewTabbed::SL_IconChanged()
	{
	MessageLog_AppendTextFormatCo(d_coBlueDark, "SL_IconChanged()\n");
	}



//////////////////////////////////////////////////////////////////////////



void
WaTabWidget::addPlusButtonTab()
	{
	// add tab to act as a [plus button]
	QTabBar *pTabBar = tabBar();
	//int index =
	pTabBar->addTab("");

	//QIcon iconAdd(":/ico/Add");
	//pTabBar->setTabIcon(index, iconAdd);

	//QString strName = iconAdd.name();
	//MessageLog_AppendTextFormatCo(d_coBlack, "WaTabWidget::addPlusButtonTab - Icon: $Q\n", &strName );

	QWidget *pButton = pTabBar->tabButton(pTabBar->count(), QTabBar::RightSide);
	if ( pButton)		pButton->resize(0, 0);
	//pTabBar->setStyleSheet("QTabBar::tab { height: 23px; width: 20px; }");
	}

void
WaTabWidget::keyPressEvent(QKeyEvent *event)
	{
	//QTabWidget::keyPressEvent(event);
	int fGoForward = event->key() == Qt::Key_Tab     && (event->modifiers() & Qt::ControlModifier );
	int fGoBack    = event->key() == Qt::Key_Backtab && (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier) );

	//MessageLog_AppendTextFormatCo(d_coBlueDark, "WaTabWidget::keyReleaseEvent idx=$i, next=$i\n", currentIndex(), index);
	int newIndex = (count() + currentIndex() + fGoForward - fGoBack) % ( count() );// wrap around ignoring [plus] tab button
	if ( fGoForward || fGoBack)
		setCurrentIndex(newIndex);

	//MessageLog_AppendTextFormatCo(d_coBlueDark, "WaTabWidget::keyReleaseEvent fwd=$i, back=$i\n", fGoForward, fGoBack);
	//MessageLog_AppendTextFormatCo(d_coBlueDark, "WaTabWidget::keyReleaseEvent key=$x, mod=$x\n", event->key(), event->modifiers());
	}

WaTabWidget::WaTabWidget(QWidget *parent) : QTabWidget(parent)
	{
	WaTabBar *tabBar = new WaTabBar();
	setTabBar(tabBar);
//	setMovable(true);

	//QObject::connect(this, &QTabWidget::currentChanged, this, &WaTabWidget::SL_currentChanged);

	addPlusButtonTab();
	//tabBar->setStyleSheet("QTabBar::tab { height: 23px; width: 20px; }");
	}

int
WaTabWidget::addTab(QWidget *widget, const QString &label)
	{
	// remove [plus button]
	//pTabBar->removeTab( count() );

	// add tab as normal
	int iPos = QTabWidget::addTab(widget, label);
	MessageLog_AppendTextFormatCo(d_coBlueSky, "WaTabWidget::addTab($i)\n", iPos);
	//setCurrentIndex(iPos);

	// add [plus button] tab
	//addPlusButtonTab();
	return iPos;
	}

void
WaTabWidget::removeTab(int index)
	{
	// remove [plus button]
	//pTabBar->removeTab( count()  );
	MessageLog_AppendTextFormatCo(d_coBlue, "WaTabWidget::removeTab - selIndex $i / $i\n", index, count() );

	// select previous tab before remove
	if ( index == count() - 1 )
		setCurrentIndex( (index > 0) ? index - 1 : index);


	// remove tab as normal
	QTabWidget::removeTab(index);// TODO: delete page widget

	// add [plus button] tab
	//addPlusButtonTab();
	}

void
WaTabWidget::setTabsClosable(bool closeable)
	{
	QTabWidget::setTabsClosable(closeable);

	// make last tab have no close button
	QWidget *pButton = tabBar()->tabButton(count(), QTabBar::RightSide);
		if ( pButton)
			pButton->resize(0,0);
	}

int
WaTabWidget::count()
	{
	return QTabWidget::count() - 1;
	}



/////////////////////////////////////////////////////////////////////////
/*
void
WaTabBar::movePlusButton()
	{
	int size = 0;
	for(int i=0; i < count(); i++)
		{
		size += tabRect(i).width();
		}
	int h = geometry().top();
	int	w = width();

	if ( size > w )
		plusButton->move(w-54, 0);
	else
		plusButton->move(size, 0);
	}
*/
/*
void
WaTabBar::reinitializePlusButton()
	{
	addTab("+");
	}

void
WaTabBar::tabLayoutChange()
	{
	QTabBar::tabLayoutChange();
	movePlusButton();
	MessageLog_AppendTextFormatCo(d_coRed, "WaTabBar::tabLayoutChange()\n");
	}

void
WaTabBar::resizeEvent(QResizeEvent *event)
	{
	QTabBar::resizeEvent(event);
	MessageLog_AppendTextFormatCo(d_coRed, "WaTabBar::resizeEvent()\n");
	movePlusButton();
	}

QSize
WaTabBar::tabSizeHint(int index)
	{
	QSize size = QTabBar::tabSizeHint(index);
	int nWidth = ( index == count() -1 ) ? 20 : size.width();
	MessageLog_AppendTextFormatCo(d_coRed, "WaTabBar::tabSizeHint($i)\n", index);
	return QSize(nWidth, size.height());
	}
*/

WaTabBar::WaTabBar(QWidget *parent) : QTabBar(parent)
	{
	/*
	 *
	int iPos = addTab("+");
	QIcon iconAdd(":/ico/Add");

	plusButton = new QPushButton("+");
	plusButton->setParent(this);
	plusButton->setMaximumSize(20, 20);
	plusButton->setMinimumSize(20, 20);
	plusButton->raise();
	//plusButton->setIcon(iconAdd);
	//connect()

	movePlusButton();
	plusButton->show();
	*/
	}


/*
void WaTabBar::setTabsClosable(bool closable)
	{
	QTabBar::setTabsClosable(closable);

	// hide last tab's close button
	int iCount = count();
	tabButton(iCount-1, QTabBar::RightSide)->resize(0,0);
	}

int WaTabBar::addTab(const QString &text)
	{
	int iPos = QTabBar::addTab(text);
	return iPos;
	}

int WaTabBar::addTab(const QIcon &icon, const QString &text)
	{
	}
*/
