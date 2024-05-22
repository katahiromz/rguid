# CMake settings for guid

# CMake minimum version
cmake_minimum_required(VERSION 3.5)

# project name and languages
project(rguid CXX)

##############################################################################
# options

option(RGUID_WANT_EXE "Do you want rguid.exe?" OFF)
option(RGUID_USE_WON32 "Do you use Won32?" OFF)
option(RGUID_VERBOSE "Verbose mode" ON)

##############################################################################

# libguid.a
add_library(guid STATIC guid.cpp WonStringFromGUID2.cpp WonCLSIDFromString.cpp)
if(RGUID_USE_WON32)
    target_compile_definitions(guid PRIVATE _WON32)
endif()

if(RGUID_USE_WON32)
    set(RGUID_DEFINITIONS "_WON32")
endif()
set(RGUID_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
set(RGUID_LIBRARIES guid)

if(RGUID_VERBOSE)
    message(STATUS "RGUID_DEFINITIONS: ${RGUID_DEFINITIONS}")
    message(STATUS "RGUID_INCLUDE_DIRS: ${RGUID_INCLUDE_DIRS}")
    message(STATUS "RGUID_LIBRARIES: ${RGUID_LIBRARIES}")
endif()

# Usage:
# add_subdirectory(rguid)
# get_directory_property(RGUID_DEFINITIONS DIRECTORY rguid DEFINITION RGUID_DEFINITIONS)
# get_directory_property(RGUID_INCLUDE_DIRS DIRECTORY rguid DEFINITION RGUID_INCLUDE_DIRS)
# get_directory_property(RGUID_LIBRARIES DIRECTORY rguid DEFINITION RGUID_LIBRARIES)

##############################################################################

if(RGUID_WANT_EXE)
    # rguid.exe
    add_executable(rguid rguid.cpp guid.cpp WonStringFromGUID2.cpp WonCLSIDFromString.cpp)
    target_compile_definitions(rguid PRIVATE ${RGUID_DEFINITIONS})
    target_include_directories(rguid PRIVATE ${RGUID_INCLUDE_DIRS})
    target_link_libraries(rguid PRIVATE shlwapi)
endif()

##############################################################################
