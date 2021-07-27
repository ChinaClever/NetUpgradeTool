INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/aes/

win32: {

#    LIBS += -L$$PWD/lib/ -licom
#    CONFIG += qaxcontainer
} else: {
}


DEPENDPATH += $$PWD


HEADERS += \
    $$PWD/base64.h \
    $$PWD/hex.h \
    $$PWD/aes.h \
    $$PWD/aes_locl.h \
    $$PWD/modes.h \
    $$PWD/aesencrypt.h

SOURCES += \
    $$PWD/aes_core.c  \
    $$PWD/aes.c \
    $$PWD/aes_cbc.c \
    $$PWD/aes_ecb.c\
    $$PWD/cbc128.c  \
    $$PWD/base64.cpp \
    $$PWD/hex.cpp \
    $$PWD/aesencrypt.cpp

