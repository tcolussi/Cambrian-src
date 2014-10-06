#ifndef MENUACTIONS_H
#define MENUACTIONS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


enum EMenuAction	// Must be in sync with c_mapepszmMenuActions[]
	{
	eMenuActionSeparator = 0xFF,		// Add a separator when adding menu actions to a menu (the separator is usually to separate group of menu items)
	ezMenuActionNone = 0,				// No action to be taken (this is also the zero-terminator in an array rgze[])

	// The following eMenuAction_Presence* must be at the beginning of this enumeration because they are serialized
	eMenuAction_PresenceAway,			// Your status automatically changes to away when your computer is idle for more than 20 minutes
	eMenuAction_PresenceAwayExtended,
	eMenuAction_PresenceBusy,
	eMenuAction_PresenceInvisible,			// The account is connected, however invisible to others (others will see the contact as offline)
	eMenuAction_PresenceAccountDisabled,	// The account has been disabled by the user and therefore will not attempt to connect unless explicitly enabled
	// The following presences are not serialized because they are dynamic and depends on the network condition
	eMenuAction_PresenceAccountOnline,			// The account is ready to chat (connected to the server)
	eMenuAction_PresenceAccountOffline,			// The account offline, however will become online next time the application starts, or the global status changes
	eMenuAction_PresenceAccountConnecting,
	eMenuAction_PresenceAccountDisconnected,		// The account is not connected to the server (probably there is no Internet connection or the server is down)
	eMenuAction_PresenceLast = eMenuAction_PresenceAccountDisconnected,

	eMenuAction_MessageNew,			// There is a new message available to read

	eMenuAction_ProfileDelete,
	eMenuAction_ProfileProperties,
	eMenuAction_ProfileMyRecommendations,	// Display all the recommendations I made
	eMenuAction_ProfileShowDeletedObjects,		// Display all hidden objects such as contacts and groups
	eMenuAction_ProfilePermanentlyDeleteObject,	// Permanently delete a hidden object

	eMenuAction_ApplicationHide,			// Hide the application from the Navigation Tree (the application will keep running in memory)
	eMenuAction_ApplicationDelete,			// Permanently delete the application
	eMenuAction_ApplicationProperties,

	eMenuAction_AccountAcceptInvitation,	// Accept the invitation sent by a friend (the invitation is typically sent by email or by another messaging system)
	eMenuAction_AccountRegister,		// Register (create a new account)
	eMenuAction_AccountLogin,			// Login (add an existing account)
	eMenuAction_AccountRename,			// Change the display name of the selected account
	eMenuAction_AccountDelete,			// Delete/remove the selected account
	eMenuAction_AccountProperties,		// Display the properties of the selected account
	eMenuAction_AccountReconnect,		// Reconnect to the server

	eMenuAction_Contact,				// Generic icon for a contact
	eMenuAction_ContactAdd,				// Add a new contact to the selected account
	eMenuAction_ContactAddToGroup,		// Add the selected contact to an existing group
	eMenuAction_ContactRename,
	eMenuAction_ContactRemove,			// Delete/remove the selected contact
	eMenuAction_ContactUndelete,		// Undelete a removed/deleted contact
	eMenuAction_ContactSendFile,		// Send a file to a contact
	eMenuAction_ContactInvite,			// Create an invitation for a new contact
	eMenuAction_ContactApprove,			// Add the selected contact to your list of approved contacts
	eMenuAction_ContactSubscribe,
	eMenuAction_ContactUnsubscribe,
	eMenuAction_ContactProperties,		// Display the properties of the selected contact
	eMenuAction_ContactPing,			// Ping to the contact

	eMenuAction_Contact_SubMenuView,
	eMenuAction_Contact_SubMenuView_Recommendations,	// View the recommendations made by the contact
	eMenuAction_Contact_SubMenuView_Transactions,		// Display the (Bitcoin) transactions related to the contact

	eMenuAction_SynchronizeWithContact,
	eMenuAction_SynchronizeWithGroup,

	eMenuAction_Group,
	eMenuAction_GroupNew,
	eMenuAction_GroupCreate,
	eMenuAction_GroupDelete,
	eMenuAction_GroupUndelete,
	eMenuAction_GroupRename,
	eMenuAction_GroupAddContacts,
	eMenuAction_GroupRemoveContact,
	eMenuAction_GroupProperties,		// Display the properties of the group

	eMenuAction_GroupChannel,

	eMenuAction_TreeItemRecommended,	// Generic menu action to recommend an item in the Navigation Tree
	eMenuAction_TreeItemRename,			// Generic menu action to rename an item in the Navigation Tree
	eMenuAction_MessageEdit,
	eMenuAction_Copy,
	eMenuAction_CopyHyperlink,
	eMenuAction_SelectAll,

	eMenuAction_DisplayProfileInfo,		// Show the node displaying the profile(s) info
	eMenuAction_DisplayCertificates,	// Display the certificates securing (encrypting) the communication
	eMenuAction_DisplayBallotMaster,	// Manage ballots to create polls
	eMenuAction_DisplayGroupManager,	// Manage groups

    eMenuAction_WikiSubMenu,
	//eMenuAction_WikiNavShellContacts,
	//eMenuAction_WikiNavShellSideBar,
	//eMenuAction_WikiNavShellHeader,
    eMenuAction_WikiOfficeKingpin,
    eMenuAction_WikiPomodoro,
    eMenuAction_WikiJapiTests,
    eMenuAction_WikiScratch,
	//eMenuAction_WikiHtml5Xik,
	//eMenuAction_WikiGroupManager,
	//eMenuAction_WikiBallotMaster,
	eMenuAction_AppDefaultNewTab,
	eMenuAction_AppUnderconstruction,
	//eMenuAction_AppHome,
	eMenuAction_AppConfiguration,

	eMenuAction_DisplayWallet,		// Ccreate a new wallet
	eMenuAction_DisplaySecureWebBrowsing,	// New instance of a web browser

	eMenuAction_WalletGenerateDummyTransactions,
	eMenuAction_WalletClose,
	eMenuAction_BallotSend,					// Send a ballot to poll votes from the group
	eMenuAction_BallotReSend,				// Resend an existing ballot
	eMenuAction_BallotAddToBallotmaster,	// Make a copy of the selected ballot and add it to the Ballotmaster as a future template

	eMenuAction_ShowLogMessages,		// Show the MessageLog
	eMenuAction_ShowLogErrors,			// Show the ErrorLog (aka Recent Errors)
	eMenuAction_FindText,				// Search text in the message log
	eMenuAction_DebugSendChatLog,		// Send the Chat Log to the Cambrian developer for debugging purpose

	eMenuAction_PlaySound,

	eMenuAction_ConfigurationSaveAsXmlFile,	// The user may save the configuration under a different file name.  Useful to perform a backup.
	eMenuAction_ConfigurationOpenFromXmlFile,	// Open a configuration file

	eMenuAction_Close,					// Close the window
	eMenuAction_Quit,					// Quit the chat application

	///////////////////////////////////
	// The following are no longer menu actions, but icons.  Since most menu actions have an associated icon, it is convenient to have the icons in the same enumeration.
	eMenuIcon_PresenceInsecureOnline,	// The user is online using a non-Cambrian insecure client.
	eMenuIconFailure,
	eMenuIconCertificate,
	eMenuIconServerSecure,
	eMenuIconServerWarning,
	eMenuIconServerDeny,
	eMenuIconQuestion,
	eMenuIconWarning,
	eMenuIconDeny,
	eMenuIconAdd,
	eMenuIconRemove,
	eMenuIconPencil_10x10,
	eMenuIconPencil_16x16,
	eMenuIconBitcoin,
	eMenuIconMarketplace,
	eMenuIconMarketplaceArbitration,
	eMenuIconMarketplaceMediation,
	eMenuIconMarketplaceLawyers,
	eMenuIconJurisdiction,
	eMenuIconCorporations,
	eMenuIconIdentities,

	eMenuIconMenu,
	eMenuIconSettings,
	eMenuIconCommunicate,
	eMenuIconBanking,
	eMenuIconBank,
	eMenuIconBankNew,
	eMenuIconExchange,
	eMenuIconReputation,
	eMenuIconHealthcare,
	eMenuIconSecurityGuard,
	eMenuIconSecurityOptions,
	eMenuIconJobs,
	eMenuIconSell,
	eMenuIconServices,
	eMenuIconPersonals,
	eMenuIconIssue,
	eMenuIconIssueAsset,
	eMenuIconIssueCurrency,
	eMenuIconIssueFuture,
	eMenuIconIssueDividends,
	eMenuIconMarket,
	eMenuIconVote,
	eMenuIconBroadcast,
	eMenuIconListen,
	eMenuIconRipple,
	eMenuIconComponent,
	eMenuIconCommunity,

	eMenuIconFacebook,
	eMenuIconGoogleHangout,
	eMenuIconBitstamp,
	eMenuIconCoinbase,

	eMenuIconCoffeeExchange,
	eMenuIconCoffeeBuy,
	eMenuIconCoffeeSell,

	eMenuIconHome,
	eMenuIconGoBack,
	eMenuIconGoForward,

	eMenuActionMax,	// Last menu action to be defined in c_mapepszmMenuActions[]

	// The following 'special actions' do not appear on menus, however they are actions for the virtual method TreeItem_EDoMenuAction().
	eMenuSpecialAction_ITreeItemRenamed,	// Notification message the display name of a ITreeItem was changed.  This is an opportunity to validate the new name, or if the new name is empty, set it to the default value.
	eMenuSpecialAction_GroupFirst,
	eMenuSpecialAction_GroupLast = eMenuSpecialAction_GroupFirst + 100,

	// The following menu actions are for sharing icons with other menu actions
	eMenuIconXmpp = eMenuAction_AccountLogin,
	eMenuIconAccept = eMenuAction_ContactApprove,	// At the moment the "Accept" icon is the same as the "Approve Peer"

	eMenuIconMax
	}; // EMenuAction

