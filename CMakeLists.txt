cmake_minimum_required(VERSION 3.20)  # Updated minimum version
project(xletlib)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

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

# Fetch uvgRTP
FetchContent_Declare(
    uvgrtp
    GIT_REPOSITORY https://github.com/ultravideo/uvgRTP.git
    GIT_TAG        master  # Specify the branch, tag, or commit
)
FetchContent_GetProperties(uvgrtp)
if(NOT uvgrtp_POPULATED)
    FetchContent_Populate(uvgrtp)
    add_subdirectory(${uvgrtp_SOURCE_DIR} ${uvgrtp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Fetch nlohmann_json
FetchContent_Declare(
    signal_slots
    GIT_REPOSITORY https://github.com/cppdvl/signal_n_slots.git
    GIT_TAG        patriarchal-master
    )
FetchContent_MakeAvailable(signal_slots)

FetchContent_Declare(
        dotenv-cpp
        GIT_REPOSITORY git@github.com:laserpants/dotenv-cpp.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(dotenv-cpp)

# Include directories
include_directories(${uvgrtp_SOURCE_DIR}/include)
include_directories(${signal_slots_SOURCE_DIR})

# Define your library or executable target
add_library(xletlib STATIC
    # List your source files here
    xlet.h
    # Other source files...
    xlet.cpp
    udp.cpp
    uds.cpp
)

# Link libraries
target_link_libraries(xletlib
    PRIVATE
        uvgrtp
)

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
        router/relayrouter.cpp)
target_include_directories(relayrouter
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(relayrouter
    PRIVATE
        xletlib)

