#include "TBrowserTab.h"

TBrowserTab::TBrowserTab(TBrowserTabs *pBrowserTabs)
	{
	Assert(pBrowserTabs != NULL);
	Assert(pBrowserTabs->EGetRuntimeClass() == RTI(TBrowserTabs));
	m_pBrowserTabs = pBrowserTabs;
	m_pwWebViewTab = NULL;
	}

void
TBrowserTab::SetUrl(CStr &sUrl)
	{
	m_url = sUrl;
	if ( m_pwWebViewTab != NULL)
		{
		m_pwWebViewTab->NavigateToAddress(sUrl);
	}
}

void TBrowserTab::Show()
	{
	Assert(m_pwWebViewTab != NULL);
	Assert(m_pBrowserTabs != NULL);
	Assert(m_pBrowserTabs->m_pawLayoutBrowser != NULL);
	Assert(m_pBrowserTabs->m_pawLayoutBrowser->m_pTabWidget != NULL);

	m_pBrowserTabs->m_pawLayoutBrowser->m_pTabWidget->setCurrentWidget(m_pwWebViewTab);
}

void TBrowserTab::NavigateForward()
	{
	m_pwWebViewTab->m_pwWebView->forward();
	}

void TBrowserTab::NavigateBack()
	{
	m_pwWebViewTab->m_pwWebView->back();
	}

void TBrowserTab::NavigateReload()
	{
	m_pwWebViewTab->m_pwWebView->reload();
	}

POBJECT TBrowserTab::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject *piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pBrowserTabs);
	}

void
TBrowserTab::XmlExchange(CXmlExchanger *pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("url", &m_url);
	}

void
TBrowserTab::TreeItem_RemoveAllReferencesToObjectsAboutBeingDeleted()
	{
	}

void
TBrowserTab::TreeItem_GotFocus()
	{
	}

IXmlExchange *
TBrowserTab::S_PaAllocateBrowser(PVOID pBrowserTabsParent)
	{
	Assert(pBrowserTabsParent != NULL);
	return new TBrowserTab((TBrowserTabs*) pBrowserTabsParent);
	}

