///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ITREEITEM_H
#define ITREEITEM_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_coTreeItem_Default			d_coBlack
#define d_coTreeItem_UnreadMessages		d_coGreenDark


///////////////////////////////////////////////////////////////////////////////////////////////////
//	class CTreeWidgetItem
//
//	The class CTreeWidgetItem provides a layer of abstraction between the QTreeWidget and the CChatConfiguration.
//	This way, if for some reason, it is better to use a different control, such as a QTreeView, then the impact on the code is minimal.
//
//	Every Tree Item in the navigation tree must inherit CTreeWidgetItem so we can easily determine
//	which ITReeItem is selected in the navigation tree.
//	Of course, the m_piTreeItem could be stored in the 'type' (aka member variable rtti) of the QTreeWidgetItem,
//	however since the member variable rtti is declared as an 'int', I feel it is quite risky to store a pointer on a 64-bit architecture.
//	Anyways, if there is a need for such optimization, we may do it later.
//
//	INTERFACE NOTES
//	Although the class CTreeWidgetItem contains the word 'Widget', its parent class QTreeWidgetItem is NOT inheriting from QWidget nor from QObject.
class CTreeWidgetItem : public QTreeWidgetItem
{
public:
	ITreeItem * m_piTreeItem;	// Interface of the object containing the data of the Tree Item.  The data is stored and allocated by the CChatConfiguration object.

public:
	inline void setVisible(bool fVisible) { setHidden(!fVisible); }
	void SetItemVisibleAlongWithItsParents(BOOL fVisible);
	void ItemFlagsAdd(Qt::ItemFlag efItemFlagsAdd);
	void ItemFlagsRemove(Qt::ItemFlag efItemFlagsRemove);
	void ItemFlagsEditingEnable() { ItemFlagsAdd(Qt::ItemIsEditable); }
	void ItemFlagsEditingDisable() { ItemFlagsRemove(Qt::ItemIsEditable); }
};

class WTreeWidget : public QTreeWidget
{
public:
	BOOL FIsEditingTreeWidgetItem() const { return (state() == EditingState); }	// This wrapper is necessary because the method state() is protected
	void DisableEditingTreeWidgetItem() { setState(NoState); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	class WLayout
//	This base class contains the core functionality for evey widget to appear on the right side of the Navigation Tree.
//	This class inherits QSplitter so the widget takes the entire space when the application window is resized as well as
//	displaying several widgets such as WNotice(s).
class WLayout : public WSplitterVertical
{
public:
	virtual ~WLayout();
	virtual void Layout_SetFocus();
	virtual void Layout_NoticeDisplay(IN INotice * piwNotice);
	void Layout_NoticeAuxiliaryAdd(PA_DELETING INotice * paiwNotice);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Interface for any object to be selected from the navigation tree WChatNavigation.
//
//	This interface inherits IXmlExchangeObjectID instead of IXmlExchange, so every ITreeItem has an ID in the XML file.  Of course,
//	there is a bit of waste of memory (4 bytes) because some object may not need an identifier, however the tradeoff may well worth code orthogonality.
class ITreeItem : public IXmlExchangeObjectID
{
public:
	CStr m_strNameDisplayTyped;				// Text to display in the Navigation Tree.  This text may be anything the user typed when renaming the Tree Item.
	CTreeWidgetItem * m_paTreeWidgetItem;	// Corresponding widget to appear in the Navigation Tree.  The text of this Tree Item is initialized with TreeItem_PszGetNameDisplay() which by default uses m_strNameDisplayTyped.

	enum // Values for m_uFlagsTreeItem
		{
		FTI_kfRecommended						= 0x00000001,	// The Tree Item (typically a contact or a group) was recommended by the user.  This flag is used for /api Contact.Recommendations.Get
		FTI_kfFavorites							= 0x00000002,	// NYI: The Tree Item is among the user's favorites.  A favorite is private to the user, while a recommendation is public.
		FTI_kfTreeItemNameDisplayedSuggested	= 0x00000004,	// The content of member variable m_strNameDisplayTyped was suggested by another contact, typically when querying a group for its name and list of contacts.
		FTI_kmFlagsSerializeMask				= 0x0000FFFF,	// Bits to save to disk
		FTI_kezIconNone							= 0x00000000,
		FTI_kefIconWarning						= 0x00010000,
		FTI_kefIconError						= 0x00020000,
		FTI_kemIconMask							= 0x000F0000,
		FTI_kfTreeItemBit						= 0x00100000,	// Generic bit to store a boolean value (this bit is used to efficiently merge or delete arrays)
		FTI_kfTreeItemAboutBeingDeleted			= 0x00200000,	// The Tree Item is about being deleted (therefore any reference/pointer to it must be removed).  This flag is typically used when deleting accounts and contacts.
		FTI_kfTreeItemNameDisplayedGenerated	= 0x00400000,	// The member variable m_strNameDisplayTyped has been generated, and therefore should not be serialized (saved to disk).  Since this flag is not stored to disk nor m_strNameDisplayTyped, the display name will be re-geneated each time Cambrian starts.
		// The following flags FTI_kfChatLog* are used by ITreeItemChatLog and ITreeItemChatLogEvents.  The motivation for storing those flags in m_uFlagsTreeItem is avoiding another member variable.
		FTI_kfChatLogBrushColor					= 0x01000000,	// Which brush color to use when displaying events in the Chat Log.
		FTI_kfChatLogEventsRepopulateAll		= 0x02000000,	// Repopulate all events of the Chat Log next time the Tree Item gets the focus
		FTI_kfChatLogEventsIncludeFromWallet	= 0x04000000	// Include wallet events when repopulating the Chat Log.  This is a small optimization preventing searching all wallets when there is nothing
		};
	UINT m_uFlagsTreeItem;			// Various flags regarding the Tree Item (icon, error, Chat Log).  Some of those values are serialized (saved to disk)

public:
	ITreeItem();
	virtual ~ITreeItem();

	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);	// From IXmlExchange
	virtual BOOL TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC;
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;
	virtual void TreeItem_IconUpdate();
	//virtual void TreeItem_IconUpdateOnNewMessageArrived(PSZUC pszMessage);
	virtual void TreeItem_IconUpdateOnMessagesRead();
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenuContext);
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);
	virtual void TreeItem_GotFocus();

