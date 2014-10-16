///////////////////////////////////////////////////////////////////////////////////////////////////
//	WNavigationTree.cpp
//
//	Implementation of the widget displaying the Navigation Tree which displays the profiles, contacts, groups, and other things such as certificates.
//	The navigation wiget contains a tree where the user may expand and collapse nodes.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WNavigationTree.h"

extern CChatConfiguration g_oConfiguration;
const QString c_sNavigation("Navigation");

QToolButton * g_pwButtonStatusOfNavigationTree;
WButtonTextWithIcon * g_pwButtonSwitchProfile;
WMenu * g_pwMenuSwitchProfile;

WNavigationTreeCaption::WNavigationTreeCaption()
	{
	QToolButton * pwButtonUndock = new QToolButton(this);
	pwButtonUndock->setToolTip("Float / Unfloat");
	#if 1
	pwButtonUndock->setStyleSheet("QToolButton { border: none; padding: 3px; }");
	#else
	pwButtonUndock->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	#endif
	QPixmap oPixmap = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
	pwButtonUndock->setIcon(oPixmap);
//	pwButtonUndock->setMaximumSize(oPixmap.size());
	pwButtonUndock->setCursor(Qt::ArrowCursor);
	pwButtonUndock->setFocusPolicy(Qt::ClickFocus);

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setMargin(0);
//	layout->setSpacing(10);
//	layout->addStretch();
	#if 0
	layout->addWidget(g_pwButtonStatusOfNavigationTree, Qt::AlignLeft | Qt::AlignHCenter);
	#endif
	layout->addWidget(g_pwButtonSwitchProfile); // , Qt::AlignLeft | Qt::AlignHCenter);
	//layout->addWidget(new QWidget);
	layout->addStretch();
	//layout->addWidget(pwButtonUndock, Qt::AlignRight | Qt::AlignHCenter);
	layout->addWidget(pwButtonUndock);
	setLayout(layout);
	connect(pwButtonUndock, SIGNAL(clicked()), g_pwNavigationTree, SLOT(SL_ToggleDocking()));
	setCursor(Qt::OpenHandCursor);		// This cursor shows to the user he/she may drag the widget to undock the Navigation Tree
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
WNavigationTree::WNavigationTree() : QDockWidget(tr("Navigation Tree"))
	{
	g_pwNavigationTree = this;
	setObjectName(c_sNavigation);
	m_pTreeWidgetItemEditing = NULL;

	//setStyleSheet("border: 1px solid red;");
    //g_pwButtonSwitchProfile = new WButtonIconForToolbarWithDropDownMenu(this, eMenuIcon_ClassProfile, NULL);
    g_pwButtonSwitchProfile = new WButtonTextWithIcon("Switch Role |Open the Role Management screen", eMenuIcon_ClassProfile) ;
    g_pwButtonSwitchProfile->setCursor(Qt::ArrowCursor);	// We need to explicitly set the cursor to arrow because the caption uses the OpenHandCursor which every child inherits
    //added new Action to invoke the new rolepage
    connect(g_pwButtonSwitchProfile, SIGNAL(clicked()), this, SLOT(SL_RolePageShow()));
    /*g_pwMenuSwitchProfile = g_pwButtonSwitchProfile->PwGetMenu();
    connect(g_pwMenuSwitchProfile, SIGNAL(aboutToShow()), this, SLOT(SL_MenuProfilesShow()));
    connect(g_pwMenuSwitchProfile, SIGNAL(triggered(QAction*)), this, SLOT(SL_MenuProfileSelected(QAction*)));*/

#if 1
	setTitleBarWidget(PA_CHILD new WNavigationTreeCaption);	// Customize the title bar of the navigation tree
#endif

	QWidget * pwWidgetLayout = new QWidget(this);	// Since the QDockWidget can handle only one widget, we create a widget with a layout inside
	OLayoutVertical * pLayoutVertical = new OLayoutVertical(pwWidgetLayout);		// Vertical layout to stack the profile switcher, search, tree view and the status.
	Layout_MarginsClear(INOUT pLayoutVertical);
	pLayoutVertical->setSpacing(0);

	WEditSearch * pwEditSearch = new WEditSearch;
	connect(pwEditSearch, SIGNAL(textChanged(QString)), this, SLOT(SL_EditSearchTextChanged(QString)));
	pLayoutVertical->addWidget(pwEditSearch);


	//setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(/*DockWidgetClosable |*/ DockWidgetMovable | DockWidgetFloatable);	// Hide the close button in the title bar

	m_pwTreeView = new WTreeWidget;
	m_pwTreeView->header()->hide();
	m_pwTreeView->setContextMenuPolicy(Qt::CustomContextMenu);	// Send the signal customContextMenuRequested()
	//m_pwTreeView->setFrameStyle(QFrame::NoFrame);
	connect(m_pwTreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(SL_TreeCustomContextMenuRequested(QPoint)));
	connect(m_pwTreeView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(SL_TreeItemSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(m_pwTreeView, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemClicked(QTreeWidgetItem*,int)));

	pLayoutVertical->addWidget(m_pwTreeView);

	OLayoutHorizontal * pLayout = new OLayoutHorizontal(pLayoutVertical);
	//pLayout->addWidget(new WLabel("Status:"));

	g_pwButtonStatusOfNavigationTree = new WButtonIconForToolbarWithDropDownMenu(this, eMenuIcon_PresenceAccountOffline, "Status: Online");
	g_pwButtonStatusOfNavigationTree->setToolTip("Change your status");

	EMenuIcon eMenuIcon_Presence = (EMenuIcon)(g_uPreferences & P_kmPresenceMask);
	if (eMenuIcon_Presence == eMenuIcon_zNull)
		eMenuIcon_Presence = eMenuIcon_PresenceAccountOnline;
	Widget_SetIconButton(INOUT g_pwButtonStatusOfNavigationTree, eMenuIcon_Presence);
	g_pwMenuStatus = (WMenu *)g_pwButtonStatusOfNavigationTree->menu();
	g_pwMenuStatus->InitAsDymanicMenu();
    /*
	g_pwMenuStatus = new WMenu;
	g_pwMenuStatus->InitAsDymanicMenu();
	g_pwButtonStatusOfNavigationTree->setMenu(g_pwMenuStatus);
	*/
//	connect(g_pMenuStatus, SIGNAL(aboutToShow()), g_pwMainWindow, SLOT(SL_MenuAboutToShow()));
//	connect(g_pMenuStatus, SIGNAL(triggered(QAction*)), g_pwMainWindow, SLOT(SL_MenuActionTriggered(QAction*)));

//	pwButtonStatus->addAction(new QAction("test", pwButtonStatus));

	pLayout->addWidget(g_pwButtonStatusOfNavigationTree);
	pLayout->addStretch();
	//WButtonTextWithIcon * pwButtonAddContact = new WButtonTextWithIcon("Add Peer |Add a new peer to your profile", eMenuAction_ContactAdd);
	WButtonIconForToolbar * pwButtonAddContact = new WButtonIconForToolbar(eMenuIcon_ContactAdd, "Add a new peer to your " d_sza_profile);
	pLayout->addWidget(pwButtonAddContact, Qt::AlignBottom);
	connect(pwButtonAddContact, SIGNAL(clicked()), this, SLOT(SL_ContactNew()));

	//pLayoutVertical->addWidget(g_pwButtonStatusOfNavigationTree);
#if 1
	setWidget(PA_CHILD pwWidgetLayout);
#else
	setWidget(PA_CHILD m_pwTreeView);
#endif
	}

WNavigationTree::~WNavigationTree()
	{
	//EMessageBoxInformation("WNavigationTree::~WNavigationTree()");
	}

//	Unselect any Tree Item from the Navigation Tree.
//	This method is used when deleting the entire Navigation Tree as a small optimization so Qt does not select a new one each time a selected Tree Item is being deleted.
void
WNavigationTree::NavigationTree_TreeItemUnselect()
	{
	Assert(m_pwTreeView != NULL);
	m_pwTreeView->setCurrentItem(NULL);
	}

void
WNavigationTree::NavigationTree_SelectTreeItemWidget(CTreeItemW * poTreeItem)
	{
	if (poTreeItem != NULL)
		{
		Assert(poTreeItem->m_piTreeItem != NULL);
		Assert(PGetRuntimeInterfaceOf_ITreeItem(poTreeItem->m_piTreeItem) != NULL);
		poTreeItem->setVisible(true);			// Any selected Tree Item should be visible within the Navigation Tree
		m_pwTreeView->setCurrentItem(poTreeItem);
		}
	}

//	Enable the user to type something to rename a Tree Item.
//	When the editing is done, a notification eMenuSpecialAction_ITreeItemRenamed will be sent to the Tree Item to validate the text.
void
WNavigationTree::NavigationTree_RenameTreeItemWidget(CTreeItemW * poTreeItem)
	{
//	MessageLog_AppendTextFormatCo(d_coGreenDarker, "NavigationTree_RenameTreeItemWidget(0x$p) m_pTreeWidgetItemEditing=0x$p\n", pwTreeItem, m_pTreeWidgetItemEditing);
	Assert(poTreeItem != NULL);
	Assert(poTreeItem->m_piTreeItem != NULL);
	Assert(PGetRuntimeInterfaceOf_ITreeItem(poTreeItem->m_piTreeItem) != NULL);

	if (!m_pwTreeView->FIsEditingTreeWidgetItem())
		{
		// Make the Tree Item editable, so the method editItem() does not fail.  This line must be executed before calling connect() otherwise we get two signals itemChanged().
		poTreeItem->ItemFlagsEditingEnable();

//		MessageLog_AppendTextFormatCo(d_coGreenDarker, "\tConnecting signal QTreeWidget::itemChanged(0x$p)...\n", pwTreeItem);
		// We need to connect the signal itemChanged() so get notified when the user is done editing.
		// This signal cannot be connected in the constructor of WNavigationTree, otherwise we will receive a signal everytime the method setText() is called.  This design decision is as stupid as the WM_SETTEXT sending the notification EN_CHANGE.
		// or when the method setText() is called.  This is NOT about efficiency, but functionality to prevent double-entry of the same text.
		connect(m_pwTreeView, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemEdited(QTreeWidgetItem*,int)));
		}

	// Finally, begin editing
	m_pTreeWidgetItemEditing = poTreeItem;
	m_pwTreeView->editItem(poTreeItem);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	SL_TreeItemEdited(), slot of signal QTreeWidget::itemChanged()
void
WNavigationTree::SL_TreeItemEdited(QTreeWidgetItem * pItemEdited, int iColumn)
	{
	Assert(pItemEdited != NULL);
//	MessageLog_AppendTextFormatCo(d_coGreenDarker, "SL_TreeItemEdited(0x$p, iColumn=$i, editing=$i) m_pTreeWidgetItemEditing=0x$p\n", pItemEdited, iColumn, m_pwTreeView->FIsEditingTreeWidgetItem(), m_pTreeWidgetItemEditing);
	if (pItemEdited == m_pTreeWidgetItemEditing)
		{
		m_pTreeWidgetItemEditing = NULL;
		((CTreeItemW *)pItemEdited)->ItemFlagsEditingDisable();	// Prevent the item from being editable.  This is important otherwise Qt will not send the signal itemChanged() if the user attempts to edit the same item again.
		ITreeItem * piTreeItem = ((CTreeItemW *)pItemEdited)->m_piTreeItem;
		Assert (piTreeItem != NULL);
		Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItem) == piTreeItem);
		piTreeItem->m_strNameDisplayTyped =  pItemEdited->text(iColumn);
		piTreeItem->TreeItem_EDoMenuAction(eMenuSpecialAction_ITreeItemRenamed);	// Notify the Tree Item the display name was changed (the method TreeItem_EDoMenuAction() may change the display text)
		pItemEdited->setText(iColumn, (CString)piTreeItem->TreeItem_PszGetNameDisplay());		// Make sure the GUI always display the updated text
		}

	// If the tree view is no longer in edit mode, then disconnect the signal, so we no longer receive the signals QTreeWidget::itemChanged()
	if (m_pTreeWidgetItemEditing == NULL || !m_pwTreeView->FIsEditingTreeWidgetItem())
		{
		m_pTreeWidgetItemEditing = NULL;
		m_pwTreeView->DisableEditingTreeWidgetItem();
//		MessageLog_AppendTextFormatCo(d_coGreenDarker, "\tDisconnecting signal QTreeWidget::itemChanged(0x$p)...\n", pItemEdited);
		disconnect(m_pwTreeView, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemEdited(QTreeWidgetItem*,int)));
		}
	} // SL_TreeItemEdited()

