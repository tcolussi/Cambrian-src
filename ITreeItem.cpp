///////////////////////////////////////////////////////////////////////////////////////////////////
//	ITreeItem.cpp
//
//	Implementations for the interfaces IRuntimeObject and ITreeItem.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Recursively remove all child items from the Navigation Tree.
//	This does not delete any ITreeItem.
void
CTreeItemW::RemoveAllChildItemsR()
	{
	int cChildren = childCount();
	while (--cChildren >= 0)
		{
		CTreeItemW * pChild = (CTreeItemW *)child(cChildren);
		Assert(pChild->m_piTreeItem != NULL);
		Assert(pChild->m_piTreeItem->m_paTreeItemW_YZ == pChild);
		pChild->RemoveAllChildItemsR();
		Assert(pChild->m_piTreeItem != NULL);
		Assert(pChild->m_piTreeItem->m_paTreeItemW_YZ == pChild);
		pChild->m_piTreeItem->m_paTreeItemW_YZ = NULL;
		delete pChild;
		}
	}

ITreeItem *
CTreeItemW::PFindChildItemMatchingRuntimeClass(RTI_ENUM rti) const
	{
	// Search if there is already a node recommendations
	int cChildren = childCount();
	while (--cChildren >= 0)
		{
		CTreeItemW * pChild = (CTreeItemW *)child(cChildren);
		Assert(pChild->m_piTreeItem != NULL);
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pChild->m_piTreeItem) != NULL);
		if (pChild->m_piTreeItem->EGetRuntimeClass() == rti)
			return pChild->m_piTreeItem;
		}
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
ITreeItem::ITreeItem()
	{
	m_paTreeItemW_YZ = NULL;
	m_uFlagsTreeItem = 0;
	}

ITreeItem::~ITreeItem()
	{
	NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(IN this);
	NoticeListRoaming_TreeItemDeleting(IN this);
//	Assert(m_paTreeItemW_YZ == NULL);	// Use TreeItemW_RemoveFromNavigationTree() instead.
	delete m_paTreeItemW_YZ;	// This will remove the widget from the Navigation Tree.  Of course, this is not the most elegant mechanism, but it is the safest to avoid bugs
	}

void
ITreeItem::TreeItem_MarkForDeletion()
	{
	m_uFlagsTreeItem = (m_uFlagsTreeItem & ~FTI_kfRecommended) | (FTI_kfTreeItem_DoNotSerializeToDisk | FTI_kfObjectInvisible);	// If an object is to be deleted, then it should not serialized to disk nor be visible in the Navigation Tree.  Also, the object should no longer be recommended.
	TreeItemW_RemoveFromNavigationTree();
	}

void ITreeItem::TreeItem_MarkForRecommendation(bool rec)
	{
	if (rec)
		m_uFlagsTreeItem |= FTI_kfRecommended;
	else
		m_uFlagsTreeItem &= ~FTI_kfRecommended;
	}

bool ITreeItem::isRecommended()
	{
		return ((m_uFlagsTreeItem & FTI_kfRecommended) != 0);
	}

//	ITreeItem::IRuntimeObject::PGetRuntimeInterface()
POBJECT
ITreeItem::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	if (rti == RTI(ITreeItem))
		return (ITreeItem *)this;
	return IXmlExchangeObjectID::PGetRuntimeInterface(rti, piParent);
	}

//	ITreeItem::IXmlExchange::XmlExchange()
//
//	Variables used: ("N" + "F")
void
ITreeItem::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	IXmlExchangeObjectID::XmlExchange(pXmlExchanger);
	if (pXmlExchanger->m_fSerializing && m_paTreeItemW_YZ != NULL)
		{
		// Remember the state of the Tree Item is expanded or collapsed
		if (m_paTreeItemW_YZ->isExpanded())
			m_uFlagsTreeItem |= FTI_kfTreeItem_IsExpanded;
		else
			m_uFlagsTreeItem &= ~FTI_kfTreeItem_IsExpanded;
		}
	pXmlExchanger->XmlExchangeStrConditional("N", INOUT &m_strNameDisplayTyped, (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated) == 0);	// Save the name before the flags (it makes the XML file a bit more organized, as the name is always present while the flags are not)
	pXmlExchanger->XmlExchangeUIntHexFlagsMasked("F", INOUT &m_uFlagsTreeItem, FTI_kmTreeItem_FlagsSerializeMask);

	#if 0
	//m_uFlagsTreeItem |= FTI_kfObjectInvisible;
	if (m_uFlagsTreeItem != 0)
		MessageLog_AppendTextFormatCo(d_coBlack, "0x$p: ITreeItem::XmlExchange($s) m_uFlagsTreeItem = 0x$x\n", this, TreeItem_PszGetNameDisplay(), m_uFlagsTreeItem);
	#endif

	if (!pXmlExchanger->m_fSerializing && m_strNameDisplayTyped.FIsEmptyString())
		pXmlExchanger->XmlExchangeStr("NameDisplay", INOUT &m_strNameDisplayTyped);	// Compatibility with the old file format (to be removed in 2015)
	} // XmlExchange()

