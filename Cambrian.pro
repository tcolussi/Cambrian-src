#-------------------------------------------------
#
# Project created by QtCreator 2013-07-07T08:41:29
#
#-------------------------------------------------


QT     += core gui multimedia quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = SocietyPro
TEMPLATE = app

INCLUDEPATH += C:\OpenSSL-Win32\include
LIBS +=  -LC:\OpenSSL-Win32\lib\MinGW -lssleay32 -leay32

PRECOMPILED_HEADER = PreCompiledHeaders.h

#-------------------------------------------------
# Compiler options
#DEFINES += COMPILE_WITH_OPEN_TRANSACTIONS
#DEFINES += COMPILE_WITH_SPLASH_SCREEN
#DEFINES += COMPILE_WITH_CRYPTOMANIA
#DEFINES += COMPILE_WITH_TOOLBAR


SOURCES += \
    CArray.cpp \
    CBin.cpp \
    CChatConfiguration.cpp \
    CHashTable.cpp \
    CMemoryAccumulator.cpp \
    CMemoryHeap.cpp \
    CStr.cpp \
    DDialog.cpp \
    DDialogProperties.cpp \
    DebugUtil.cpp \
    DialogAccountNew.cpp \
    DialogContactNew.cpp \
    DialogInvitations.cpp \
    EnumerationMap.cpp \
    ErrorMessages.cpp \
    FilePathUtilities.cpp \
    GroupChat.cpp \
    HashMd5.cpp \
    HashSha1.cpp \
    HashSha256.cpp \
    Idle.cpp \
    IEvent.cpp \
    IEventBallot.cpp \
    InternetWebServices.cpp \
    IRuntimeObject.cpp \
    ITreeItem.cpp \
    ITreeItemChatLogEvents.cpp \
    List.cpp \
    main.cpp \
    MenuActions.cpp \
    QtUtilities.cpp \
    Sockets.cpp \
    SocketTasks.cpp \
    Sort.cpp \
    StringNumeric.cpp \
    StringsShared.cpp \
    StringUtilities.cpp \
    TAccount.cpp \
    TApplicationMayanX.cpp \
    TBrowser.cpp \
    TCertificate.cpp \
    TContact.cpp \
    TGroup.cpp \
    TMarketplace.cpp \
    TProfile.cpp \
    TWallet.cpp \
    WChatInput.cpp \
    WChatLog.cpp \
    WFindText.cpp \
    WGrid.cpp \
    WLayoutAccount.cpp \
    WLayoutApplicationMayanX.cpp \
    WLayoutBrowser.cpp \
    WLayoutCertificate.cpp \
    WLayoutChatLog.cpp \
    WLayoutContainer.cpp \
    WLayoutProfile.cpp \
    WMainWindow.cpp \
    WMarketplace.cpp \
    WNavigationTree.cpp \
    WNotices.cpp \
    Xcp.cpp \
    XcpApi.cpp \
    Xml.cpp \
    ProfileSwitching.cpp \
    TRecommendations.cpp \
    ApiWebSockets.cpp \
    Identities.cpp \
    TApplicationBallotmaster.cpp \
    CVaultEvents.cpp \
    ApiJavaScript.cpp \
    IApplication.cpp \
    WLayoutTabbedBrowser.cpp \
    TBrowserTabs.cpp \
    TBrowserTab.cpp \
    WDashboard.cpp \
    DDialogChannels.cpp \
    TChannel.cpp \
    WQmlToolbar.cpp \
	MenuIcons.cpp \
	WChatLogHtml.cpp \
    TCorporation.cpp \
    WToolbar.cpp \
	WToolbarActions.cpp \
	ICrypto.cpp


HEADERS += \
    CArray.h \
    CBin.h \
    CChatConfiguration.h \
    CHashTable.h \
    CMemoryAccumulator.h \
    CMemoryHeap.h \
    CStr.h \
    DataTypesAndMacros.h \
    DDialog.h \
    DDialogProperties.h \
    DebugUtil.h \
    DialogAccountNew.h \
    DialogContactNew.h \
    DialogInvitations.h \
    EnumerationMap.h \
    ErrorCodes.h \
    ApiJavaScript.h \
    ErrorMessages.h \
    FilePathUtilities.h \
    GroupChat.h \
    IEvent.h \
    IEventBallot.h \
    InternetWebServices.h \
    IRuntimeObject.h \
    ITreeItem.h \
    ITreeItemChatLogEvents.h \
    List.h \
    MenuActions.h \
    QtUtilities.h \
    Sockets.h \
    SocketTasks.h \
    Sort.h \
    StringNumeric.h \
    StringsShared.h \
    StringUtilities.h \
    TAccount.h \
    TApplicationMayanX.h \
    TBrowser.h \
    TCertificate.h \
    TContact.h \
    TGroup.h \
    TMarketplace.h \
    TProfile.h \
    TWallet.h \
    WChatInput.h \
    WChatLog.h \
    WFindText.h \
    WGrid.h \
    WLayoutAccount.h \
    WLayoutApplicationMayanX.h \
    WLayoutBrowser.h \
    WLayoutCertificate.h \
    WLayoutChatLog.h \
    WLayoutContainer.h \
    WLayoutProfile.h \
    WMainWindow.h \
    WMarketplace.h \
    WNavigationTree.h \
    WNotices.h \
    Xcp.h \
    XcpApi.h \
    Xml.h \
    ProfileSwitching.h \
    TRecommendations.h \
    ApiWebSockets.h \
    Identities.h \
    TApplicationBallotmaster.h \
    CVaultEvents.h \
    IApplication.h \
    WLayoutTabbedBrowser.h \
    TBrowserTabs.h \
    TBrowserTab.h \
    WDashboard.h \
    DDialogChannels.h \
    TChannel.h \
    WQmlToolbar.h \
    MenuIcons.h \
	WChatLogHtml.h \
    TCorporation.h \
    WToolbar.h \
	WToolbarActions.h \
	ICrypto.h


RC_ICONS = Icons/SocietyPro.ico

FORMS += \
    startupscreen.ui

OTHER_FILES += \
    FeatureList.txt

RESOURCES += \
    Resources.qrc
