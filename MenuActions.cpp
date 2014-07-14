///////////////////////////////////////////////////////////////////////////////////////////////////
//	MenuActions.cpp
//
//	Central repository for most of the menu actions.
//	Since menu actions are reused in many places, such as the main menu and context menus,
//	it is important to not duplicate menu actions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define _			d_sz0	// Use the underscore as a separator between menu actions (this is much simpler to read than d_sz0)

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Array mapping an eMenuAction to a pszmActionDescription.
//	This array must be in sync with the values of EMenuAction.
//
//	The beginning of the string must be the text of the action, and subsequent strings may be any extra such as:
//
//	a = accelerator
//	s = status text
//	i = icon of the action
//	d = identifier for translation (NYI)
//
const PSZAC c_mapepszmMenuActions[eMenuActionMax] =
	{
	NULL,
	"Away" _ "i=StatusAway" _,					// eMenuAction_PresencePresenceAway
	"Extended Away" _ "i=StatusAwayExtended" _,	// eMenuAction_PresencePresenceAwayExtended
	"Do Not Disturb" _ "i=StatusBusy" _,		// eMenuAction_PresencePresenceBusy
	"Invisible" _ "i=StatusInvisible" _,		// eMenuAction_PresencePresenceInvisible
	"Account Disabled" _,						// eMenuAction_PresenceAccountDisabled
	"Online" _ "i=StatusOnline" _,				// eMenuAction_PresenceAccountOnline
	"Offline" _ "i=StatusOffline" _,			// eMenuAction_PresenceAccountOffline
	"Connecting" _ "i=StatusConnecting" _,		// eMenuAction_PresenceAccountConnecting
	"Disconnected" _ "i=StatusDisconnected" _,	// eMenuAction_PresenceAccountDisconnected

	"New Message" _ "i=ChatMessage3" _,	// eMenuAction_MessageNew

	"Delete Profile" _ "i=Remove" _, // eMenuAction_ProfileDelete
	"Profile Properties" _ "s=Display the properties of my profile" _, // eMenuAction_ProfileProperties
	"My Recommendations" _ "s=Display all the recommendations I made" _ "i=Reputation",  // eMenuAction_ProfileMyRecommendations

	"Delete Application" _ "i=Remove" _, // eMenuAction_ApplicationDelete
	"Application Properties" _, // eMenuAction_ApplicationProperties


	"Accept Contact Invitation..." _ "s=Accept the invitation sent by a friend" _ "i=XMPP" _,	// eMenuAction_AccountAcceptInvitation
	"Sign Up..."  _ "s=Register (create) a new XMPP account" _ "i=XMPP" _,		// eMenuAction_AccountRegister
	"Login..." _ "s=Login to an existing XMPP account" _ "i=XMPP" _, // eMenuAction_AccountLogin
	"Rename Account..." _ "i=ContactEdit" _ "s=Change the display name of the selected account" _,				// eMenuAction_AccountRename
	"Remove Account..." _ "i=Remove" _ "s=Remove the selected account from the Navigation Tree (this action does NOT delete the account on the server)" _, // "-i=list-remove.png" _, // eMenuAction_AccountDelete
	"Account Properties..." _ "i=Properties" _ "s=Display the properties of the selected account" _, // eMenuAction_AccountProperties
	"Reconnect" _ "i=ServerConnect" _ "s=Reconnect to the server" _, // eMenuAction_AccountReconnect

	"Contact" _ "i=Contact" _, //eMenuAction_Contact
	"Add Contact..." _ /*"a=Ctrl+Shift+C" _*/ "i=ContactAdd" _ "s=Add a new contact for the selected account" _,	// eMenuAction_ContactAdd
	"Add Contact to Group" _ "i=ContactAdd" _ "s=Add the selected contact to a group" _,	// eMenuAction_ContactAddToGroup
	"Rename Contact..." _ "i=ContactEdit" _ "s=Change the display name of the selected contact" _,	// eMenuAction_ContactRename
	"Remove Contact..." _ "i=ContactRemove" _ "s=Remove the selected contact from my list of approved contacts" _,	// eMenuAction_ContactRemove
	"Send File to Contact..." _ "i=FileUpload" _ "s=Upload a file to the selected contact" _, // eMenuAction_ContactSendFile	("a=Ctrl+Shift+F" does not work because it is not connected to the main menu)
	"Create Invitation..." _ "i=Forward" _ "s=Create a one-click invitation to send to my contact" _, // eMenuAction_ContactInvite
	"Approve Contact" _ "i=Accept" _ "s=Add the unsolicited contact to my list of approved contacts" _, // eMenuAction_ContactApprove
	"Subscribe" _ , // eMenuAction_ContactSubscribe
	"Unsubscribe" _ , // eMenuAction_ContactUnsubscribe
	"Contact Properties..." _ "i=Properties" _ "s=Display the properties of the selected contact" _, // eMenuAction_ContactProperties
	"Ping Contact" _ "i=Ping" _ "s=Send a network packet to the contact and calculate how long it take to receive a response" _, // eMenuAction_ContactPing

	"View" _ "s=Display extra information related to the contact" _ "i=Find" _, // eMenuAction_Contact_SubMenuView
	"Recommendations" _ "s=View the recommendations made by the contact" _ "i=Reputation" _,	// eMenuAction_Contact_SubMenuView_Recommendations
	"Bitcoin Transactions" _ "s=Display the Bitcoin transactions related to the contact" _ "i=Bitcoin" _, // eMenuAction_Contact_SubMenuView_Transactions

	"Synchronize with Contact" _ "i=OutOfSync" _ "s=Make sure me and my contact are up-to-date with the messages sent to each other" _, // eMenuAction_Synchronize

	"Group" _ "i=Group" _, // eMenuAction_Group
	"<New Group...>" _ "i=GroupAdd" _, // eMenuAction_GroupNew
	"Delete Group" _ "i=GroupDelete" _, // eMenuAction_GroupDelete
	"Rename Group" _ "i=GroupEdit" _, // eMenuAction_GroupRename
	"Add Contacts to Group..." _ "i=ContactAdd" _ "s=Add people to the group" _, // eMenuAction_GroupAddContacts
	"Remove Contact from Group" _ "i=ContactRemove" _, // eMenuAction_GroupRemoveContact
	"Group Properties..." _ "i=Properties" _, // eMenuAction_GroupProperties

	"Recommend" _ "s=Recommend the selected item to my contacts" _ "i=Reputation" _,	// eMenuAction_TreeItemRecommended
	"Rename..." _, // eMenuAction_TreeItemRename
	"Edit Message..." _ "i=Pencil" _, // eMenuAction_MessageEdit
	"Copy" _ "i=Copy" _, // eMenuAction_Copy
	"Copy Link Location..." _ "i=Copy" _, // eMenuAction_CopyHyperlink
	"Select All" _ "i=EditSelectAll" _, // eMenuAction_SelectAll

	"Display Certificates..." _ "i=Certificate" _, // eMenuAction_DisplayCertificates
	"New Wallet" _ "i=Bitcoin" _, // eMenuAction_DisplayWallet
	"New Web Browser" _ "i=Browser" _, // eMenuAction_DisplaySecureWebBrowsing

	"Generate Random Transactions" _ "i=Add" _, // eMenuAction_WalletGenerateDummyTransactions
	"Close Wallet" _ "i=Close" _, // eMenuAction_WalletClose
	"Send Ballot..." _ "s=Send a ballot to poll votes from the group" _ "i=Vote" _, // eMenuAction_BallotSend
	"Resend Ballot..." _ "s=Resend an existing ballot" _ "i=Vote" _, // eMenuAction_BallotReSend

	"Message Log..." _ "a=Ctrl+Shift+L" _ "i=LogMessages" _,	// eMenuAction_ShowLogMessages
	"Recent Error..." _ "a=Ctrl+Shift+E" _ "i=LogErrors" _,	// eMenuAction_ShowLogErrors
	"Find Text..." _ "a=Ctrl+F" _ "i=Find" _,	// eMenuAction_FindText
	"[Debug] Send Chat Log to Cambrian" _ "s=Send the entire chat history to chatlogs@xmpp.cambrian.org for analysis" _ "i=Forward" _,	// eMenuAction_DebugSendChatLog

	"Play Sounds" _ "s=Play a sound when a new message arrive" _ ,  // eMenuAction_PlaySound

	"Backup Configuration..." _ "i=FileSaveAs" _ "s=Save the current configuration into a backup file" _, // eMenuAction_ConfigurationSaveAsXmlFile
	"Restore Configuration..." _ "i=FileOpen" _ "s=Restore a previously saved backup" _, // eMenuAction_ConfigurationOpenFromXmlFile

	"Close" _ "i=Close" _ "s=Close the window" _, // eMenuAction_Close
	"Quit" _ "a=Ctrl+Q" _ "i=Quit" _ "s=Logout and Exit" _, // eMenuAction_Quit

	///////////////////////////////////
	"Insecure" _ "i=InsecureOnline" _,	// eMenuIcon_PresenceInsecureOnline
	"Failure" _ "i=Error" _,	// eMenuIconFailure
	"Certificate" _ "i=Certificate" _, // eMenuIconCertificate
	"Approve Server" _ "i=Server" _, // eMenuIconServerSecure
	"ServerWarning" _ "i=ServerWarning" _, // eMenuIconServerWarning
	"Revoke Server" _ "i=ServerDeny" _, // eMenuIconServerDeny
	"Question" _ "i=Question" _, // eMenuIconQuestion
	"Warning" _ "i=Warning" _, // eMenuIconWarning
	"Deny" _ "i=Deny" _, // eMenuIconDeny
	"Add" _ "i=Add" _, // eMenuIconAdd
	"Remove" _ "i=Remove" _, // eMenuIconRemove
	"Pencil" _ "i=Pencil" _, // eMenuIconPencil_10x10
	"Pencil" _ "i=Pencil_16x16" _, // eMenuIconPencil_16x16
	"Bitcoin" _ "i=Bitcoin" _, // eMenuIconBitcoin
	"Marketplace" _ "i=Marketplace" _, // eMenuIconMarketplace
	"Arbitration" _ "i=Judge" _, // eMenuIconMarketplaceArbitration
	"Mediation" _ "i=Mediation" _, // eMenuIconMarketplaceMediation
	"Lawyer" _ "i=Lawyer" _, // eMenuIconMarketplaceLawyers
	"Jurisdiction" _ "i=Arbitration" _, // eMenuIconJurisdiction
	"Corporations" _ "i=Corporation" _, // eMenuIconCorporations
	"Identities" _ "i=Identities" _, // eMenuIconIdentities

	"Settings" _ "i=Settings" _, // eMenuIconSettings
	"Communicate" _ "i=Chat" _, // eMenuIconCommunicate
	"Banking" _ "i=Banking" _, // eMenuIconBanking
	"Bank" _ "i=Bank" _, // eMenuIconBank
	"BankNew" _ "i=BankNew" _, // eMenuIconBankNew
	"Exchange" _ "i=Exchange" _, // eMenuIconExchange
	"Reputation" _ "i=Reputation" _, // eMenuIconReputation
	"Healthcare" _ "i=Healthcare" _, // eMenuIconHealthcare
	"SecurityGuard" _ "i=SecurityGuard" _, // eMenuIconSecurityGuard

	"SecurityOptions" _ "i=SecurityOptions" _, // eMenuIconSecurityOptions
	"Jobs" _ "i=Work" _, // eMenuIconJobs
	"Sell" _ "i=Sell" _, // eMenuIconSell
	"Services" _ "i=Services" _, // eMenuIconServices
	"Personals" _ "i=Personals" _, // eMenuIconPersonals

	"Issue" _ "i=Issue" _, // eMenuIconIssue
	"IssueAsset" _ "i=IssueAsset" _, // eMenuIconIssueAsset
	"IssueCurrency" _ "i=IssueCurrency" _, // eMenuIconIssueCurrency
	"IssueFuture" _ "i=Futures" _, // eMenuIconIssueFuture
	"Dividends" _ "i=Dividends" _, // eMenuIconIssueDividends
	"Market" _ "i=Market" _, // eMenuIconMarket
	"Vote" _ "i=Vote" _, // eMenuIconVote
	"Announce" _ "i=Announce" _, // eMenuIconBroadcast
	"Listen" _ "i=Listen" _, // eMenuIconListen
	"Ripple" _ "i=Ripple" _, // eMenuIconRipple
	"Component" _ "i=Component" _, // eMenuIconComponent
	"Community" _ "i=Community" _, // eMenuIconCommunity

	"Facebook" _ "i=Facebook" _, // eMenuIconFacebook
	"Google" _ "i=Google" _, // eMenuIconGoogleHangout
	"Bitstamp" _ "i=Bitstamp" _, // eMenuIconBitstamp
	"Coinbase" _ "i=Coinbase" _, // eMenuIconCoinbase

	"CoffeeExchange" _ "i=CoffeeExchange" _, // eMenuIconCoffeeExchange
	"CoffeeExchangeBuy" _ "i=CoffeeExchangeBuy" _, // eMenuIconCoffeeBuy
	"CoffeeExchangeSell" _ "i=Sell" _, // eMenuIconCoffeeSell

	"Home" _ "i=Home" _, // eMenuIconHome
	"GoBack" _ "i=GoBack" _, // eMenuIconGoBack
	"GoForward" _ "i=GoForward" _, // eMenuIconGoForward

	}; // c_mapepszmMenuActions[]

