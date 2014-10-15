///////////////////////////////////////////////////////////////////////////////////////////////////
//	WMainWindow.cpp
//
//	Implementation of the main application window.
//	The main application window is essentially divided in two widgets: WChatNavigation and WLayoutContainer.
//	Those widgets are dockable as the user may see several WLayoutContainer.
//
//	For debugging purpose, the WMainWindow includes a widget WChatDebugger to log errors (and warnings).
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WNavigationTree.h"
#include "WDashboard.h"
#include "WLayoutContainer.h"
#include "WQmlToolbar.h"
#include <QSound>
#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
	#include <iostream>
	#include <OTX_WRAP.h>
	OTX_WRAP * pOTX;
#endif

#ifdef DEBUG
	//#define DEBUG_DISABLE_TIMER		// Useful for debugging the code booting the application without displaying the connection notifications in the Message Log
#endif

QString g_sUrlPathHtmlApplications;		// URL where the HTML applications are located (typically this is a sub-folder where SocietyPro.exe is installed, however for developers, they may change the Registry value to another folder).  Since this value is used in a browser, it begins with either "file://" or "http://"
#ifdef Q_OS_WIN
HWND g_hwndMainWindow;							// Win-32 handle of the main window.  This handle is used by the MessageLog.
#endif
WMainWindow * g_pwMainWindow;				// Pointer to the widget of the main window
WNavigationTree * g_pwNavigationTree;		// Navigation tree (displayed on the left side of the window - unless the user changes the position)
WLayoutContainer * g_pwChatLayoutContainer;	// Container to display a widget according to what item is selected in the navigation tree.
QStatusBar * g_pwStatusBar;
QSystemTrayIcon * g_poSystemTrayIcon;	// Object displaying the icon in the System Tray (under Windows, this icon is typically at the bottom right of the screen).
int g_cMessagesSystemTray;				// Number of messages in the System Tray (this count is useful to determine when it is appropriate to destroy and re-create the System Tray)
WMenu * g_pMenuSystemTray;					// Context menu for the System Tray
QIcon * g_pIconCambrian;
QIcon * g_pIconNewMessage;

#define d_ttiReconnectFirst		200		// The first timer interval is very short so the application may establish a socket connection quickly
#define d_ttiReconnectMinute	60000	// Attempt to reconnect every minute
// Seconds: 136 years
TIMESTAMP_MINUTES g_tsmMinutesSinceApplicationStarted;	// Total number of minutes since the application started (this counter keeps increasing until overflow... every 8171 years)
TIMESTAMP_MINUTES g_tsmTimerQueueNextCallbackToTrigger = d_tsm_cMinutesMax;	// Which minute it is time to trigger the next callback in the queue
TIMESTAMP_MINUTES g_cMinutesIdleKeyboardOrMouse;		// Total number of minutes the user has been idle, from the keyboard or mouse.  This variable is used to send a <presence> to indicate the user is either 'away' or 'extended away'
TIMESTAMP_MINUTES g_cMinutesIdleNetworkDataReceived;	// Total number of minutes the network has been idle (this is the number of minutes since the last network packet was received)
enum EIdleState
	{
	eIdleState_zActive,
	eIdleState_Away,
	eIdleState_AwayExtended,
	};
EIdleState g_eIdleState;

//	The QNetworkConfigurationManager works on some computers, so more testing needs to be done.
bool g_fIsConnectedToInternet = true;	// true => The device is connected to the Internet.  Since SL_NetworkOnlineStateChanged() is unreliable, assume the machine is connected to the Internet.
QNetworkConfigurationManager * g_poNetworkConfigurationManager;
void
WMainWindow::SL_NetworkOnlineStateChanged(bool fIsOnline)
	{
	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlue), "[$@] SL_NetworkOnlineStateChanged() $s\n", fIsOnline ? "online" : "offline");
	g_fIsConnectedToInternet = fIsOnline;
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
		if (fIsOnline)
			pAccount->Socket_ReconnectIfDisconnected();
		pAccount->IconUpdate_NetworkOnlineStateChanged();
		}
	}

void
MainWindow_SetIdleState(EIdleState eIdleState)
	{
	Assert(eIdleState != g_eIdleState);
	MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlack), "[$@] MainWindow_SetIdleState() changing idle state from $i to $i\n", g_eIdleState, eIdleState);
	g_eIdleState = eIdleState;

	// The quickest way to broadcast an idle message is to temporary change the global variable and restore it later
	const UINT uPreferences = g_uPreferences;
	EMenuAction eMenuAction = (EMenuAction)(uPreferences & P_kmPresenceMask);
	if (eMenuAction > eMenuAction_PresenceAwayExtended)
		return;	// Don't touch those states
	switch (eIdleState)
		{
	case eIdleState_zActive:
		eMenuAction = eMenuAction_PresenceAccountOnline;
		break;
	case eIdleState_Away:
		eMenuAction = eMenuAction_PresenceAway;
		break;
	case eIdleState_AwayExtended:
		eMenuAction = eMenuAction_PresenceAwayExtended;
		break;
		}
	Configuration_GlobalSettingsPresenceUpdate(eMenuAction);
	g_uPreferences = uPreferences;
	}

QString
MainWindow_SGetUrlPathOfApplication(const QString & sPathRelativeApplication)
	{
	return g_sUrlPathHtmlApplications + sPathRelativeApplication;
	}

void
MainWindow_SetCurrentLayout(IN WLayout * pwLayout)
	{
	Assert(g_pwChatLayoutContainer != NULL);
	Endorse(pwLayout == NULL);
	g_pwChatLayoutContainer->SetLayout(IN pwLayout);
	}

void
MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD WLayout * pawLayoutAutoDelete)
	{
	Assert(g_pwChatLayoutContainer != NULL);
	g_pwChatLayoutContainer->SetLayoutAutoDelete(PA_CHILD pawLayoutAutoDelete);
	}

//	Function to safely delete any layout.  If the deleting layout is currently selected, then select another layout.
void
MainWindow_DeleteLayout(PA_DELETING WLayout * pawLayoutToDelete)
	{
	if (g_pwChatLayoutContainer != NULL)
		g_pwChatLayoutContainer->DeleteLayout(PA_DELETING pawLayoutToDelete);
	}

