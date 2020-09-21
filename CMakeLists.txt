cmake_minimum_required(VERSION 3.15)
project(StaticServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -g -Wall")

include_directories(include)

set (SOURCE
        src/misc.cpp
        src/socket.cpp
        src/http.cpp
        src/file.cpp
        )

add_executable(StaticServer main.cpp ${SOURCE})