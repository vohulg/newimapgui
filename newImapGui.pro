#-------------------------------------------------
#
# Project created by QtCreator 2014-05-12T17:02:07
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = newImapGui
TEMPLATE = app


SOURCES += main.cpp\
        imapgui.cpp \
    imap.cpp \
    imapaddress.cpp \
    imapmailbox.cpp \
    imapmessage.cpp

HEADERS  += imapgui.h \
    imap.h \
    imapaddress.h \
    imapmailbox.h \
    imapmessage.h

FORMS    += imapgui.ui
