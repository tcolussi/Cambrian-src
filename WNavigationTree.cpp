///////////////////////////////////////////////////////////////////////////////////////////////////
//	WNavigationTree.cpp
//
//	Implementation of the widget displaying the navigation, such as events, accounts, servers and certificates.
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
WButtonIconForToolbarWithDropDownMenu * g_pwButtonSwitchProfile;
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
WNavigationTree::WNavigationTree() : QDockWidget(tr("Navigation"))
	{
	g_pwNavigationTree = this;
	setObjectName(c_sNavigation);
	m_pTreeWidgetItemEditing = NULL;

	//setStyleSheet("border: 1px solid red;");
	g_pwButtonSwitchProfile = new WButtonIconForToolbarWithDropDownMenu(this, eMenuIconIdentities, NULL);
	g_pwButtonSwitchProfile->setCursor(Qt::ArrowCursor);	// We need to explicitly set the cursor to arrow because the caption uses the OpenHandCursor which every child inherits
	g_pwMenuSwitchProfile = g_pwButtonSwitchProfile->PwGetMenu();
	connect(g_pwMenuSwitchProfile, SIGNAL(aboutToShow()), this, SLOT(SL_MenuProfilesShow()));
	connect(g_pwMenuSwitchProfile, SIGNAL(triggered(QAction*)), this, SLOT(SL_MenuProfileSelected(QAction*)));

#if 1
	setTitleBarWidget(PA_CHILD new WNavigationTreeCaption);	// Customize the title bar of the navigation tree
#endif

	QWidget * pwWidgetLayout = new QWidget(this);	// Since the QDockWidget can handle only one widget, we create a widget with a layout inside
	OLayoutVertical * pLayoutVertical = new OLayoutVertical(pwWidgetLayout);		// Vertical layout to stack the profile switcher, search, tree view and the status.
	Layout_MarginsClear(INOUT pLayoutVertical);
	pLayoutVertical->setSpacing(0);

	/*
	OLayoutHorizontal * pLayout = new OLayoutHorizontal(pLayoutVertical);
	WButtonIconForToolbarWithDropDownMenu * pwButtonProfileSwitch = new WButtonIconForToolbarWithDropDownMenu(this, eMenuIconAdd, "Profile");
	connect(pwButtonProfileSwitch->menu(), SIGNAL(aboutToShow()), g_pwMainWindow, SLOT(SL_MenuAboutToShow()));
	pLayout->addWidget(pwButtonProfileSwitch);
	*/
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

	g_pwButtonStatusOfNavigationTree = new WButtonIconForToolbarWithDropDownMenu(this, eMenuAction_PresenceAccountOffline, "Status: Online");
	g_pwButtonStatusOfNavigationTree->setToolTip("Change your status");
	/*
	//g_pwButtonStatusOfNavigationTree->setStyleSheet("QToolButton { border: none; padding: 3px; }");
	g_pwButtonStatusOfNavigationTree->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	g_pwButtonStatusOfNavigationTree->setPopupMode(QToolButton::InstantPopup);
	g_pwButtonStatusOfNavigationTree->setCursor(Qt::ArrowCursor);
	g_pwButtonStatusOfNavigationTree->setFocusPolicy(Qt::ClickFocus);
	*/
	EMenuAction eMenuAction_Presence = (EMenuAction)(g_uPreferences & P_kmPresenceMask);
	if (eMenuAction_Presence == ezMenuActionNone)
		eMenuAction_Presence = eMenuAction_PresenceAccountOnline;
	Widget_SetIconButton(INOUT g_pwButtonStatusOfNavigationTree, eMenuAction_Presence);
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
	//WButtonTextWithIcon * pwButtonAddContact = new WButtonTextWithIcon("Add Contact |Add a new contact to your profile", eMenuAction_ContactAdd);
	WButtonIconForToolbar * pwButtonAddContact = new WButtonIconForToolbar(eMenuAction_ContactAdd, "Add a new contact to your profile");
	pLayout->addWidget(pwButtonAddContact, Qt::AlignBottom);
	//pwButtonAddContact->addAction(PGetMenuAction(eMenuAction_ContactAdd));
	connect(pwButtonAddContact, SIGNAL(clicked()), this, SLOT(SL_ContactNew()));

	//pLayoutVertical->addWidget(g_pwButtonStatusOfNavigationTree);

#if 1
	setWidget(PA_CHILD pwWidgetLayout);
#else
	setWidget(PA_CHILD m_pwTreeView);
#endif


	// Create the root nodes
	g_pTreeItemCommunication = new TTreeItemMyInbox;
	/*
	new TTreeItemDemo(NULL, "Applications", eMenuIconComponent);
	new TTreeItemDemo(NULL, "Marketplace", eMenuIconMarketplace);
	new TTreeItemDemo(NULL, "Finance", eMenuIconSell);
	new TTreeItemDemo(NULL, "Registry", eMenuIconCorporations);
	*/
	(void)new TMyProfiles;
	/*
	TTreeItemDemo * pIDs = new TTreeItemDemo(NULL, "My Profiles", eMenuIconSettings);
	TTreeItemDemo * pID = new TTreeItemDemo(pIDs, "Jon Peters", eMenuIconIdentities);
		TTreeItemDemo * pApplications = new TTreeItemDemo(pID, "Applications", eMenuIconComponent);
			TTreeItemDemo * pCommunications = new TTreeItemDemo(pApplications, "Communications", eMenuIconCommunicate);
				TTreeItemDemo * pAccount = new TTreeItemDemo(pCommunications, "jon", eMenuIconXmpp);
					new TTreeItemDemo(pAccount, "Dan", eMenuAction_PresenceAccountOnline);
					new TTreeItemDemo(pAccount, "Hiro", eMenuAction_PresenceAccountOnline);
					new TTreeItemDemo(pAccount, "Light", eMenuAction_PresenceAway);
				TTreeItemDemo * pFB = new TTreeItemDemo(pCommunications, "Jon Peters", eMenuIconFacebook);
					new TTreeItemDemo(pFB, "Brittany", eMenuAction_PresenceAccountOnline);
					new TTreeItemDemo(pFB, "Mom", eMenuAction_PresenceAccountOffline);
			TTreeItemDemo * pMarketplace = new TTreeItemDemo(pApplications, "Marketplace", eMenuIconMarketplace);
				new TTreeItemDemo(pMarketplace, "Jurisdictions", eMenuIconJurisdiction);
				new TTreeItemDemo(pMarketplace, "Services", eMenuIconServices);
				new TTreeItemDemo(pMarketplace, "Smart Contracts", eMenuIconMarketplaceLawyers);
				new TTreeItemDemo(pMarketplace, "For Sale", eMenuIconSell);
				new TTreeItemDemo(pMarketplace, "Jobs", eMenuIconJobs);
				new TTreeItemDemo(pMarketplace, "Housing", eMenuIconHome);
				new TTreeItemDemo(pMarketplace, "Gigs", eMenuIconRipple);
			TTreeItemDemo * pFinance = new TTreeItemDemo(pApplications, "Finance", eMenuIconIssueDividends);
				new TTreeItemDemo(pFinance, "Exchange", eMenuIconExchange);
				new TTreeItemDemo(pFinance, "Wallets", eMenuIconBitcoin);
				new TTreeItemDemo(pFinance, "Banking", eMenuIconBank);
			TTreeItemDemo * pRegistry = new TTreeItemDemo(pApplications, "Registry", eMenuIconVote);
				new TTreeItemDemo(pRegistry, "Companies", eMenuIconCorporations);
				new TTreeItemDemo(pRegistry, "Jurisdictions", eMenuIconJurisdiction);
				new TTreeItemDemo(pRegistry, "Communities", eMenuIconCommunity);
				new TTreeItemDemo(pRegistry, "Arbitrator", eMenuIconMarketplaceArbitration);
				new TTreeItemDemo(pRegistry, "Mediator", eMenuIconMarketplaceMediation);
				new TTreeItemDemo(pRegistry, "Oracle", eMenuIconListen);
			new TTreeItemDemo(pApplications, "Calendar", eMenuIconIssueFuture);
		new TTreeItemDemo(pIDs, "Cambrian Inc.", eMenuIconIdentities);
		*/
	}

