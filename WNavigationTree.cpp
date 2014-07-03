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

WLineEditSearch::WLineEditSearch(QWidget * pwParent) : QLineEdit(pwParent)
	{
	m_pwButtonSearch = new QToolButton(this);
	QPixmap pixmap(":/ico/Find");
	m_pwButtonSearch->setIcon(QIcon(pixmap));
//	m_pwButtonSearch->setIconSize(pixmap.size());
	m_pwButtonSearch->setCursor(Qt::ArrowCursor);
	m_pwButtonSearch->setStyleSheet("QToolButton { border: none; padding: 1px; }");
	m_pwButtonSearch->setFocusPolicy(Qt::NoFocus);

	//int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	//setStyleSheet(QString("QLineEdit { border: none; padding-left: %1px; } ").arg(m_pwButtonSearch->sizeHint().width() + frameWidth + 1));
	setStyleSheet("QLineEdit { border: none; padding: 2px 2px 2px 20px; } ");
	//setStyleSheet("QLineEdit { padding-left: 5px; } ");
	setPlaceholderText("Search");
//	setFocusPolicy(Qt::ClickFocus);	// Prevent the edit control to receive the focus when the application starts
	//setMinimumSize(40, 30);
	}

#if 0
QSize WNavigationTreeCaption::minimumSizeHint() const
{
	QSize result(50, m_pwLineEdit->height());
	return result;
}
#endif

QToolButton * g_pwButtonStatusOfNavigationTree;

WNavigationTreeCaption::WNavigationTreeCaption(QWidget *parent) : QWidget(parent)
	{
	g_pwButtonStatusOfNavigationTree = new QToolButton(this);
	g_pwButtonStatusOfNavigationTree->setToolTip("Change your status");
	g_pwButtonStatusOfNavigationTree->setStyleSheet("QToolButton { border: none; padding: 3px; }");
	g_pwButtonStatusOfNavigationTree->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	g_pwButtonStatusOfNavigationTree->setPopupMode(QToolButton::InstantPopup);
	g_pwButtonStatusOfNavigationTree->setCursor(Qt::ArrowCursor);
	g_pwButtonStatusOfNavigationTree->setFocusPolicy(Qt::ClickFocus);
	EMenuAction eMenuAction_Presence = (EMenuAction)(g_uPreferences & P_kmPresenceMask);
	if (eMenuAction_Presence == ezMenuActionNone)
		eMenuAction_Presence = eMenuAction_PresenceAccountOnline;
	Widget_SetIconButton(INOUT g_pwButtonStatusOfNavigationTree, eMenuAction_Presence);
	g_pwMenuStatus = new WMenu;
	g_pwMenuStatus->InitAsDymanicMenu();
	g_pwButtonStatusOfNavigationTree->setMenu(g_pwMenuStatus);
//	connect(g_pMenuStatus, SIGNAL(aboutToShow()), g_pwMainWindow, SLOT(SL_MenuAboutToShow()));
//	connect(g_pMenuStatus, SIGNAL(triggered(QAction*)), g_pwMainWindow, SLOT(SL_MenuActionTriggered(QAction*)));

//	pwButtonStatus->addAction(new QAction("test", pwButtonStatus));

	m_pwLineEdit = new WLineEditSearch(this);

	QToolButton * pwButtonUndock = new QToolButton(this);
	pwButtonUndock->setToolTip("Float / Unfloat");
	pwButtonUndock->setStyleSheet("QToolButton { border: none; padding: 3px; }");
	QPixmap oPixmap = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
	pwButtonUndock->setIcon(oPixmap);
//	pwButtonUndock->setMaximumSize(oPixmap.size());
	pwButtonUndock->setCursor(Qt::ArrowCursor);
	pwButtonUndock->setFocusPolicy(Qt::ClickFocus);

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setMargin(0);
//	layout->setSpacing(10);
//	layout->addStretch();
	layout->addWidget(g_pwButtonStatusOfNavigationTree, Qt::AlignLeft | Qt::AlignHCenter);
	layout->addWidget(m_pwLineEdit);
	layout->addWidget(pwButtonUndock);
	setLayout(layout);
	/*
	m_pwLineEdit = new WLineEditSearch(this);
	*/
	m_pwLineEdit->installEventFilter(this);
	connect(m_pwLineEdit, SIGNAL(textChanged(QString)), this, SLOT(SL_TextChanged(QString)));
	connect(pwButtonUndock, SIGNAL(clicked()), this, SLOT(SL_ToggleDocking()));
	//m_pwLineEdit = new LineEdit(this);
	//m_pwLineEdit->setFrame(false);
	//m_pwLineEdit->setPlaceholderText("Search Contacts");
	/*
	QToolButton * padLock = new QToolButton;
	padLock->setIcon(QIcon(":/ico/Find"));
	padLock->setCursor(Qt::ArrowCursor);
	padLock->setToolTip("Find Contact");

	int extent = m_pwLineEdit->height() - 2;
	padLock->resize(extent, extent);
	padLock->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

	QHBoxLayout *layout = new QHBoxLayout(m_pwLineEdit);
	layout->setMargin(m_pwLineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2);
	layout->setSpacing(10);
	layout->addStretch();
	layout->addWidget(padLock);
	m_pwLineEdit->setLayout(layout);
	*/
	// http://aseigo.blogspot.com/2006/08/sweep-sweep-sweep-ui-floor.html
	setCursor(Qt::OpenHandCursor);		// This cursor shows to the user he/she may drag the widget to undock the Navigation Tree
	}