/*
QAction * g_rgzpActionsShortcuts[10];	// Array of all actions containing a shortcut.  This array is necessary because the method QMenu::addAction() changes the parent of the action, and consequently the shortcuts are no longer dispatched to the main application.
UINT g_iActionsShortcutLast;
*/

QAction *
PaAllocateMenuAction(EMenuAction eMenuAction)
	{
	Assert(eMenuAction > ezMenuActionNone);
	Assert(eMenuAction < LENGTH(c_mapepszmMenuActions));

	PSZAC pszmMenuAction = c_mapepszmMenuActions[eMenuAction];
	Assert(pszmMenuAction != NULL);
	if (pszmMenuAction == NULL)
		return NULL;	// Just in case, otherwise the application crashes
	Assert(pszmMenuAction[0] != '\0');
	PSZAC pszaMenuAccelerator = NULL;
	PSZAC pszaMenuStatusMessage = NULL;
	PSZAC pszaMenuIcon = NULL;
	PSZAC pszmActionDescription = pszmMenuAction;
	// Search for the description for any information about the menu action
	while (TRUE)
		{
		// Skip to the next string
		while (*pszmActionDescription++ != '\0')
			;
		const UINT ch = (BYTE)pszmActionDescription[0];
		pszmActionDescription += 2;	// Skip the first letter and the '='
		switch (ch)
			{
		case '\0':
			goto Done;
		case 'a':
			Assert(pszaMenuAccelerator == NULL && "Accelerator defined twice");		// This happend when there is a missing _ and pszmActionDescription goes to the next menu action
			pszaMenuAccelerator = pszmActionDescription;
			break;
		case 's':
			Assert(pszaMenuStatusMessage == NULL);
			pszaMenuStatusMessage = pszmActionDescription;
			//MessageLog_AppendTextFormatCo(d_coGray, "pszaMenuStatusMessage=$s\n", pszaMenuStatusMessage);
			break;
		case 'i':
			Assert(pszaMenuIcon == NULL && "Icon defined twice");		// This happend when there is a missing _ and pszmActionDescription goes to the next menu action
			pszaMenuIcon = pszmActionDescription;
			break;
		default:
			MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Expecting valid template for menu description '$s'\n", pszmActionDescription - 2);
			Assert(FALSE && "Unknown template menu description (must be 'a', 's', or 'i'");
			} // switch
		Assert(pszmActionDescription[-1] == '=');
		} // while
	Done:
	Assert(pszmMenuAction != NULL);
	//QAction * paAction = new QAction(pszmMenuAction, s_poParentOfQActionForMenu);
	QAction * paAction = new QAction(pszmMenuAction, g_pwMainWindow);
	Assert(paAction->parent() == g_pwMainWindow);
	if (pszaMenuStatusMessage != NULL)
		paAction->setStatusTip(pszaMenuStatusMessage);
	if (pszaMenuAccelerator != NULL)
		{
		paAction->setShortcut(QKeySequence(pszaMenuAccelerator));
		/*
		g_rgzpActionsShortcuts[g_iActionsShortcutLast++] = paAction;
		Assert((g_iActionsShortcutLast < LENGTH(g_rgzpActionsShortcuts)) && "Please increase the length of g_rgzpActionsShortcuts[]");
		*/
		#ifdef DEBUG
		if (paAction->shortcut().isEmpty())
			MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Invalid accelerator $s\n", pszaMenuAccelerator);
		#endif
		}
	if (pszaMenuIcon != NULL)
		{
		QString sIcon;
		sIcon.sprintf(":/ico/%s", pszaMenuIcon);
		paAction->setIcon(QIcon(sIcon));
		#ifdef DEBUG
		if (paAction->icon().pixmap(QSize(16,16)).isNull())
			MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Unable to load the menu icon named '$Q'\n", &sIcon);
		#endif
		}
	return paAction;
	} // PaAllocateMenuAction()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Array of pointers of menu actions corresponding to EMenuAction.
