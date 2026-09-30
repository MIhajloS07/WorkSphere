cmake_minimum_required(VERSION 3.16)

project(WorkSphere VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

add_executable(WorkSphere
    main.cpp
    workspherewindow.cpp
    workspherewindow.h
    workspherewindow.ui
    models/Manager.h
    models/Manager.cpp
    models/Employee.h
    models/Employee.cpp
    database/Database.h
    database/Database.cpp
    models/Worker.h
    models/Worker.cpp
    models/Project.h
    models/Project.cpp
    widgetmodels/CircularProgressWidget.h
    widgetmodels/CircularProgressWidget.cpp
)

target_link_libraries(WorkSphere PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WorkSphere)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/logo.svg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

