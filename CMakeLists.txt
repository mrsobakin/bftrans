cmake_minimum_required(VERSION 3.10)

project(bftrans)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(lib/asm)
add_subdirectory(lib/labels)

target_link_libraries(${PROJECT_NAME} PRIVATE asm labels)
target_include_directories(${PROJECT_NAME} PRIVATE .)