WNavigationTree::~WNavigationTree()
	{
	}

void
WNavigationTree::NavigationTree_SelectTreeItemWidget(CTreeWidgetItem * poTreeItem)
	{
	Assert(poTreeItem != NULL);
	Assert(poTreeItem->m_piTreeItem != NULL);
	Assert(poTreeItem->m_piTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
	m_pwTreeView->setCurrentItem(poTreeItem);
	}

//	Enable the user to type something to rename a Tree Item.
//	When the editing is done, a notification eMenuSpecialAction_ITreeItemRenamed will be sent to the Tree Item to validate the text.
void
WNavigationTree::NavigationTree_RenameTreeItemWidget(CTreeWidgetItem * poTreeItem)
	{
//	MessageLog_AppendTextFormatCo(d_coGreenDarker, "NavigationTree_RenameTreeItemWidget(0x$p) m_pTreeWidgetItemEditing=0x$p\n", pwTreeItem, m_pTreeWidgetItemEditing);
	Assert(poTreeItem != NULL);
	Assert(poTreeItem->m_piTreeItem != NULL);
	Assert(poTreeItem->m_piTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);

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
		((CTreeWidgetItem *)pItemEdited)->ItemFlagsEditingDisable();	// Prevent the item from being editable.  This is important otherwise Qt will not send the signal itemChanged() if the user attempts to edit the same item again.
		ITreeItem * piTreeItem = ((CTreeWidgetItem *)pItemEdited)->m_piTreeItem;
		Assert (piTreeItem != NULL);
		Assert(piTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
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
			CTreeWidgetItem * pTreeWidgetItem = (CTreeWidgetItem *)*oIterator++;
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
			CTreeWidgetItem * pTreeWidgetItem = (CTreeWidgetItem *)*oIterator++;
			if (pTreeWidgetItem == NULL)
				return;
			ITreeItem * piItem = pTreeWidgetItem->m_piTreeItem;
			Assert(piItem != NULL);
			Assert(piItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
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
#define d_iProfile_DisplayNew		(-2)

void
WNavigationTree::SL_MenuProfilesShow()
	{
	g_pwMenuSwitchProfile->clear();
	int cProfiles;
	TProfile ** prgpProfiles = (TProfile **)g_oConfiguration.m_arraypaProfiles.PrgpvGetElements(OUT &cProfiles);
	for (int iProfile = 0; iProfile < cProfiles; iProfile++)
		{
		TProfile * pProfile = prgpProfiles[iProfile];
		Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
		g_pwMenuSwitchProfile->ActionAddFromText(pProfile->m_strNameProfile, iProfile, eMenuIconIdentities);
		}
	g_pwMenuSwitchProfile->ActionAddFromText((PSZUC)"<View All Profiles>", d_iProfile_DisplayAll, eMenuIconIdentities);
	g_pwMenuSwitchProfile->ActionAddFromText((PSZUC)"<Create New Profile...>", d_iProfile_DisplayNew, eMenuIconIdentities);
	}

void
WNavigationTree::SL_MenuProfileSelected(QAction * pAction)
	{
	const int iProfile = pAction->data().toInt();
	if (iProfile != d_iProfile_DisplayNew)
		g_oConfiguration.NavigationTree_ProfileSwitch((TProfile *)g_oConfiguration.m_arraypaProfiles.PvGetElementAtSafe_YZ(iProfile));
	else
		{
		TMyProfiles::s_pThis->TreeItemWidget_Expand();		// Make sure all the profiles are visible
		TMyProfiles::s_pThis->TreeItemLayout_SetFocus();	// Select the Tree Item to create a new profile
		}
	}

const EMenuActionByte c_rgzeActionsMenuNavigationTree[] =
	{
	eMenuAction_AccountAcceptInvitation,
	eMenuAction_AccountLogin,
	eMenuAction_AccountRegister,
	eMenuActionSeparator,
	eMenuAction_DisplayWallet,
	eMenuAction_DisplaySecureWebBrowsing,
	ezMenuActionNone
	};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	SL_TreeCustomContextMenuRequested(), slot of signal QWidget::customContextMenuRequested()
void
WNavigationTree::SL_TreeCustomContextMenuRequested(const QPoint & ptPos)
	{
	WMenu oMenu;
	CTreeWidgetItem * pTreeItemWidget = (CTreeWidgetItem *)m_pwTreeView->itemAt(ptPos);
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
WNavigationTree::SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn)
	{
	Assert(pItemClicked != NULL);
	UNUSED_PARAMETER(iColumn);
	ITreeItem * pTreeItem = ((CTreeWidgetItem *)pItemClicked)->m_piTreeItem;
	Assert(pTreeItem != NULL);	// This is a rare case when an object was added to the navigation tree, but no associated ITreeItem
	if (pTreeItem != NULL)
		{
		Assert(pTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
		pTreeItem->TreeItem_IconUpdateOnMessagesRead();	// If the user click on a Tree Item (typically a contact or group), assume all messages related to this Tree Item have been read
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTreeWidgetItem *
ITreeItem::TreeItemWidget_PAllocate()
	{
	Assert(m_paTreeWidgetItem == NULL && "Memory leak!");
	m_paTreeWidgetItem = new CTreeWidgetItem;
	m_paTreeWidgetItem->m_piTreeItem = this;
	m_paTreeWidgetItem->setText(0, (CString)TreeItem_PszGetNameDisplay());
	return m_paTreeWidgetItem;
	}

void
ITreeItem::TreeItem_DisplayWithinNavigationTreeBefore(ITreeItem * pTreeItemBefore)
	{
	Assert(pTreeItemBefore->PGetRuntimeInterface(RTI(ITreeItem)) == pTreeItemBefore);
	Assert(pTreeItemBefore->m_paTreeWidgetItem != NULL);
	CTreeWidgetItem * poTreeWidgetItem = pTreeItemBefore->m_paTreeWidgetItem;
	QTreeWidgetItem * poParent = poTreeWidgetItem->parent();
	Assert(poParent != NULL);
	int iChild = poParent->indexOfChild(poTreeWidgetItem);
	poParent->insertChild(iChild, TreeItemWidget_PAllocate());
	}

void
ITreeItem::TreeItem_DisplayWithinNavigationTree(ITreeItem * pParent)
	{
	Assert(g_pwNavigationTree != NULL);
	TreeItemWidget_PAllocate();
	Assert(m_paTreeWidgetItem != NULL);
	if (pParent != NULL)
		{
		Assert(pParent->m_paTreeWidgetItem != NULL);
		pParent->m_paTreeWidgetItem->addChild(PA_CHILD m_paTreeWidgetItem);
		}
	else
		{
		// No parent, so add the Tree Item at the root
		g_pwNavigationTree->m_pwTreeView->addTopLevelItem(PA_CHILD m_paTreeWidgetItem);
		}
	}

void
ITreeItem::TreeItem_DisplayWithinNavigationTree(ITreeItem * pParent, EMenuAction eMenuActionIcon)
	{
	TreeItem_DisplayWithinNavigationTree(pParent);
	TreeItem_SetIcon(eMenuActionIcon);
	}

//	This method is used mostly to do a prototype and populate the Navigation Tree quickly
void
ITreeItem::TreeItem_DisplayWithinNavigationTree(ITreeItem * pParent, PSZAC pszName, EMenuAction eMenuActionIcon)
	{
	m_strNameDisplayTyped.BinInitFromStringWithNullTerminator(pszName);
	TreeItem_DisplayWithinNavigationTree(pParent, eMenuActionIcon);
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

TTreeItemDemo::TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuAction eMenuActionIcon)
	{
	m_pawLayoutDemo = NULL;
	TreeItem_DisplayWithinNavigationTree(pParent, pszName, eMenuActionIcon);
	}

TTreeItemDemo::TTreeItemDemo(ITreeItem * pParent, PSZAC pszName, EMenuAction eMenuActionIcon, PSZAC pszDescription, PSZAC pszSearch)
	{
	m_pawLayoutDemo = NULL;
	m_strDescription = (PSZUC)pszDescription;
	m_strSearch = (PSZUC)pszSearch;
	TreeItem_DisplayWithinNavigationTree(pParent, pszName, eMenuActionIcon);
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
ITreeItem::TreeItem_PAllocateChild(PSZAC pszName, EMenuAction eMenuActionIcon)
	{
	TTreeItemDemo * pChild = new TTreeItemDemo;
	pChild->TreeItem_DisplayWithinNavigationTree(this, pszName, eMenuActionIcon);
	return pChild;
	}

TTreeItemDemo *
ITreeItem::TreeItem_PAllocateChild(PSZAC pszName, EMenuAction eMenuActionIcon, PSZAC pszDescription, PSZAC pszSearch)
	{
	TTreeItemDemo * pChild = TreeItem_PAllocateChild(pszName, eMenuActionIcon);
	pChild->m_strDescription = (PSZUC)pszDescription;
	pChild->m_strSearch = (PSZUC)pszSearch;
	return pChild;
	}

void
ITreeItem::TreeItem_AllocateChildren_VEZ(EMenuAction eMenuActionIcon, PSZAC pszName, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszName);
	while (TRUE)
		{
		TreeItem_PAllocateChild(pszName, eMenuActionIcon);
		pszName = va_arg(vlArgs, PSZAC);
		if (pszName == NULL)
			break;
		}
	}

void
ITreeItem::TreeItem_SelectWithinNavigationTree()
	{
	Assert(m_paTreeWidgetItem != NULL && "No Tree Item to select");
	g_pwNavigationTree->NavigationTree_SelectTreeItemWidget(m_paTreeWidgetItem);
	}

//	Select the Tree Item and make sure it is expanded
void
ITreeItem::TreeItem_SelectWithinNavigationTreeExpanded()
	{
	TreeItem_SelectWithinNavigationTree();
	TreeItemWidget_Expand();
	}

//	Remove the Tree Item from the Navigation Tree
void
ITreeItem::TreeItem_RemoveFromNavigationTree()
	{
	delete m_paTreeWidgetItem;	// Delete the CTreeWidgetItem (if any).  The destructor will automatically remove the CTreeWidgetItem from the GUI.
	m_paTreeWidgetItem = NULL;
	}

/*
void
ITreeItem::TreeItem_Rename()
	{
	m_paTreeWidgetItem->setFlags(m_paTreeWidgetItem->flags() | Qt::ItemIsEditable);	// Make the item editable, so the method editItem() does not fail
	g_pwNavigationTree->m_pwTreeView->editItem(m_paTreeWidgetItem);
	}
*/
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
		Assert(pTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
		pTreeItem->TreeItem_SelectWithinNavigationTree();
		}
	}


ITreeItem *
NavigationTree_PGetSelectedTreeItem()
	{
	CTreeWidgetItem * pwTreeItem = (CTreeWidgetItem *)g_pwNavigationTree->m_pwTreeView->currentItem();
	if (pwTreeItem != NULL)
		{
		Assert(pwTreeItem->m_piTreeItem != NULL);
		Assert(pwTreeItem->m_piTreeItem->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
		return pwTreeItem->m_piTreeItem;
		}
	return NULL;
	}

//	Return the pointer of the selected object if it matches the interface rti, otherwise return NULL.
ITreeItem *
NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI_ENUM rti)
	{
	ITreeItem * piTreeItem = NavigationTree_PGetSelectedTreeItem();
	if (piTreeItem == NULL && rti == RTI(TAccountXmpp))
		{
		// This is a special exception where there is no selected Tree Item, however there is just one account, so we assume the user wants to access that account
		return (ITreeItem *)g_arraypAccounts.PvGetElementUnique_YZ(); // May return NULL
		}
	if (piTreeItem != NULL)
		{
		#ifdef DEBUG
		ITreeItem * piTreeItemMatchingInterface = (ITreeItem *)piTreeItem->PGetRuntimeInterface(rti);
		if (piTreeItemMatchingInterface != NULL)
			{
			// Make sure the returned pointer matching the interface rti is inherited from ITreeItem, otherwise there could be a big crash.
			Assert(piTreeItemMatchingInterface->PGetRuntimeInterface(RTI(ITreeItem)) != NULL);
			}
		#endif
		return (ITreeItem *)piTreeItem->PGetRuntimeInterface(rti);	// May return NULL if piTreeItem does not have an interface compatible to rti.
		}
	return NULL;
	}

TProfile *
NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile()
	{
	return (TProfile *)NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI(TProfile));
	}

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
	g_pwNavigationTree->NavigationTree_RenameTreeItemWidget((CTreeWidgetItem *)g_pwNavigationTree->m_pwTreeView->currentItem());
	}
