cmake_minimum_required(VERSION 3.10)

project(AsyncStackDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG  "-g")

add_subdirectory(libunifex)

add_executable(AsyncStackDemo src/main.cpp)

target_link_directories(AsyncStackDemo PRIVATE "libunifex/source")

target_link_libraries(AsyncStackDemo PRIVATE "unifex")
