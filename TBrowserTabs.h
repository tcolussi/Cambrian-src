//	TBrowserTabs.h
#ifndef TBROWSERTABS_H
#define TBROWSERTABS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TBrowserTab.h"

class WLayoutTabbedBrowser;

class CArrayPtrBrowserTabs : public CArrayPtrTreeItems
{
public:
	inline TBrowserTab ** PrgpGetBrowserTabStop(OUT TBrowserTab *** pppWebViewStop) const { return (TBrowserTab **)  PrgpvGetElementsStop(OUT (void***) pppWebViewStop); }
};



class TBrowserTabs : public ITreeItem
{
	RTI_IMPLEMENTATION(TBrowserTabs)

protected:
	TProfile * m_pProfile;

public:
	WLayoutTabbedBrowser *m_pawLayoutBrowser;
	CArrayPtrBrowserTabs m_arraypaTabs;

	TBrowserTabs(TProfile * pProfile);
	~TBrowserTabs();

	void SetIconAndName(EMenuAction eMenuActionIcon, PSZAC pszName);
	TBrowserTab * AddTab(CStr &sUrl);
	TBrowserTab * AddTab();
	TBrowserTab * GetCurrentBrowserTab();
	void DeleteTab(TBrowserTab *pBrowserTab);
	int GetTabsCount();

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);		// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem
	void TreeItemBrowser_DisplayWithinNavigationTree();

public:
	static IXmlExchange* S_PaAllocateBrowserTabbed(PVOID pProfileParent);

}; // TBrowser


class CArrayPtrBrowsersTabbed : public CArrayPtrTreeItems
{
public:
	inline TBrowserTabs ** PrgpGetBrowsersStop(OUT TBrowserTabs *** pppBrowserStop) const { return (TBrowserTabs **)PrgpvGetElementsStop(OUT (void ***)pppBrowserStop); }
};

#endif // TBROWSERTABS_H
