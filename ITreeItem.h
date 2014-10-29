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
	void RemoveAllChildItemsR();
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
		FTI_kfTreeItem_IsExpanded					= 0x00000001,	// Expand this Tree Item when adding it to the Navigation Tree.  For instance, a user may wish to expand a group to see each group member.
		FTI_kfTreeItem_NameDisplayedSuggested		= 0x00000002,	// The content of member variable m_strNameDisplayTyped was suggested by another contact, typically when querying a group for its name and list of contacts.
		FTI_kfTreeItem_RequiresEventOnTimer			= 0x00000004,	// The Tree Item has time sensitive information and requires a timer event.  This flag is in the base class because any parent of the Tree Item also has this flag set when one of its child needs a timer event.  This way, it is possible to optimize timer requirements.
		FTI_kfRecommended							= 0x00000010,	// The Tree Item (typically a contact or a group) was recommended by the user.  This flag is used for /api Contact.Recommendations.Get
		FTI_kfFavorites								= 0x00000020,	// NYI: The Tree Item is among the user's favorites.  A favorite is private to the user, while a recommendation is public.
		FTI_kfObjectInvisible						= 0x00000040,	// The Tree Item is no longer visible (by default) in the Navigation Tree.  This is typically when a contact or group has been deleted and accessible in the 'Recycle Bin'.  An invisible item is something 'half deleted' which may permanently deleted later.
		FTI_kmTreeItem_FlagsSerializeMask			= 0x0000FFFF,	// Bits to save to disk

		//FTI_kfTreeItem_AboutBeingDeleted			= 0x00010000,	// The Tree Item is about being deleted (therefore any reference/pointer to it must be removed).  This flag is typically used when deleting contacts and groups.
		FTI_kfTreeItem_DoNotSerializeToDisk			= 0x00020000,	// This flag indicates the object is 'deleted' and therefore should not be serialized into disk.  Using this flag is a safe mechanism to 'delete' objects, because if the object is not serialized, then it won't be present next time the application starts.
		FTI_kfTreeItem_Temporary					= 0x00040000,	// The object is temporary and should not be saved to disk. This value is used for performance testing to generate thousand of temporary objects.
		//FTI_kfTreeItem_CannotBeDeletedFromMemory	= 0x00040000,	// The Tree Item cannot be deleted because it is in used by another object(s) which cannot be deleted from memory.  A typical case is a JavaScript object OJapi having a reference to the Tree Item.  Deleting such an object would crash the application.
		FTI_kfTreeItem_NameDisplayedGenerated		= 0x00080000,	// The member variable m_strNameDisplayTyped has been generated, and therefore should not be serialized (saved to disk).  Since this flag is not stored to disk nor m_strNameDisplayTyped, the display name will be re-geneated each time Cambrian starts.

		FTI_keIcon_zNone							= 0x00000000,
		FTI_keIcon_Warning							= 0x00100000,
		FTI_keIcon_Error							= 0x00200000,
		FTI_keIcon_mComposingText					= 0x00300000,	// The user is typing something, and therefore display the pencil icon next to the Tree Item.  Originally, this flag was related to a contact, however was used later when a user was typing something in a group chat, and now used for almost any object where the user is typing something.  Also, this flag is propagated to its parent if the Tree Item is not visible in the Navigation Tree.
		FTI_kmIconMask								= 0x00300000,
		FTI_kfTreeItem_fBit							= 0x01000000,	// Generic bit to store a boolean value (this bit is used to efficiently merge or delete arrays)
		FTI_kfTreeItemInToolbarTabs					= 0x02000000,	// The Tree Item is visible in the toolbar tabs and therefore should be updated there as well

		// The following flags FTI_kfChatLog* are used by ITreeItemChatLog and ITreeItemChatLogEvents.  The motivation for storing those flags in m_uFlagsTreeItem is avoiding another member variable.
		FTI_kfChatLog_BrushColor					= 0x10000000,	// Which brush color to use when displaying events in the Chat Log.  Since there are only two brushes of similar color, a single bit is sufficient to remember which one.
		FTI_kfChatLogEvents_RepopulateAll			= 0x20000000,	// Repopulate all events of the Chat Log next time the Tree Item gets the focus
		FTI_kfChatLogEvents_IncludeFromWallet		= 0x40000000	// Include wallet events when repopulating the Chat Log.  This is a small optimization preventing searching all wallets when there is nothing
		};
	UINT m_uFlagsTreeItem;			// Various flags regarding the Tree Item (icon, error, Chat Log).  Some of those values are serialized (saved to disk)

