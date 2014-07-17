//	TBrowser.h

#ifndef TBROWSER_H
#define TBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WLayoutBrowser;

class TBrowser : public ITreeItem
{
	RTI_IMPLEMENTATION(TBrowser)
protected:
	TProfile * m_pProfile;
	CStr m_strUrl;
	UINT m_uFlags;	// Flags to remember which icon to display
	WLayoutBrowser * m_pawLayoutBrowser;
public:
	TBrowser(TProfile * pProfile);
	~TBrowser();

	void SetIconNameAndUrl(EMenuAction eMenuActionIcon, PSZAC pszName, PSZAC pszUrl = NULL);

	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);		// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem
	void TreeItemBrowser_DisplayWithinNavigationTree();

public:
	static IXmlExchange * S_PaAllocateBrowser(POBJECT pProfileParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	friend class WLayoutBrowser;
}; // TBrowser


class CArrayPtrBrowsers : public CArrayPtrTreeItems
{
public:
	inline TBrowser ** PrgpGetBrowsersStop(OUT TBrowser *** pppBrowserStop) const { return (TBrowser **)PrgpvGetElementsStop(OUT (void ***)pppBrowserStop); }
};


#endif // TBROWSER_H
