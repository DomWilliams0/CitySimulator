#include "ai.hpp"
#include "entity.hpp"

void InputBrain::tick(float delta)
{
	if (input->isPressed(KEY_RIGHT))
		motion->steeringLinear.x = movementForce;
	else if (input->isPressed(KEY_LEFT))
		motion->steeringLinear.x = -movementForce;
	else
		motion->steeringLinear.x = 0;

	if (input->isPressed(KEY_DOWN))
		motion->steeringLinear.y = movementForce;
	else if (input->isPressed(KEY_UP))
		motion->steeringLinear.y = -movementForce;
	else
		motion->steeringLinear.y = 0;
}

void AIBrain::tick(float delta)
{
	// do things
}