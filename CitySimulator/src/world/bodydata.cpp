#include "bodydata.hpp"
#include "service/logging_service.hpp"

void interactWithSlidingDoor(EntityIdentifier entity, sf::Vector2i tilePos)
{
	Logger::logDebuggier(format("Entity %1% interacted with a door at (%2%, %3%)",
								_str(entity.id), _str(tilePos.x), _str(tilePos.y)));
}

