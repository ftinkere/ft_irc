cmake_minimum_required(VERSION 3.20)
project(ft_irc)

set(CMAKE_CXX_STANDARD 98)

add_executable(ft_irc srcs/main.cpp srcs/ListeningSocket.cpp srcs/ListeningSocket.hpp srcs/Socket.cpp srcs/Socket.hpp srcs/Client.cpp srcs/Client.hpp srcs/Parser.cpp srcs/Parser.hpp srcs/Command.cpp srcs/Command.hpp)
add_executable(experimental srcs/experimental.cpp)