	void TreeItemLayout_SetFocus();

	void TreeItem_SelectWithinNavigationTree();
	void TreeItem_SelectWithinNavigationTreeExpanded();
	void TreeItem_RemoveFromNavigationTree();
	void TreeItem_DisplayWithinNavigationTreeBefore(ITreeItem * pTreeItemBefore);
	void TreeItem_DisplayWithinNavigationTree(ITreeItem * pParent);
	void TreeItem_DisplayWithinNavigationTree(ITreeItem * pParent, EMenuAction eMenuActionIcon);
	void TreeItem_DisplayWithinNavigationTree(ITreeItem * pParent, PSZAC pszName, EMenuAction eMenuActionIcon);
	TTreeItemDemo * TreeItem_PAllocateChild(PSZAC pszName, EMenuAction eMenuActionIcon);
	TTreeItemDemo * TreeItem_PAllocateChild(PSZAC pszName, EMenuAction eMenuActionIcon, PSZAC pszDescription, PSZAC pszSearch = NULL);
	void TreeItem_AllocateChildren_VEZ(EMenuAction eMenuActionIcon, PSZAC pszName, ...);
	void TreeItem_Rename();
	void TreeItem_SetTextColor(QRGB coTextColor);
	void TreeItem_SetIconWithToolTip(EMenuAction eMenuIcon, const QString & sToolTip = c_sEmpty);
	void TreeItem_SetIcon(EMenuAction eMenuIcon);
	void TreeItem_SetTextColorAndIcon(QRGB coTextColor, EMenuAction eMenuIcon);
	void TreeItem_SetTextToDisplayNameTyped();
	void TreeItem_SetTextToDisplayNameIfGenerated();
	void TreeItem_SetTextToDisplayMessagesUnread(int cMessagesUnread);
	void TreeItem_SetIconError(PSZUC pszuErrorMessage, EMenuAction eMenuIcon = eMenuIconFailure);
	ITreeItem * TreeItem_PGetParent() const;

	CTreeWidgetItem * TreeItemWidget_PAllocate();
	BOOL TreeItemWidget_FIsExpanded() const;
	void TreeItemWidget_Expand();
	void TreeItemWidget_Collapse();
	void TreeItemWidget_EnsureVisible();
	void TreeItemWidget_ToggleVisibility();
	void TreeItemWidget_Hide();

	void TreeItem_DisplayTransactionsBitcoin();
protected:
	PSZUC _PszGetDisplayNameOr(const CStr & strName) const;
	void _FlushDisplayNameIfGenerated();
}; // ITreeItem

class TTreeItemDemo : public ITreeItem
{
	RTI_IMPLEMENTATION(TTreeItemDemo)
protected:
	class WLayoutDemo * m_pawLayoutDemo;
public:
	CStr m_strSearch;
	CStr m_strDescription;
public:
	TTreeItemDemo();
	TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuAction eMenuActionIcon);
	TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuAction eMenuActionIcon, PSZAC pszDescription, PSZAC pszSearch = NULL);
	~TTreeItemDemo();
	virtual void TreeItem_GotFocus();
}; // TTreeItemDemo


class CArrayPtrTreeItems : public CArrayPtrXmlSerializableObjects
{
public:
	inline ITreeItem ** PrgpGetTreeItemsStop(OUT ITreeItem *** pppTreeItemStop) const { return (ITreeItem **)PrgpvGetElementsStop(OUT (void ***)pppTreeItemStop); }
	void DeleteTreeItem(PA_DELETING ITreeItem * paTreeItem);
	void DeleteAllTreeItems();

	void ForEach_ClearFlagTreeItem(UINT kfFlagTreeItem) const;
	void ForEach_SetFlagTreeItem(UINT kfFlagTreeItem) const;
	void RemoveAllTreeItemsMatchingFlag(UINT kfFlagTreeItem);

	void ForEach_SetFlagTreeItemAboutBeingDeleted() const;
	void RemoveAllTreeItemsAboutBeingDeleted();
	void RemoveTreeItems(const CArrayPtrTreeItems & arraypTreeItemsToRemove);
};

#endif // ITREEITEM_H
