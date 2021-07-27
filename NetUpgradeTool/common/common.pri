
INCLUDEPATH += $$PWD

include(ips/ips.pri)
include(excel/excel.pri)
include(exports/exports.pri)
include(tablecom/tablecom.pri)
include(network/network.pri)
include(upgrade/upgrade.pri)
include(iptables/iptables.pri)
#include(aes/aes.pri)
include(cryptopp/cryptopp.pri)

HEADERS += \
    $$PWD/datapacket.h \
    $$PWD/myMd5.h \
    $$PWD/msgbox.h

	
SOURCES += \
    $$PWD/datapacket.cpp \
    $$PWD/myMd5.cpp \
    $$PWD/msgbox.cpp

FORMS += \