WLayout *
MainWindow_PwGetCurrentLayout_NZ()
	{
	Assert(g_pwChatLayoutContainer != NULL);
	return g_pwChatLayoutContainer->PwGetLayout_NZ();
	}

void
MainWindow_DisplayNoticeWithinCurrentLayout(IN INotice * piwNotice)
	{
	MainWindow_PwGetCurrentLayout_NZ()->Layout_NoticeDisplay(IN piwNotice);
	}

void
MainWindow_SetFocusToCurrentLayout()
	{
	MainWindow_PwGetCurrentLayout_NZ()->Layout_SetFocus();
	}

const EMenuActionByte c_rgzeActionsMenuSystemTray[] =
	{
	eMenuAction_PresenceAccountOnline,
	eMenuAction_PresenceAway,
	eMenuAction_PresenceBusy,
//	eMenuAction_PresenceInvisible,
	eMenuAction_PresenceAccountOffline,
	eMenuActionSeparator,
	eMenuAction_Quit,
	ezMenuActionNone
	};

//	Ideally, the QSystemTrayIcon should be created within the constructor of WMainWindow, however this implementation does not work.
//	The implementation of QSystemTrayIcon is rather buggy, and often displays messages notifications hours after having been received.
//	A workaround to force the an immediate display of the message, QCoreApplication::processEvents(), causes Cambrian to freeze when displaying a message during its startup.
//	The drastic solution is therefore to destroy the object QSystemTrayIcon after the user read messages, to make sure a balloon message does not show up hours later to "notify" the user a new message arrived.
//	It follows of course, then any destroyed QSystemTrayIcon must be re-created later to display a notification message.
void
SystemTray_Create()
	{
	Assert(g_pwMainWindow != NULL);
	Assert(g_pMenuSystemTray != NULL);
	Assert(g_pIconCambrian != NULL);
	g_poSystemTrayIcon = new QSystemTrayIcon(g_pwMainWindow);
	g_poSystemTrayIcon->setIcon(*g_pIconCambrian);
	g_poSystemTrayIcon->setContextMenu(g_pMenuSystemTray);
	g_poSystemTrayIcon->show();
	QObject::connect(g_poSystemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), g_pwMainWindow, SLOT(SL_SystemTrayActivated(QSystemTrayIcon::ActivationReason)));
	QObject::connect(g_poSystemTrayIcon, SIGNAL(messageClicked()), g_pwMainWindow, SLOT(SL_SystemTrayMessageClicked()));
	}

void
SystemTray_Destroy()
	{
	Assert(g_poSystemTrayIcon != NULL);
	if (g_cMessagesSystemTray > 0)
		{
		g_cMessagesSystemTray = 0;
		delete g_poSystemTrayIcon;
		SystemTray_Create();
		}
	}

//	Special class for a drop-down menu in the main menu bar.
//	This class is necessary for Mac because the OS does not display an empty menu, so the
//	constructor will add an dummy menu item.
class WMenuDropdown : public WMenu
{
public:
	WMenuDropdown(PSZAC pszName);
};

WMenuDropdown::WMenuDropdown(PSZAC pszName) : WMenu(pszName)
	{
	#ifdef Q_OS_MAC
	//ActionAdd(eMenuAction_Quit);	// This does not work
	addAction(c_sEmpty);
	#endif
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WMainWindow::WMainWindow() : QMainWindow()
	{
	#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
	//Define the OTX pointer to access to all objects
	OTX_WRAP * otw=new OTX_WRAP(this); // <-- Here the wallet is opened and if does not exists create a new wallet given a passphrase
	pOTX=otw;
	#endif

	g_pwMainWindow = this;
	#if defined(Q_OS_WIN)
	g_hwndMainWindow = (HWND)winId();
	#endif
	g_pwMenuBar = new QMenuBar;
	g_pwStatusBar = new QStatusBar;
	//g_pwStatusBar->setStyleSheet("border: 1px solid red;");
	m_cTimerEvents = 0;
	m_tidFlashIconNewMessage = d_zNA;

	setWindowTitle(d_szApplicationName " v" d_szApplicationVersion);
//	setCentralWidget(new QWidget);

	QRect rc = QApplication::desktop()->screenGeometry();
	const int cx = rc.width() / 8;
	const int cy = rc.height() / 8;
	rc.adjust(cx, cy, -cx, -cy);
	setGeometry(rc);
	MessageLog_ModuleInitialize();	// Initialize/create the MessageLog/ErrorLog as early as possible in case there is an error/assert during the creation of the other widgets

	g_pwMenuCambrian = new WMenuDropdown("SocietyPro");
	g_pwMenuContacts = new WMenuDropdown("Peers");
	g_pwMenuTools = new WMenuDropdown("Applications");
	#if 0
	g_pwMenuAdvanced = new WMenu("test");
	//g_pwMenuAdvanced->setTitle("test");
	g_pwMenuAdvanced->setIcon(PGetMenuAction(eMenuAction_FindText)->icon());
	g_pwMenuBar->setCornerWidget(g_pwMenuAdvanced);
	#else
	#ifdef Q_OS_MAC
	g_pwMenuAdvanced = new WMenuDropdown("Advanced");	// This is necessary because on Mac OSX the 'hamburger menu' does not show up unless there is text
	#else
	g_pwMenuAdvanced = new WMenuDropdown(NULL);
	#endif
	g_pwMenuAdvanced->InitAsDymanicMenu();
	WButtonIconForToolbar * pwButtonTest = new WButtonIconForToolbar(eMenuIcon_Menu);
	pwButtonTest->setStyleSheet("QToolButton { border: none;  padding-top:3px; padding-right:5px; padding-bottom:3px } QToolButton::menu-indicator { image: none; }");
	pwButtonTest->setPopupMode(QToolButton::InstantPopup);
	pwButtonTest->setMenu(g_pwMenuAdvanced);

	//pwButtonTest->setMenu(g_pwMenuAdvanced);
	g_pwMenuBar->setCornerWidget(pwButtonTest);
	#endif
	setMenuBar(PA_CHILD g_pwMenuBar);
	setStatusBar(PA_CHILD g_pwStatusBar);
	void Menu_InitializeAllMenuActionsHavingAccelerators();
	Menu_InitializeAllMenuActionsHavingAccelerators();

	addDockWidget(Qt::LeftDockWidgetArea, PA_CHILD new WNavigationTree);
	#if 1
	addDockWidget(Qt::RightDockWidgetArea, PA_CHILD new WDashboard);
	#endif
	#if 1
	addDockWidget(Qt::TopDockWidgetArea, PA_CHILD new WQmlToolbar);
	#endif

	g_pwChatLayoutContainer = new WLayoutContainer;
	setCentralWidget(PA_CHILD g_pwChatLayoutContainer);
	g_pIconCambrian = new QIcon(":/ico/IconHaven");
	Assert(!g_pIconCambrian->pixmap(QSize(16,16)).isNull() && "Unable to load icon");	// By the way, the QIcon is unable to load a .ico from the resource, so we have to create a .png of 16x16
//	setWindowIcon(*g_pIconCambrian);	// This line is no longer necessary because the file Cambrian.pro contains the keyword RC_ICONS

	g_pMenuSystemTray = new WMenu;
	g_pMenuSystemTray->ActionsAdd(c_rgzeActionsMenuSystemTray);
	Assert(g_poSystemTrayIcon == NULL);
	SystemTray_Create();
	Assert(g_poSystemTrayIcon != NULL);

	g_poNetworkConfigurationManager = new QNetworkConfigurationManager(this);
	if (!g_poNetworkConfigurationManager->isOnline())
		MessageLog_AppendTextFormatSev(eSeverityWarning, "Your computer appears as offline according to QNetworkConfigurationManager.\n");
	connect(g_poNetworkConfigurationManager, SIGNAL(onlineStateChanged(bool)), this, SLOT(SL_NetworkOnlineStateChanged(bool)));

	// We are done creating the main window, so restore (load) the settings from the Registry.
	SettingsRestore();

	m_ttiReconnect = d_ttiReconnectFirst;
	#ifndef DEBUG_DISABLE_TIMER
	m_tidReconnect = startTimer(d_ttiReconnectFirst);
	#else
	m_tidReconnect = d_zNA;
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "INFO: Timer disabled for WMainWindow.  Enable it for automatic connection to XMPP accounts...\n");	// This is useful for debugging and not having to wait for the XMPP account(s) to connect, however it is an imporant reminder for the release build.
	#endif
	#if 0
	CBin bin;
	bin.BinFileReadE("C:\\CambrianSetup\\Release\\CambrianSetup.exe");
	SHashSha1 hash;
	HashSha1_CalculateFromCBin(OUT &hash, bin);
	MessageLog_AppendTextFormatCo(d_coBlack, "Hash of $I bytes: {hf}\n", bin.CbGetData(), &hash);
	#endif
//	MessageLog_AppendTextFormatCo(d_coBlack, "Sizeof(QFile) is $I bytes\n", sizeof(QFile));
	/*
	QDateTime dt;
	dt.setMSecsSinceEpoch(0xFFFFFFFF);
	QString s = dt.toString();
	MessageLog_AppendTextFormatCo(d_coBlack, "$t = $Q\n", dt.currentMSecsSinceEpoch(), &s);	// 3TSmc9t = Thu Feb 19 11:02:47 1970
	*/
	}

