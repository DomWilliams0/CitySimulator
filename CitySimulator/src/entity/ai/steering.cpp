#include "ai.hpp"

PhysicsComponent * BaseSteering::getEntity() const
{
	return entity;
}

void BaseSteering::setEntity(PhysicsComponent *entity)
{
	this->entity = entity;
}

const sf::Vector2f &BaseTargetedSteering::getTarget() const
{
	return target;
}

void BaseTargetedSteering::setTarget(const sf::Vector2f &target)
{
	this->target = target;
}

void SeekSteering::tick(b2Vec2 &steeringOut, float delta)
{
	// todo
}

void ArriveSteering::tick(b2Vec2 &steeringOut, float delta)
{
	// todo
}
