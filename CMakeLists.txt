cmake_minimum_required(VERSION 3.10)

project(netcat VERSION 1.0 DESCRIPTION "Vlad's straightforward netcat" LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(netcat src/main.cpp)

if(WIN32)
  target_link_libraries(NetworkApp wsock32 ws2_32)
endif()
