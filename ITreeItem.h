///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ITREEITEM_H
#define ITREEITEM_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_coTreeItem_Default			d_coBlack
#define d_coTreeItem_UnreadMessages		d_coGreenDark


///////////////////////////////////////////////////////////////////////////////////////////////////
//	class CTreeItemW
//
//	The class CTreeItemW displays a ITreeItem into the Navigation Tree.
//	Since the Navigation Tree may display only one profile at the time, a ITreeItem may exist without having a corresponding CTreeItemW,
//	however every CTreeItemW has a corresponding ITreeItem.
//
//	Every Tree Item in the Navigation Tree must inherit CTreeItemW so we can easily determine which ITreeItem is selected in the Navigation Tree.
//
//	Of course, the m_piTreeItem could be stored in the 'type' (aka member variable rtti) of the QTreeWidgetItem,
//	however since the member variable rtti is declared as an 'int', I feel it is quite risky to store a pointer on a 64-bit architecture.
//	Anyways, if there is a need for such optimization, we may do it later.
//
//	NAMING CONVENTION
//	The class name CTreeItemW was carefully chosen to be meaningful and distinguishable from CTreeWidgetItem.
//	The word 'Widget' is not part of the class name because, its parent class QTreeWidgetItem is NOT inheriting from QWidget nor from QObject.
//
class CTreeItemW : public CTreeWidgetItem
{
public:
	ITreeItem * m_piTreeItem;	// Interface of the object containing the data of the Tree Item.  The data is stored and allocated by the CChatConfiguration object.

public:
	ITreeItem * PFindChildItemMatchingRuntimeClass(RTI_ENUM rti) const;
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
	CStr m_strNameDisplayTyped;			// Text to display in the Navigation Tree.  This text may be anything the user typed when renaming the Tree Item.
	CTreeItemW * m_paTreeItemW_YZ;		// Corresponding object to appear in the Navigation Tree.  The text of this Tree Item is initialized with TreeItem_PszGetNameDisplay() which by default uses m_strNameDisplayTyped.  This pointer may be NULL if the Tree Item exists, however not visible in the Navigation Tree.

	enum // Values for m_uFlagsTreeItem
		{
		FTI_kfTreeItem_IsExpanded				= 0x00000001,	// Expand this Tree Item when adding it to the Navigation Tree.  For instance, a user may wish to expand a group to see each group member.
		FTI_kfTreeItem_NameDisplayedSuggested	= 0x00000002,	// The content of member variable m_strNameDisplayTyped was suggested by another contact, typically when querying a group for its name and list of contacts.
		FTI_kfRecommended						= 0x00000010,	// The Tree Item (typically a contact or a group) was recommended by the user.  This flag is used for /api Contact.Recommendations.Get
		FTI_kfFavorites							= 0x00000020,	// NYI: The Tree Item is among the user's favorites.  A favorite is private to the user, while a recommendation is public.
		FTI_kmTreeItem_FlagsSerializeMask		= 0x0000FFFF,	// Bits to save to disk

		FTI_kezIconNone							= 0x00000000,
		FTI_kefIconWarning						= 0x00010000,
		FTI_kefIconError						= 0x00020000,
		FTI_kemIconMask							= 0x000F0000,
		FTI_kfTreeItem_fBit						= 0x00100000,	// Generic bit to store a boolean value (this bit is used to efficiently merge or delete arrays)
		FTI_kfTreeItem_AboutBeingDeleted		= 0x00200000,	// The Tree Item is about being deleted (therefore any reference/pointer to it must be removed).  This flag is typically used when deleting accounts and contacts.
		FTI_kfTreeItem_NameDisplayedGenerated	= 0x00400000,	// The member variable m_strNameDisplayTyped has been generated, and therefore should not be serialized (saved to disk).  Since this flag is not stored to disk nor m_strNameDisplayTyped, the display name will be re-geneated each time Cambrian starts.
		// The following flags FTI_kfChatLog* are used by ITreeItemChatLog and ITreeItemChatLogEvents.  The motivation for storing those flags in m_uFlagsTreeItem is avoiding another member variable.
		FTI_kfChatLog_BrushColor				= 0x01000000,	// Which brush color to use when displaying events in the Chat Log.
		FTI_kfChatLogEvents_RepopulateAll		= 0x02000000,	// Repopulate all events of the Chat Log next time the Tree Item gets the focus
		FTI_kfChatLogEvents_IncludeFromWallet	= 0x04000000	// Include wallet events when repopulating the Chat Log.  This is a small optimization preventing searching all wallets when there is nothing
		};
	UINT m_uFlagsTreeItem;			// Various flags regarding the Tree Item (icon, error, Chat Log).  Some of those values are serialized (saved to disk)

public:
	ITreeItem();
	virtual ~ITreeItem();

	inline UINT TreeItemFlags_FuIsRecommended() const { return (m_uFlagsTreeItem & FTI_kfRecommended); }

	void TreeItem_SetNameDisplaySuggested(PSZUC pszNameDisplay);

	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);	// From IXmlExchange
	virtual BOOL TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC;
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;
	virtual void TreeItem_IconUpdate();
	virtual void TreeItem_IconUpdateOnMessagesRead();
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenuContext);
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);
	virtual void TreeItem_GotFocus();

	void TreeItemLayout_SetFocus();

	// Methods interacting with the CTreeItemW
	void TreeItemW_SelectWithinNavigationTree();
	void TreeItemW_SelectWithinNavigationTreeExpanded();
	void TreeItemW_RemoveFromNavigationTree();
	void TreeItemW_DisplayWithinNavigationTreeBefore(ITreeItem * pTreeItemBefore);
	void TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ);
	void TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ, EMenuAction eMenuActionIcon);
	void TreeItemW_DisplayWithinNavigationTreeExpand(ITreeItem * pParent_YZ, PSZAC pszName, EMenuAction eMenuActionIcon);
	TTreeItemDemo * TreeItemW_PAllocateChild(PSZAC pszName, EMenuAction eMenuActionIcon);
	TTreeItemDemo * TreeItemW_PAllocateChild(PSZAC pszName, EMenuAction eMenuActionIcon, PSZAC pszDescription, PSZAC pszSearch = NULL);
	void TreeItemW_AllocateChildren_VEZ(EMenuAction eMenuActionIcon, PSZAC pszName, ...);
	void TreeItemW_Rename();
	void TreeItemW_SetTextColor(QRGB coTextColor);
	void TreeItemW_SetIconWithToolTip(EMenuAction eMenuIcon, const QString & sToolTip = c_sEmpty);
	void TreeItemW_SetIcon(EMenuAction eMenuIcon);
	void TreeItemW_SetTextColorAndIcon(QRGB coTextColor, EMenuAction eMenuIcon);
	void TreeItemW_UpdateText();
	void TreeItemW_SetTextToDisplayNameIfGenerated();
	void TreeItemW_SetTextToDisplayMessagesUnread(int cMessagesUnread);
	void TreeItemW_SetIconError(PSZUC pszuErrorMessage, EMenuAction eMenuIcon = eMenuIconFailure);
	ITreeItem * TreeItemW_PGetParent() const;

	CTreeItemW * TreeItemW_PAllocate();
	BOOL TreeItemW_FIsExpanded() const;
	void TreeItemW_Expand();
	void TreeItemW_ExpandAccordingToSavedState();
	void TreeItemW_Collapse();
	void TreeItemW_EnsureVisible();
	void TreeItemW_ToggleVisibility();
	void TreeItemW_Hide();

	void TreeItemW_DisplayTransactionsBitcoin();
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
