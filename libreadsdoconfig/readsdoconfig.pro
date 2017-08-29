TARGET = libreadsdoconfig
TEMPLATE = lib
CONFIG += staticlib 

QMAKE_CFLAGS_WARN_ON -= -Wall   # Optional - disable warnings when compiling this library
QMAKE_CXXFLAGS_WARN_ON -= -Wall # Optional - disable warnings when compiling this library

HEADERS += \
	readsdoconfig.h

SOURCES += \
	main.c \
	readsdoconfig.c