//	The purpose of using pointers to QAction instead of the object QAction, is because most of the menu actions should be initiated on demand: there is no point to load the text, accelerators, status bar description of each menu action if they will never be used during a session.
//	Also, some actions (such as eMenuAction_AccountDelete and eMenuAction_ContactRemove) may share the same QAction object.
QAction * g_rgpActions[eMenuActionMax];

QAction *
PGetMenuAction(EMenuAction eMenuAction)		// TODO:  Rename to PGetMenuAction_NZ() and make sure a NULL pointer is never returned.
	{
	Assert(eMenuAction > ezMenuActionNone);
	Assert(eMenuAction < LENGTH(g_rgpActions));

	QAction * pAction = g_rgpActions[eMenuAction];
	if (pAction != NULL)
		{
		pAction->setEnabled(true);
		/*
		pAction->setParent(g_pwMainWindow);
		if (s_poParentOfQActionForMenu != NULL)
			{
			Assert(s_poParentOfQActionForMenu == g_pwMainWindow);
			pAction->setParent(s_poParentOfQActionForMenu);		// If the action is initialized by the MenuBar, then it means the WMainWindow is capable to handle the action, and therefore must have the WMainWindow as its parent
			}
		*/
		return pAction;
		}
	// We need to initialize the action
	pAction = PaAllocateMenuAction(eMenuAction);
	g_rgpActions[eMenuAction] = pAction;
	return pAction;
	} // PGetMenuAction()

