AUTHOR	= Roberto Alsina
TEMPLATE	= app
EMAIL	= ralsina@kde.org
LANGUAGE	= C++
SOURCES         = numbering.c
INCLUDEPATH = ../../../include/ ../../../cd/include
LIBS += -L../../../lib -liupcontrols -liup -lcdiup -lcd
DESTDIR=../../../bin

