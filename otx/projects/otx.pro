TEMPLATE = subdirs
CONFIG  += ordered

win32:SUBDIRS += curl

SUBDIRS += libidn
SUBDIRS += jsoncpp
SUBDIRS += bitcoin-api
SUBDIRS += nmcrpc
SUBDIRS += otx

RESOURCES += \
    ../../Resources.qrc










