cmake_minimum_required(VERSION 3.3)
project(CitySimulator)

# source files
set(SOURCE_FILES
        src/ai.cpp
        src/animation.cpp
        src/camera.cpp
        src/config.cpp
        src/constants.cpp
        src/ecs.cpp
        src/entity.cpp
        src/events.cpp
        src/fps.cpp
        src/game.cpp
        src/gamestate.cpp
        src/input.cpp
        src/logger.cpp
        src/maploader.cpp
        src/rendering.cpp
        src/services.cpp
        src/SFMLDebugDraw.cpp
        src/utils.cpp
        src/world.cpp
        include/ai.hpp
        include/animation.hpp
        include/bodydata.hpp
        include/config.hpp
        include/constants.hpp
        include/entity.hpp
        include/events.hpp
        include/game.hpp
        include/gamestate.hpp
        include/maploader.hpp
        include/PackingTreeNode.h
        include/services.hpp
        include/SFMLDebugDraw.h
        include/state.hpp
        include/utils.hpp
        include/world.hpp
        )

# create as static library
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

# sfml
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML REQUIRED system window graphics)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
endif()

# boost::filesystem
find_package(Boost 1.59.0 REQUIRED system filesystem)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

# box2d
find_package(Box2D REQUIRED)
if(BOX2D_FOUND)
    include_directories(${BOX2D_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${BOX2D_LIBRARIES})
endif()