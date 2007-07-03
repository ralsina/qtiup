AUTHOR	= Roberto Alsina
TEMPLATE	= app
EMAIL	= ralsina@kde.org
LANGUAGE	= C++
SOURCES         = example1.c
INCLUDEPATH = ../../../include/ ../../../cd/include
LIBS += -L../../../lib -liupcontrols -liup -lcdiup -lcd
DESTDIR=../../../bin