WMainWindow::~WMainWindow()
	{
	//EMessageBoxInformation("WMainWindow::~WMainWindow()");

	// We are destroying the WMainWindow, so flush everything (both for performance, and also to prevent the application to crash)
	g_listaNoticesRoaming.Notices_DeleteAll();
	g_listaNoticesAuxiliary.Notices_DeleteAll();
	MainWindow_SetCurrentLayout(NULL);	// Make sure there is no layout related to a selected Tree Item.  This is important before destroying the configuration object, otherwise there may be a dangling pointer
	g_pwChatLayoutContainer = NULL;		// Make sure the Navigation Tree does not call TreeItem_GotFocus() when receiving a signal that a new Tree Item was selected.
	g_pwNavigationTree->NavigationTree_TreeItemUnselect();
	g_oConfiguration.Destroy();	// Destroy the configuration, with the accounts, contacts and groups.
	MessageLog_ModuleShutdown();
	#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
    delete pOTX;  // Destroy pointer to OTX
	#endif
	}


// Save anything worth saving before the application quits. This method is called before the destructor of the Main Window and the Navigation Tree.
void
WMainWindow::SL_Quitting()
	{
	MessageLog_AppendTextFormatSev(eSeverityComment, "WMainWindow::SL_Quitting()\n");
	//EMessageBoxInformation("WMainWindow::SL_Quitting()");
	Configuration_Save();	// Before destroy the main window, save all open configurations
	SettingsSave();			// Also remember the settings (window position)
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	SL_TreeItemSelectionChanged(), slot of signal QTreeWidget::currentItemChanged()
//
//	The user selected a new Tree Item.
//	This is time to load and display the chat layout corresponding to the selected Tree Item.
void
WNavigationTree::SL_TreeItemSelectionChanged(QTreeWidgetItem * pItemCurrent, QTreeWidgetItem * UNUSED_PARAMETER(pItemPrevious))
	{
//	MessageLog_AppendTextFormatSev(eSeverityNoise, "SL_TreeItemSelectionChanged() - pwItemCurrent=0x$p, pwItemPrevious=0x$p\n", pwItemCurrent, pwItemPrevious);
//	Assert(pwItemCurrent != NULL);	// This may be NULL if there are no more
	UNUSED_PARAMETER(pItemPrevious);
	if (pItemCurrent == NULL)
		return;	// Just in case
	ITreeItem * pTreeItem = ((CTreeItemW *)pItemCurrent)->m_piTreeItem;
	if (pTreeItem == NULL)
		{
		MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "SL_TreeItemSelectionChanged() - No associated ITreeItem with selected item from the Navigation Tree!\n");
		return;	// In the future, this should not happen, but in the meantime, it is better than having a crash
		}
	Assert(pTreeItem != NULL);
	Assert(PGetRuntimeInterfaceOf_ITreeItem(pTreeItem) == pTreeItem);
	if (g_pwChatLayoutContainer != NULL)
		{
		pTreeItem->TreeItem_GotFocus();		// Call the virtual  method which will take care of calling MainWindow_SetCurrentChatLayout() with the proper WLayout
		NoticeListRoaming_TreeItemGotFocus(IN pTreeItem);
		void Dashboard_RemoveSelection();
		//Dashboard_RemoveSelection();
		}
	else
		{
		//TRACE1("WNavigationTree::SL_TreeItemSelectionChanged() - Ignoring the selection of $S because application is shutting down...", &pTreeItem->m_strNameDisplay);
		}
	} // SL_TreeItemSelectionChanged()

