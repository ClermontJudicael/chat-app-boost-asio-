cmake_minimum_required(VERSION 3.16)
project(BoostAsioServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost (no components needed for header-only Asio)
find_package(Boost REQUIRED)

# Include directories
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/server
)

# Sources
set(SOURCES
    main.cpp
    server/TCPServer.cpp
)

# Executable
add_executable(server_app ${SOURCES})