//	TreeItem_RemoveAllReferencesToObjectsAboutBeingDeleted(), virtual
//
//	This virtual method is implemented for orphan Tree Items (the parent Tree Item does not care about the object), and therefore the object must be notified in some way.
void
ITreeItem::TreeItem_RemoveAllReferencesToObjectsAboutBeingDeleted()
	{
	}

//	TreeItem_FContainsMatchingText(), virtual
//
//	Return TRUE if the object contains the text content of pszTextSearchLowercase.
//	Objects inheriting from ITreeItem must implement this method if they want to provide additional fields to search.
//
//	PERFORMANCE NOTES
//	To improve the search performances, the text to search (pszTextSearchLowercase) must be in lowercase.
BOOL
ITreeItem::TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC
	{
	return (PszrStringContainsSubStringLowerCase(TreeItem_PszGetNameDisplay(), (PSZAC)pszTextSearchLowercase) != NULL);
	}

//	TreeItem_PszGetNameDisplay(), virtual
//
//	Return the text to display int he Navigation Tree.
//	This virtual method enables the objects inheriting from ITreeItem to implement dynamic text.
PSZUC
ITreeItem::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return m_strNameDisplayTyped;
	}

//	Wrapper to display m_strNameDisplayTyped if not empty, or strName
PSZUC
ITreeItem::_PszGetDisplayNameOr(const CStr & strName) const
	{
	PSZUC pszNameDisplay = m_strNameDisplayTyped;
	if (pszNameDisplay[0] != '\0')
		return pszNameDisplay;
	return strName;
	}

void
ITreeItem::_FlushDisplayNameIfGenerated()
	{
	if (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated)
		m_strNameDisplayTyped.Empty();
	}

//	TreeItem_IconUpdate(), virtual
//
//	Give the opportunity of the Tree Item to update its icon to reflect its new state.
//	Originally, this method belonged to a contact, where a contact could become online, offline, busy and so on.
//	As Cambrian evolved, other Tree Items needed to dynamically update their icons, and therefore, this method became virtual for all Tree Items.
void
ITreeItem::TreeItem_IconUpdate()
	{
	}

/*
//	TreeItem_IconUpdateOnNewMessageArrived(), virtual
//
//	Update the icon of the Tree Item because a new message arrived.
//	Again, this method was originally for contacts, however other Tree Items such as groups, are receiving messages.
//	Considering it is likely many other Tree Items will receive messages, it is logical to have this method virtual for all Tree Items.
void
ITreeItem::TreeItem_IconUpdateOnNewMessageArrived(PSZUC pszMessage)
	{
	Assert(pszMessage);
	}
*/

//	TreeItem_IconUpdateOnMessagesRead(), virtual
//
//	This virtual method is complement to TreeItem_IconUpdateOnNewMessageArrived() and it is called
//	when the user clicks on the Tree Item or set the focus to the Layout displaying the messages related to the Tree Item.
//	In a nutshell, this method is called whenever the GUI assumes the reader read all the messages related to the Tree Item.
void
ITreeItem::TreeItem_IconUpdateOnMessagesRead()
	{

	}

//	TreeItem_MenuAppendActions(), virtual
//
//	Give the opportunity of the Tree Item to add menu items to the context menu.
void
ITreeItem::TreeItem_MenuAppendActions(IOUT WMenu * pMenuContext)
	{
	Assert(pMenuContext != NULL);
	}