//	WMainWindow::QObject::event()
//
//	Filter the global events of Cambrian.  This virtual method is mostly used to detect when the user is no longer idle.
bool
WMainWindow::event(QEvent * pEvent)
	{
	const QEvent::Type eEvent = pEvent->type();
//	TRACE1("WMainWindow::event($i)\n", eEvent);
//	MessageLog_AppendTextFormatCo(d_coRed, "[$@] event($i)\n", eEvent);
	switch (eEvent)
		{
	case QEvent::Close:	// The close button was clicked.  This is NOT the same a quitting the application, as the user may click on the Quit menu item to exit.
		MessageLog_AppendTextFormatSev(eSeverityComment, "WMainWindow::event(Close)\n");
		//EMessageBoxInformation("QEvent::Close");
		#ifdef DEBUG
		QCoreApplication::exit(0);	// This line is necessary so the QWebInspector is closed when the application closes
		break;		// When debugging, close Cambrian
		#endif
		hide();	// Hide Cambrian from the System Tray (only the icon in the System Tray will remain)
		//showMinimized();	// Minimize Cambrian to the System Tray
		pEvent->ignore();	// This line is important, otherwise the Close event will be processed by the calling method, regardless if true is returned.
		return true;	// The event was processed
	case QEvent::Quit:
		TRACE0("WMainWindow::event(Quit)\n");
		EMessageBoxInformation("WMainWindow::event(Quit) - If you see this message, please contact Dan.  I am doing a test to see if this event is ever used!");
		break;
	case QEvent::ActivationChange:
		// 2014-04-09: I don't think setIcon() is necessary, and I think the killTimer() should be moved with the destruction of g_poSystemTrayIcon
		g_poSystemTrayIcon->setIcon(*g_pIconCambrian);	// Restore the default Cambrian icon
		if (m_tidFlashIconNewMessage != d_zNA)
			{
			killTimer(m_tidFlashIconNewMessage);
			m_tidFlashIconNewMessage = d_zNA;
			}
		break;
	case QEvent::HoverMove:	// (129) The user moved the mouse
	case QEvent::Wheel:	// (31) The user moved the mouse wheel
		goto NoMoreIdle;
	default:
		if (eEvent >= QEvent::MouseButtonPress && eEvent <= QEvent::Leave)
			{
			NoMoreIdle:
			if (g_eIdleState != eIdleState_zActive)
				MainWindow_SetIdleState(eIdleState_zActive);
			}
		} // switch
	return QMainWindow::event(pEvent);
	} // event()

const QString c_sPathConfig("PathConfig");		extern QString g_sPathConfiguration;
const QString c_sWindowMain("WindowMain");
const QString c_sGeometry("Geometry");
const QString c_sGeometryState("State");

void
WMainWindow::SettingsSave()
	{
	OSettingsRegistry oSettings;
	oSettings.beginGroup(c_sWindowMain);
	oSettings.setValue(c_sGeometry, saveGeometry());
	oSettings.setValue(c_sGeometryState, saveState());
	oSettings.endGroup();
	oSettings.sync();
	}

void
TimerEventCallback(PVPARAM pvParam)
	{
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "TimerEventCallback($I)\n", pvParam);
	}

void
WMainWindow::SettingsRestore()
	{
	OSettingsRegistry oSettings;
	/*
	g_sPathConfiguration = oSettings.value(c_sPathConfig).toString();	// Attempt to read the configuration path from the Registry
	if (g_sPathConfiguration.isEmpty())
		g_sPathConfiguration = QCoreApplication::applicationDirPath();
	*/
	oSettings.beginGroup(c_sWindowMain);
	restoreGeometry(oSettings.value(c_sGeometry, saveGeometry()).toByteArray());
	restoreState(oSettings.value(c_sGeometryState, saveState()).toByteArray());
	oSettings.endGroup();
	if (!g_pwNavigationTree->isVisibleTo(this))
		{
		MessageLog_AppendTextFormatSev(eSeverityWarning, "The Navigation Tree is not visible, so restoring its position!\n");
		g_pwNavigationTree->setFloating(false);		// Dock the Navigation Tree within the main window to ensure it is visible to the user.
		}

	void ApiWebSocket_Init(UINT uPort);
	ApiWebSocket_Init(oSettings.value("Port").toUInt());

	g_sUrlPathHtmlApplications = oSettings.value("Apps").toString();
	if (g_sUrlPathHtmlApplications.isEmpty())
		g_sUrlPathHtmlApplications = "file:///" + QCoreApplication::applicationDirPath() + "/Apps/";
	else if (!g_sUrlPathHtmlApplications.startsWith("file:", Qt::CaseInsensitive))
		g_sUrlPathHtmlApplications = QUrl::fromLocalFile(g_sUrlPathHtmlApplications).toString();	// Make sure the URL begins with "file://"
	//MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "$Q\n", &g_sPathHtmlApplications);

	#if 0
	TimerQueue_CallbackPostponeOrAdd(5*60, TimerEventCallback, (PVPARAM)1);
	TimerQueue_CallbackPostponeOrAdd(7*60, TimerEventCallback, (PVPARAM)2);
	TimerQueue_CallbackPostponeOrAdd(6*60, TimerEventCallback, 0);
	TimerQueue_CallbackPostponeOrAdd(2*60, TimerEventCallback, 0);
	TimerQueue_CallbackPostponeOrAdd(8*60, TimerEventCallback, 0);
	/*
	TimerQueue_CallbackPostponeOrAdd(2*60, TimerEventCallback, 0);
	TimerQueue_CallbackPostponeOrAdd(1*60, TimerEventCallback, 0);
	*/
	TimerQueue_DisplayToMessageLog();
	#endif

	#ifdef DEBUG
	return;		// Don't save the path if running a debug build
	#endif
	#ifdef Q_OS_WIN
	// Make sure Cambrian starts next time Windows restart
	QString sPathCambrianExe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
	HKEY haKey = NULL;
	DWORD dwDisposition;
	::RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_QUERY_VALUE, NULL, OUT &haKey, OUT IGNORED &dwDisposition);
	Report(haKey != NULL);
	::RegSetValueEx(haKey, d_szwApplicationName, d_zNA, REG_SZ, IN (const BYTE *)sPathCambrianExe.utf16(), sPathCambrianExe.size() * sizeof(CHW));
	(void)::RegCloseKey(PA_DELETING haKey);
	#endif // Q_OS_WIN
	} // SettingsRestore()

