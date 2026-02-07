cmake_minimum_required(VERSION 4.0)
project(CleanGraduator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)
set(QT_VERSION_MAJOR 5)
find_package(OpenGL REQUIRED)

add_subdirectory(src/domain)
add_subdirectory(src/application)
add_subdirectory(src/infrastructure)
add_subdirectory(src/ui)
add_subdirectory(src/app)