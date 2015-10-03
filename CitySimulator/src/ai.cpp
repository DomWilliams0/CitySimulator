#include "ai.hpp"
#include "entity.hpp"

void EntityBrain::turnTowards(DirectionType direction)
{
	ren->anim.turn(direction);
}

void EntityBrain::setMoving(bool moving)
{
	ren->anim.setPlaying(moving, true);
	vel->velocity.x = moving ? 20 : 0;
}