CInvitation * Invitation_PazAllocateFromText(const CStr & strInvitationText);
void Invitation_NoticeDisplay(PA_CHILD CInvitation * pazInvitation);

QString
SGetFullPathInvitation()
	{
	return QCoreApplication::applicationDirPath() + "/Invitation.txt";	// The invitation is stored in the folder where the program is installed
	}

void
Invitation_EraseFile()
	{
	const QString sPathInvitation = SGetFullPathInvitation();
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Erasing invitation file $Q\n", &sPathInvitation);
//	return;
	QFile::setPermissions(sPathInvitation, QFileDevice::WriteUser);
	QDir dir;
	dir.remove(sPathInvitation);
	}

CInvitationFromUrlQueryString * g_paInvitation;

void
WMainWindow::ConfigurationLoadFromXml()
	{
	g_oConfiguration.XmlConfigurationLoadFromFile();
	// After loading the configuration, check if there is an invitation stored at the time of installation
	const QString sPathInvitation = SGetFullPathInvitation();
	CStr strInvitation;
	if (strInvitation.FileReadE(sPathInvitation) == errSuccess)
		{
		g_paInvitation = Invitation_PazAllocateFromUrlQueryString(strInvitation);
		/*
		// We have a pending invitation, so display it to the user
		CInvitation * paInvitation = Invitation_PazAllocateFromText(strInvitation);
		if (paInvitation != NULL)
			Invitation_NoticeDisplay(PA_DELETING paInvitation);
		else
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "The invitation file $Q is corrupted... deleting it!\n", &sPathInvitation);
			Invitation_EraseFile();
			}
		*/
		}
	EMenuAction eMenuAction_Presence = (EMenuAction)(g_uPreferences & P_kmPresenceMask);
	NavigationTree_UpdatePresenceIcon((eMenuAction_Presence == ezMenuActionNone) ? eMenuAction_PresenceAccountOnline : eMenuAction_Presence);
	}


CArrayPtrContacts g_arraypContactsRecentMessages;

//	SL_SystemTrayMessageClicked(), slot of signal QSystemTrayIcon::messageClicked()
void
WMainWindow::SL_SystemTrayMessageClicked()
	{
	showNormal();
	activateWindow();
	raise();
	TContact * pContact = (TContact *)g_arraypContactsRecentMessages.PvGetElementFirst_YZ();
	if (pContact != NULL)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "SL_SystemTrayMessageClicked() - Selecting messages for $S...\n", &pContact->m_strJidBare);
		NavigationTree_SelectTreeItem(IN pContact);
		pContact->ChatLog_ScrollToLastMessage();	// Make sure the last message in the history is visible to the user
		pContact->TreeItem_IconUpdateOnMessagesRead();	// Since the contact is selected, assume the user read all his/her messages
		/*
		g_poSystemTrayIcon->deleteLater();	// Since there was a balloon message stored within the System Tray Icon, the safest way to ensure the message is no longer present is to destroy the entire System Tray Icon.
		SystemTray_Create();				// It follows we need to recreate the System Tray Icon
		*/
		}
	}

#pragma GCC diagnostic ignored "-Wswitch"
//	SL_SystemTrayActivated(), slot of signal QSystemTrayIcon::activated()
void
WMainWindow::SL_SystemTrayActivated(QSystemTrayIcon::ActivationReason eActivationReason)
	{
	switch (eActivationReason)
		{
	case QSystemTrayIcon::MiddleClick:
		//g_poSystemTrayIcon->setToolTip("tool tip");
		break;
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		SL_SystemTrayMessageClicked();
		break;
		} // switch
	} // SL_SystemTrayActivated()
#pragma GCC diagnostic warning "-Wswitch"

void
WMainWindow::TimerStartFlashIconNewMessage()
	{
	if (m_tidFlashIconNewMessage == d_zNA)
		{
		m_tidFlashIconNewMessage = startTimer(1000);
		m_fuAlternateDisplayIconNewMessage = FALSE;
		}
	}

#ifdef Q_OS_WIN
DWORD g_dwTickLast = ::GetTickCount();
UINT g_cTimerFailures;
#endif

