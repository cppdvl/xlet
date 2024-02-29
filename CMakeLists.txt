cmake_minimum_required(VERSION 3.20)  # Updated minimum version
project(xletlib)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)


#SET MAC INTEL FLAGS
option (USE_MAC_INTEL "Use Mac Intel flags" OFF)
if (USE_MAC_INTEL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64")
endif ()

# Set the default build type to Debug if none was specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Enable verbose makefiles
set(CMAKE_VERBOSE_MAKEFILE ON)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include FetchContent module
include(FetchContent)

# Fetch nlohmann_json
FetchContent_Declare(
    signal_slots
    GIT_REPOSITORY https://github.com/cppdvl/signal_n_slots.git
    GIT_TAG        patriarchal-master
    )
FetchContent_MakeAvailable(signal_slots)

# Include directories
include_directories(${signal_slots_SOURCE_DIR})

# Define your library or executable target
add_library(xletlib STATIC
    # List your source files here
    xlet.h
    # Other source files...
    xlet.cpp
    udp.cpp
    uds.cpp
    router/relayrouter.cpp
)




target_include_directories(xletlib
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
# Link libraries

option (MAKE_EXAMPLES "Build examples" OFF)
if (MAKE_EXAMPLES)
    add_executable(udsexample
        examples/udsexample.cpp)

target_include_directories(udsexample
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(udsexample
    PRIVATE
        xletlib)

add_executable(udpexample
    examples/udpexample.cpp)
target_include_directories(udpexample
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(udpexample
    PRIVATE
        xletlib)

add_executable(relayrouter
        examples/relayrouter.cpp)
target_include_directories(relayrouter
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(relayrouter
    PRIVATE
        xletlib)
endif()

