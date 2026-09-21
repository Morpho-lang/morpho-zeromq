cmake_minimum_required(VERSION 3.13)
project(morpho-zeromq)

# Morpho loads plugin symbols (zeromq_initialize) at runtime.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Build the library as a plugin
add_library(zeromq MODULE "")

# Suppress 'lib' prefix
set_target_properties(zeromq PROPERTIES PREFIX "")

# Add sources
add_subdirectory(src)

# Locate the morpho.h header file and store in MORPHO_HEADER
find_file(MORPHO_HEADER
          morpho.h
          HINTS
          /usr/local/opt/morpho
          /opt/homebrew/opt/morpho
          /usr/local/include/morpho
          /opt/homebrew/include/morpho
          /home/linuxbrew/.linuxbrew/include/morpho
          "C:/Program Files/Morpho/include/morpho"
          )

# Identify folder that morpho.h is located in from MORPHO_HEADER and store in MORPHO_INCLUDE
get_filename_component(MORPHO_INCLUDE ${MORPHO_HEADER} DIRECTORY)

# Add morpho headers to MORPHO_INCLUDE
target_include_directories(zeromq PUBLIC ${MORPHO_INCLUDE})

# Add general header search paths
target_include_directories(zeromq PUBLIC
    /usr/local/include
    /opt/homebrew/include
    "C:/Program Files/Morpho/include"
)

# Add morpho headers in subfolders to MORPHO_INCLUDE
file(GLOB morpho_subdirectories LIST_DIRECTORIES true ${MORPHO_INCLUDE}/*)
foreach(dir ${morpho_subdirectories})
    IF(IS_DIRECTORY ${dir})
        target_include_directories(zeromq PUBLIC ${dir})
    ELSE()
        CONTINUE()
    ENDIF()
endforeach()

# Locate libmorpho
find_library(MORPHO_LIBRARY
    NAMES morpho libmorpho
    HINTS
    "C:/Program Files/Morpho/lib/"
)

# Prefer pkg-config for libzmq / libczmq when available
set(ZMQ_LINK_LIBS "")
set(CZMQ_LINK_LIBS "")

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(ZMQ QUIET IMPORTED_TARGET libzmq)
    if(ZMQ_FOUND)
        set(ZMQ_LINK_LIBS PkgConfig::ZMQ)
    endif()

    pkg_check_modules(CZMQ QUIET IMPORTED_TARGET libczmq)
    if(CZMQ_FOUND)
        set(CZMQ_LINK_LIBS PkgConfig::CZMQ)
    endif()
endif()

if(NOT ZMQ_FOUND)
    find_path(ZMQ_INCLUDE_DIRS NAMES zmq.h)
    find_library(ZMQ_LIBRARIES NAMES zmq libzmq)
    if(ZMQ_INCLUDE_DIRS AND ZMQ_LIBRARIES)
        set(ZMQ_FOUND TRUE)
        set(ZMQ_LINK_LIBS ${ZMQ_LIBRARIES})
        target_include_directories(zeromq PUBLIC ${ZMQ_INCLUDE_DIRS})
    endif()
endif()

if(NOT CZMQ_FOUND)
    find_path(CZMQ_INCLUDE_DIRS NAMES czmq.h)
    find_library(CZMQ_LIBRARIES NAMES czmq libczmq)
    if(CZMQ_INCLUDE_DIRS AND CZMQ_LIBRARIES)
        set(CZMQ_FOUND TRUE)
        set(CZMQ_LINK_LIBS ${CZMQ_LIBRARIES})
        target_include_directories(zeromq PUBLIC ${CZMQ_INCLUDE_DIRS})
    endif()
endif()

if(NOT ZMQ_FOUND)
    message(FATAL_ERROR
        "Could not find ZeroMQ (libzmq). "
        "Install it (e.g. brew install zeromq / apt install libzmq3-dev) "
        "or set ZMQ_INCLUDE_DIR / ZMQ_LIBRARY.")
endif()

if(NOT CZMQ_FOUND)
    message(FATAL_ERROR
        "Could not find CZMQ (libczmq). "
        "Install it (e.g. brew install czmq / apt install libczmq-dev) "
        "or set CZMQ_INCLUDE_DIR / CZMQ_LIBRARY.")
endif()

target_link_libraries(zeromq ${MORPHO_LIBRARY} ${ZMQ_LINK_LIBS} ${CZMQ_LINK_LIBS})

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")

# Install the resulting binary. RUNTIME is needed so Windows copies zeromq.dll.
install(TARGETS zeromq
    LIBRARY DESTINATION lib/
    RUNTIME DESTINATION lib/
)
