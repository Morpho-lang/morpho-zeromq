#-------------------------------------------------------------------------------
# morpho-zeromq/CMakeLists.txt
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

# Default to a Release build when no build type is set.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "CMAKE_BUILD_TYPE unset; defaulting to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

project(morpho-zeromq LANGUAGES C)

# Build the library as a plugin.
add_library(zeromq MODULE "")

# Suppress the 'lib' prefix.
set_target_properties(zeromq PROPERTIES PREFIX "")

# CZMQ includes the Windows socket headers.
if(WIN32)
    target_compile_definitions(zeromq PRIVATE WIN32_LEAN_AND_MEAN)
endif()

#-------------------------------------------------------------------------------
# Release LTO
#-------------------------------------------------------------------------------

include(CheckIPOSupported)

check_ipo_supported(
    RESULT ZEROMQ_IPO_SUPPORTED
    OUTPUT ZEROMQ_IPO_OUTPUT
    LANGUAGES C
)

if(ZEROMQ_IPO_SUPPORTED)
    set_property(TARGET zeromq PROPERTY
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    message(STATUS "IPO/LTO enabled for Release")
else()
    message(STATUS "IPO/LTO not enabled (${ZEROMQ_IPO_OUTPUT})")
endif()

#-------------------------------------------------------------------------------
# Sources
#-------------------------------------------------------------------------------

add_subdirectory(src)

#-------------------------------------------------------------------------------
# Dependencies
#-------------------------------------------------------------------------------

find_package(morpho CONFIG REQUIRED)

# The CZMQ CMake package exports `czmq` and pulls ZeroMQ in with it.
# Homebrew and Debian do not ship that package, so locate both libraries.
find_package(czmq CONFIG QUIET)
if(NOT TARGET czmq)
    find_path(CZMQ_INCLUDE_DIR NAMES czmq.h)
    find_library(CZMQ_LIBRARY NAMES czmq libczmq)
    find_path(ZMQ_INCLUDE_DIR NAMES zmq.h)
    find_library(ZMQ_LIBRARY NAMES zmq libzmq)

    if(NOT CZMQ_INCLUDE_DIR OR NOT CZMQ_LIBRARY
            OR NOT ZMQ_INCLUDE_DIR OR NOT ZMQ_LIBRARY)
        message(FATAL_ERROR "Could not find CZMQ and ZeroMQ.")
    endif()
endif()

#-------------------------------------------------------------------------------
# Linking
#-------------------------------------------------------------------------------

target_link_libraries(zeromq PRIVATE morpho::morpho)

if(TARGET czmq)
    target_link_libraries(zeromq PRIVATE czmq)
else()
    target_include_directories(zeromq PRIVATE
        ${CZMQ_INCLUDE_DIR}
        ${ZMQ_INCLUDE_DIR}
    )
    target_link_libraries(zeromq PRIVATE ${CZMQ_LIBRARY} ${ZMQ_LIBRARY})
endif()

#-------------------------------------------------------------------------------
# Installation
#-------------------------------------------------------------------------------

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}"
        CACHE PATH "Install prefix" FORCE)
endif()

# LIBRARY is the Unix plugin; RUNTIME is the Windows DLL. Both land in lib/.
install(TARGETS zeromq
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)