/*
void QDockWidgetTitleButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	QStyleOptionToolButton opt;
	opt.init(this);
	opt.state |= QStyle::State_AutoRaise;

	if (style()->styleHint(QStyle::SH_DockWidget_ButtonsHaveFrame, 0, this))
	{
		if (isEnabled() && underMouse() && !isChecked() && !isDown())
			opt.state |= QStyle::State_Raised;
		if (isChecked())
			opt.state |= QStyle::State_On;
		if (isDown())
			opt.state |= QStyle::State_Sunken;
		style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
	}

	opt.icon = icon();
	opt.subControls = 0;
	opt.activeSubControls = 0;
	opt.features = QStyleOptionToolButton::None;
	opt.arrowType = Qt::NoArrow;
	int size = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
	opt.iconSize = QSize(size, size);
	style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}
*/
/*
void WNavigationTreeCaption::paintEvent(QPaintEvent *)
	{
	QRect rcCaption = rect();
	QPainter oPainter(this);
	QBrush oBrush(d_coWhite);
	oPainter.fillRect(rcCaption, oBrush);
	QPixmap oPixmap = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);	// Get the bitmap for the icon to undock the Navigation Tree
	oPainter.drawPixmap(rcCaption.right() - oPixmap.width() - 1, (rcCaption.height() - oPixmap.height()) / 2, oPixmap);
	}

void WNavigationTreeCaption::mousePressEvent(QMouseEvent * pEvent)
	{
	if (pEvent->pos().x() > width() - 14)
		{
		// The user clicked on the 'restore' icon
		pEvent->accept();
		QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
		Q_ASSERT(dw != 0);
		dw->setFloating(!dw->isFloating());
		}
	else
		{
		pEvent->ignore();	// This line is necessary so the user may drag the title bar and to dock/undock the Navigation Tree
		}
	}
*/
/*
void WNavigationTreeCaption::updateMask()
	{
	QDockWidget *dw = qobject_cast<QDockWidget*>(parent());
	Q_ASSERT(dw != 0);

	QRect rect = dw->rect();
	QPixmap bitmap(dw->size());

		{
		QPainter painter(&bitmap);
		///initialize to transparent
		painter.fillRect(rect, Qt::color0);
		QRect contents = rect;
		contents.setTopLeft(geometry().bottomLeft());
		contents.setRight(geometry().right());
		contents.setBottom(contents.bottom()-y());
		painter.fillRect(contents, Qt::color1);
		//let's pait the titlebar
		QRect titleRect = this->geometry();
		contents.setTopLeft(titleRect.bottomLeft());
		contents.setRight(titleRect.right());
		contents.setBottom(rect.bottom()-y());
		QRect rect = titleRect;
		painter.drawPixmap(rect.topLeft(), leftPm.mask());
		painter.fillRect(rect.left() + leftPm.width(), rect.top(),
			rect.width() - leftPm.width() - rightPm.width(),
			rightPm.height(), Qt::color1);
		painter.drawPixmap(rect.topRight() - QPoint(rightPm.width() - 1, 0), rightPm.mask());
		painter.fillRect(contents, Qt::color1);
		}
	dw->setMask(bitmap);
	}
*/

