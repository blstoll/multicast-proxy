cmake_minimum_required(VERSION 3.15)
project(multicast_proxy)

set(CMAKE_CXX_STANDARD 14)

add_executable(multicast_proxy server.cpp messages.h multicast_registry.cpp multicast_registry.h registration.cpp registration.h constants.h registry.cpp registry.h)
add_executable(client_proxy client.cpp messages.h constants.h)
