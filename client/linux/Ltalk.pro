QT += core gui
QT += network
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/floating_settings_window.cpp \
    src/group_list_item.cpp \
    src/main_page.cpp \
    src/ltalk.h \
    src/login_page.cpp \
    src/center.cpp \
    src/main.cpp \
    src/tray_icon.cpp \
    src/user_list_item.cpp

HEADERS += \
    src/floating_settings_window.h \
    src/group_list_item.h \
    src/main_page.h \
    src/login_page.h \
    src/center.h \ \
    src/ltalk.h \
    src/tray_icon.h \
    src/user_list_item.h

FORMS += \
    src/form/floating_settings_window.ui \
    src/form/main_page.ui \
    src/form/login_page.ui \
    src/form/group_list_item.ui \
    src/form/user_list_item.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/ui.qrc