void
WNavigationTree::SL_EditSearchTextChanged(const QString & sTextFind)
	{
	NavigationTree_DisplayTreeItemsContainingText(sTextFind);
	}


void
WNavigationTree::NavigationTree_DisplayTreeItemsContainingText(const QString & sSearch)	//TODO: Move to SL_EditSearchTextChanged()
	{
	QTreeWidgetItemIterator oIterator(m_pwTreeView);
	if (sSearch.isEmpty())
		{
		// Show the entire tree
		while (TRUE)
			{
			CTreeItemW * pTreeWidgetItem = (CTreeItemW *)*oIterator++;
			if (pTreeWidgetItem == NULL)
				return;
			pTreeWidgetItem->setHidden(false);
			}
		}
	else
		{
		CStr strSearch = sSearch;
		strSearch.TransformContentToLowercaseSearch();
		while (TRUE)
			{
			CTreeItemW * pTreeWidgetItem = (CTreeItemW *)*oIterator++;
			if (pTreeWidgetItem == NULL)
				return;
			ITreeItem * piItem = pTreeWidgetItem->m_piTreeItem;
			Assert(piItem != NULL);
			Assert(PGetRuntimeInterfaceOf_ITreeItem(piItem) == piItem);
			/*
			if (piItem->TreeItem_FContainsMatchingText(strSearch))
				MessageLog_AppendTextFormatCo(d_coBlack, "$S is visible\n", &piItem->m_strNameDisplay);
			*/
			pTreeWidgetItem->SetItemVisibleAlongWithItsParents(piItem->TreeItem_FContainsMatchingText(strSearch));
			}
		}
	} // NavigationTree_DisplayTreeItemsContainingText()

