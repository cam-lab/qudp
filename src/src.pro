TEMPLATE    = lib
TARGET      = qudp

#CONFIG     += staticlib

PRJ_DIR      = ..

include($${PRJ_DIR}/build.pri)

HEADERS    += \
               $${INC_DIR}/tqueue.h       \
               $${INC_DIR}/SysUtils.h     \
               $${INC_DIR}/SysUtilsQt.h   \
               $${INC_DIR}/qudp_lib.h     \
               $${INC_DIR}/UDP_defs.h     \
               qudp.h
SOURCES    += \
               qudp.cpp                   \
               qudp_lib.cpp

QT         += testlib network

DEFINES += ENA_FW_QT

win32 {
    DEFINES += ENA_WIN_API
    contains(CONFIG,staticlib) {
        message(staticlib build)
    } else {
        message(dynamiclib build)
        LIBS += -lws2_32
    }
}
