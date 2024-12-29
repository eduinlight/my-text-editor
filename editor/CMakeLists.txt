cmake_minimum_required(VERSION 3.22)

project($ENV{PROJECT_NAME} VERSION $ENV{PROJECT_VERSION} LANGUAGES CXX)

# for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# specify the C standard c++17
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# FLAGS
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})

include(cmake/CPM.cmake)
