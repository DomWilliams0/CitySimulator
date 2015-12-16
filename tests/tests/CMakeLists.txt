include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(CitySimulatorTests
        util.cpp
        world.cpp)

target_link_libraries(CitySimulatorTests gtest gtest_main)