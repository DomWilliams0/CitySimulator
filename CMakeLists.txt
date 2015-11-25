cmake_minimum_required(VERSION 3.3)
project(CitySimulator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# debug
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# version
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)

# source files
include_directories(${CMAKE_SOURCE_DIR}/include)
set(SOURCE_FILES
        src/ai.cpp
        src/animation.cpp
        src/config.cpp
        src/constants.cpp
        src/core.cpp
        src/ecs.cpp
        src/entity.cpp
        src/fps.cpp
        src/game.cpp
        src/gamestate.cpp
        src/input.cpp
        src/logger.cpp
        src/maploader.cpp
        src/SFMLDebugDraw.cpp
        src/utils.cpp
        src/world.cpp
        )

add_executable(CitySimulator ${SOURCE_FILES})

# sfml
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML REQUIRED system window graphics)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
endif()

# boost::filesystem
find_package(Boost 1.34.0 REQUIRED system filesystem)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

# box2d
find_package(Box2D REQUIRED)
if (BOX2D_FOUND)
    include_directories(${BOX2D_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${BOX2D_LIBRARIES})
endif()
