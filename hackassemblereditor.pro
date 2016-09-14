#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T00:58:43
#
#-------------------------------------------------

QT += core gui widgets

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -std=gnu++11

TARGET = hackassemblereditor
TEMPLATE = app

SOURCES += main.cpp \
    hackassembler/assembler.cpp \
    hackassembler/code.cpp \
    hackassembler/parser.cpp \
    hackassembler/symboltable.cpp \
    helpers/assemblercontroller.cpp \
    helpers/hacksyntaxhighlighter.cpp \
    ui/aboutdialog.cpp \
    ui/hackassemblereditor.cpp

HEADERS  += \
    hackassembler/assembler.h \
    hackassembler/code.h \
    hackassembler/parser.h \
    hackassembler/symboltable.h \
    helpers/assemblercontroller.h \
    helpers/hacksyntaxhighlighter.h \
    ui/aboutdialog.h \
    ui/hackassemblereditor.h

FORMS    += \
    ui/aboutdialog.ui \
    ui/hackassemblereditor.ui

RESOURCES += \
    hackassemblereditor.qrc
