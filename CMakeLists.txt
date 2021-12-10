cmake_minimum_required(VERSION 3.20)
project(ft_irc)

set(CMAKE_CXX_STANDARD 98)

include_directories(includes)

add_executable(ft_irc srcs/main.cpp srcs/ListenSocket.cpp includes/ListenSocket.hpp srcs/Socket.cpp includes/Socket.hpp srcs/Client.cpp includes/Client.hpp includes/Parser.hpp srcs/Command.cpp includes/Command.hpp includes/commands.hpp srcs/commands/register.cpp srcs/Reply.cpp includes/Reply.hpp srcs/Channel.cpp includes/Channel.hpp srcs/commands/messages.cpp srcs/commands/monitoring.cpp srcs/commands/channels.cpp srcs/commands/oper.cpp srcs/commands/utils.cpp includes/types.hpp srcs/commands/pingpong.cpp)
#add_executable(experimental srcs/experimental.cpp srcs/ListenSocket.cpp includes/ListenSocket.hpp srcs/Socket.cpp includes/Socket.hpp srcs/Client.cpp includes/Client.hpp srcs/Parser.cpp includes/Parser.hpp srcs/Command.cpp includes/Command.hpp includes/commands.hpp srcs/commands/register.cpp)
add_executable(experimental srcs/experimental.cpp)
