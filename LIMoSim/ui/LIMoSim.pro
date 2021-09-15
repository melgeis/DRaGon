QT += qml quick network positioning
QT += widgets printsupport # for QCustomPlot
CONFIG += c++11

SOURCES += main.cc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += $$PWD/../LIMoSim
INCLUDEPATH += $$PWD/../ # project view from core

#LIBS += -L -ssl

# SSL: libeay32.dll, ssleay32.dll from Qtxy\Tools\QtCreator\bin

include(../LIMoSim/LIMoSim.pri)
include(UI.pri)
include(../app/App.pri)


#message("copying resources from $$PWD/../misc/resources to $$OUT_PWD")
#@
#QMAKE_POST_LINK += $$quote($(COPY) $$PWD/../misc/resources/* $$OUT_PWD/)
#@
