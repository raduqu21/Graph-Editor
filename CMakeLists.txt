cmake_minimum_required(VERSION 3.16)

project(proiectinfo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES proiectinfo_ro_RO.ts)

set(RES_FILES "")
if(WIN32)
    set(RES_FILES "resurse.rc")
endif()

set(PROJECT_SOURCES
    main.cpp
    window.cpp
    window.h
    window.ui
    ${TS_FILES}
    ${RES_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(proiectinfo
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(proiectinfo SHARED ${PROJECT_SOURCES})
    else()
        add_executable(proiectinfo ${PROJECT_SOURCES})
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(proiectinfo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(proiectinfo PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(proiectinfo)
endif()