void
WNavigationTree::SL_ToggleDocking()
	{
	MessageLog_AppendTextFormatSev(eSeverityComment, "WNavigationTree::SL_ToggleDocking()\n");
	setFloating(!isFloating());
	// Attempt to have a border when the widget is floating
	//setStyleSheet("QWidget#Navigation { border: 1px solid red; }");
	//setStyleSheet("QWidget { border: 1px solid red; }");
	}

void
WNavigationTree::SL_ContactNew()
	{
	void DisplayDialogContactNew();
	DisplayDialogContactNew();
	}

#define d_iProfile_DisplayAll		(-1)
#define d_iProfile_CreateNew		(-2)

void
WNavigationTree::SL_RolePageShow()
{
g_pwMainWindow->SL_showRolePage();
}

void
WNavigationTree::SL_MenuProfilesShow()
	{
	g_pwMenuSwitchProfile->clear();
	int cProfiles;
	TProfile ** prgpProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfiles(OUT &cProfiles);
	for (int iProfile = 0; iProfile < cProfiles; iProfile++)
		{
		TProfile * pProfile = prgpProfiles[iProfile];
		Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
		g_pwMenuSwitchProfile->ActionAddFromText(pProfile->m_strNameProfile, iProfile, eMenuIconIdentities);
		}
    /*Disabled because the menu become a button Switch Role
     * if (cProfiles > 1)
		g_pwMenuSwitchProfile->ActionAddFromText((PSZUC)"<View All " d_sza_Profile "s>", d_iProfile_DisplayAll, eMenuIconIdentities);
        g_pwMenuSwitchProfile->ActionAddFromText((PSZUC)"<Manage Roles...>", d_iProfile_CreateNew, eMenuIconIdentities);*/
	}

