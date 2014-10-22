//	Central repository for all icons, mostly used by menus.
//
//	Since the same icon is reused often, they have their own enum.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "MenuIcons.h"

//	Array mapping an eMenuIcon to its resource name.  Must be in sync with EMenuIcon
const PSZAC c_mapepszIconResources[eMenuIconMax] =
	{
	NULL,

	"StatusAway",					// eMenuIcon_PresencePresenceAway
	"StatusAwayExtended",	// eMenuIcon_PresencePresenceAwayExtended
	"StatusBusy",		// eMenuIcon_PresencePresenceBusy
	"StatusInvisible",		// eMenuIcon_PresencePresenceInvisible
	"NYI",						// eMenuIcon_PresenceAccountDisabled
	"StatusOnline",				// eMenuIcon_PresenceAccountOnline
	"StatusOffline",			// eMenuIcon_PresenceAccountOffline
	"StatusConnecting",		// eMenuIcon_PresenceAccountConnecting
	"StatusDisconnected",	// eMenuIcon_PresenceAccountDisconnected
	"ServerConnect",		// eMenuIcon_AccountReconnect
	"Chat",				// eMenuIcon_Chat
	"ChatMessage3",		// eMenuIcon_MessageNew
	"Pencil",			// eMenuIcon_Pencil_10x10
	"Pencil_16x16",		// eMenuIcon_Pencil_16x16

	"CCircleGreen",		// eMenuIcon_ClassContactOnline
	"CCircleGray",		// eMenuIcon_ClassContactOffline
	"CSquare",			// eMenuIcon_ClassGroup
	"CHash",			// eMenuIcon_ClassChannel
	"CTriangle",		// eMenuIcon_ClassOrganization
	"CHexagon",			// eMenuIcon_ClassJuristiction
    "Role",				// eMenuIcon_ClassProfile

	"MenuOverflow",		// eMenuIcon_Overflow
	"MenuOverflowH",	// eMenuIcon_OverflowHovered

	"Error",			// eMenuIcon_Failure
	"LogErrors",		// eMenuIcon_ShowLogErrors
	"LogMessages",		// eMenuIcon_ShowLogMessages

	"Certificate",		// eMenuIcon_Certificate
	"Server",			// eMenuIcon_ServerSecure
	"ServerWarning",	// eMenuIcon_ServerWarning
	"ServerDeny",		// eMenuIcon_ServerDeny
	"Question",			// eMenuIcon_Question
	"Warning",			// eMenuIcon_Warning
	"Accept",			// eMenuIcon_AcceptOrApprove
	"Deny",				// eMenuIcon_Deny

	"Add",				// eMenuIcon_Add
	"Remove",			// eMenuIcon_Remove
	"ContactAdd",		// eMenuIcon_ContactAdd
	"Contact",			// eMenuIcon_Contact
	"Group",			// eMenuIcon_Group
	"GroupAdd",			// eMenuIcon_GroupAdd

	"Deleted",			// eMenuIcon_RecycleBin
	"Hashtag",			// eMenuIcon_HashtagBlack

	"Copy",				// eMenuIcon_Copy
	"Find",				// eMenuIcon_Find
	"Forward",			// eMenuIcon_MessageEnvelopeForward (old: eMenuAction_ContactInvite)
	"FileUpload",		// eMenuIcon_FileUpload
	"Vote",				// eMenuIcon_Vote (old: eMenuAction_BallotSend)
	"Reputation",		// eMenuIcon_Recommend

	"XMPP",				// eMenuIcon_XMPP
	"Bitcoin",			// eMenuIcon_Bitcoin

	"Browser",			// eMenuIcon_Browser (old: eMenuAction_DisplaySecureWebBrowsing)
	"GoBack",			// eMenuIcon_GoBack
	"GoForward",		// eMenuIcon_GoForward
   #ifdef COMPILE_WITH_OPEN_TRANSACTIONS
    "contractOTserver", // eMenuIcon_contractOTserver
   #endif
    //"Close",			// eMenuIcon_Close
	":/images/ui/prevPageIcon.png", // eMenuIcon_Toolbar_GoBack
	":/images/ui/nextPageIcon.png", // eMenuIcon_Toolbar_GoForward
	":/images/ui/refreshPageIcon.png", // eMenuIcon_Toolbar_Reload
	":/images/ui/magIcon.png", // eMenuIcon_Toolbar_Find
	"SocietyPro", // eMenuIcon_Toolbar_SocietyPro
	"Pantheon", // eMenuIcon_Toolbar_Pantheon
	":/images/ui/applicationsIcon.png", // eMenuIcon_Toolbar_Grid
	":/images/ui/peersIcon.png", // eMenuIcon_Toolbar_Friends
	":/images/ui/messagesIcon.png", // eMenuIcon_Toolbar_Messages
	":/images/ui/notificationsIcon.png", // eMenuIcon_Toolbar_Notifications
	":/images/ui/moreIcon.png", // eMenuIcon_Toolbar_Overflow

	":/images/ui/tabLeftBorder.png", // eMenuIcon_ToolbarTab_EdgeLeft
	":/images/ui/tabRightBorder.png", // eMenuIcon_ToolbarTab_EdgeRight
	":/images/ui/currentTabLeftBorder.png", // eMenuIcon_ToolbarTab_EdgeLeftSelected
	":/images/ui/currentTabRightBorder.png", // eMenuIcon_ToolbarTab_EdgeRightSelected
	":/images/ui/addTabButtonNormal.png", // eMenuIcon_ToolbarTab_New
	":/images/ui/addTabButtonHovered.png", // eMenuIcon_ToolbarTab_NewHover
	":/images/ui/closeTabIcon.png", // eMenuIcon_ToolbarTab_Close

	"Close",			// eMenuIcon_Close
	"Quit",				// eMenuIcon_Quit
	"Menu"				// eMenuIcon_Menu
	};

