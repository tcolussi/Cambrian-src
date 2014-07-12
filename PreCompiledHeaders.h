///////////////////////////////////////////////////////////////////////////////////////////////////
//	Primary include file to build the application Cambrian
//
//	This file contains standard system include files and project include files that are used frequently, but are changed infrequently.
//

//	Some of the #define below are used by the setup/installer program
#define d_szApplicationName				"Cambrian"
#define d_szwApplicationName		   L"Cambrian"
#define d_szwApplicationNameSetup	   L"Cambrian Setup"	// Used for project CambrianSetup
#define d_szApplicationVersion			"0.0.8.6"
#define d_szwApplicationVersion		   L"0.0.8.6"
#define d_szUrlBaseDowloadInvitation	"http://download.cambrian.org/"	// Base URL to download the installation program (this field is used to create an invitation)
#define d_szXmppServerDefault			"xmpp.cambrian.org"	// Default server to create a new XMPP account

//#define DEBUG_IMPORT_OLD_CONFIG_XML

#ifndef PRECOMPILEDHEADERS_H
#define PRECOMPILEDHEADERS_H

//#define QT_NO_CAST_FROM_ASCII
#include <QtWidgets>
#include <QtNetwork>
#include <QTextBrowser>
#include <QWebView>
#include <QMainWindow>

//	Determine which platform / operating system Cambrian was compiled
#if defined(Q_OS_WIN)
	#define d_szOS		"Windows"
#elif defined (Q_OS_MAC)
	#define d_szOS		"Mac"
#elif defined(Q_OS_LINUX)
	#define d_szOS		"Linux"
#else
	#define d_szOS		"Other"
#endif

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

	eRTI_TBrowser,
	eRTI_TWallet,
	eRTI_TWalletView,
	eRTI_TRecommendations,
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

	eRTI_Max	// Must be last
	}; // RTI_ENUM

#define RTI(class_name)		(eRTI_##class_name)

#define RTI_IMPLEMENTATION(class_name)	\
		public:	\
		virtual RTI_ENUM EGetRuntimeClass() const { return eRTI_##class_name; }


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
				class TWallet;
				class TWalletView;
				class TRecommendations;
				class TTreeItemInbox;
				class TContactNew;

class CArrayPtrTreeItems;
	class CArrayPtrContacts;
	class CArrayPtrGroups;
	class CArrayPtrContactAliases;
		class CArrayPtrGroupMembers;

class CChatConfiguration;
class CInvitation;
class CInvitationFromUrlQueryString;

class WLayoutChatLog;
class DDialogAccountAdd;

class CSslCertificate;
class ISocketUI;
class CSocketCore;
	class CSocketXmpp;

class ITask;
	class CTaskSendText;
	class CTaskGroupSendText;
	class CTaskFileUpload;
	class CTaskFileDownload;
	class CTaskPing;

class IEvent;
	class CEventMessageXmlRawSent;
	class CEventMessageTextSent;
	class CEventMessageTextReceived;
	class CEventFileSent;
	class CEventFileReceived;
	class IEventWalletTransaction;
	class CEventPing;
	class CEventGroupMemberJoin;
	class CEventBallotSent;

class CVaultEvents;

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

#include "DataTypesAndMacros.h"
#include "ErrorCodes.h"
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
#include "IRuntimeObject.h"
#include "ErrorMessages.h"
#include "Xml.h"
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
#include "ITreeItemChatLogEvents.h"
#include "GroupChat.h"
#include "TContact.h"
#include "TGroup.h"
#include "TAccount.h"
#include "TProfile.h"
#include "Sockets.h"
#include "TCertificate.h"
#include "TWallet.h"
#include "TBrowser.h"

#include "CChatConfiguration.h"

#include "WLayoutChatLog.h"

#endif // PRECOMPILEDHEADERS_H