void
WNavigationTree::SL_MenuProfileSelected(QAction * pAction)
	{
	const int iProfile = pAction->data().toInt();
	#ifdef COMPILE_WITH_OPEN_TRANSACTIONS


    std::cout << pAction->text().toStdString();

    // open only if Manage Role Screen is created
    if (pAction->text().toStdString().compare("<Manage Roles...>")==0)
      pOTX->openRoleCreationScreen();


    if (!pOTX->RCS_ACTION_CANCEL)
	#endif
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile((TProfile *)g_oConfiguration.m_arraypaProfiles.PvGetElementAtSafe_YZ(iProfile), iProfile == d_iProfile_CreateNew);
	}

const EMenuActionByte c_rgzeActionsMenuNavigationTree[] =
	{
	eMenuAction_AccountAcceptInvitation,
	eMenuAction_AccountLogin,
	eMenuAction_AccountRegister,
	eMenuActionSeparator,
	eMenuAction_DisplayWallet,
	//eMenuAction_DisplaySecureWebBrowsing,
	ezMenuActionNone
	};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	SL_TreeCustomContextMenuRequested(), slot of signal QWidget::customContextMenuRequested()
void
WNavigationTree::SL_TreeCustomContextMenuRequested(const QPoint & ptPos)
	{
	WMenu oMenu;
	CTreeItemW * pTreeItemWidget = (CTreeItemW *)m_pwTreeView->itemAt(ptPos);
	if (pTreeItemWidget != NULL)
		{
		Assert(pTreeItemWidget->m_piTreeItem != NULL);
		pTreeItemWidget->m_piTreeItem->TreeItem_MenuAppendActions(IOUT &oMenu);
		}
	else
		{
		// Nothing was selected, so display a context menu offering options register or login to an account
		if (g_arraypAccounts.PvGetElementUnique_YZ() != NULL)
			oMenu.ActionAddWithSeparator(eMenuAction_ContactAdd);	// Add the menu item to add a contact
		oMenu.ActionsAdd(c_rgzeActionsMenuNavigationTree);
		}
	oMenu.DisplayContextMenuAndSendActionsToMainWindow();
	} // SL_TreeCustomContextMenuRequested()

