QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adminDialog.cpp \
    aiChatDialog.cpp \
    appointment.cpp \
    appointmentManager.cpp \
    expert.cpp \
    expertDialog.cpp \
    expertManager.cpp \
    main.cpp \
    mainwindow.cpp \
    patientDialog.cpp \

HEADERS += \
    adminDialog.h \
    aiChatDialog.h \
    appointment.h \
    appointmentManager.h \
    expert.h \
    expertDialog.h \
    expertManager.h \
    mainwindow.h \
    patientDialog.h

FORMS += \
    adminDialog.ui \
    aiChatDialog.ui \
    expertDialog.ui \
    mainwindow.ui \
    patientDialog.ui

RESOURCES += resources.qrc

LIBS += -L$$PWD/bin/ -llibcrypto-1_1-x64 -llibssl-1_1-x64

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

