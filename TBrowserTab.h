#ifndef TBROWSERTAB_H
#define TBROWSERTAB_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutTabbedBrowser.h"

class TBrowserTabs;

class TBrowserTab : public ITreeItem
{
	RTI_IMPLEMENTATION(TBrowserTab)

protected:
	TBrowserTabs * m_pBrowserTabs;

public:
	WWebViewTabbed * m_pwWebViewTab;
	TBrowserTab(TBrowserTabs * pBrowserTabs);
	CStr m_url;

public:
	void SetUrl(CStr &sUrl);
	void Show();
	void NavigateForward();
	void NavigateBack();
	void NavigateReload();


	// IRuntimeObject interface
	POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject *piParent) const;

	// IXmlExchange interface
	void XmlExchange(CXmlExchanger *pXmlExchanger);

	// ITreeItem interface
	void TreeItem_RemoveAllReferencesToObjectsAboutBeingDeleted();
	void TreeItem_GotFocus();

	// static
	static IXmlExchange * S_PaAllocateBrowser(PVOID pProfileParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()

};

#endif // TBROWSERTAB_H
