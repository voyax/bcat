# Copyright 2019-2020, Collabora, Ltd.
# SPDX-License-Identifier: BSL-1.0

set(SOURCES
	edit_main_factory.cpp
	edit_pickandalign.cpp
	edit_slice.cpp
	${VCGDIR}/wrap/gui/coordinateframe.cpp
)

set(HEADERS
	edit_main_factory.h
	edit_pickandalign.h
	edit_slice.h
)

set(RESOURCES edit_main.qrc)

set(UI pickandalign.ui)

add_meshlab_plugin(edit_mainplugin ${SOURCES} ${HEADERS} ${RESOURCES} ${UI})

target_link_libraries(edit_mainplugin PRIVATE OpenGL::GLU)
