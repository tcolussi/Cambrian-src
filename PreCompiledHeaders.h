///////////////////////////////////////////////////////////////////////////////////////////////////
//	Primary include file to build the application Cambrian
//
//	This file contains standard system include files and project include files that are used frequently, but are changed infrequently.
//

//	Some of the #define below are used by the setup/installer program
#define d_szApplicationName				"SocietyPro"
#define d_szwApplicationName		   L"SocietyPro"
#define d_szwApplicationNameSetup	   L"SocietyPro Setup"	// Used for project CambrianSetup
#define d_szApplicationVersion			"0.1.5.5"
#define d_szwApplicationVersion		   L"0.1.5.5"
#define d_szUrlBaseDowloadInvitation	"http://download.cambrian.org/"	// Base URL to download the installation program (this field is used to create an invitation)
#define d_szXmppServerDefault			"xmpp.cambrian.org"	// Default server to create a new XMPP account

//#define COMPILE_WITH_OPEN_TRANSACTIONS
//#define COMPILE_WITH_SPLASH_SCREEN
#define COMPILE_WITH_CHATLOG_HTML

//#define DEBUG_IMPORT_OLD_CONFIG_XML

#ifndef PRECOMPILEDHEADERS_H
#define PRECOMPILEDHEADERS_H

//#define QT_NO_CAST_FROM_ASCII
#include <QtWidgets>
#include <QtNetwork>
#include <QTextBrowser>
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QMainWindow>

//	Determine which platform / operating system Cambrian was compiled
#if defined(Q_OS_WIN)
	#define d_szOS		"Windows"
#elif defined (Q_OS_MAC)
	#define d_szOS		"Mac"
#elif defined(Q_OS_LINUX)
	#define d_szOS		"Linux"
#else
	#define d_szOS		"Unknown OS"
#endif

//	Define a handle of a Chat Log
#ifdef COMPILE_WITH_CHATLOG_HTML
	#define HChatLog		WChatLogHtml
#else
	#define HChatLog		WChatLog
#endif

#include "DataTypesAndMacros.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Runtime Information
//
//	Enumeration of all the runtime classes and interfaces used in the project.
//	An interface is a base class which will never have an instance because its sole
//	purpose is to be inherited by another class.
//
enum RTI_ENUM	// rti
	{
	eRTI_Nil = -1,		// Invalid object
	eRTI_Null = 0,		// Null/empty object

	eRTI_IOrphanOf,		// The object is an orphan of another object.  There is no class or interface for this value, however any class responding to this virtual interface shall return the pointer of the parent object.  This interface is used to automatically delete orphan objects when a parent node is remove from the Navigation Tree.

	eRTI_IRuntimeObject,
	eRTI_IXmlExchangeObjectID,

	eRTI_ITreeItem,
		eRTI_ITreeItemChatLogEvents,
	eRTI_TTreeItemDemo,
	eRTI_TAccountXmpp,
	eRTI_TAccountAlias,
	eRTI_TContact,
	eRTI_TContactNew,
	eRTI_IContactAlias,
	eRTI_TGroup,
	eRTI_TGroupMember,

	eRTI_ICertificate,
	eRTI_TCertificate,
	eRTI_TCertificateServerName,
	eRTI_TCertificates,
	eRTI_CTreeItemWelcome,
	eRTI_TTreeItemInbox,
	eRTI_TProfiles,
	eRTI_TProfile,
	eRTI_IApplication,
		eRTI_TApplicationMayanX,
		eRTI_TApplicationBallotmaster,

	eRTI_TBrowser,
	eRTI_TBrowserTab,
	eRTI_TBrowserTabs,
	eRTI_TWallet,
	eRTI_TWalletView,
	eRTI_TRecommendations,
	eRTI_TMyRecommendations,
	eRTI_TDisplayDeletedItems,
	eRTI_TMarketplace,
	eRTI_CMarketplaceFutures,
	eRTI_CMarketplaceArbitrators,
	eRTI_CExchangeCoffee,
	eRTI_CExchangeCoffeeTrade,

	eRTI_INotice,
	eRTI_INoticeWithLayout,
	eRTI_INoticeWithIcon,
	eRTI_WNoticeMessageSponsored,
	eRTI_WNoticeContactInvite,
	eRTI_WNoticeWarning,
	eRTI_WNoticeWarningCertificateNotAuthenticated,

	// Special enums which are serialized to disk.  The values should have at least 2 letters so they do not collide with the previous enum values.
	eRTI_szCServiceBallotmaster = _USZU3('B', 'A', 'L')

	}; // RTI_ENUM

#define RTI(class_name)				(eRTI_##class_name)
#define RTI_SZ(class_name)			(eRTI_sz##class_name)

