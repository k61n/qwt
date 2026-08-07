cmake_minimum_required(VERSION 3.18)

project(qwt VERSION 6.3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if (${PREFER_QT})
    if (${PREFER_QT} EQUAL 6)
        set(QT_MODULES Concurrent OpenGL OpenGLWidgets PrintSupport Svg Widgets)
        find_package(Qt6 COMPONENTS ${QT_MODULES} REQUIRED)
        set(V 6)
    elseif (${PREFER_QT} EQUAL 5)
        set(QT_MODULES Concurrent OpenGL PrintSupport Svg Widgets)
        find_package(Qt5 COMPONENTS ${QT_MODULES} REQUIRED)
        set(V 5)
    endif ()
else ()
    set(QT_MODULES Concurrent OpenGL OpenGLWidgets PrintSupport Svg Widgets)
    find_package(Qt6 COMPONENTS ${QT_MODULES} QUIET)
    if (${Qt6_FOUND})
        set(V 6)
    else ()
        set(QT_MODULES Concurrent OpenGL PrintSupport Svg Widgets)
        find_package(Qt5 COMPONENTS ${QT_MODULES} REQUIRED)
        set(V 5)
    endif ()
endif ()

foreach (module IN LISTS QT_MODULES)
    get_target_property(module_path Qt${V}::${module} LOCATION)
    message(STATUS "Found Qt${V}::${module}: ${module_path}")
endforeach ()

file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(REMOVE_ITEM SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_glcanvas.cpp"
)
file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
list(REMOVE_ITEM HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_glcanvas.h"
)

if (MSVC)
    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
endif ()

add_library(qwt_static STATIC ${SOURCES} ${HEADERS})
if (MSVC AND BUILD_SHARED_LIBS)
    set_target_properties(qwt_static PROPERTIES OUTPUT_NAME "qwt_static")
else ()
    set_target_properties(qwt_static PROPERTIES OUTPUT_NAME "qwt")
endif ()
foreach (module IN LISTS QT_MODULES)
    target_link_libraries(qwt_static PUBLIC Qt${V}::${module})
endforeach ()

if (BUILD_SHARED_LIBS)
    add_library(qwt_shared SHARED ${SOURCES} ${HEADERS})
    set_target_properties(qwt_shared PROPERTIES
        VERSION ${qwt_VERSION}
        SOVERSION ${qwt_VERSION_MAJOR}.${qwt_VERSION_MINOR}
        OUTPUT_NAME "qwt"
    )
    if (MSVC)
        target_compile_definitions(qwt_shared PUBLIC QWT_DLL PRIVATE QWT_MAKEDLL)
    endif ()
    foreach (module IN LISTS QT_MODULES)
        target_link_libraries(qwt_shared PUBLIC Qt${V}::${module})
    endforeach ()
endif ()

include(GNUInstallDirs)
install(TARGETS qwt_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
if (BUILD_SHARED_LIBS)
    install(TARGETS qwt_shared
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif ()
install(FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qwt)
