include (../../shared.pri)

HEADERS += \
    filter_mesh_booleans.h

SOURCES += \
	filter_mesh_booleans.cpp

TARGET = filter_meshing_booleans

win32-msvc:QMAKE_CXXFLAGS = /bigobj