typedef BYTE EMenuActionByte;		// A menu action stored wihin a single byte (as long as we have less than ~250 menu actions)


//	Wrapper to the QMenu to add our own methods
class WMenu : public QMenu
{
public:
	WMenu();
	WMenu(PSZAC pszName);
	~WMenu();
	void ActionAdd(EMenuAction eMenuAction);
	void ActionAdd(EMenuAction eMenuAction, PSZAC pszText);
	void ActionAddWithSeparator(EMenuAction eMenuAction);
	void ActionsAdd(const EMenuActionByte rgzeMenuActions[]);
	void ActionsAddSubMenu(EMenuAction eMenuActionSubMenuName, const EMenuActionByte rgzeActionsSubMenuItems[]);
	void ActionAddFromText(PSZUC pszText, int idAction, EMenuAction eMenuIcon);
	void ActionSetCheck(EMenuAction eMenuAction, BOOL fuChecked);
	WMenu * PMenuAdd(PSZAC pszText, EMenuAction eMenuIcon);

	void InitAsDymanicMenu();
	void _ConnectActionsToMainWindow();
	QAction * _PExec();
	void DisplayContextMenuAndSendActionsToMainWindow();
	EMenuAction EDisplayContextMenu();

public:
	static BOOL s_fActionReservedForContextMenu;				// TRUE => Do not let the WMainWindow process the menu action because the action is reserved by the context menu and will be returned by EDisplayContextMenu(). FALSE => The action shall be processed by the WMainWindow.
	static QAction * s_pActionCurrentlyProcessedByMainWindow;	// Pointer to the action currently being processed by the WMainWindow
}; // WMenu

extern QMenuBar * g_pwMenuBar;
extern WMenu * g_pwMenuStatus;
extern WMenu * g_pwMenuCambrian;
extern WMenu * g_pwMenuContacts;
extern WMenu * g_pwMenuTools;
extern WMenu * g_pwMenuAdvanced;

void ConnectMenuActions(WMenu * pMenuSignal, QObject * pObjectSlots);

void Widget_SetIcon(INOUT QWidget * pwWidget, EMenuAction eMenuAction);
void Widget_SetIconButton(INOUT QAbstractButton * pwButton, EMenuAction eMenuAction);

QAction * PGetMenuAction(EMenuAction eMenuAction);
PSZAC PszGetMenuActionText(EMenuAction eMenuAction);

void DisplayDialogAccountNew(EMenuAction eMenuAction);

#endif // MENUACTIONS_H