//	Array of all icons
QIcon g_rgoIcons[eMenuIconMax];

const QIcon &
OGetIcon(EMenuIcon eMenuIcon)
	{
	Assert(eMenuIcon >= 0 && eMenuIcon < eMenuIconMax);
	QIcon * poIcon = g_rgoIcons + eMenuIcon;
	if (poIcon->isNull())
		{
		PSZAC pszResource = c_mapepszIconResources[eMenuIcon];
		if (pszResource != NULL && pszResource[0] != ':')
			*poIcon = QIcon(":/ico/" + QString(pszResource));
		else
			*poIcon = QIcon(QString(pszResource));
		Assert(!poIcon->isNull());
		#ifdef DEBUG
		if (poIcon->pixmap(QSize(16,16)).isNull())
			MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Unable to load eMenuIcon [$i] named '$s'\n", eMenuIcon, c_mapepszIconResources[eMenuIcon]);
		#endif
		}
	return *poIcon;
	}

#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
//	Flags for the uFlagsMenuItem
#define FMI_kmEMenuIcon				0x00FF	// Mask to extract the EMenuIcon for the menu item
#define FMI_kmEMenuInitFunction		0xFF00	// Mask Function to initialize the menu item


typedef void (* PFn_MenuInit)(WMenu * pwMenu, QAction * poAction, PVPARAM pvContext);

enum EMenuInitFunction
	{
	eMenuInitFunction_

	};

struct SMenuItem
	{
	PSZAC pszmText;	// Text for the menu item (including status text and accelerator)
	UINT uFlagsMenuItem;	// Various flags for the menu item (including its icon)
	};

SMenuItem g_rgMenuItems[eMenuItemMax];

void ActionAdd(EMenuItem eMenuItem)
	{
	WMenu oMenu;
	//oMenu.addAction
	QAction * oMenu.addAction();
	oMenu.removeAction();
	}
#endif

