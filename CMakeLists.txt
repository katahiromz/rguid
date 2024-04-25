# CMake settings for guid

# CMake minimum version
cmake_minimum_required(VERSION 3.5)

# project name and languages
project(guid CXX)

##############################################################################

# libguid.a
add_library(guid STATIC guid.cpp)

# rguid.exe
add_executable(rguid rguid.cpp)
target_compile_definitions(rguid PRIVATE UNICODE _UNICODE)
target_link_libraries(rguid PRIVATE shlwapi guid)
