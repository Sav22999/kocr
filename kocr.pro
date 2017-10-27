#-------------------------------------------------
#
# Project created by QtCreator 2017-10-24T19:24:43
#
#-------------------------------------------------

#Please use Qt5.6.0, in the future I'll shift from webkit to webengine

QT       += core gui webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kocr
TEMPLATE = app


SOURCES += main.cpp\
        kocrmainwindow.cpp

HEADERS  += kocrmainwindow.h

FORMS    += kocrmainwindow.ui
