#include "ai.hpp"
#include "entity.hpp"

void InputBrain::tick(float delta)
{
	// walking
	bool right = input->isPressed(KEY_RIGHT);
	bool left = input->isPressed(KEY_LEFT);
	bool down = input->isPressed(KEY_DOWN);
	bool up = input->isPressed(KEY_UP);

	if (right != left)
		motion->steeringLinear.x = right ? 1.f : -1.f;
	else
		motion->steeringLinear.x = 0.f;

	if (down != up)
		motion->steeringLinear.y = down ? 1.f : -1.f;
	else
		motion->steeringLinear.y = 0.f;
}

void AIBrain::tick(float delta)
{
	// do things
}
