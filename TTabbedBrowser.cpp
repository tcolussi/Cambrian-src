//	TTabbedBrowser.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TTabbedBrowser.h"
#include "WLayoutBrowser.h"


TTabbedBrowser::TTabbedBrowser(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
	m_pProfile = pProfile;
	m_pawLayoutBrowser = new WLayoutTabbedBrowser();
	}

TTabbedBrowser::~TTabbedBrowser()
	{
	TRACE1("TTabbedBrowser::~TTabbedBrowser(0x$p)\n", this);
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutBrowser);
	}


void
TTabbedBrowser::SetIconAndName(EMenuAction eMenuActionIcon, PSZAC pszName)
	{
	m_strNameDisplayTyped.BinInitFromStringWithNullTerminator(pszName);
	}

void
TTabbedBrowser::AddTab(CStr &sUrl)
	{
	m_pawLayoutBrowser->AddTab(sUrl);
	}

int
TTabbedBrowser::GetTabsCount()
	{
	return m_pawLayoutBrowser->GetTabsCount();
	}


//	TBrowser::IRuntimeObject::PGetRuntimeInterface()
POBJECT
TTabbedBrowser::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pProfile);
	}

//	TBrowser::IXmlExchange::XmlExchange()
void
TTabbedBrowser::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	}

const EMenuActionByte c_rgzeActionsMenuBrowser[] =
	{
	eMenuAction_TreeItemRename,
	eMenuAction_Close,
	ezMenuActionNone
	};

//	TBrowser::ITreeItem::TreeItem_MenuAppendActions()
void
TTabbedBrowser::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuBrowser);
	}

//	TBrowser::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TTabbedBrowser::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
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
TTabbedBrowser::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);
	}

void
TTabbedBrowser::TreeItemBrowser_DisplayWithinNavigationTree()
	{
	TreeItemW_DisplayWithinNavigationTree((m_pProfile->m_paTreeItemW_YZ != NULL) ?  m_pProfile : NULL);
	TreeItemW_SetIcon(eMenuAction_DisplaySecureWebBrowsing);
	}

IXmlExchange *
TTabbedBrowser::S_PaAllocateBrowser(POBJECT pProfileParent)
	{
	Assert(pProfileParent != NULL);
	return new TTabbedBrowser((TProfile *)pProfileParent);
	}

