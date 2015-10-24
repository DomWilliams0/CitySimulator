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


	// debug
	if (input->isFirstPressed(KEY_YIELD_CONTROL))
	{
		// todo get tile position from collider component instead, which will give the centre instead of corner
		sf::Vector2i tilePos(motion->getTilePosition());

		sf::FloatRect rect;
		if (motion->world->getCollisionMap().getRectAt(tilePos, rect))
			printf("at %d,%d which is %d, %d\n", tilePos.x, tilePos.y, (int)rect.left, (int)rect.top);
		else
			printf("nop\n");
	}

}

void AIBrain::tick(float delta)
{
	// do things
}