public:
	ITreeItem();
	virtual ~ITreeItem();

	inline UINT TreeItemFlags_FIsComposingText() const { return ((m_uFlagsTreeItem & FTI_kmIconMask) == FTI_keIcon_mComposingText); }
	inline UINT TreeItemFlags_FuIsRecommended() const { return (m_uFlagsTreeItem & FTI_kfRecommended); }
	inline UINT TreeItemFlags_FuIsInvisible() const { return (m_uFlagsTreeItem & FTI_kfObjectInvisible); }
	inline BOOL TreeItemFlags_FCanDisplayWithinNavigationTree() const { return (m_uFlagsTreeItem & (FTI_kfObjectInvisible | FTI_kfTreeItem_DoNotSerializeToDisk)) == 0; }	// Any invisible or deleted item should not be displayed within the Navigation Tree
	inline UINT TreeItemFlags_FuIsDeleted() const { return (m_uFlagsTreeItem & FTI_kfTreeItem_DoNotSerializeToDisk); }
	inline UINT TreeItemFlags_FuIsDeletedOrTemporary() const { return (m_uFlagsTreeItem & (FTI_kfTreeItem_DoNotSerializeToDisk | FTI_kfTreeItem_Temporary)); }
	inline void TreeItemFlags_SetFlagSerializeToDisk_Yes() { m_uFlagsTreeItem &= ~FTI_kfTreeItem_DoNotSerializeToDisk; }
	void TreeItem_MarkForDeletion();
	void TreeItem_MarkForRecommendation(bool rec);
	bool isRecommended();

	void TreeItem_SetNameDisplaySuggested(PSZUC pszNameDisplay);
	CString TreeItem_SGetNameDisplay() CONST_MCC;

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);	// From IXmlExchange
	virtual void TreeItem_RemoveAllReferencesToObjectsAboutBeingDeleted();
	virtual BOOL TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC;
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;
	virtual void TreeItem_IconUpdate();
	virtual void TreeItem_IconUpdateOnMessagesRead();
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenuContext);
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);
	virtual void TreeItem_GotFocus();

	void TreeItemLayout_SetFocus();
	EMenuAction TreeItem_EDisplayContextMenu(const EMenuActionByte rgzeMenuActions[]);

	// Methods interacting with the CTreeItemW
	void TreeItemW_SelectWithinNavigationTree();
	void TreeItemW_SelectWithinNavigationTreeExpanded();
	void TreeItemW_RemoveFromNavigationTree();
	void TreeItemW_DisplayWithinNavigationTreeBefore(ITreeItem * pTreeItemBefore);
	void TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ);
	void TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ, EMenuIcon eMenuIcon);
	void TreeItemW_DisplayWithinNavigationTreeExpand(ITreeItem * pParent_YZ, PSZAC pszName, EMenuIcon eMenuIcon);
	TTreeItemDemo * TreeItemW_PAllocateChild(PSZAC pszName, EMenuIcon eMenuIcon);
	TTreeItemDemo * TreeItemW_PAllocateChild(PSZAC pszName, EMenuIcon eMenuIcon, PSZAC pszDescription, PSZAC pszSearch = NULL);
	void TreeItemW_AllocateChildren_VEZ(EMenuIcon eMenuIcon, PSZAC pszName, ...);
	void TreeItemW_Rename();
	void TreeItemW_SetTextColor(QRGB coTextColor);
	void TreeItemW_SetIconWithToolTip(EMenuIcon eMenuIcon, const QString & sToolTip = c_sEmpty);
	void TreeItemW_SetIcon(EMenuIcon eMenuIcon);
	void TreeItemW_SetTextColorAndIcon(QRGB coTextColor, EMenuIcon eMenuIcon);
	void TreeItemW_UpdateText();
	void TreeItemW_SetTextToDisplayNameIfGenerated();
	void TreeItemW_SetTextToDisplayMessagesUnread(int cMessagesUnread);
	void TreeItemW_SetIconComposingText();
	void TreeItemW_SetIconError(PSZUC pszuErrorMessage, EMenuIcon eMenuIcon = eMenuIcon_Failure);
	ITreeItem * TreeItemW_PGetParent() const;

	CTreeItemW * TreeItemW_PAllocate();
	BOOL TreeItemW_FIsExpanded() const;
	void TreeItemW_Expand();
	void TreeItemW_ExpandAccordingToSavedState();
	void TreeItemW_Collapse();
	void TreeItemW_EnsureVisible();
	void TreeItemW_ToggleVisibility();
	void TreeItemW_ToggleVisibilityAndSetFocus();
	void TreeItemW_Hide();

	void TreeItemW_DisplayTransactionsBitcoin();
protected:
	PSZUC _PszGetDisplayNameOr(const CStr & strName) const;
	void _FlushDisplayNameIfGenerated();
}; // ITreeItem


class CArrayPtrTreeItems : public CArrayPtrXmlSerializableObjects
{
public:
	inline ITreeItem ** PrgpGetTreeItemsStop(OUT ITreeItem *** pppTreeItemStop) const { return (ITreeItem **)PrgpvGetElementsStop(OUT (void ***)pppTreeItemStop); }
	inline ITreeItem ** PrgpGetTreeItemsStopMax(OUT ITreeItem *** pppTreeItemStop, int cTreeItemsMax) const { return (ITreeItem **)PrgpvGetElementsStopMax(OUT (void ***)pppTreeItemStop, cTreeItemsMax); }
	void DeleteTreeItem(PA_DELETING ITreeItem * paTreeItem);
	void DeleteAllTreeItems();

	void ForEach_ClearFlagTreeItem(UINT kfFlagTreeItem) const;
	void ForEach_SetFlagTreeItem(UINT kfFlagTreeItem) const;
	void RemoveAllTreeItemsMatchingFlag(UINT kfFlagTreeItem);

	void ForEach_SetFlagTreeItemDoNotSerializeToDisk() const;
	void RemoveAllUnserializableTreeItems();
	void RemoveTreeItems(const CArrayPtrTreeItems & arraypTreeItemsToRemove);
};

class ITreeItemOfProfile : public ITreeItem
{
public:
	TProfile * m_pProfile;

public:
	ITreeItemOfProfile(TProfile * pProfile) { m_pProfile = pProfile; }
};

class ITreeItemOfProfileOrphaned : public ITreeItemOfProfile
{
public:
	ITreeItemOfProfileOrphaned(TProfile * pProfile) : ITreeItemOfProfile(pProfile) {  }
};

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
	TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuIcon eMenuIcon);
	TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuIcon eMenuIcon, PSZAC pszDescription, PSZAC pszSearch = NULL);
	~TTreeItemDemo();
	virtual void TreeItem_GotFocus();
}; // TTreeItemDemo

#endif // ITREEITEM_H
