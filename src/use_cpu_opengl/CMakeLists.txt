# This project is supposed to be compiled only with windows.
# It creates a binary file inside the MESHLAB_DISTRIB folder that is called
# UseCPUOpenGL. This binary file is automatically run by meshlab
# only if glewInit() fails (therefore, drivers do not support newer OpenGL
# versions). When UseCPUOpenGL is run, a file called "opengl32.dll" will be
# renamed/placed in the meshlab folder, overriding the system one and making meshlab
# able to start using a CPU implementation of OpenGL.

if(WIN32)

# Check if opengl32.dll exists (Mesa3D software renderer)
# This DLL is not included in the repo and must be obtained separately
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/opengl32.dll")
	message(STATUS "use_cpu_opengl: opengl32.dll not found, skipping build")
	message(STATUS "  To enable this component, download Mesa3D opengl32.dll to ${CMAKE_CURRENT_SOURCE_DIR}/")
	return()
endif()

set(SOURCES
	main.cpp
)

set(RESOURCES 
	res.qrc
)

add_executable(UseCPUOpenGL ${SOURCES} ${RESOURCES})

target_link_libraries(
        UseCPUOpenGL
        PUBLIC Qt5::Core
               Qt5::Widgets
)

set_target_properties(UseCPUOpenGL PROPERTIES LINK_FLAGS "/MANIFESTUAC:level=\'requireAdministrator\'")

install(TARGETS UseCPUOpenGL DESTINATION ${MESHLAB_BIN_INSTALL_DIR})

endif()