void
WNavigationTree::SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int UNUSED_PARAMETER(iColumn))
	{
	Assert(pItemClicked != NULL);
	UNUSED_PARAMETER(iColumn);
	ITreeItem * pTreeItem = ((CTreeItemW *)pItemClicked)->m_piTreeItem;
	Assert(pTreeItem != NULL);	// This is a rare case when an object was added to the navigation tree, but no associated ITreeItem
	if (pTreeItem != NULL)
		{
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
		pTreeItem->TreeItem_IconUpdateOnMessagesRead();	// If the user click on a Tree Item (typically a contact or group), assume all messages related to this Tree Item have been read
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTreeItemW *
ITreeItem::TreeItemW_PAllocate()
	{
	Assert(m_paTreeItemW_YZ == NULL && "Memory leak!");
	m_paTreeItemW_YZ = new CTreeItemW;
	m_paTreeItemW_YZ->m_piTreeItem = this;
	m_paTreeItemW_YZ->setText(0, (CString)TreeItem_PszGetNameDisplay());
	return m_paTreeItemW_YZ;
	}

//	This method will NOT add the Tree Item to the Navigation Tree if the 'before' is not present
void
ITreeItem::TreeItemW_DisplayWithinNavigationTreeBefore(ITreeItem * pTreeItemBefore)
	{
	Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItemBefore) == pTreeItemBefore);
	CTreeItemW * poTreeWidgetItem = pTreeItemBefore->m_paTreeItemW_YZ;
	if (poTreeWidgetItem != NULL)
		{
		QTreeWidgetItem * poParent = poTreeWidgetItem->parent();
		Assert(poParent != NULL);
		int iChild = poParent->indexOfChild(poTreeWidgetItem);
		poParent->insertChild(iChild, TreeItemW_PAllocate());
		}
	}

void
ITreeItem::TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ)
	{
	Assert(g_pwNavigationTree != NULL);
	(void)TreeItemW_PAllocate();
	Assert(m_paTreeItemW_YZ != NULL);
	if (pParent_YZ != NULL)
		{
		if (pParent_YZ->m_paTreeItemW_YZ != NULL)
			pParent_YZ->m_paTreeItemW_YZ->addChild(PA_CHILD m_paTreeItemW_YZ);
		}
	else
		{
		// No parent, so add the Tree Item at the root
		g_pwNavigationTree->m_pwTreeView->addTopLevelItem(PA_CHILD m_paTreeItemW_YZ);
		}
	}