//	TreeItem_EDoMenuAction(), virtual
//
//	Notify the Tree Item the user selected a menu action.
//	Return ezMenuActionNone if the Tree Item handled the menu action and does not want any future processing.
//	Return eMenuAction if the menu action was not handled, or the Tree Item does not care if the parent (Main Window) handles the menu action again.
EMenuAction
ITreeItem::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_TreeItemRecommended:
		m_uFlagsTreeItem ^= FTI_kfRecommended;
		return ezMenuActionNone;
	case eMenuSpecialAction_ITreeItemRenamed:
		m_uFlagsTreeItem &= ~FTI_kfTreeItem_NameDisplayedGenerated;
		return ezMenuActionNone;
	default:
		return eMenuAction;
		}
	}

//	TreeItem_GotFocus(), virtual
//
//	Notification the TreeItem received the focus (was selected from the Navigation Tree)
//	Each class inheriting from ITreeItem should implement this virtual method, create an object inheriting WLayout and
//	call MainWindow_SetCurrentChatLayout().
void
ITreeItem::TreeItem_GotFocus()
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Object must implement TreeItem_GotFocus()\n");
	MainWindow_SetCurrentLayout(NULL);	// Make sure there are no old layout
	}

//	Set the focus on the layout of the Tree Item
void
ITreeItem::TreeItemLayout_SetFocus()
	{
	TreeItemW_SelectWithinNavigationTree();	// Make sure the Tree Item of the layout is visible and selected
	MainWindow_SetFocusToCurrentLayout();
	}

#define d_coGreenLime		MAKE_QRGB(0, 255, 0)

void
ITreeItem::TreeItemW_SetIconWithToolTip(EMenuIcon eMenuIcon, const QString & sToolTip)
	{
	if (m_paTreeItemW_YZ == NULL)
		return;	// Sometimes the Tree Item is not visible in the Navigation Tree, so there is no need to update its icon or tool tip
	m_paTreeItemW_YZ->setIcon(0, OGetIcon(eMenuIcon));
	m_paTreeItemW_YZ->setToolTip(0, sToolTip);
    #ifdef COMPILE_WITH_TOOLBAR
    if (m_uFlagsTreeItem & ITreeItem::FTI_kfTreeItemInToolbarTabs)

        Toolbar_TabRedraw(this);	// Also update the icon on the toolbar
    #endif
	}

//	Set the icon of the QTreeWidgetItem from a menu action.
void
ITreeItem::TreeItemW_SetIcon(EMenuIcon eMenuIcon)
	{
	TreeItemW_SetIconWithToolTip(eMenuIcon);
	}

void
ITreeItem::TreeItemW_SetTextColorAndIcon(QRGB coTextColor, EMenuIcon eMenuIcon)
	{
	if (m_paTreeItemW_YZ == NULL)
		return;	// Sometimes the Tree Item is not visible in the Navigation Tree, so there is no need to update its icon or tool tip
	m_paTreeItemW_YZ->setTextColor(0, coTextColor);
	TreeItemW_SetIcon(eMenuIcon);
	}

void
ITreeItem::TreeItemW_SetTextColor(QRGB coTextColor)
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setTextColor(0, coTextColor);
	}

void
ITreeItem::TreeItemW_SetTextToDisplayNameIfGenerated()
	{
	if (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated)
		{
		m_strNameDisplayTyped.Empty();
		TreeItemW_UpdateText();
		}
	}

//	Update the text in the Navigation Tree to the default value
void
ITreeItem::TreeItemW_UpdateText()
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setText(0, CString(TreeItem_PszGetNameDisplay()));
	}

void
ITreeItem::TreeItemW_SetTextToDisplayMessagesUnread(int cMessagesUnread)
	{
	if (cMessagesUnread <= 0)
		{
		TreeItemW_UpdateText();
		return;
		}
	else if (m_paTreeItemW_YZ != NULL)
		{
		CStr str;
		str.Format("$s ($i)", TreeItem_PszGetNameDisplay(), cMessagesUnread);
		m_paTreeItemW_YZ->setText(0, str);
		}
	}

void
ITreeItem::TreeItemW_SetIconComposingText()
	{
	m_uFlagsTreeItem |= FTI_keIcon_mComposingText;
	TreeItem_IconUpdate();
	}