QIcon
GetMenuIcon(EMenuAction eMenuActionIcon)
	{
	return PGetMenuAction(eMenuActionIcon)->icon();
	}

void
Menu_InitializeAllMenuActionsHavingAccelerators()
	{
	int iMenuAction = eMenuActionMax;
	while (--iMenuAction > 0)
		{
		PSZAC pszmMenuAction = c_mapepszmMenuActions[iMenuAction];
		Assert(pszmMenuAction != NULL);
		if (pszmMenuAction == NULL)
			continue;	// Just in case, otherwise the application crashes
		// Search for the description for any information about the menu action
		while (TRUE)
			{
			// Skip to the next string
			while (*pszmMenuAction++ != '\0')
				;
			const UINT ch = (BYTE)pszmMenuAction[0];
			pszmMenuAction += 2;	// Skip the first letter and the '='
			if (ch == '\0')
				break;
			if (ch == 'a')
				{
				QAction * pAction = PGetMenuAction((EMenuAction)iMenuAction);	// Load the action which will enable the accelerator
				g_pwMenuCambrian->addAction(pAction);	// Add the action to the menu, so it can trigger a signal to the WMainWindow
				break;
				}
			} // while
		} // while
	} // Menu_InitializeAllMenuActionsHavingAccelerators()

PSZAC
PszGetMenuActionText(EMenuAction eMenuAction)
	{
	Assert(eMenuAction > ezMenuActionNone);
	Assert(eMenuAction < LENGTH(g_rgpActions));
	return c_mapepszmMenuActions[eMenuAction];	// May return NULL
	}