//	WMainWindow::QObject::timerEvent()
void
WMainWindow::timerEvent(QTimerEvent * pTimerCallback)
	{
//	MessageLog_AppendTextFormatCo(d_coRed, "WMainWindow::timerEvent($i)\n", pTimerCallback->timerId());
	const int tidEvent = pTimerCallback->timerId();
	if (tidEvent ==  m_tidReconnect)
		{
		if (g_oMutex.tryLock())
			{
			if (m_ttiReconnect == d_ttiReconnectFirst)
				{
				m_ttiReconnect = d_ttiReconnectMinute;
				killTimer(m_tidReconnect);
				m_tidReconnect = startTimer(d_ttiReconnectMinute, Qt::VeryCoarseTimer);	// Set a longer timer to not consume too much CPU resources
				}
			else
				{
				#ifdef Q_OS_WIN
				DWORD dwTickPrevious = g_dwTickLast;
				g_dwTickLast = ::GetTickCount();
				int nInterval = g_dwTickLast - dwTickPrevious;
				if (nInterval <= 30000)
					{
					// We received a tick count within 30 seconds or less
					g_cTimerFailures++;
					MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "[$@] Timer failure #$I!  (interval of $I miliseconds)\n", g_cTimerFailures, nInterval);
					}
				#endif
				}

			//MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "g_tsmMinutesSinceApplicationStarted = $I, g_tsmTimerQueueNextCallbackToTrigger = $I\n", g_tsmMinutesSinceApplicationStarted + 1, g_tsmTimerQueueNextCallbackToTrigger);
			if (++g_tsmMinutesSinceApplicationStarted >= g_tsmTimerQueueNextCallbackToTrigger)
				{
				// We have at least one event in the queue
				TimerQueue_ExecuteExpiredCallbacks();
				}
			// Periodically attempt to reconnect
			Configuration_NetworkReconnectIfDisconnected();
			if ((g_tsmMinutesSinceApplicationStarted & 0x0F) == 0)
				{
				// Every 15 minutes, do some processing
				//TRACE1("Autosave begins at timestamp $i", g_tsmMinutesSinceApplicationStarted);
				Configuration_Save();	// Save the configuration(s) /w chat history.  This function will not write anything to the disk if nothing was changed since the last save.
				//TRACE0("Autosave ends");

				// Free the buffers in case they have allocated large blocks of memory which are no longer needed
				g_strScratchBufferStatusBar.FreeBuffer();
				g_strScratchBufferSocket.FreeBuffer();
				}
			#if 0
			if ((++g_cMinutesIdleNetworkDataReceived & 0x03) == 0)
			#else
			if (++g_cMinutesIdleNetworkDataReceived > 0)		// This line is for debugging by forcing an 'idle' every timer tick.  This helps to test the code paths which may rarely be taken
			#endif
				{
				// Every 4 minutes, ping each socket to make sure the connection is alive.
				Configuration_OnTimerNetworkIdle();	// Send a 'ping' to the server.  This is workaround because the socket(s) get disconnected without any notification.  I have the feeling this 'temporary' workaround will stay there for many decades.
				}
			if (++g_cMinutesIdleKeyboardOrMouse > 20 || g_eIdleState != eIdleState_zActive)
				{
				// The user has been more than 20 minutes idle or the user is already idle
				EIdleState eIdleState = eIdleState_zActive;
				g_cMinutesIdleKeyboardOrMouse = g_pfnUGetIdleTimeInMinutes();	// Calculate how long the user has really been idle
				if (g_cMinutesIdleKeyboardOrMouse > 20)
					eIdleState = (g_cMinutesIdleKeyboardOrMouse < 4*60) ? eIdleState_Away : eIdleState_AwayExtended;	// Extended away is after 4 hours of inactivity
				if (eIdleState != g_eIdleState)
					MainWindow_SetIdleState(eIdleState);
				}
			g_oMutex.unlock();
			}
		else
			{
			MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "WMainWindow::timerEvent() - g_oMutex is locked!\n");
			}
		}
	else if (tidEvent == m_tidFlashIconNewMessage)
		{
		m_fuAlternateDisplayIconNewMessage = ~m_fuAlternateDisplayIconNewMessage;
		g_poSystemTrayIcon->setIcon(m_fuAlternateDisplayIconNewMessage ? *g_pIconNewMessage : *g_pIconCambrian);
		}
	QMainWindow::timerEvent(pTimerCallback);
	} // timerEvent()

TIMESTAMP_MINUTES g_tsmSoundPlayedLast;	// Last timestamp where the sound was played

void
MainWindow_SystemTrayNewMessageArrived(TContact * pContactFrom, PSZUC pszMessage)
	{
	Assert(pContactFrom != NULL);
	Assert(pContactFrom->EGetRuntimeClass() == RTI(TContact));
	Assert(pszMessage != NULL);
	if (QGuiApplication::focusObject() != NULL)
		return;
	g_poSystemTrayIcon->show();
	CStr strTip;
//	if (QSystemTrayIcon::isSystemTrayAvailable() && QSystemTrayIcon::supportsMessages())
		{
		strTip.Format("New message from $s", pContactFrom->TreeItem_PszGetNameDisplay());
		g_poSystemTrayIcon->showMessage(strTip, CString(pszMessage), QSystemTrayIcon::Information);
		strTip.Empty();
		}
	g_arraypContactsRecentMessages.AddMRU(pContactFrom, 5);
	TContact ** ppContactStop;
	TContact ** ppContact = g_arraypContactsRecentMessages.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		strTip.BinAppendText_VE("$i unread messages from $s\n", pContact->ChatLog_GetCountMessagesUnread(), pContactFrom->TreeItem_PszGetNameDisplay());
		}
	strTip.TrimTailingWhiteSpacesNZ();
	g_poSystemTrayIcon->setToolTip(strTip);
	g_cMessagesSystemTray++;

	if (g_pIconNewMessage == NULL)
		g_pIconNewMessage = new QIcon(":/ico/SysTrayNewMessage");
	g_poSystemTrayIcon->setIcon(*g_pIconNewMessage);
	//QCoreApplication::processEvents();	// Force the message to be displayed NOW! (BTW this line of code does not work AND creates a deadlock when a new message arrives at the same time Cambrian starts)

	if ((g_uPreferences & P_kfDontPlaySoundWhenNewMessageArrive) == 0)
		{
		if (g_tsmMinutesSinceApplicationStarted != g_tsmSoundPlayedLast)
			{
			g_tsmSoundPlayedLast = g_tsmMinutesSinceApplicationStarted;	// Prevent the sound to play more than once per minute
			QSound::play(":/snd/NewMessage");
			}
		}

	QApplication::alert(g_pwMainWindow);
	g_pwMainWindow->TimerStartFlashIconNewMessage();
	} // MainWindow_SystemTrayNewMessageArrived()

