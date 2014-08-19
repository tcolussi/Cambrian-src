//	TBrowserTabs.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TBrowserTabs.h"
#include "WLayoutTabbedBrowser.h"


TBrowserTabs::TBrowserTabs(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
	m_pProfile			= pProfile;
	m_pawLayoutBrowser	= NULL;
	}

TBrowserTabs::~TBrowserTabs()
	{
	TRACE1("TBrowserTabs::~TBrowserTabs(0x$p)\n", this);
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutBrowser);
	}

void
TBrowserTabs::SetIconAndName(EMenuAction eMenuActionIcon, PSZAC pszName)
	{
	m_strNameDisplayTyped.BinInitFromStringWithNullTerminator(pszName);
	}

TBrowserTab*
TBrowserTabs::AddTab(CStr &sUrl)
	{
	TBrowserTab* pTab = AddTab();
	pTab->m_url = sUrl;
	return pTab;
	}

TBrowserTab*
TBrowserTabs::AddTab()
	{
	TBrowserTab *pBrowserTab = new TBrowserTab(this);
	m_arraypaTabs.Add(pBrowserTab);

	if ( m_pawLayoutBrowser)
		m_pawLayoutBrowser->AddTab(pBrowserTab);

	return pBrowserTab;
	}

int
TBrowserTabs::GetTabsCount()
	{
	return m_arraypaTabs.GetSize();
	}


//	TBrowser::IRuntimeObject::PGetRuntimeInterface()
POBJECT
TBrowserTabs::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pProfile);
	}

//	TBrowser::IXmlExchange::XmlExchange()
void
TBrowserTabs::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeObjects('T', INOUT_F_UNCH_S &m_arraypaTabs, TBrowserTab::S_PaAllocateBrowser, this);
	}

IXmlExchange *
TBrowserTabs::S_PaAllocateBrowserTabbed(PVOID pProfileParent)
	{
	Assert(pProfileParent != NULL);
	return new TBrowserTabs((TProfile*) pProfileParent);
	}

const EMenuActionByte c_rgzeActionsMenuBrowser[] =
	{
	eMenuAction_TreeItemRename,
	eMenuAction_Close,
	ezMenuActionNone
	};

//	TBrowser::ITreeItem::TreeItem_MenuAppendActions()
void
TBrowserTabs::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuBrowser);
	}

//	TBrowser::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TBrowserTabs::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_Close:
		MainWindow_SetCurrentLayout(NULL);
		m_pProfile->m_arraypaBrowsers.DeleteTreeItem(PA_DELETING this);
		return ezMenuActionNone;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

void
TBrowserTabs::TreeItem_GotFocus()
	{
	m_pawLayoutBrowser = new WLayoutTabbedBrowser(this);
	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);

	// add tabs added before the layout was initialized
	TBrowserTab **ppBrowserTabStop;
	TBrowserTab **ppBrowserTab = m_arraypaTabs.PrgpGetBrowserTabStop(&ppBrowserTabStop);
	while(ppBrowserTab != ppBrowserTabStop)
		{
		TBrowserTab *pBrowserTab = *ppBrowserTab++;
		m_pawLayoutBrowser->AddTab(pBrowserTab);
		}
	}

void
TBrowserTabs::TreeItemBrowser_DisplayWithinNavigationTree()
	{
	TreeItemW_DisplayWithinNavigationTree((m_pProfile->m_paTreeItemW_YZ != NULL) ?  m_pProfile : NULL);
	TreeItemW_SetIcon(eMenuAction_DisplaySecureWebBrowsing);
	}