//	Set the icon of a widget (typically a window or a dialog) using the same icon of a menu action.
void
Widget_SetIcon(INOUT QWidget * pwWidget, EMenuAction eMenuAction)
	{
	Assert(pwWidget != NULL);
	QAction * pAction = PGetMenuAction(eMenuAction);
	Assert(pAction != NULL);
	if (pAction != NULL)
		pwWidget->setWindowIcon(pAction->icon());
	}

//	See also class WButtonIconForToolbar
void
Widget_SetIconButton(INOUT QAbstractButton * pwButton, EMenuAction eMenuAction)
	{
	Assert(pwButton != NULL);
	QAction * pAction = PGetMenuAction(eMenuAction);
	Assert(pAction != NULL);
	if (pAction != NULL)
		pwButton->setIcon(pAction->icon());
	}


//	Find the eMenuAction from a pointer to a QAction object.  The performance of this function is O(n).
//	This function is usfeul to determine what action was selected by the user from a menu or toolbar.
//	Return ezMenuActionNone (error) if pActionSearch cannot be found in g_rgpActions[].
EMenuAction
EFindMenuAction(const QAction * pActionSearch)
	{
	Assert(pActionSearch != NULL);
	EMenuAction eMenuAction = (EMenuAction)pActionSearch->data().toInt();
	if (eMenuAction != ezMenuActionNone)
		return eMenuAction;
	for (EMenuAction eMenuAction = ezMenuActionNone; eMenuAction < eMenuActionMax; eMenuAction = (EMenuAction)(eMenuAction + 1))
		{
		if (g_rgpActions[eMenuAction] == pActionSearch)
			return eMenuAction;
		}
	QString sAction = pActionSearch->text();
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "EFindMenuAction() is unable to find the identifier '$i' for action '$Q'\n", eMenuAction, &sAction);
	return ezMenuActionNone;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
BOOL WMenu::s_fActionReservedForContextMenu;
QAction * WMenu::s_pActionCurrentlyProcessedByMainWindow;

WMenu::WMenu() : QMenu(g_pwMainWindow)
	{
	}

WMenu::WMenu(PSZAC pszName) : QMenu(pszName, g_pwMenuBar)
	{
	InitAsDymanicMenu();
//	MenuBarInitialize(this);		// We need to initialize the menu so the accelerators/shotcuts are there.  In the future, an optimization could be to skip menu items without accelerators/shortcuts
	Assert(g_pwMenuBar != NULL);
	g_pwMenuBar->addAction(menuAction());
	}

WMenu::~WMenu()
	{
	//MessageLog_AppendTextFormatCo(d_coBlack, "~WMenu()\n");
	WMenu::s_fActionReservedForContextMenu = FALSE;
	}

/*
//	Return the pointer of the QAction TO BE HANDLED.
//	The method may return NULL if the QAction has already been handled by the main window.
//
//	The Qt framework will automatically handle any QAction having a parent widget via the signal QMenu::triggered().
//	The parent widget is necessary to display the action text in the status bar.
//	As a result, we must have a mechanism to determine if a QAction was handled	by the parent.
//
QAction *
WMenu::PContextMenuDisplay()
	{
	QAction * pActionSelected = exec(QCursor::pos());
	Endorse(pActionSelected == NULL);	// The user cancelled the context menu
	if (pActionSelected != NULL)
		{
		// This code must be fixed
		if (pActionSelected->parent() == NULL)
			return pActionSelected;	// An action without a parent means the caller is responsible of executing it
		Assert(pActionSelected->parent() == g_pwMainWindow);	// The only allowed parent is the WMainWindow
		}
	return NULL;	// An action with a parent means the WMainWindow already executed it
	}
*/

void
WMenu::ActionAdd(const EMenuAction eMenuAction)
	{
	Assert(eMenuAction < 0xFF);
	const EMenuActionByte rgzeMenuActions[2] = { (EMenuActionByte)eMenuAction, ezMenuActionNone };
	ActionsAdd(rgzeMenuActions);
	}

void
WMenu::ActionAddWithSeparator(const EMenuAction eMenuAction)
	{
	Assert(eMenuAction < 0xFF);
	const EMenuActionByte rgzeMenuActions[3] = { (EMenuActionByte)eMenuAction, eMenuActionSeparator, ezMenuActionNone };
	ActionsAdd(rgzeMenuActions);
	}

void
WMenu::ActionsAdd(const EMenuActionByte rgzeMenuActions[])
	{
	Assert(rgzeMenuActions != NULL);
	while (TRUE)
		{
		const EMenuAction eMenuAction = (EMenuAction)*rgzeMenuActions++;
		switch (eMenuAction)
			{
		case ezMenuActionNone:
			return;		// We have reached the null-terminator, so we are done adding menu actions.
		case eMenuActionSeparator:
			addSeparator();
			break;
		default:
			Assert(eMenuAction < eMenuActionMax);
			QAction * pAction = PGetMenuAction(eMenuAction);
			if (pAction != NULL)
				{
				// Initialize the menu actions
				if (eMenuAction == eMenuAction_PlaySound)
					{
					pAction->setCheckable(true);
					pAction->setChecked((g_uPreferences & P_kfDontPlaySoundWhenNewMessageArrive) == 0);
					}
				addAction(pAction);
				} // if
			} // switch
		} // while
	} // ActionsAdd()

