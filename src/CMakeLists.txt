##
## Copyright 2020 Dmitry Sokolov <mr.dmitry.sokolov@gmail.com>
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

cmake_minimum_required(VERSION 3.16)

project(qLogcat LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY CACHE)
get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" DIRECTORY CACHE)


set(SOURCE_FILES
    pch.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    logcatdatamodel.cpp
    logcatdatamodel.h
    logcatdatamodel_def.h
    logcatfilterproxy.cpp
    logcatfilterproxy.h
)

if(ANDROID)
  add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
else()
  add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

target_precompile_headers(${PROJECT_NAME}
    PRIVATE pch.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

if(NOT (ANDROID OR IOS))
    # To allow app running from IDE
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qt.conf "[Paths]\nPrefix = ${QT_ROOT_DIR}\n")
endif()


if(NOT (ANDROID OR IOS))
    get_filename_component(MY_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../Install" ABSOLUTE CACHE)
    set(CMAKE_INSTALL_PREFIX ${MY_INSTALL_DIR} CACHE PATH "" FORCE)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME})
endif()

if(WIN32)
    install(CODE "execute_process(COMMAND \"${QT_BIN_DIR}/windeployqt.exe\" --dir \"${CMAKE_INSTALL_BINDIR}\" \"$<TARGET_FILE:${PROJECT_NAME}>\")")
endif()
