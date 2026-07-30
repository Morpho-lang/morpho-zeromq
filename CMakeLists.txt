cmake_minimum_required(VERSION 3.13)
project(morpho-zeromq)

# Build the library as a plugin
add_library(zeromq MODULE "")

# Suppress 'lib' prefix
set_target_properties(zeromq PROPERTIES PREFIX "")

# Add sources
add_subdirectory(src)

find_path(MORPHO_INCLUDE_DIR
    NAMES morpho.h morpho/morpho.h
    HINTS
        $ENV{MORPHO_INCLUDE_DIR}
        /usr/local/opt/morpho/include
        /opt/homebrew/opt/morpho/include
        /home/linuxbrew/.linuxbrew/include/morpho
        /home/linuxbrew/.linuxbrew/include
        /usr/local/include/morpho
        /usr/local/include
        /opt/homebrew/include
)

find_library(MORPHO_LIBRARY
    NAMES morpho libmorpho
    HINTS
        $ENV{MORPHO_LIBRARY}
        /usr/local/opt/morpho/lib
        /opt/homebrew/opt/morpho/lib
        /home/linuxbrew/.linuxbrew/lib
        /usr/local/lib
        /opt/homebrew/lib
)

find_path(ZMQ_INCLUDE_DIR
    NAMES zmq.h
    HINTS
        /opt/homebrew/include
        /usr/local/include
        /home/linuxbrew/.linuxbrew/include
)

find_library(ZMQ_LIBRARY
    NAMES zmq libzmq
    HINTS
        /opt/homebrew/lib
        /usr/local/lib
        /home/linuxbrew/.linuxbrew/lib
)

find_path(CZMQ_INCLUDE_DIR
    NAMES czmq.h
    HINTS
        /opt/homebrew/include
        /usr/local/include
        /home/linuxbrew/.linuxbrew/include
)

find_library(CZMQ_LIBRARY
    NAMES czmq libczmq
    HINTS
        /opt/homebrew/lib
        /usr/local/lib
        /home/linuxbrew/.linuxbrew/lib
)

if(NOT MORPHO_INCLUDE_DIR)
    message(FATAL_ERROR
        "Could not find Morpho headers (morpho.h). "
        "Install Morpho or set MORPHO_INCLUDE_DIR to the include directory "
        "that contains morpho.h or morpho/morpho.h.")
endif()

if(NOT MORPHO_LIBRARY)
    message(FATAL_ERROR
        "Could not find libmorpho. "
        "Install Morpho or set MORPHO_LIBRARY to the full path of the Morpho library.")
endif()

if(NOT ZMQ_INCLUDE_DIR OR NOT ZMQ_LIBRARY)
    message(FATAL_ERROR
        "Could not find ZeroMQ (libzmq). "
        "Install it (e.g. brew install zeromq) or set ZMQ_INCLUDE_DIR / ZMQ_LIBRARY.")
endif()

if(NOT CZMQ_INCLUDE_DIR OR NOT CZMQ_LIBRARY)
    message(FATAL_ERROR
        "Could not find CZMQ (libczmq). "
        "Install it (e.g. brew install czmq) or set CZMQ_INCLUDE_DIR / CZMQ_LIBRARY.")
endif()

get_filename_component(MORPHO_INCLUDE_PARENT ${MORPHO_INCLUDE_DIR} DIRECTORY)

# Allow #include <morpho.h>, <classes.h>, <czmq.h>, etc.
target_include_directories(zeromq PUBLIC
    ${MORPHO_INCLUDE_DIR}
    ${MORPHO_INCLUDE_PARENT}
    ${ZMQ_INCLUDE_DIR}
    ${CZMQ_INCLUDE_DIR}
)

file(GLOB morpho_subdirectories LIST_DIRECTORIES true ${MORPHO_INCLUDE_DIR}/*)
foreach(dir ${morpho_subdirectories})
    if(IS_DIRECTORY ${dir})
        target_include_directories(zeromq PUBLIC ${dir})
    endif()
endforeach()

target_link_libraries(zeromq ${MORPHO_LIBRARY} ${ZMQ_LIBRARY} ${CZMQ_LIBRARY})

# Install into this package by default (Morpho looks under <package>/lib).
# Also override CMake's cached /usr/local default so make install can't escape the package.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Install prefix" FORCE)
endif()

install(TARGETS zeromq LIBRARY DESTINATION lib/)
