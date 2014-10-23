TEMPLATE = subdirs
CONFIG  += ordered

win32:SUBDIRS += curl


INCLUDEPATH += $${SOLUTION_DIR}..src/opentxs
INCLUDEPATH += $${SOLUTION_DIR}..src/opentxs/core/util
INCLUDEPATH += $${SOLUTION_DIR}..src/libidn/libidn
INCLUDEPATH += $${SOLUTION_DIR}..src/xmlrpc-c
INCLUDEPATH += $${SOLUTION_DIR}..src/zmq
INCLUDEPATH += $${SOLUTION_DIR}..src/boost
#SUBDIRS += libidn
#SUBDIRS += jsoncpp
#SUBDIRS += bitcoin-api
#SUBDIRS += nmcrpc
SUBDIRS += otx

RESOURCES += \
    ../../Resources.qrc










