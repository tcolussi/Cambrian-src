//	TBrowser.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

IXmlExchange *
TBrowser::S_PaAllocateBrowser(POBJECT pProfileParent)
	{
	Assert(pProfileParent != NULL);
	return new TBrowser((TProfile *)pProfileParent);
	}

TBrowser::TBrowser(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
	m_pProfile = pProfile;
	m_uFlags = 0;
	m_pawLayoutBrowser = NULL;
	}

void
TBrowser::SetIconNameAndUrl(EMenuAction eMenuActionIcon, PSZAC pszName, PSZAC pszUrl)
	{
	m_uFlags = (eMenuActionIcon == eMenuIconMarketplace);
	m_strNameDisplayTyped.BinInitFromStringWithNullTerminator(pszName);
	m_strUrl.BinInitFromStringWithNullTerminator(pszUrl);
	}

void
TBrowser::TreeItemBrowser_DisplayWithinNavigationTree()
	{
	TreeItemW_DisplayWithinNavigationTree(NULL);
	TreeItemW_SetIcon(m_uFlags ? eMenuIconMarketplace : eMenuAction_DisplaySecureWebBrowsing);
	}

//	TBrowser::IXmlExchange::XmlExchange()
void
TBrowser::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("URL", INOUT_F_UNCH_S &m_strUrl);
	pXmlExchanger->XmlExchangeUInt("Flags", INOUT_F_UNCH_S &m_uFlags);
	}

const EMenuActionByte c_rgzeActionsMenuBrowser[] =
	{
	eMenuAction_TreeItemRename,
	eMenuAction_Close,
	ezMenuActionNone
	};

//	TBrowser::ITreeItem::TreeItem_MenuAppendActions()
void
TBrowser::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuBrowser);
	}

//	TBrowser::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TBrowser::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
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

