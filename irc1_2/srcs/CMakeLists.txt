cmake_minimum_required(VERSION 3.20)
project(srcs)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(srcs
        Client.cpp
        Client.hpp
        ListeningSocket.cpp
        ListeningSocket.hpp
        main.cpp
        Parser.cpp
        Parser.hpp
        Socket.cpp
        Socket.hpp)
