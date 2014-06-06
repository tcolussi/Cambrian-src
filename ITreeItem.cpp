///////////////////////////////////////////////////////////////////////////////////////////////////
//	ITreeItem.cpp
//
//	Implementations for the interfaces IRuntimeObject and ITreeItem.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Make sure the Tree Item is visible and so its parent
void
CTreeWidgetItem::SetItemVisibleAlongWithItsParents(BOOL fVisible)
	{
	setVisible(fVisible);
	if (fVisible)
		{
		CTreeWidgetItem * pParent = (CTreeWidgetItem *)parent();
		if (pParent != NULL)
			{
			// Recursively expand and make the parent visible
			pParent->setExpanded(true);
			pParent->SetItemVisibleAlongWithItsParents(TRUE);
			}
		}
	}

//	We need this method, otherwise the Qt framework will blindly send the signal QTreeWidget::itemChanged() regardless if the flags have been changed, and
//	this will cause a stack overflow in slot SL_TreeItemEdited().
void
CTreeWidgetItem::ItemFlagsAdd(Qt::ItemFlag efItemFlagsAdd)
	{
	const Qt::ItemFlags eFlags = flags();
	const Qt::ItemFlags eFlagsNew = (eFlags | efItemFlagsAdd);
	if (eFlagsNew != eFlags)
		setFlags(eFlagsNew);
	}

void
CTreeWidgetItem::ItemFlagsRemove(Qt::ItemFlag efItemFlagsRemove)
	{
	const Qt::ItemFlags eFlags = flags();
	const Qt::ItemFlags eFlagsNew = (eFlags & ~efItemFlagsRemove);
	if (eFlagsNew != eFlags)
		setFlags(eFlagsNew);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
ITreeItem::ITreeItem()
	{
	m_paTreeWidgetItem = NULL;
	m_uFlagsTreeItem = 0;
	}

ITreeItem::~ITreeItem()
	{
	NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(IN this);
	NoticeListRoaming_TreeItemDeleting(IN this);
	delete m_paTreeWidgetItem;	// This will remove the widget from the Navigation Tree.  Of course, this is not the most elegant mechanism, but it is the safest to avoid bugs
	}

//	ITreeItem::IRuntimeObject::PGetRuntimeInterface()
void *
ITreeItem::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	if (rti == RTI(ITreeItem))
		return (ITreeItem *)this;
	return IXmlExchangeObjectID::PGetRuntimeInterface(rti);
	}

//	ITreeItem::IXmlExchange::XmlExchange()
void
ITreeItem::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	IXmlExchangeObjectID::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStrConditional("NameDisplay", INOUT &m_strNameDisplayTyped, (m_uFlagsTreeItem & FTI_kfTreeItemNameDisplayedGenerated) == 0);
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
	if (m_uFlagsTreeItem & FTI_kfTreeItemNameDisplayedGenerated)
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
	case eMenuSpecialAction_ITreeItemRenamed:
		m_uFlagsTreeItem &= ~FTI_kfTreeItemNameDisplayedGenerated;
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

void
ITreeItem::TreeItemLayout_SetFocus()
	{
	TreeItem_SelectWithinNavigationTree();	// Make sure the Tree Item of the layout is visible and selected
	MainWindow_SetFocusToCurrentLayout();
	}

#define d_coGreenLime		MAKE_QRGB(0, 255, 0)

void
ITreeItem::TreeItem_SetIconWithToolTip(EMenuAction eMenuIcon, const QString & sToolTip)
	{
	#if 0
	MessageLog_AppendTextFormatCo((eMenuActionIcon == eMenuIconFailure) ? d_coOrange : d_coGreenLime, "TreeItem_SetIcon(eMenuAction=$i, $s) for '$S'\n", eMenuActionIcon, PszGetMenuActionText(eMenuActionIcon), &m_strNameDisplay);
	if (eMenuActionIcon == (EMenuAction)6 && m_strNameDisplay.FCompareStringsNoCase((PSZUC)"abc"))
		MessageLog_AppendTextFormatCo(d_coRed, "!");	// Dummy statement for debugging
	#endif

	Assert(m_paTreeWidgetItem != NULL);
	QAction * pAction = PGetMenuAction(eMenuIcon);
	Assert(pAction != NULL);
	if (pAction != NULL)
		{
		if (m_paTreeWidgetItem != NULL)
			m_paTreeWidgetItem->setIcon(0, pAction->icon());
		}
	if (m_paTreeWidgetItem != NULL)
		m_paTreeWidgetItem->setToolTip(0, sToolTip);
	}

//	Set the icon of the QTreeWidgetItem from a menu action.
void
ITreeItem::TreeItem_SetIcon(EMenuAction eMenuIcon)
	{
	TreeItem_SetIconWithToolTip(eMenuIcon);
	}

void
ITreeItem::TreeItem_SetTextColorAndIcon(QRGB coTextColor, EMenuAction eMenuIcon)
	{
	Assert(m_paTreeWidgetItem != NULL);
	m_paTreeWidgetItem->setTextColor(0, coTextColor);
	TreeItem_SetIcon(eMenuIcon);
	}

void
ITreeItem::TreeItem_SetTextColor(QRGB coTextColor)
	{
	Assert(m_paTreeWidgetItem != NULL);
	m_paTreeWidgetItem->setTextColor(0, coTextColor);
	}

