#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
	#include <OTX_WRAP.h>
	#include <QPointer>
	extern  OTX_WRAP * pOTX;
#endif
#include <WQmlToolbar.h>
#include "ui_startupscreen.h"
extern bool g_fIsConnectedToInternet;
extern TIMESTAMP_MINUTES g_tsmMinutesSinceApplicationStarted;
extern UINT g_cMinutesIdleKeyboardOrMouse;
extern UINT g_cMinutesIdleNetworkDataReceived;
typedef UINT (* PFn_UGetIdleTimeInMinutes)();
extern PFn_UGetIdleTimeInMinutes g_pfnUGetIdleTimeInMinutes;

namespace Ui {
class startupScreen;
}

singleton WMainWindow : public QMainWindow
{
	Q_OBJECT
#ifdef COMPILE_WITH_SPLASH_SCREEN
protected:
    OJapiCambrian * m_paCambrian;
    QWebFrame * m_poFrame;
#endif
private:
	UINT m_cTimerEvents;				// Count how many timer events have been processed.  The motivation is some tasks may be performed at larger intervals than at every timer event.
	int m_tidFlashIconNewMessage;		// Timer identifier to flash the icon a new message has arrived
	BOOL m_fuAlternateDisplayIconNewMessage;
	int m_tidReconnect;					// Timer identifier to reconnect in case of a disconnection
	int m_ttiReconnect;
    Ui::startupScreen *ui;
public:

    //Pointer to QML toolbar
    // WQmlToolbar *p_QmlToolbar;
    explicit WMainWindow();
	~WMainWindow();
#ifdef COMPILE_WITH_SPLASH_SCREEN
     void maximizeApp(QString Url);
     void hideRolePage();
     void initRolePage();
#endif
	virtual void timerEvent(QTimerEvent * pTimerCallback);	// From QObject
	virtual bool event(QEvent * pEvent);				// From QObject

public:
	void SettingsSave();
	void SettingsRestore();
	void ConfigurationLoadFromXml();
	void MenuBarInitializeAndConnectActions(WMenu * pMenu);
	void TimerStartFlashIconNewMessage();

public slots:
	#ifdef COMPILE_WITH_SPLASH_SCREEN
    void SL_showRolePage();
    void SL_InitJavaScript();
    #endif
	void SL_Quitting();
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
QString MainWindow_SGetUrlPathOfApplication(const QString & sPathRelativeApplication);
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
TProfile * NavigationTree_PGetSelectedUnique();
TAccountXmpp * NavigationTree_PGetSelectedTreeItemMatchingInterfaceTAccount();
TContact * NavigationTree_PGetSelectedTreeItemMatchingInterfaceTContact();
ITreeItemChatLogEvents * NavigationTree_PGetSelectedTreeItemMatchingContractOrGroup();
void NavigationTree_RenameSelectedItem();

void NavigationTree_UpdatePresenceIcon(EMenuAction eMenuAction_Presence);
void NavigationTree_UpdateNameOfSelectedProfile();
void NavigationTree_SelectProfile(TProfile * pProfileToSelect);
void NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(TProfile * pProfileSelected, BOOL fCreateNewProfile = FALSE);
void NavigationTree_DisplayProfilesToogle();
void NavigationTree_DisplayCertificatesToogle();

void Dashboard_RefreshAccordingToSelectedProfile(TProfile * pProfileSelected);
void Dashboard_RefreshAll();
void Dashboard_RefreshGroupsAndChannels();
void Dashboard_RefreshGroups();
void Dashboard_RefreshChannels();
void Dashboard_RefreshContacts();
void Dashboard_NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ);
void Dashboard_NewEventRelatedToBallot(IEventBallot * pEventBallot);
void Dashboard_BumpContact(TContact * pContact);
void Dashboard_BumpTreeItem(ITreeItem * pTreeItem);
void Dashboard_RedrawContact(TContact * pContact);
void Dashboard_RedrawGroup(TGroup * pGroup);

void TimerQueue_CallbackAdd(int cSeconds, PFn_TimerQueueCallback pfnCallback, PVPARAM pvParam);
void TimerQueue_CallbackPostponeOrAdd(int cSeconds, PFn_TimerQueueCallback pfnCallback, PVPARAM pvParam);
void TimerQueue_CallbackRemove(PVPARAM pvParam);
void TimerQueue_ExecuteExpiredCallbacks();
void TimerQueue_DisplayToMessageLog();

void LaunchApplication_BallotmasterVote(CEventBallotReceived * pEventBallotReceived_NZ);
void LaunchApplication_Ballotmaster(TGroup * pGroupToSelect_YZ);
void LaunchApplication_Channels(TGroup * pChannelToSelect_YZ);
void LaunchApplication_GroupManager(TGroup * pGroupToSelect_YZ);
void LaunchApplication_Corporations(TGroup * pCorporationToSelect_YZ);
void LaunchApplication_PeerManager(TContact * pContactToSelect_YZ);

void Toolbar_TabAdd(ITreeItem * pTreeItem);
void Toolbar_TabSelect(ITreeItem * pTreeItem);
void Toolbar_TabRedraw(ITreeItem * pTreeItem);

#endif // MAINWINDOW_H
