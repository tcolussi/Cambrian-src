//	MenuIcons.h
#ifndef MENUICONS_H
#define MENUICONS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

enum EMenuIcon			// Must be in sync with c_mapepszIconResources[]
	{
	eMenuIcon_zNull = 0,

	// The following eMenuIcon_Presence* must be at the beginning of this enumeration because they are serialized
	eMenuIcon_PresenceAway,			// Your status automatically changes to away when your computer is idle for more than 20 minutes
	eMenuIcon_PresenceAwayExtended,
	eMenuIcon_PresenceBusy,
	eMenuIcon_PresenceInvisible,			// The account is connected, however invisible to others (others will see the contact as offline)
	eMenuIcon_PresenceAccountDisabled,	// The account has been disabled by the user and therefore will not attempt to connect unless explicitly enabled
	// The following presences are not serialized because they are dynamic and depends on the network condition
	eMenuIcon_PresenceAccountOnline,			// The account is ready to chat (connected to the server)
	eMenuIcon_PresenceAccountOffline,			// The account offline, however will become online next time the application starts, or the global status changes
	eMenuIcon_PresenceAccountConnecting,
	eMenuIcon_PresenceAccountDisconnected,		// The account is not connected to the server (probably there is no Internet connection or the server is down)
	eMenuIcon_PresenceLast = eMenuIcon_PresenceAccountDisconnected,
	eMenuIcon_AccountReconnect,

	eMenuIcon_Chat,
	eMenuIcon_MessageNew,			// There is a new message available to read
	eMenuIcon_Pencil_10x10,
	eMenuIcon_Pencil_16x16,

	eMenuIcon_ClassContactOnline,
	eMenuIcon_ClassContactOffline,	// Use a circle to draw a contact
	eMenuIcon_ClassGroup,			// Square
	eMenuIcon_ClassChannel,			// Hash (#)
	eMenuIcon_ClassOrganization,	// Triangle
	eMenuIcon_ClassJuristiction,	// Hexagon
	eMenuIcon_ClassProfile,			// Hat to display a role

	eMenuIcon_Overflow,				// Icon to draw the overflow icon in the dashboard / comm panel
	eMenuIcon_OverflowHovered,

	eMenuIcon_Failure,
	eMenuIcon_ShowLogErrors,
	eMenuIcon_ShowLogMessages,

	eMenuIcon_Certificate,
	eMenuIcon_ServerSecure,
	eMenuIcon_ServerWarning,
	eMenuIcon_ServerDeny,
	eMenuIcon_Question,
	eMenuIcon_Warning,

	eMenuIcon_AcceptOrApprove,		// Green checkmark
	eMenuIcon_Deny,

	eMenuIcon_Add,
	eMenuIcon_Remove,
	eMenuIcon_ContactAdd,
	eMenuIcon_Contact,
	eMenuIcon_Group,
	eMenuIcon_GroupAdd,

	eMenuIcon_RecycleBin,
	eMenuIcon_HashtagBlack,

	eMenuIcon_Copy,
	eMenuIcon_Find,
	eMenuIcon_MessageEnvelopeForward,
	eMenuIcon_FileUpload,
	eMenuIcon_Vote,
	eMenuIcon_Recommend,

	eMenuIcon_XMPP,
	eMenuIcon_Bitcoin,
	eMenuIcon_Browser,
	eMenuIcon_GoBack,
	eMenuIcon_GoForward,
    #ifdef COMPILE_WITH_OPEN_TRANSACTIONS
    eMenuIcon_contractOTserver, // eMenuIcon_contractOTserver
    #endif
	eMenuIcon_Toolbar_GoBack,
	eMenuIcon_Toolbar_GoForward,
	eMenuIcon_Toolbar_Reload,
	eMenuIcon_Toolbar_Find,
	eMenuIcon_Toolbar_SocietyPro,
	eMenuIcon_Toolbar_Pantheon,
	eMenuIcon_Toolbar_Grid,
	eMenuIcon_Toolbar_Friends,
	eMenuIcon_Toolbar_Messages,
	eMenuIcon_Toolbar_Notifications,
	eMenuIcon_Toolbar_Overflow,

	eMenuIcon_ToolbarTab_EdgeLeft,
	eMenuIcon_ToolbarTab_EdgeRight,
	eMenuIcon_ToolbarTab_EdgeLeftSelected,
	eMenuIcon_ToolbarTab_EdgeRightSelected,
	eMenuIcon_ToolbarTab_New,
	eMenuIcon_ToolbarTab_NewHover,
	eMenuIcon_ToolbarTab_Close,

	eMenuIcon_Close,
	eMenuIcon_Quit,
	eMenuIcon_Menu,

	eMenuIconMax	// Must be last
	};

const QIcon & OGetIcon(EMenuIcon eMenuIcon);


enum EMenuItems
	{
	eMenuItem_zNull = 0,
	eMenuItem_Enable,
	eMenuItem_Disable,
	eMenuItem_ContactRemove,

	eMenuItemMax	// Must be last
	};

#endif // MENUICONS_H
