AUTHOR	= Roberto Alsina
TEMPLATE	= app
EMAIL	= ralsina@kde.org
LANGUAGE	= C++
SOURCES         = iupcanvas.c
INCLUDEPATH = ../../../include/ ../../../cd/include
LIBS += -L../../../lib -liup -lcdiup -lcd
DESTDIR=../../../bin

