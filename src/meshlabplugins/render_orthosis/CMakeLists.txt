# Copyright 2019-2020, Collabora, Ltd.
# SPDX-License-Identifier: BSL-1.0

set(SOURCES orthosisRender.cpp textfile.cpp)

set(HEADERS orthosisRender.h textfile.h)

add_meshlab_plugin(render_orthosis ${SOURCES} ${HEADERS})