void
ITreeItem::TreeItemW_SetIconError(PSZUC pszuErrorMessage, EMenuIcon eMenuIcon)
	{
	Assert(pszuErrorMessage != NULL);
	CString sError(pszuErrorMessage);
	if ((m_uFlagsTreeItem & FTI_kmIconMask) == FTI_keIcon_Error)
		{
		// There is already an error, so concatenate both, unless this is the same error.
		// Sometimes a server may send two stanzas with the same identical error.  I have no idea what is the motivation (or if it is a bug), however the same error should be displayed only once
		if (m_paTreeItemW_YZ != NULL)
			{
			QString sErrorPrevious = m_paTreeItemW_YZ->toolTip(0);
			if (!sErrorPrevious.contains(sError))
				{
				if (!sErrorPrevious.endsWith("</ol>"))
					{
					sError.Format("<div style='margin:-25px;'><ol><li>$Q</li><li>$Q</li></ol></div>", &sErrorPrevious, &sError);
					}
				else
					{
					sErrorPrevious.truncate(sErrorPrevious.length() - 5);	// Remove the "</ul>"
					sError.Format("$Q<li>$Q</li></ol></div>", &sErrorPrevious, &sError);
					}
				}
			}
		}
	//qApp->setStyleSheet("QToolTip { background-color: red }");
	//qApp->setStyleSheet("QToolTip { indent: 3 }");
	TreeItemW_SetIconWithToolTip(eMenuIcon, sError);
	m_uFlagsTreeItem = (m_uFlagsTreeItem & ~FTI_kmIconMask) | FTI_keIcon_Error;
	} // TreeItemW_SetIconError()

BOOL
ITreeItem::TreeItemW_FIsExpanded() const
	{
	if (m_paTreeItemW_YZ != NULL)
		return m_paTreeItemW_YZ->isExpanded();
	return FALSE;
	}

void
ITreeItem::TreeItemW_Expand()
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setExpanded(true);
	}

void
ITreeItem::TreeItemW_ExpandAccordingToSavedState()
	{
	if ((m_uFlagsTreeItem & FTI_kfTreeItem_IsExpanded) && m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setExpanded(true);
	}

void
ITreeItem::TreeItemW_Collapse()
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setExpanded(false);
	}

void
ITreeItem::TreeItemW_EnsureVisible()
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->SetItemVisibleAlongWithItsParents(TRUE);
	}

void
ITreeItem::TreeItemW_ToggleVisibility()
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setVisible(m_paTreeItemW_YZ->isHidden());
	}

void
ITreeItem::TreeItemW_ToggleVisibilityAndSetFocus()
	{
	TreeItemW_ToggleVisibility();
	TreeItemW_Expand();
	TreeItemLayout_SetFocus();
	}

void
ITreeItem::TreeItemW_Hide()
	{
	if (m_paTreeItemW_YZ != NULL)
		m_paTreeItemW_YZ->setVisible(false);
	}

ITreeItem *
ITreeItem::TreeItemW_PGetParent() const
	{
	if (m_paTreeItemW_YZ != NULL)
		{
		CTreeItemW * poParent = (CTreeItemW *)m_paTreeItemW_YZ->parent();
		if (poParent != NULL)
			{
			Assert(poParent->m_piTreeItem != NULL);
			Assert(PGetRuntimeInterfaceOf_ITreeItem(poParent->m_piTreeItem) != NULL);
			return poParent->m_piTreeItem;
			}
		}
	return NULL;
	}


//	Remove the Tree Item from the array (and from the Navigation Tree if present) and delete it from memory if possible.
void
CArrayPtrTreeItems::DeleteTreeItem(PA_DELETING ITreeItem * paTreeItem)
	{
	Assert(paTreeItem != NULL);
	Assert(PGetRuntimeInterfaceOf_ITreeItem(paTreeItem) == paTreeItem);
	(void)RemoveElementAssertI(paTreeItem);
	delete paTreeItem;	// The deletion of ITreeItem will also delete m_paTreeItemW_YZ (if present) which will remove the 'widget' from the Navigation Tree
	/*
	if ((paTreeItem->m_uFlagsTreeItem & ITreeItem::FTI_kfTreeItem_CannotBeDeletedFromMemory) == 0)
		{
		delete paTreeItem;	// The deletion of ITreeItem will also delete m_paTreeItemW_YZ (if present) which will remove the 'widget' from the Navigation Tree
		}
	else
		{
		// The Tree Item cannot be deleted, therefore make sure it will never be serialized
		paTreeItem->m_uFlagsTreeItem |= ITreeItem::FTI_kfTreeItem_DoNotSerializeToDisk;
		delete paTreeItem->m_paTreeItemW_YZ;	// Make sure it is no longer visible in the Navigation Tree
		paTreeItem->m_paTreeItemW_YZ = NULL;
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[MemoryLeak] TreeItem 0x$p '$S' is not deleted because it is used by another object which cannot be deleted\n", paTreeItem, &paTreeItem->m_strNameDisplayTyped);
		}
	*/
	}