void
MainWindow_SystemTrayMessageReadForContact(TContact * pContact)
	{
	if (g_arraypContactsRecentMessages.RemoveElementI(pContact) >= 0)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "MainWindow_SystemTrayMessageReadForContact($S) - Destroying System Tray Icon...\n", &pContact->m_strJidBare);
		SystemTray_Destroy();
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
EAnswer
EMessageBoxQuestion(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	CStr struMessage;
	return (EAnswer)QMessageBox::question(g_pwMainWindow, d_szApplicationName, QString::fromUtf8((PSZAC)struMessage.Format_VL(pszFmtTemplate, vlArgs)), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
	}

EAnswer
EMessageBoxInformation(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	CStr struMessage;
	return (EAnswer)QMessageBox::information(g_pwMainWindow, d_szApplicationName, QString::fromUtf8((PSZAC)struMessage.Format_VL(pszFmtTemplate, vlArgs)));
	}

EAnswer
EMessageBoxWarning(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	CStr struMessage;
	return (EAnswer)QMessageBox::warning(g_pwMainWindow, d_szApplicationName, QString::fromUtf8((PSZAC)struMessage.Format_VL(pszFmtTemplate, vlArgs)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Default routine displaying messages to the status bar.
//	This routine must have the same interface as PFn_StatusBarDisplayFunction()
void
StatusBar_DisplayFunctionDefault(ESeverity eSeverity, PSZUC pszuTextStatusBar)
	{
	if (pszuTextStatusBar == NULL || pszuTextStatusBar[0] == '\0')
		g_pwStatusBar->showMessage(c_sEmpty);
	else
		g_pwStatusBar->showMessage(QString::fromUtf8((PSZAC)pszuTextStatusBar), 60*1000);	// Show the message for a full minute (60,000 ms)
	g_pwStatusBar->repaint();	// Force the Status Bar to redraw itself
	if (eSeverity >= eSeverityErrorWarning)
		{
		ErrorLog_AddNewMessage("The StatusBar displayed the following error message", pszuTextStatusBar);
		MessageLog_AppendTextWithNewLine(d_coRed, pszuTextStatusBar);
		}
	}

PFn_StatusBarDisplayFunction g_pfnStatusBarDisplayFunction = StatusBar_DisplayFunctionDefault;	// This pointer CANNOT be NULL.

void
StatusBar_SetDisplayFunction(PFn_StatusBarDisplayFunction pfnStatusBarDisplayFunction)
	{
	Assert(pfnStatusBarDisplayFunction != NULL);
	g_pfnStatusBarDisplayFunction = pfnStatusBarDisplayFunction;
	}

void
StatusBar_RemoveDisplayFunction()
	{
	g_pfnStatusBarDisplayFunction = StatusBar_DisplayFunctionDefault;	// Use the default display function
	}

void
StatusBar_ClearText()
	{
	g_pfnStatusBarDisplayFunction(eSeverityNull, NULL);
	}

void
StatusBar_SetTextU(PSZUC pszuText)
	{
	g_pfnStatusBarDisplayFunction(eSeverityInfoTextBlack, pszuText);
	}

void
StatusBar_SetTextErrorU(PSZUC pszuTextError)
	{
	g_pfnStatusBarDisplayFunction(eSeverityErrorWarning, pszuTextError);
	}

void
StatusBar_SetTextFormat(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	StatusBar_SetTextFormatSev_VL(eSeverityInfoTextBlack, pszFmtTemplate, vlArgs);
	}

void
StatusBar_SetTextFormatError(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	StatusBar_SetTextFormatSev_VL(eSeverityErrorWarning, pszFmtTemplate, vlArgs);
	}

void
StatusBar_SetTextFormatSev(ESeverity eSeverity, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	StatusBar_SetTextFormatSev_VL(eSeverity, pszFmtTemplate, vlArgs);
	}

CStr g_strScratchBufferStatusBar;		// Scratch buffer used by the StatusBar and the MessageLog.  Other applications are welcome to use this buffer as long as they are aware the priority belongs to the StatusBar.

void
StatusBar_SetTextFormatSev_VL(ESeverity eSeverity, PSZAC pszFmtTemplate, va_list vlArgs)
	{
	g_pfnStatusBarDisplayFunction(eSeverity, g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs));
	}


struct STimerQueueCallback
{
	STimerQueueCallback * pNext;				// Next callback in the timer queue
	TIMESTAMP_MINUTES tsmTrigger;				// Timestamp to trigger the callback
	PFn_TimerQueueCallback pfnCallback;			// Callback function then the event is triggered
	PVPARAM pvParam;							// Parameter for the callback function
	// pfnFAbort								// Function to determine if the callback should be aborted and therefore removed from the queue
};

STimerQueueCallback * g_pTimerCallbackNext;
STimerQueueCallback * g_pTimerCallbackDeleted;

STimerQueueCallback *
_PAllocateTimerCallback(TIMESTAMP_MINUTES tsmTrigger)
	{
	STimerQueueCallback * pTimerCallbackNew = g_pTimerCallbackDeleted;	// Try to recycle previously allocated callback structure
	if (pTimerCallbackNew != NULL)
		g_pTimerCallbackDeleted = g_pTimerCallbackDeleted->pNext;
	else
		pTimerCallbackNew = new STimerQueueCallback;
	pTimerCallbackNew->tsmTrigger = tsmTrigger;
	if (g_pTimerCallbackNext == NULL || tsmTrigger < g_pTimerCallbackNext->tsmTrigger)
		{
		pTimerCallbackNew->pNext = g_pTimerCallbackNext;
		g_pTimerCallbackNext = pTimerCallbackNew;
		g_tsmTimerQueueNextCallbackToTrigger = pTimerCallbackNew->tsmTrigger;
		return pTimerCallbackNew;
		}
	// Insert the STimerQueueCallback to make sure all callbacks are sorted from the earliest to the latest
	STimerQueueCallback * pTimerCallbackQueued = g_pTimerCallbackNext;
	while (TRUE)
		{
		STimerQueueCallback * pTimerCallbackQueuedNext = pTimerCallbackQueued->pNext;
		if (pTimerCallbackQueuedNext == NULL || tsmTrigger < pTimerCallbackQueuedNext->tsmTrigger)
			break;
		pTimerCallbackQueued = pTimerCallbackQueuedNext;
		} // while
	pTimerCallbackNew->pNext = pTimerCallbackQueued->pNext;
	pTimerCallbackQueued->pNext = pTimerCallbackNew;
	return pTimerCallbackNew;
	}

void
TimerQueue_CallbackAdd(int cSeconds, PFn_TimerQueueCallback pfnCallback, PVPARAM pvParam)
	{
	Assert(cSeconds > 0);
	Assert(pfnCallback != NULL);
	//MessageLog_AppendTextFormatCo(d_coRed, "TimerQueue_CallbackAdd($I, 0x$p, 0x$p)\n", cSeconds, pfnCallback, pvParam);
	STimerQueueCallback * pTimerCallback = _PAllocateTimerCallback(g_tsmMinutesSinceApplicationStarted + (cSeconds / 60));
	pTimerCallback->pfnCallback = pfnCallback;
	pTimerCallback->pvParam = pvParam;
	}

//	Postpone an existing callback, or add it to the queue.
//	INTERFACE NOTES
//	The postponed value cSeconds must be equal or greater than the previous postponed value.
//	In other words, it is not allowed to 'postpone' an existing callback earlier than its previous callback.
//	If such a scenario happens, then the function will add a new callback for cSeconds which will result in two (2) callbacks.
void
TimerQueue_CallbackPostponeOrAdd(int cSeconds, PFn_TimerQueueCallback pfnCallback, PVPARAM pvParam)
	{
	Assert(cSeconds > 0);
	Assert(pfnCallback != NULL);
	TIMESTAMP_MINUTES tsmTrigger = g_tsmMinutesSinceApplicationStarted + (cSeconds / 60);
	// Find the existing callback (if any)
	STimerQueueCallback ** ppTimerCallbackRemove = &g_pTimerCallbackNext;
	STimerQueueCallback * pTimerCallback = g_pTimerCallbackNext;
	while (pTimerCallback != NULL && pTimerCallback->tsmTrigger <= tsmTrigger)
		{
		if (pTimerCallback->pvParam == pvParam && pTimerCallback->pfnCallback == pfnCallback)
			{
			MessageLog_AppendTextFormatCo(d_coRed, "TimerQueue_CallbackPostponeOrAdd($I, 0x$p, 0x$p) - Found existing callback at 0x$p\n", cSeconds, pfnCallback, pvParam, pTimerCallback);
			STimerQueueCallback * pTimerCallbackNext = pTimerCallback->pNext;
			if (pTimerCallbackNext == NULL || pTimerCallbackNext->tsmTrigger > tsmTrigger)
				{
				MessageLog_AppendTextFormatCo(d_coRed, "\t Updating trigger from $I to $I\n", pTimerCallback->tsmTrigger, tsmTrigger);
				pTimerCallback->tsmTrigger = tsmTrigger;
				return;
				}
			*ppTimerCallbackRemove = pTimerCallbackNext;	// Remove pTimerCallback from the linked list
			// Keep searching until the appropriate position
			while (TRUE)
				{
				STimerQueueCallback * pTimerCallbackInsert = pTimerCallbackNext;
				pTimerCallbackNext = pTimerCallbackNext->pNext;
				if (pTimerCallbackNext == NULL || pTimerCallbackNext->tsmTrigger > tsmTrigger)
					{
					if (pTimerCallbackNext == NULL)
						MessageLog_AppendTextFormatCo(d_coRed, "\t Appending trigger $I at end of list (just after $I)\n", tsmTrigger, pTimerCallbackInsert->tsmTrigger);
					else
						MessageLog_AppendTextFormatCo(d_coRed, "\t Appending trigger $I between $I and $I\n", tsmTrigger, pTimerCallbackInsert->tsmTrigger, pTimerCallbackNext->tsmTrigger);
					pTimerCallbackInsert->pNext = pTimerCallback;
					pTimerCallback->pNext = pTimerCallbackNext;
					pTimerCallback->tsmTrigger = tsmTrigger;
					return;
					}
				}
			} // if (found existing)
		ppTimerCallbackRemove = &pTimerCallback->pNext;
		pTimerCallback = pTimerCallback->pNext;
		}
	pTimerCallback = _PAllocateTimerCallback(tsmTrigger);
	pTimerCallback->pfnCallback = pfnCallback;
	pTimerCallback->pvParam = pvParam;
	} // TimerQueue_CallbackPostponeOrAdd()

//	Remove all callbacks matching pvParam.
//	This method assumes all pvParam are pointers to objects in memory, and therefore each pvParam is unique,
//	and if the object is deleted, then all its references must be removed.
void
TimerQueue_CallbackRemove(PVPARAM pvParam)
	{
	STimerQueueCallback ** ppTimerCallbackRemove = &g_pTimerCallbackNext;
	STimerQueueCallback * pTimerCallback = g_pTimerCallbackNext;
	while (pTimerCallback != NULL)
		{
		if (pTimerCallback->pvParam == pvParam)
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] TimerQueue_CallbackRemove() - @$I Removing callback matching pvParam 0x$p.\n", pTimerCallback->tsmTrigger, pvParam);
			*ppTimerCallbackRemove = pTimerCallback->pNext;
			}
		else
			ppTimerCallbackRemove = &pTimerCallback->pNext;
		pTimerCallback = pTimerCallback->pNext;
		}
	}

void
TimerQueue_ExecuteExpiredCallbacks()
	{
	Assert(g_pTimerCallbackNext != NULL);
	STimerQueueCallback * pTimerCallback = g_pTimerCallbackNext;
	while (pTimerCallback != NULL)
		{
		if (pTimerCallback->tsmTrigger > g_tsmMinutesSinceApplicationStarted)
			{
			g_tsmTimerQueueNextCallbackToTrigger = pTimerCallback->tsmTrigger;
			MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] TimerQueue_ExecuteExpiredCallbacks() - Next callback in $I minutes.\n", g_tsmTimerQueueNextCallbackToTrigger - g_tsmMinutesSinceApplicationStarted);
			break;
			}
		//MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "TimerQueue_ExecuteExpiredCallbacks() Executing 0x$p pfnCallback(0x$p)\n", pTimerCallback->pfnCallback, pTimerCallback->pvParam);
		pTimerCallback->pfnCallback(pTimerCallback->pvParam);	// Invoke the callback
		// Remove the callback from the main queue and put it into the recycled queue
		STimerQueueCallback * pTimerCallbackNext = pTimerCallback->pNext;
		pTimerCallback->pNext = g_pTimerCallbackDeleted;
		g_pTimerCallbackDeleted = pTimerCallback;
		pTimerCallback = pTimerCallbackNext;
		if (pTimerCallback == NULL)
			{
			g_tsmTimerQueueNextCallbackToTrigger = d_tsm_cMinutesMax;
			MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] TimerQueue_ExecuteExpiredCallbacks() - Queue is now empty.\n");
			break;
			}
		} // while
	g_pTimerCallbackNext = pTimerCallback;
	}

void
TimerQueue_DisplayToMessageLog()
	{
	STimerQueueCallback * pTimerCallback = g_pTimerCallbackNext;
	while (pTimerCallback != NULL)
		{
		MessageLog_AppendTextFormatCo(d_coRed, "\t [+$I min] @$I: pfnCallback = 0x$p, pvParam = 0x$p\n",
			pTimerCallback->tsmTrigger - g_tsmMinutesSinceApplicationStarted, pTimerCallback->tsmTrigger, pTimerCallback->pfnCallback, pTimerCallback->pvParam);
		pTimerCallback = pTimerCallback->pNext;
		}
	}