#define RTI_IMPLEMENTATION(class_name)	\
		public:	\
		virtual RTI_ENUM EGetRuntimeClass() const { return eRTI_##class_name; }
#define RTI_IMPLEMENTATION_SZ(class_name)	\
		public:	\
		virtual RTI_ENUM EGetRuntimeClass() const { return eRTI_sz##class_name; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CString;
class CBin;
	class CStr;
class CArrayPsz;
class CXmlNode;
	class CXmlTree;
		class CXmlExchanger;

class WWidget;
class WLabel;
	class WLabelSelectable;
		class WLabelSelectableWrap;
	class WLabelIcon;
class WSpacer;
class WGroupBox;
class WButtonIconForToolbar;
class WButtonText;
	class WButtonTextWithIcon;

class WEditTextArea;
	class WEditTextAreaReadOnly;
class WEdit;
	class WEditReadOnly;
class WTextBrowser;
	class WChatLog;
class WChatLogHtml;

class WSplitter;
	class WSplitterVertical;
class OLayoutForm;
class OLayoutHorizontal;
	class OLayoutHorizontalAlignLeft;
class OLayoutVertical;
	class OLayoutVerticalAlignTop;
class WTable;
class OTableRow;
class CTreeItemW;

class INotice;
	class INoticeWithLayout;
		class WNoticeWarning;
		class INoticeWithIcon;
			class WNoticeMessageSponsored;

class IRuntimeObject;
	class IXmlExchange;
		class IXmlExchangeObjectID;
			class ITreeItem;
				class TTreeItemDemo;
				class ITreeItemChatLog;
					class TAccountCore;
						class TAccountXmpp;
					class ITreeItemChatLogEvents;
						class TContact;
						class TGroup;
				class IContactAlias;
					class TGroupMember;
				class TAccountAlias;
				class ICertificate;
					class TCertificate;
					class TCertificates;
				class TCertificateServerName;
				class TProfile;
				class IApplication;
					class TApplicationMayanX;
					class TApplicationBallotmaster;
				class TWallet;
				class TWalletView;
				class TRecommendations;
				class TMyRecommendations;
				class TTreeItemInbox;
				class TContactNew;
			class IService;
				class CServiceBallotmaster;

class CArrayPtrTreeItems;
	class CArrayPtrContacts;
	class CArrayPtrGroups;
	class CArrayPtrContactAliases;
		class CArrayPtrGroupMembers;

class CChatConfiguration;
class CInvitation;
class CInvitationFromUrlQueryString;

class CHashTableIdentifiersOfContacts;
class CHashTableIdentifiersOfGroups;

class WLayoutChatLog;
class DDialogAccountAdd;

class CSslCertificate;
class ISocketUI;
class CSocketCore;
	class CSocketXmpp;

class CTaskSendReceive;
/*
class ITask;
	class CTaskDownloader;
	class CTaskFileUpload;
	class CTaskFileDownload;
*/

class IEvent;
	class IEventMessageText;
		class CEventMessageXmlRawSent;
		class CEventMessageTextSent;
		class CEventMessageTextReceived;
	class IEventFile;
		class CEventFileSent;
		class CEventFileReceived;
	class IEventWalletTransaction;
		class CEventWalletTransactionSent;
		class CEventWalletTransactionReceived;
	class CEventPing;
	class CEventGroupMemberJoin;	
	class IEventBallot;
		class CEventBallotSent;
		class CEventBallotReceived;
        class CEventBallotPoll;
	class IEventUpdater;
		class CEventUpdaterSent;
		class CEventUpdaterReceived;

class CEventBallotAttatchment;

class CVaultEvents;

class CArrayPtrEventsRecent;

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiCambrian;
class OJapiProfile;
class OJapiContact;
class OJapiGroup;
class OJapiAppInfo;

#define POJapi				QObject *	// Every object exposed to the JavaScript engine must be stored as QVariant.  Fortunately QVariant accepts a QObject * however since QObject * is quite generic, we #define a new type of object to distinguish between those for JavaScript and those used by Qt, such as QWdiget. It makes the code easier to read.  BTW: Using a typedef does not work, as the compiler no longer recognizes the QObject *.
#define POCapi				QObject *

//	The following pointers are declared in this file because they will be used by TContact and TGroup.  The motivation for such a design is to have a mechanism to know which TContact and TGroup are used by JavaScript and consequently not delete them while in use, otherwise there will be a crash.
#define POJapiContact		POJapi
#define POJapiGroup			POJapi
#define POJapiProfile		POJapi
#define POJapiBrowserTab	POJapi
#define POJapiEvent			POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
extern const QBrush c_brushGreenSuperPale;
extern const QBrush c_brushGreenSuperSuperPale;
extern const QBrush c_brushFileTransfer;		// Display file transfers with the blue background color
extern const QBrush c_brushDebugPurple;			// Display debugging events with a light purple color

/*
#ifndef NDEBUG
	#define _DEBUG	// The GNU compiler automatically defines NDEBUG if this is not a debug build.
#endif
*/
#ifdef QT_DEBUG
	#define DEBUG
#endif
#define DEBUG_WANT_ASSERT	// We want Assert() in the release build

#include "ErrorCodes.h"
#include "Sort.h"
#include "StringsShared.h"
#include "StringUtilities.h"
#include "StringNumeric.h"
#include "DebugUtil.h"
#include "List.h"
#include "CBin.h"
#include "CStr.h"
#include "CArray.h"
#include "CMemoryAccumulator.h"
#include "CMemoryHeap.h"
#include "CHashTable.h"
#include "IRuntimeObject.h"
#include "ErrorMessages.h"
#include "Xml.h"
#include "MenuIcons.h"
#include "MenuActions.h"
#include "EnumerationMap.h"
#include "FilePathUtilities.h"
#include "QtUtilities.h"
#include "InternetWebServices.h"
#include "DDialog.h"
#include "WNotices.h"

#include "ITreeItem.h"
#include "WMainWindow.h"

#include "IEvent.h"
#include "SocketTasks.h"
#include "TBrowser.h"
#include "TBrowserTabs.h"
#include "CVaultEvents.h"
#include "ITreeItemChatLogEvents.h"
#include "GroupChat.h"
#include "TContact.h"
#include "TGroup.h"
#include "TChannel.h"
#include "TAccount.h"
#include "IApplication.h"
#include "TProfile.h"
#include "Sockets.h"
#include "TCertificate.h"
#include "TWallet.h"
#include "CChatConfiguration.h"

#include "WLayoutChatLog.h"

#include "ApiJavaScript.h"
#include "IEventBallot.h"

//	The word 'Profile' was renamed to 'Role'
#define d_sza_Profile	"Role"
#define d_sza_profile	"role"

#endif // PRECOMPILEDHEADERS_H
