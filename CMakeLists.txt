cmake_minimum_required(VERSION 3.3)
project(CitySimulator)

# flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -DDEBUG")

# debug
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# version
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)

# game executable
add_executable(CitySimulator_run game.cpp)
include_directories(CitySimulator/include)

add_subdirectory(CitySimulator)
add_subdirectory(CitySimulator_tests)

# link to game "library"
target_link_libraries(CitySimulator_run CitySimulator)