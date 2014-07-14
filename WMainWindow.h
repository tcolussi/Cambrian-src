#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

extern bool g_fIsConnectedToInternet;
extern TIMESTAMP_MINUTES g_tsmMinutesSinceApplicationStarted;
extern UINT g_cMinutesIdleKeyboardOrMouse;
extern UINT g_cMinutesIdleNetworkDataReceived;
typedef UINT (* PFn_UGetIdleTimeInMinutes)();
extern PFn_UGetIdleTimeInMinutes g_pfnUGetIdleTimeInMinutes;

singleton WMainWindow : public QMainWindow
{
	Q_OBJECT
private:
	UINT m_cTimerEvents;				// Count how many timer events have been processed.  The motivation is some tasks may be performed at larger intervals than at every timer event.
	int m_tidFlashIconNewMessage;		// Timer identifier to flash the icon a new message has arrived
	BOOL m_fuAlternateDisplayIconNewMessage;
	int m_tidReconnect;					// Timer identifier to reconnect in case of a disconnection
	int m_ttiReconnect;

public:
	explicit WMainWindow();
	~WMainWindow();

	//virtual void closeEvent(QCloseEvent * pEvent);		// From QWidget
	//virtual void changeEvent(QEvent * pEvent);			// From QWidget
	virtual void timerEvent(QTimerEvent * pTimerEvent);	// From QObject
	virtual bool event(QEvent * pEvent);				// From QObject

public:
	void SettingsSave();
	void SettingsRestore();
	void ConfigurationLoadFromXml();
	void MenuBarInitializeAndConnectActions(WMenu * pMenu);
	void TimerStartFlashIconNewMessage();

public slots:
	void SL_MenuAboutToShow();
	void SL_MenuActionTriggered(QAction * pAction);
	void SL_SystemTrayMessageClicked();
	void SL_SystemTrayActivated(QSystemTrayIcon::ActivationReason eActivationReason);

	void SL_NetworkOnlineStateChanged(bool fIsOnline);
	void SL_WebSocketMessage(CBin & binszvFrameData);
}; // WMainWindow

extern WMainWindow * g_pwMainWindow;
extern CInvitationFromUrlQueryString * g_paInvitation;
CInvitationFromUrlQueryString * Invitation_PazAllocateFromUrlQueryString(PSZUC pszUrl);
void Invitation_EraseFile();

void MainWindow_MenuActionExecute(QAction * pAction);
void MainWindow_SetCurrentLayout(IN WLayout * pwLayout);
void MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD WLayout * pawLayoutAutoDelete);
void MainWindow_DeleteLayout(PA_DELETING WLayout * pawLayoutToDelete);
void MainWindow_SetFocusToCurrentLayout();
WLayout * MainWindow_PwGetCurrentLayout();
void MainWindow_DisplayNoticeWithinCurrentLayout(IN INotice * piwNotice);
void MainWindow_SystemTrayNewMessageArrived(TContact * pContactFrom, PSZUC pszMessage);
void MainWindow_SystemTrayMessageReadForContact(TContact * pContact);
void SystemTray_Destroy();

void NavigationTree_SelectTreeItem(ITreeItem * pTreeItem);
ITreeItem * NavigationTree_PGetSelectedTreeItem();
ITreeItem * NavigationTree_PGetSelectedTreeItemMatchingInterface(RTI_ENUM rti);
TProfile * NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
TAccountXmpp * NavigationTree_PGetSelectedTreeItemMatchingInterfaceTAccount();
TContact * NavigationTree_PGetSelectedTreeItemMatchingInterfaceTContact();
ITreeItemChatLogEvents * NavigationTree_PGetSelectedTreeItemMatchingContractOrGroup();
void NavigationTree_RenameSelectedItem();

void NavigationTree_UpdatePresenceIcon(EMenuAction eMenuAction_Presence);
void NavigationTree_UpdateNameOfSelectedProfile();
void NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(TProfile * pProfileSelected, BOOL fCreateNewProfile = FALSE);

#endif // MAINWINDOW_H
