#-------------------------------------------------
#
# SOPRO by Central Services 2014.
#
# IMPORTANT: This project needs OTX libraries.
#            In order to be able to compile
#            sopro, open "otx/otx.pro" and build
#            all libraries.
#            Take note that the "OUTPUT DIRECTORY"
#            must be the same output directory of
#            the otx.pro in order to access all compiled
#            libraries.
#-------------------------------------------------

QT       += core gui network multimedia sql quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = SocietyPro
TEMPLATE = app
INCLUDEPATH += otx/src/jsoncpp
INCLUDEPATH += otx/src/core
INCLUDEPATH += otx/src/opentxs
INCLUDEPATH += otx/src
PRECOMPILED_HEADER = otx/src/core/stable.hpp

#-------------------------------------------------
# Linked Libs
DEFINES    += "OT_ZMQ_MODE=1"
# MAC AND LINUX:
unix: {

##  LIBS += -L$${OUT_PWD}/../curl
##  LIBS += -lcurl

    LIBS       += -L$${PWD}/otx/libs/mac -lzmq -lxmlrpc_client++ -lxmlrpc -lxmlrpc++ -lotapi -lot -lcurl -ldl
    LIBS += -L"/usr/local/lib" -lcrypto -lssl -lz
    LIBS += -L$${OUT_PWD}/bitcoin-api  -lbitcoin-api
    LIBS += -L$${OUT_PWD}/jsoncpp -ljsoncpp
    LIBS += -L$${OUT_PWD}/libidn -llibidn
    LIBS += -L$${OUT_PWD}/nmcrpc -lnmcrpc
    LIBS += -L$${OUT_PWD}/otx -lOTX




}
#windows
else:  {
LIBS       +=  -lzmq -lxmlrpc_client++ -lxmlrpc -lxmlrpc++ -lotapi -lot -ldl
}

win32: {
    QMAKE_LIBDIR += $${DESTDIR}

    equals(TEMPLATE,vcapp):{
        QMAKE_LIBDIR += $(SystemDrive)/OpenSSL-Win$(PlatformArchitecture)/lib/VC
        QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/$(PlatformName)/$(Configuration)/
    }
    else:{
        !contains(QMAKE_HOST.arch, x86_64):{
            QMAKE_LIBDIR += C:/OpenSSL-Win32/lib/VC
            CONFIG(debug, debug|release):{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/Win32/Debug/
            }
            else:{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/Win32/Release/
            }
        }
        else:{
            QMAKE_LIBDIR += C:/OpenSSL-Win64/lib/VC
            CONFIG(debug, debug|release):{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/x64/Debug/
            }
            else:{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/x64/Release/
            }
        }
    }

    LIBS += bitcoin-api.lib
    LIBS += jsoncpp.lib
    LIBS += curl.lib
    LIBS += nmcrpc.lib

    LIBS += otlib.lib
    LIBS += otapi.lib

    LIBS += Advapi32.lib
}


QMAKE_CFLAGS_WARN_OFF += -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS_WARN_OFF += -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS += -std=c++11 -DCXX_11
CONFIG += c++11

PRECOMPILED_HEADER = PreCompiledHeaders.h


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
    OTX_WRAP.cpp \
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
    WToolbarActions.cpp

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
    OTX_WRAP.h \
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
    WToolbarActions.h


FORMS += \
    startupscreen.ui

RC_ICONS = Icons/HavenIcon.ico

OTHER_FILES += \
    FeatureList.txt

RESOURCES += \
    Resources.qrc
