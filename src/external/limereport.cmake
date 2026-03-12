# Copyright 2024, MeshLab Contributors
# SPDX-License-Identifier: BSL-1.0
# 
# LimeReport integration for MeshLab
# This downloads and builds LimeReport from source

option(ALLOW_BUNDLED_LIMEREPORT "Allow building LimeReport from source" ON)

set(LIMEREPORT_DIR ${CMAKE_CURRENT_LIST_DIR}/limeReport)

# Check if prebuilt libraries exist
if(WIN32)
    set(LIMEREPORT_LIB_NAME "limereport.lib")
    set(QTZINT_LIB_NAME "QtZint.lib")
else()
    set(LIMEREPORT_LIB_NAME "liblimereport.a")
    set(QTZINT_LIB_NAME "libQtZint.a")
endif()

set(LIMEREPORT_LIB "${LIMEREPORT_DIR}/${LIMEREPORT_LIB_NAME}")
set(QTZINT_LIB "${LIMEREPORT_DIR}/${QTZINT_LIB_NAME}")

if(EXISTS "${LIMEREPORT_LIB}" AND EXISTS "${QTZINT_LIB}")
    message(STATUS "- LimeReport - using prebuilt libraries")
    
    # Create imported target for LimeReport
    add_library(external-limereport STATIC IMPORTED GLOBAL)
    set_target_properties(external-limereport PROPERTIES
        IMPORTED_LOCATION "${LIMEREPORT_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIMEREPORT_DIR}/include"
    )
    
    # Create imported target for QtZint
    add_library(external-qtzint STATIC IMPORTED GLOBAL)
    set_target_properties(external-qtzint PROPERTIES
        IMPORTED_LOCATION "${QTZINT_LIB}"
    )
    
    # Link QtZint to LimeReport
    set_target_properties(external-limereport PROPERTIES
        INTERFACE_LINK_LIBRARIES "external-qtzint;Qt5::Widgets;Qt5::Xml;Qt5::Sql;Qt5::PrintSupport;Qt5::Script;Qt5::Svg"
    )
    
    set(LIMEREPORT_FOUND TRUE CACHE INTERNAL "LimeReport library found")
else()
    message(STATUS "- LimeReport - prebuilt libraries not found, LimeReport features will be disabled")
    message(STATUS "  To enable LimeReport, place ${LIMEREPORT_LIB_NAME} and ${QTZINT_LIB_NAME} in ${LIMEREPORT_DIR}/")
    set(LIMEREPORT_FOUND FALSE CACHE INTERNAL "LimeReport library found")
endif()
