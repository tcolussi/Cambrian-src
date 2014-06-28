#-------------------------------------------------
#
# Project created by QtCreator 2013-07-07T08:41:29
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = Cambrian
TEMPLATE = app

PRECOMPILED_HEADER = PreCompiledHeaders.h

SOURCES += main.cpp\
	WMainWindow.cpp \
	WChatLog.cpp \
	TAccount.cpp \
    CChatConfiguration.cpp \
    CArray.cpp \
    DebugUtil.cpp \
	TContact.cpp \
    WChatInput.cpp \
    Xml.cpp \
    CMemoryAccumulator.cpp \
    List.cpp \
    CMemoryHeap.cpp \
    CBin.cpp \
    StringUtilities.cpp \
	DialogAccountNew.cpp \
    Sockets.cpp \
    HashMd5.cpp \
    MenuActions.cpp \
    DialogContactNew.cpp \
    ITreeItem.cpp \
    EnumerationMap.cpp \
    SocketTasks.cpp \
    QtUtilities.cpp \
    ErrorMessages.cpp \
    IRuntimeObject.cpp \
    FilePathUtilities.cpp \
    StringsShared.cpp \
    WFindText.cpp \
	TCertificate.cpp \
    HashSha1.cpp \
    TMarketplace.cpp \
    WMarketplace.cpp \
    CHashTable.cpp \
    WLayoutChatLog.cpp \
    WLayoutContainer.cpp \
    WNavigationTree.cpp \
    WLayoutCertificate.cpp \
	DialogInvitations.cpp \
	WNotices.cpp \
    CStr.cpp \
    DDialogProperties.cpp \
    Idle.cpp \
    TWallet.cpp \
    WGrid.cpp \
    DDialog.cpp \
    TGroup.cpp \
    IEvent.cpp \
    WLayoutApplicationMayanX.cpp \
    Sort.cpp \
    TProfile.cpp \
    StringNumeric.cpp \
    WLayoutProfile.cpp \
    HashSha256.cpp \
    WLayoutAccount.cpp \
    InternetWebServices.cpp \
	TBrowser.cpp \
    TApplicationMayanX.cpp \
    GroupChat.cpp \
    ChatLogEvents.cpp \
    Xcp.cpp \
    WLayoutBrowser.cpp \
    XcpApi.cpp


HEADERS += PreCompiledHeaders.h \
	QtUtilities.h \
	DDialog.h \
	WMainWindow.h \
	IEvent.h \
	WChatLog.h \
	Sockets.h  \
	TAccount.h \
    CChatConfiguration.h \
    DataTypesAndMacros.h \
    CArray.h \
    DebugUtil.h \
	TContact.h \
    WChatInput.h \
	WFindText.h \
    Xml.h \
    CMemoryAccumulator.h \
    List.h \
    CMemoryHeap.h \
    CBin.h \
    ErrorCodes.h \
    ErrorMessages.h \
    StringUtilities.h \
	DialogInvitations.h \
	DialogAccountNew.h \
    MenuActions.h \
    DialogContactNew.h \
    ITreeItem.h \
    EnumerationMap.h \
    SocketTasks.h \
    IRuntimeObject.h \
    FilePathUtilities.h \
	StringsShared.h \
	TCertificate.h \
    TMarketplace.h \
    WMarketplace.h \
    CHashTable.h \
    WLayoutContainer.h \
    WLayoutChatLog.h \
    WNavigationTree.h \
    WLayoutCertificate.h \
	WNotices.h \
    CStr.h \
    DDialogProperties.h \
    TWallet.h \
	WGrid.h \
	TGroup.h \
    WLayoutApplicationMayanX.h \
    Sort.h \
    TProfile.h \
    StringNumeric.h \
    WLayoutProfile.h \
    WLayoutAccount.h \
    InternetWebServices.h \
	TBrowser.h \
    TApplicationMayanX.h \
    GroupChat.h \
    ChatLogEvents.h \
    Xcp.h \
    WLayoutBrowser.h \
    XcpApi.h


FORMS +=

RC_ICONS = Icons/HavenIcon.ico

OTHER_FILES += \
    FeatureList.txt

RESOURCES += \
    Resources.qrc