void
ITreeItem::TreeItem_SetTextToDisplayNameIfGenerated()
	{
	if (m_uFlagsTreeItem & FTI_kfTreeItemNameDisplayedGenerated)
		{
		m_strNameDisplayTyped.Empty();
		TreeItem_SetTextToDisplayNameTyped();
		}
	}

void
ITreeItem::TreeItem_SetTextToDisplayNameTyped()
	{
	Assert(m_paTreeWidgetItem != NULL);
	m_paTreeWidgetItem->setText(0, CString(TreeItem_PszGetNameDisplay()));
	}

void
ITreeItem::TreeItem_SetTextToDisplayMessagesUnread(int cMessagesUnread)
	{
	if (cMessagesUnread <= 0)
		{
		TreeItem_SetTextToDisplayNameTyped();
		return;
		}
	CStr str;
	str.Format("$s ($i)", TreeItem_PszGetNameDisplay(), cMessagesUnread);
	m_paTreeWidgetItem->setText(0, str);
	}

void
ITreeItem::TreeItem_SetIconError(PSZUC pszuErrorMessage, EMenuAction eMenuIcon)
	{
	Assert(pszuErrorMessage != NULL);
	CString sError(pszuErrorMessage);
	if (m_uFlagsTreeItem & FTI_kefIconError)
		{
		// There is already an error, so concatenate both, unless this is the same error.
		// Sometimes a server may send two stanzas with the same identical error.  I have no idea what is the motivation (or if it is a bug), however the same error should be displayed only once
		QString sErrorPrevious = m_paTreeWidgetItem->toolTip(0);
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
	//qApp->setStyleSheet("QToolTip { background-color: red }");
	//qApp->setStyleSheet("QToolTip { indent: 3 }");
	TreeItem_SetIconWithToolTip(eMenuIcon, sError);
	m_uFlagsTreeItem = (m_uFlagsTreeItem & ~FTI_kemIconMask) | FTI_kefIconError;
	} // TreeItem_SetIconError()

BOOL
ITreeItem::TreeItemWidget_FIsExpanded() const
	{
	Assert(m_paTreeWidgetItem != NULL);
	return m_paTreeWidgetItem->isExpanded();
	}

void
ITreeItem::TreeItemWidget_Expand()
	{
	Assert(m_paTreeWidgetItem != NULL);
	return m_paTreeWidgetItem->setExpanded(true);
	}

void
ITreeItem::TreeItemWidget_Collapse()
	{
	Assert(m_paTreeWidgetItem != NULL);
	return m_paTreeWidgetItem->setExpanded(false);
	}

void
ITreeItem::TreeItemWidget_EnsureVisible()
	{
	Assert(m_paTreeWidgetItem != NULL);
	m_paTreeWidgetItem->SetItemVisibleAlongWithItsParents(TRUE);
	}

void
ITreeItem::TreeItemWidget_ToggleVisibility()
	{
	Assert(m_paTreeWidgetItem != NULL);
	m_paTreeWidgetItem->setVisible(m_paTreeWidgetItem->isHidden());
	}

void
ITreeItem::TreeItemWidget_Hide()
	{
	Assert(m_paTreeWidgetItem != NULL);
	m_paTreeWidgetItem->setVisible(false);
	}

ITreeItem *
ITreeItem::TreeItem_PGetParent() const
	{
	CTreeWidgetItem * poParent = (CTreeWidgetItem *)m_paTreeWidgetItem->parent();
	if (poParent != NULL)
		{
		Assert(poParent->m_piTreeItem != NULL);
		Assert(poParent->m_piTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
		return poParent->m_piTreeItem;
		}
	return NULL;
	}

// The next two methods can be removed
void
CArrayPtrTreeItems::DeleteTreeItem(PA_DELETING ITreeItem * paTreeItem)
	{
	Assert(paTreeItem != NULL);
	Assert(paTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) == paTreeItem);
	(void)RemoveElementAssertI(paTreeItem);
	delete paTreeItem;
	}

void
CArrayPtrTreeItems::DeleteAllTreeItems()
	{
	ITreeItem ** ppaTreeItemStop;
	ITreeItem ** ppaTreeItem = PrgpGetTreeItemsStop(OUT &ppaTreeItemStop);
	while (ppaTreeItem != ppaTreeItemStop)
		{
		delete *ppaTreeItem++;
		}
	RemoveAllElements();
	}

void
CArrayPtrTreeItems::ForEach_SetFlagTreeItem(UINT kfFlagTreeItem) const
	{
	ITreeItem ** ppTreeItemStop;
	ITreeItem ** ppTreeItem = PrgpGetTreeItemsStop(OUT &ppTreeItemStop);
	while (ppTreeItem != ppTreeItemStop)
		{
		ITreeItem * pTreeItem = *ppTreeItem++;
		Assert(pTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) == pTreeItem);
		pTreeItem->m_uFlagsTreeItem |= kfFlagTreeItem;
		}
	}

void
CArrayPtrTreeItems::ForEach_SetFlagTreeItemAboutBeingDeleted() const
	{
	ForEach_SetFlagTreeItem(ITreeItem::FTI_kfTreeItemAboutBeingDeleted);
	}

void
CArrayPtrTreeItems::RemoveAllTreeItemsAboutBeingDeleted()
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
		Assert(pTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) == pTreeItem);
		if ((pTreeItem->m_uFlagsTreeItem & ITreeItem::FTI_kfTreeItemAboutBeingDeleted) == 0)
			*ppTreeItemDst++ = pTreeItem;
		}
	m_paArrayHdr->cElements = ppTreeItemDst - ppTreeItemStart;
	}
