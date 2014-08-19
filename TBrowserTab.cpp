#include "TBrowserTab.h"

TBrowserTab::TBrowserTab(TBrowserTabs *pBrowserTabs)
{
	Assert(pBrowserTabs != NULL);
	Assert(pBrowserTabs->EGetRuntimeClass() == RTI(TBrowserTabs));
	m_pBrowserTabs = pBrowserTabs;
}

void
TBrowserTab::SetUrl(CStr &sUrl)
	{
	m_url = sUrl;
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

