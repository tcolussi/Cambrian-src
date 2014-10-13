#-------------------------------------------------
#
# OTX Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += staticlib

TARGET      = nmcrpc
#VERSION     =

win32:DEFINES += "CURL_STATICLIB=1" "STATICLIB=1"


#-------------------------------------------------
# Common Settings

include(../common.pri)


#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/nmcrpc/nmcrpc.pri)


#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp
INCLUDEPATH +=$${SOLUTION_DIR}../src/libidn/libidn

#-------------------------------------------------
# Package Config

# MAC AND LINUX:
unix:{
    #PKGCONFIG += libidn
}


