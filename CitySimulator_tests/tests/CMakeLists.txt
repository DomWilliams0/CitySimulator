cmake_minimum_required(VERSION 3.3)
project(CitySimulator_tests)

# gtest
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# unit tests
set(TEST_FILES
        tests.cpp
        test_helpers.hpp
        config_tests.cpp
        services_test.cpp
        entity_tests.cpp
        utils_tests.cpp
        world_tests.cpp
        )

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CITYSIMULATOR_SOURCE_DIR}/include)
add_executable(${PROJECT_NAME} ${TEST_FILES})

target_link_libraries(CitySimulator_tests gtest gtest_main)
target_link_libraries(CitySimulator_tests CitySimulator)