void
WMenu::ActionsAddSubMenu(EMenuAction eMenuActionSubMenuName, const EMenuActionByte rgzeActionsSubMenuItems[])
	{
	Assert(FALSE && "Never tested!");
	QAction * pAction = PGetMenuAction(eMenuActionSubMenuName);
	if (pAction != NULL)
		{
		WMenu * pMenu = (WMenu *)addMenu(pAction->text());	// Create a submenu from the text of the action
		pMenu->ActionsAdd(rgzeActionsSubMenuItems);
		}
	}

void
WMenu::ActionAddFromText(PSZUC pszText, int idAction, EMenuAction eMenuIcon)
	{
	QAction * pAction = addAction(GetMenuIcon(eMenuIcon), (CString)pszText);
	pAction->setData(idAction);
	}

void
WMenu::ActionAdd(EMenuAction eMenuAction, PSZAC pszText)
	{
	ActionAddFromText((PSZUC)pszText, eMenuAction, eMenuAction);
	}

void
WMenu::ActionSetCheck(EMenuAction eMenuAction, BOOL fuChecked)
	{
	if (!fuChecked)
		return;
	QAction * pActionOld = PGetMenuAction(eMenuAction);
	Assert(pActionOld != NULL);
	if (pActionOld == NULL)
		return;
	QAction * pActionNew = addAction(pActionOld->text());
	//pActionNew->setStatusTip(pActionOld->statusTip());	// This code does not work because the parent of the action is the menu
	pActionNew->setData(eMenuAction);
	pActionNew->setCheckable(true);
	pActionNew->setChecked(true);
	insertAction(pActionOld, pActionNew);	// Insert the new action next to the old one
	removeAction(pActionOld);				// Remove the old action (those two methods essentially replace the old action with the new one)
		/*
		QAction * pActionNew = PaAllocateMenuAction(eMenuAction);
		//pActionNew->setParent(this);
		pActionNew->setCheckable(true);
		pActionNew->setChecked(true);
		pActionNew->setIcon(QIcon());	// Remove the icon, so the checkbox may be visible
		pActionNew->setData(eMenuAction);
		insertAction(pAction, pActionNew);
		removeAction(pAction);
		*/
	}

WMenu *
WMenu::PMenuAdd(PSZAC pszText, EMenuAction eMenuIcon)
	{
	return (WMenu *)addMenu(GetMenuIcon(eMenuIcon), pszText);
	}


void
WMenu::InitAsDymanicMenu()
	{
	connect(this, SIGNAL(aboutToShow()), g_pwMainWindow, SLOT(SL_MenuAboutToShow()));
	_ConnectActionsToMainWindow();
	}

void
WMenu::_ConnectActionsToMainWindow()
	{
	#if 0
	// This code was commented on Feb 7, 2014.
	// We have to specify the ConnectionType otherwise the Qt framework will send two consecutive signals QActions.  This way, we get them at the same time and the slot SL_MenuActionTriggered() can ignore the second QAction.
	connect(this, SIGNAL(triggered(QAction*)), g_pwMainWindow, SLOT(SL_MenuActionTriggered(QAction*)), (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));
	#else
	connect(this, SIGNAL(triggered(QAction*)), g_pwMainWindow, SLOT(SL_MenuActionTriggered(QAction*)));
	#endif
	}

QAction *
WMenu::_PExec()
	{
	_ConnectActionsToMainWindow();
	QAction * pActionSelected = exec(QCursor::pos());
	/*
	clear();
	g_pMenuAdvanced->clear();
	QAction ** ppActionShortcut = g_rgzpActionsShortcuts;
	while (TRUE)
		{
		QAction * pActionShortcut = *ppActionShortcut++;
		if (pActionShortcut == NULL)
			break;
		MessageLog_AppendTextFormatCo(d_coBlack, "shortcut=0x$p\n", pActionShortcut);
		g_pMenuAdvanced->addAction(pActionShortcut);
		pActionShortcut->setParent(g_pwMainWindow);
		pActionShortcut->setShortcutContext(Qt::ApplicationShortcut);
		}
	g_pMenuAdvanced->_ConnectActionsToMainWindow();
	*/
	return pActionSelected;
	}

void
WMenu::DisplayContextMenuAndSendActionsToMainWindow()
	{
	s_fActionReservedForContextMenu = FALSE;
//	Assert(s_fActionReservedForContextMenu == FALSE);
/*
	s_fActionReservedForContextMenu = TRUE;
	_ConnectActionsToMainWindow();
	QAction * pAction =  exec(QCursor::pos());
*/
	/*QAction * pAction =*/ _PExec();
	/*
	if (pAction != NULL)
		{
		MessageLog_AppendTextFormatCo(d_coOrange, "WMenu::DisplayContextMenuAndSendActionsToMainWindow() is returning $i\n", );
		if (EFindMenuAction(pAction) != eMenuAction_DisplayWallet)	// This line is a gross hack, however for the demo, we don't want to create two wallets
			MainWindow_MenuActionExecute(pAction);
		}
	*/
	}

