AUTHOR	= Roberto Alsina
TEMPLATE	= app
EMAIL	= ralsina@kde.org
LANGUAGE	= C++
SOURCES         = iupcanvas3.c
INCLUDEPATH = ../../../include/ ../../../cd/include
LIBS += -L../../../lib -liup -liupcontrols -lcdiup -lcd
DESTDIR=../../../bin