void
ITreeItem::TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ, EMenuIcon eMenuIcon)
	{
	TreeItemW_DisplayWithinNavigationTree(pParent_YZ);
	TreeItemW_SetIcon(eMenuIcon);
	}

//	This method is used mostly to do a prototype and populate the Navigation Tree quickly
void
ITreeItem::TreeItemW_DisplayWithinNavigationTreeExpand(ITreeItem * pParent_YZ, PSZAC pszName, EMenuIcon eMenuIcon)
	{
	m_strNameDisplayTyped.BinInitFromStringWithNullTerminator(pszName);
	TreeItemW_DisplayWithinNavigationTree(pParent_YZ, eMenuIcon);
	TreeItemW_Expand();
	}

class WLayoutDemo : public WLayout
{
public:
	WLayoutDemo(const CStr & strDescription, const CStr & strSearch);
};

WLayoutDemo::WLayoutDemo(const CStr & strDescription, const CStr & strSearch)
	{
	if (!strSearch.FIsEmptyString())
		{
		WEdit * pwEdit = new WEdit;
		pwEdit->setMaximumHeight(20);
		pwEdit->Edit_SetWatermark(strSearch);
		pwEdit->setParent(this);
		}
	QLabel * pLabel = new QLabel(strDescription, this);
	pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pLabel->setFrameShape(QFrame::Box);
	}

TTreeItemDemo::TTreeItemDemo()
	{
	m_pawLayoutDemo = NULL;
	}

TTreeItemDemo::TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuIcon eMenuIcon)
	{
	m_pawLayoutDemo = NULL;
	TreeItemW_DisplayWithinNavigationTreeExpand(pParent, pszName, eMenuIcon);
	}

TTreeItemDemo::TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuIcon eMenuIcon, PSZAC pszDescription, PSZAC pszSearch)
	{
	m_pawLayoutDemo = NULL;
	m_strDescription = (PSZUC)pszDescription;
	m_strSearch = (PSZUC)pszSearch;
	TreeItemW_DisplayWithinNavigationTreeExpand(pParent, pszName, eMenuIcon);
	}


TTreeItemDemo::~TTreeItemDemo()
	{
	delete m_pawLayoutDemo;
	}

//	TTreeItemDemo::ITreeItem::TreeItem_GotFocus()
void
TTreeItemDemo::TreeItem_GotFocus()
	{
	if (m_pawLayoutDemo == NULL)
		m_pawLayoutDemo = new WLayoutDemo(m_strDescription, m_strSearch);
	MainWindow_SetCurrentLayout(IN m_pawLayoutDemo);
	}

TTreeItemDemo *
ITreeItem::TreeItemW_PAllocateChild(PSZAC pszName, EMenuIcon eMenuIcon)
	{
	TTreeItemDemo * pChild = new TTreeItemDemo;
	pChild->TreeItemW_DisplayWithinNavigationTreeExpand(this, pszName, eMenuIcon);
	return pChild;
	}

TTreeItemDemo *
ITreeItem::TreeItemW_PAllocateChild(PSZAC pszName, EMenuIcon eMenuIcon, PSZAC pszDescription, PSZAC pszSearch)
	{
	TTreeItemDemo * pChild = TreeItemW_PAllocateChild(pszName, eMenuIcon);
	pChild->m_strDescription = (PSZUC)pszDescription;
	pChild->m_strSearch = (PSZUC)pszSearch;
	return pChild;
	}

void
ITreeItem::TreeItemW_AllocateChildren_VEZ(EMenuIcon eMenuIcon, PSZAC pszName, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszName);
	while (TRUE)
		{
		TreeItemW_PAllocateChild(pszName, eMenuIcon);
		pszName = va_arg(vlArgs, PSZAC);
		if (pszName == NULL)
			break;
		}
	}

void
ITreeItem::TreeItemW_SelectWithinNavigationTree()		// TODO: use TreeItemLayout_SetFocus() instead
	{
	Report(m_paTreeItemW_YZ != NULL && "No Tree Item to select");
	g_pwNavigationTree->NavigationTree_SelectTreeItemWidget(m_paTreeItemW_YZ);
	}