//	Filter the events from the QLineEdit
bool
WNavigationTreeCaption::eventFilter(QObject * obj, QEvent *event)
	{
	if (event->type() == QEvent::KeyPress)
		{
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Escape)
			{
			// MessageLog_AppendTextFormatCo(d_coRed, "WNavigationTreeCaption::eventFilter() - Escape key pressed\n");

			}
		}
	return QWidget::eventFilter(obj, event);
	}


void
WNavigationTreeCaption::SL_TextChanged(const QString & sText)
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "WNavigationTreeCaption::SL_TextChanged() - $Q\n", &sText);
	g_pwNavigationTree->NavigationTree_DisplayTreeItemsContainingText(sText);
	}

void
WNavigationTreeCaption::SL_ToggleDocking()
	{
	//BOOL m_fFloatingRectAdjusted = FALSE;
	QDockWidget * pwParentDockWidget = qobject_cast<QDockWidget*>(parentWidget());
//	QRect rcGeometry = pwParentDockWidget->geometry();
//	MessageLog_AppendTextFormatCo(d_coRed, "WNavigationTreeCaption::SL_ToggleDocking() - top=$i\n", rcGeometry.top());

	Q_ASSERT(pwParentDockWidget != 0);
	pwParentDockWidget->setFloating(!pwParentDockWidget->isFloating());
//	MessageLog_AppendTextFormatCo(d_coRed, "\t - top=$i\n", pwParentDockWidget->geometry().top());
	}

TTreeItemMyInbox * g_pTreeItemCommunication;

///////////////////////////////////////////////////////////////////////////////////////////////////
WNavigationTree::WNavigationTree() : QDockWidget(tr("Navigation"))
	{
	g_pwNavigationTree = this;
	setObjectName(c_sNavigation);
	m_pTreeWidgetItemEditing = NULL;

	// Customize the title bar of the navigation tree
	//QWidget * pwTitleBar = titleBarWidget();
	//setTitleBarWidget(new TitleBar(this));
	//titleBarWidget()->show();
	setTitleBarWidget(new WNavigationTreeCaption(this));
	//setTitleBarWidget(new WLineEditSearch());
//	titleBarWidget()->show();


	//setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(/*DockWidgetClosable |*/ DockWidgetMovable | DockWidgetFloatable);	// Hide the close button in the title bar

	m_pwTreeView = new WTreeWidget;
	m_pwTreeView->header()->hide();
	//m_pwTreeView->setFrameStyle(QFrame::NoFrame);

	setWidget(PA_CHILD m_pwTreeView);
	m_pwTreeView->setContextMenuPolicy(Qt::CustomContextMenu);	// Send the signal customContextMenuRequested()

	connect(m_pwTreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(SL_TreeCustomContextMenuRequested(QPoint)));
	connect(m_pwTreeView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(SL_TreeItemSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(m_pwTreeView, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemClicked(QTreeWidgetItem*,int)));

	// Create the root nodes
	g_pTreeItemCommunication = new TTreeItemMyInbox;
	new TTreeItemDemo(NULL, "Applications", eMenuIconComponent);

	new TTreeItemDemo(NULL, "Marketplace", eMenuIconMarketplace);
	new TTreeItemDemo(NULL, "Finance", eMenuIconSell);
	new TTreeItemDemo(NULL, "Registry", eMenuIconCorporations);
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

/*
CTreeWidgetItem *
WNavigationTree::PAllocateTreeItemWidget(ITreeItem * piTreeItem)
	{
	CTreeWidgetItem * poTreeItem = new CTreeWidgetItem;
	poTreeItem->m_piTreeItem = piTreeItem;
	return poTreeItem;
	}
*/

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
WNavigationTree::NavigationTree_DisplayTreeItemsContainingText(const QString & sSearch)
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