EMenuAction
WMenu::EDisplayContextMenu()
	{
	s_fActionReservedForContextMenu = TRUE;
	/*
	s_fActionReservedForContextMenu = TRUE;
	_ConnectActionsToMainWindow();	// This line is necessary so the Qt framework display the action text in the Status Bar
	QAction * pAction = exec(QCursor::pos());
	*/
	QAction * pAction = _PExec();
//	s_fActionReservedForContextMenu = FALSE;
	if (pAction != NULL)
		{
		MessageLog_AppendTextFormatCo(d_coOrange, "WMenu::EDisplayContextMenu() is returning $i\n", EFindMenuAction(pAction));
		return EFindMenuAction(pAction);
		}
	return ezMenuActionNone;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
QMenuBar * g_pwMenuBar;
WMenu * g_pwMenuStatus;	// Menu in the Navigation Tree displaying the online status
WMenu * g_pwMenuCambrian;
WMenu * g_pwMenuView;
WMenu * g_pwMenuSecurity;
WMenu * g_pwMenuAdvanced;

const EMenuActionByte c_rgzeActionsMenuStatus[] =	// This menu is displayed next to the "Search" of the Navigation Tree
	{
	eMenuAction_PresenceAccountOnline,
	eMenuAction_PresenceAway,
	eMenuAction_PresenceBusy,
//	eMenuAction_PresenceInvisible,
	eMenuAction_PresenceAccountOffline,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuCambrian[] =
	{
	/*
	eMenuAction_AccountLogin,
	eMenuAction_AccountRegister,
	eMenuActionSeparator,
	eMenuAction_DisplayWallet,
	*/
	eMenuAction_DisplaySecureWebBrowsing,
	eMenuIconMarketplace,
	eMenuActionSeparator,
	eMenuAction_Quit,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuView[] =	// Contacts
	{
	eMenuAction_ContactAdd,
	//eMenuAction_ContactInvite,
	eMenuAction_AccountAcceptInvitation,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuSecurity[] =
	{
	eMenuAction_DisplayCertificates,
	ezMenuActionNone
	};

const EMenuActionByte c_rgzeActionsMenuAdvanced[] =
	{
	eMenuAction_PlaySound,
	eMenuAction_FindText,
	eMenuActionSeparator,
	eMenuAction_ShowLogMessages,
	eMenuAction_ShowLogErrors,
	eMenuActionSeparator,
	eMenuAction_ConfigurationSaveAsXmlFile,
	eMenuAction_ConfigurationOpenFromXmlFile,
	eMenuActionSeparator,
	eMenuAction_DisplayCertificates,
	ezMenuActionNone
	};

BOOL Configuration_FIsWithoutAccounts();

void
MenuBarInitialize(WMenu * pMenu)
	{
	Assert(pMenu != NULL);
	const EMenuActionByte * prgzeActions = NULL;
	if (pMenu == g_pwMenuCambrian)
		prgzeActions = c_rgzeActionsMenuCambrian; // Configuration_FIsWithoutAccounts() ? c_rgzeActionsMenuChatWithoutAccounts : c_rgzeActionsMenuChatWithAccounts;
	else if (pMenu == g_pwMenuView)
		prgzeActions = c_rgzeActionsMenuView;
	else if (pMenu == g_pwMenuSecurity)
		prgzeActions = c_rgzeActionsMenuSecurity;
	else if (pMenu == g_pwMenuAdvanced)
		prgzeActions = c_rgzeActionsMenuAdvanced;
	else if (pMenu == g_pwMenuStatus)
		prgzeActions = c_rgzeActionsMenuStatus;
	Assert(prgzeActions != NULL && "Unknown menu to initialize");
	if (prgzeActions != NULL)
		{
		pMenu->clear();
		pMenu->ActionsAdd(prgzeActions);
		}
	}

void
WMainWindow::MenuBarInitializeAndConnectActions(WMenu * pMenu)
	{
	Assert(pMenu != NULL);
	pMenu->InitAsDymanicMenu();
	MenuBarInitialize(pMenu);		// We need to initialize the menu so the accelerators/shotcuts are there.  In the future, an optimization could be to skip menu items without accelerators/shortcuts
	}

//	SL_MenuAboutToShow(), slot of signal QMenu::aboutToShow()
void
WMainWindow::SL_MenuAboutToShow()
	{
//	MessageLog_AppendTextFormatCo(d_coGrayDark, "SL_MenuAboutToShow()\n");
	WMenu::s_fActionReservedForContextMenu = FALSE;
	MenuBarInitialize((WMenu *)sender());
	}

//#pragma GCC diagnostic ignored "-Wswitch"

///////////////////////////////////////////////////////////////////////////////////////////////////
//	SL_MenuActionTriggered(), slot of signal QMenu::triggered()
//
//	This is the main entry point for all the menu actions.
//	Unfortunately Qt has a bug where the signal triggered() is fired twice.  It appears this bug won't be fixed (see https://bugreports.qt-project.org/browse/QTBUG-25669).  As a result, we have to write code to detect the action being fired twice.
//
void
WMainWindow::SL_MenuActionTriggered(QAction * pAction)
	{
	if (pAction == NULL)
		return;	// The NULL pointer is when a context menu is called
	Assert(pAction->metaObject() != NULL);	// Call a virtual method to verify the action is a valid object.  For some reasons, when an action is queued with Qt::QueuedConnection, the object has been deleted, yet Qt calls this slot.
#if 0
	EMenuAction eMenuAction = EFindMenuAction(pAction);
	MessageLog_AppendTextFormatCo(d_coGrayDark, "SL_MenuActionTriggered(0x$p, pParent=0x$p) eMenuAction=$i ($s)\n", pAction, pAction->parent(), eMenuAction, PszGetMenuActionText(eMenuAction));
#endif
	if (WMenu::s_fActionReservedForContextMenu)
		{
		// The menu action is reserved for the local context menu, however if the action has a shortcut/accelerator, then process it
//		MessageLog_AppendTextFormatCo(d_coGrayDark, " \t Action $i ($s) is reserved for local context menu\n", eMenuAction, PszGetMenuActionText(eMenuAction));
/*
		if (pAction->shortcut().isEmpty())
			return;
*/		return;
		}
	if (WMenu::s_pActionCurrentlyProcessedByMainWindow != NULL)
		{
//		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGrayDark), " \t Ignoring action 0x$p because we are already processing another action\n", pAction);
		Assert(WMenu::s_pActionCurrentlyProcessedByMainWindow == pAction);
		return;		// We are already processing one QAction, so ignore the other
		}
	WMenu::s_pActionCurrentlyProcessedByMainWindow = pAction;
	MainWindow_MenuActionExecute(pAction);
	WMenu::s_pActionCurrentlyProcessedByMainWindow = NULL;	// We are ready to process another menu action
//	WMenu::s_fActionReservedForContextMenu = TRUE;	// Make the action reserved for the context menu to prevent two actions to be executed without showing a menu.  Everytime a menu is about to be displayed, this variables is reset.  This is a workaround because Qt sends sometimes two QAction when the user clicks on a menu item.
	} // SL_MenuActionTriggered()


//	This is the main function processing all the menu actions, regardless of the source
void
MainWindow_MenuActionExecute(QAction * pAction)
	{
	Assert(pAction != NULL);
	EMenuAction eMenuAction = EFindMenuAction(pAction);
	ITreeItem * pTreeItem = NavigationTree_PGetSelectedTreeItem();
	if (pTreeItem != NULL)
		eMenuAction = pTreeItem->TreeItem_EDoMenuAction(eMenuAction);

	switch (eMenuAction)
		{
	case ezMenuActionNone:
		return;		// This is the case when the Tree Item handled the action
	case eMenuAction_PresenceAccountOnline:
	case eMenuAction_PresenceAway:
	case eMenuAction_PresenceBusy:
	case eMenuAction_PresenceInvisible:
	case eMenuAction_PresenceAccountOffline:
		Configuration_GlobalSettingsPresenceUpdate(eMenuAction);
		return;

	case eMenuAction_AccountAcceptInvitation:
	case eMenuAction_AccountRegister:
	case eMenuAction_AccountLogin:
		DisplayDialogAccountNew(eMenuAction);
		return;
	case eMenuAction_ContactAdd:
		void DisplayDialogContactNew();
		DisplayDialogContactNew();
		return;

	case eMenuAction_TreeItemRename:
	case eMenuAction_AccountRename:
	case eMenuAction_ContactRename:
	case eMenuAction_GroupRename:
		NavigationTree_RenameSelectedItem();
		return;
	/*
	case eMenuAction_AccountDelete:
		ChatAccount_DeleteSelectedUI();
		return;
	*/
	/*
	case eMenuAction_ContactRemove:
		ChatContact_DeleteSelectedUI();
		return;
	case eMenuAction_ContactSendFile:
		ChatContact_UploadFileTo();
		return;
	case eMenuAction_FindText:
		ChatContact_FindText();
		return;
	*/
	case eMenuAction_DisplayCertificates:
		g_oConfiguration.NavigationTree_DisplayAllCertificatesToggle();
		return;
	case eMenuAction_DisplayWallet:
		g_oConfiguration.NavigationTree_DisplayWallet();
		return;
	case eMenuAction_DisplaySecureWebBrowsing:
		void NavigationTree_NewBrowser();
		NavigationTree_NewBrowser();
		return;
	case eMenuIconMarketplace:
		void NavigationTree_NewBrowserMarketplace();
		NavigationTree_NewBrowserMarketplace();
		return;
	case eMenuAction_ShowLogMessages:
		MessageLog_Show();
		return;
	case eMenuAction_ShowLogErrors:
		ErrorLog_Show();
		return;
	case eMenuAction_PlaySound:
		g_uPreferences ^= P_kfDontPlaySoundWhenNewMessageArrive;
		return;
	case eMenuAction_ConfigurationSaveAsXmlFile:
		Configuration_SaveAs();
		return;
	case eMenuAction_ConfigurationOpenFromXmlFile:
		Configuration_Restore();
		return;
	case eMenuAction_Quit:
		//g_pwMainWindow->close();
		QApplication::quit();
		return;
	default:
		QString sTextAction = pAction->text();
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "The menu action $i ($Q) is missing a code handler.  Please contact the developers.\n", eMenuAction, &sTextAction);
		} // switch
	} // MainWindow_MenuActionExecute()
