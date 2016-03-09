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
	const sf::Vector2f &pos = entity->getTilePosition();
	steeringOut.Set(target.x - pos.x, target.y - pos.y);
	steeringOut.Normalize();
}

void ArriveSteering::tick(b2Vec2 &steeringOut, float delta)
{
	// todo
}
