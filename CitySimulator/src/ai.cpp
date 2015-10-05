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


	// print out current block
	if (input->isFirstPressed(KEY_YIELD_CONTROL))
	{
		// todo get tile position from collider component instead, which will give the centre instead of corner
		sf::Vector2i tilePos(motion->getTilePosition());
		BlockType blockType = motion->world->getBlockAt(tilePos);

		printf("at %d,%d which is %d\n", tilePos.x, tilePos.y, blockType);
	}

}

void AIBrain::tick(float delta)
{
	// do things
}