//	Select the Tree Item and make sure it is expanded
void
ITreeItem::TreeItemW_SelectWithinNavigationTreeExpanded()
	{
	TreeItemW_SelectWithinNavigationTree();
	TreeItemW_Expand();
	}

//	Remove the Tree Item from the Navigation Tree
void
ITreeItem::TreeItemW_RemoveFromNavigationTree()
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "TreeItemW_RemoveFromNavigationTree($s)\n", TreeItem_PszGetNameDisplay());
	//TreeItemW_Hide();	// At the moment, hide the CTreeItemW.  Deleting the CTreeItemW also deletes any child object of the Tree Item which causes a crash.  To prevent this crash, a routine to recursively delete all child objects would be necessary.
	if (m_paTreeItemW_YZ != NULL)
		{
		m_paTreeItemW_YZ->RemoveAllChildItemsR();
		delete m_paTreeItemW_YZ;	// Delete the CTreeItemW (if any).  The destructor will automatically remove the CTreeItemW from the GUI.
		m_paTreeItemW_YZ = NULL;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WNavigationTree::NavigationTree_ExpandAllRootTreeItems()
	{
	m_pwTreeView->expandAll();
	}

void
NavigationTree_SelectTreeItem(ITreeItem * pTreeItem)
	{
	if (pTreeItem != NULL)
		{
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
		pTreeItem->TreeItemW_SelectWithinNavigationTree();
		}
	}


ITreeItem *
NavigationTree_PGetSelectedTreeItem()
	{
	CTreeItemW * pwTreeItem = (CTreeItemW *)g_pwNavigationTree->m_pwTreeView->currentItem();
	if (pwTreeItem != NULL)
		{
		Assert(pwTreeItem->m_piTreeItem != NULL);
		Assert(PGetRuntimeInterfaceOf_ITreeItem(pwTreeItem->m_piTreeItem) != NULL);
		return pwTreeItem->m_piTreeItem;
		}
	return NULL;
	}

//	Return the pointer of the selected object if it matches the interface rti, otherwise return NULL.
ITreeItem *
NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI_ENUM rti)
	{
	ITreeItem * piTreeItem = NavigationTree_PGetSelectedTreeItem();
	if (piTreeItem == NULL)
		{
		// Special exception if there is nothing selected in the Navigation Tree: use the profile to query the interface
		piTreeItem = g_oConfiguration.PGetProfileSelectedUnique_YZ();
		if (piTreeItem == NULL)
			return NULL;
		}
	#ifdef DEBUG
	ITreeItem * piTreeItemMatchingInterface = PGetRuntimeInterfaceOf_ITreeItem(piTreeItem);
	if (piTreeItemMatchingInterface != NULL)
		{
		// Make sure the returned pointer matching the interface rti is inherited from ITreeItem, otherwise there could be a big crash.
		Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItemMatchingInterface) == piTreeItem);
		}
	#endif
	return (ITreeItem *)piTreeItem->PGetRuntimeInterface(rti, NULL);	// May return NULL if piTreeItem does not have an interface compatible to rti.
	}

TProfile *
NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile()
	{
	return (TProfile *)NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI(TProfile));
	}

/*
TProfile *
NavigationTree_PGetSelectedUnique()	// I think this function is redundant and should be deleted
	{
	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	if (pProfile != NULL)
		return pProfile;
	return g_oConfiguration.PGetProfileSelectedUnique_YZ();
	}
*/

TAccountXmpp *
NavigationTree_PGetSelectedTreeItemMatchingInterfaceTAccount()
	{
	return (TAccountXmpp *)NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI(TAccountXmpp));
	}

TContact *
NavigationTree_PGetSelectedTreeItemMatchingInterfaceTContact()
	{
	return (TContact *)NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI(TContact));
	}

ITreeItemChatLogEvents *
NavigationTree_PGetSelectedTreeItemMatchingContractOrGroup()
	{
	TContact * pContact = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTContact();
	if (pContact != NULL)
		return pContact;
	return (TGroup *)NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI(TGroup));
	}

void
NavigationTree_RenameSelectedItem()
	{
	g_pwNavigationTree->NavigationTree_RenameTreeItemWidget((CTreeItemW *)g_pwNavigationTree->m_pwTreeView->currentItem());
	}
