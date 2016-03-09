cmake_minimum_required(VERSION 3.3)
project(CitySimulator)

# source files
set(SOURCE_FILES
        include/ai.hpp
        include/animation.hpp
        include/bodydata.hpp
        include/building.hpp
        include/config.hpp
        include/constants.hpp
        include/ecs.hpp
        include/events.hpp
        include/game.hpp
        include/input.hpp
        include/maploader.hpp
        include/PackingTreeNode.h
        include/SFMLDebugDraw.h
        include/service/animation_service.hpp
        include/service/base_service.hpp
        include/service/camera_service.hpp
        include/service/config_service.hpp
        include/service/entity_service.hpp
        include/service/event_service.hpp
        include/service/input_service.hpp
        include/service/locator.hpp
        include/service/logging_service.hpp
        include/service/render_service.hpp
        include/service/world_service.hpp
        include/state/gamestate.hpp
        include/state/state.hpp
        include/utils.hpp
        include/world.hpp
        src/entity/ai/ai.cpp
        src/entity/ai/steering.cpp
        src/entity/animation.cpp
        src/entity/ecs/component.cpp
        src/entity/ecs/system.cpp
        src/entity/entity.cpp
        src/game/camera.cpp
        src/game/events.cpp
        src/game/fps.cpp
        src/game/gamebase.cpp
        src/game/game.cpp
        src/game/input.cpp
        src/state/gamestate.cpp
        src/util/config.cpp
        src/util/constants.cpp
        src/util/logger.cpp
        src/util/services.cpp
        src/util/SFMLDebugDraw.cpp
        src/util/utils.cpp
        src/world/bodydata.cpp
        src/world/building.cpp
        src/world/maploader.cpp
        src/world/world.cpp
        src/world/world_rendering.cpp
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