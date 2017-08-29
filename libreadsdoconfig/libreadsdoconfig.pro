TARGET = readsdoconfig
TEMPLATE = lib
CONFIG += staticlib 

QMAKE_CFLAGS_WARN_ON -= -Wall   # Optional - disable warnings when compiling this library
QMAKE_CXXFLAGS_WARN_ON -= -Wall # Optional - disable warnings when compiling this library

HEADERS += \
	include/readsdoconfig.h

SOURCES += \
	src/main.c \
	src/readsdoconfig.c

INCLUDEPATH += include 
