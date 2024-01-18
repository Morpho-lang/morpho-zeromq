cmake_minimum_required(VERSION 3.13)
project(morpho-zeromq)

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
          )

# Identify folder that morpho.h is located in from MORPHO_HEADER and store in MORPHO_INCLUDE
get_filename_component(MORPHO_INCLUDE ${MORPHO_HEADER} DIRECTORY)

# Add morpho headers to MORPHO_INCLUDE
target_include_directories(zeromq PUBLIC ${MORPHO_INCLUDE})

# Add general 
target_include_directories(zeromq PUBLIC /usr/local/include)

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
)

# Locate zeromq
find_library(ZMQ_LIBRARY
    NAMES zmq libzmq
)

# Locate czmq
find_library(CZMQ_LIBRARY
    NAMES czmq libczmq
)

target_link_libraries(zeromq ${MORPHO_LIBRARY} ${ZMQ_LIBRARY} ${CZMQ_LIBRARY})

set(CMAKE_INSTALL_PREFIX ..)

# Install the resulting binary
install(TARGETS zeromq LIBRARY DESTINATION lib/)