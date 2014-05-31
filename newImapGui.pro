#-------------------------------------------------
#
# Project created by QtCreator 2014-05-12T17:02:07
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = newImapGui
TEMPLATE = app


SOURCES += main.cpp\
        imapgui.cpp \
    imap.cpp \
    imapaddress.cpp \
    imapmailbox.cpp \
    imapmessage.cpp \
    addacount.cpp \
    tmonitoring.cpp \
    tmailagent.cpp \
    tmycookiejar.cpp

HEADERS  += imapgui.h \
    imap.h \
    imapaddress.h \
    imapmailbox.h \
    imapmessage.h \
    addacount.h \
    tmonitoring.h \
    tmailagent.h \
    tmycookiejar.h

FORMS    += imapgui.ui \
    addacount.ui

OTHER_FILES += \
    README.txt