//	This method may be replaced with CArrayPtrRuntimeObjects::DeleteAllRuntimeObjects()
void
CArrayPtrTreeItems::DeleteAllTreeItems()
	{
	ITreeItem ** ppaTreeItemStop;
	ITreeItem ** ppaTreeItem = PrgpGetTreeItemsStop(OUT &ppaTreeItemStop);
	while (ppaTreeItem != ppaTreeItemStop)
		{
		delete *ppaTreeItem++;
		}
	Empty();
	}

void
CArrayPtrTreeItems::ForEach_ClearFlagTreeItem(UINT kfFlagTreeItem) const
	{
	ITreeItem ** ppTreeItemStop;
	ITreeItem ** ppTreeItem = PrgpGetTreeItemsStop(OUT &ppTreeItemStop);
	while (ppTreeItem != ppTreeItemStop)
		{
		ITreeItem * pTreeItem = *ppTreeItem++;
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
		pTreeItem->m_uFlagsTreeItem &= ~kfFlagTreeItem;
		}
	}

void
CArrayPtrTreeItems::ForEach_SetFlagTreeItem(UINT kfFlagTreeItem) const
	{
	ITreeItem ** ppTreeItemStop;
	ITreeItem ** ppTreeItem = PrgpGetTreeItemsStop(OUT &ppTreeItemStop);
	while (ppTreeItem != ppTreeItemStop)
		{
		ITreeItem * pTreeItem = *ppTreeItem++;
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
		pTreeItem->m_uFlagsTreeItem |= kfFlagTreeItem;
		}
	}

void
CArrayPtrTreeItems::RemoveAllTreeItemsMatchingFlag(UINT kfFlagTreeItem)
	{
	if (m_paArrayHdr == NULL)
		return;
	ITreeItem ** ppTreeItemStop;
	ITreeItem ** ppTreeItemStart = PrgpGetTreeItemsStop(OUT &ppTreeItemStop);
	ITreeItem ** ppTreeItemDst = ppTreeItemStart;
	ITreeItem ** ppTreeItemSrc = ppTreeItemStart;
	while (ppTreeItemSrc != ppTreeItemStop)
		{
		ITreeItem * pTreeItem = *ppTreeItemSrc++;
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
		if ((pTreeItem->m_uFlagsTreeItem & kfFlagTreeItem) == 0)
			*ppTreeItemDst++ = pTreeItem;
		}
	m_paArrayHdr->cElements = ppTreeItemDst - ppTreeItemStart;
	}

void
CArrayPtrTreeItems::ForEach_SetFlagTreeItemDoNotSerializeToDisk() const
	{
	ForEach_SetFlagTreeItem(ITreeItem::FTI_kfTreeItem_DoNotSerializeToDisk);
	}

void
CArrayPtrTreeItems::RemoveAllUnserializableTreeItems()
	{
	RemoveAllTreeItemsMatchingFlag(ITreeItem::FTI_kfTreeItem_DoNotSerializeToDisk);
	}

//	IMPLEMENTATION NOTES: Could also use ForEach_SetCookieValue()
void
CArrayPtrTreeItems::RemoveTreeItems(const CArrayPtrTreeItems & arraypTreeItemsToRemove)
	{
	ForEach_ClearFlagTreeItem(ITreeItem::FTI_kfTreeItem_fBit);
	arraypTreeItemsToRemove.ForEach_SetFlagTreeItem(ITreeItem::FTI_kfTreeItem_fBit);
	RemoveAllTreeItemsMatchingFlag(ITreeItem::FTI_kfTreeItem_fBit);
	}
