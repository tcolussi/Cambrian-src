//	TTabbedBrowser.h
#ifndef TTABBEDBROWSER_H
#define TTABBEDBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutTabbedBrowser.h"

class WLayoutBrowser;

class TTabbedBrowser : public ITreeItem
{
	RTI_IMPLEMENTATION(TTabbedBrowser)

protected:
	TProfile * m_pProfile;
	WLayoutTabbedBrowser *m_pawLayoutBrowser;

public:
	TTabbedBrowser(TProfile * pProfile);
	~TTabbedBrowser();

	void SetIconAndName(EMenuAction eMenuActionIcon, PSZAC pszName);
	void AddTab(CStr &sUrl);
	int GetTabsCount();

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);		// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem
	void TreeItemBrowser_DisplayWithinNavigationTree();

public:
	static IXmlExchange * S_PaAllocateBrowser(POBJECT pProfileParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	friend class WLayoutBrowser;
}; // TBrowser



#endif // TBROWSER_H
