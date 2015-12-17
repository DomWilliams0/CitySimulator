cmake_minimum_required(VERSION 3.3)
project(CitySimulator_tests)

# gtest
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# unit tests
set(TEST_FILES
        test_helpers.hpp
        util.cpp
        world.cpp
        )

include_directories(${CITYSIMULATOR_SOURCE_DIR}/include)
add_executable(${PROJECT_NAME} ${TEST_FILES})

target_link_libraries(CitySimulator_tests gtest gtest_main)
target_link_libraries(CitySimulator_tests CitySimulator)