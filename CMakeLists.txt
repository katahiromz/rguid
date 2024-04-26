# CMake settings for guid

# CMake minimum version
cmake_minimum_required(VERSION 3.5)

# project name and languages
project(rguid CXX)

##############################################################################
# options

option(RGUID_WANT_EXE "Do you want rguid.exe?" ON)
option(RGUID_USE_WON32 "Do you use Won32?" OFF)

##############################################################################

# libguid.a
add_library(guid STATIC guid.cpp StringFromGUID2.cpp CLSIDFromString.cpp)
if(RGUID_USE_WON32)
    target_compile_definitions(guid PRIVATE _WON32)
endif()

if(RGUID_WANT_EXE)
    # rguid.exe
    add_executable(rguid rguid.cpp)
    target_compile_definitions(rguid PRIVATE UNICODE _UNICODE)
    target_link_libraries(rguid PRIVATE shlwapi guid)
endif()

##############################################################################
