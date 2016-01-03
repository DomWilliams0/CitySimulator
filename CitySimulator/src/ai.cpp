#include "ai.hpp"


EntityBrain::EntityBrain(EntityID e) : entity(e)
{
	phys = Locator::locate<EntityService>()->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);
}

InputBrain::InputBrain(EntityID e) : EntityBrain(e), moving(DIRECTION_COUNT, false)
{
	EventService *es = Locator::locate<EventService>();
	es->registerListener(this, EVENT_INPUT_START_MOVING);
	es->registerListener(this, EVENT_INPUT_STOP_MOVING);

	moving.shrink_to_fit();
}

void InputBrain::tick(float delta)
{
	float acceleration = Config::getFloat("debug.movement.force");

	// walking
	bool north = moving[DIRECTION_NORTH];
	bool south = moving[DIRECTION_SOUTH];
	bool east = moving[DIRECTION_EAST];
	bool west = moving[DIRECTION_WEST];

	float x, y;

	if (east != west)
		x = east ? acceleration : -acceleration;
	else
		x = 0.f;

	if (south != north)
		y = south ? acceleration : -acceleration;
	else
		y = 0.f;

	phys->steering.Set(x, y);
}



void InputBrain::onEvent(const Event &event)
{
	if (event.entityID != entity)
		return;

	bool start = event.type == EVENT_INPUT_START_MOVING;

	float dx = 0.f;
	float dy = 0.f;

	DirectionType direction = start ? event.startMove.direction : event.stopMove.direction;
//	switch (direction)
//	{
//		case DIRECTION_NORTH:
//			dy = -1;
//			break;
//		case DIRECTION_SOUTH:
//			dy = 1;
//			break;
//		case DIRECTION_WEST:
//			dx = -1;
//			break;
//		case DIRECTION_EAST:
//			dx = 1;
//			break;
//		default:
//			error("Invalid direction in human %1% moving event: %2%", start ? "start" : "stop",
//			      std::to_string(direction));
//	}

	moving[direction] = start;
}

void AIBrain::tick(float delta)
{
	// do things
}
