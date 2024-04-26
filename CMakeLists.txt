# CMake settings for guid

# CMake minimum version
cmake_minimum_required(VERSION 3.5)

# project name and languages
project(rguid CXX)

##############################################################################

option(WANT_RGUID_EXE "Do you want rguid.exe?" ON)

##############################################################################

# libguid.a
add_library(guid STATIC guid.cpp StringFromGUID2.cpp CLSIDFromString.cpp)
target_compile_definitions(guid PRIVATE UNICODE _UNICODE _WON32)

if(WANT_RGUID_EXE)
    # rguid.exe
    add_executable(rguid rguid.cpp)
    target_compile_definitions(rguid PRIVATE UNICODE _UNICODE)
    target_link_libraries(rguid PRIVATE shlwapi guid)
endif()

##############################################################################
