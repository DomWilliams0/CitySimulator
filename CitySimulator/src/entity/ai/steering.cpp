#include <ai.hpp>

PhysicsComponent *BaseSteering::getEntity() const
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

double BaseTargetedSteering::getDistanceSqrd(const sf::Vector2f &entityPos) const
{
	float dx = target.x - entityPos.x;
	float dy = target.y - entityPos.y;
	return dx * dx + dy * dy;
}

void SeekSteering::tick(b2Vec2 &steeringOut, float delta)
{
	const sf::Vector2f &pos = entity->getTilePosition();
	steeringOut.Set(target.x - pos.x, target.y - pos.y);
	steeringOut.Normalize();
}

void ArriveSteering::tick(b2Vec2 &steeringOut, float delta)
{
	double distance = getDistanceSqrd(entity->getTilePosition());

	// arrived
	if (distance <= arrivalThreshold)
	{
		steeringOut.SetZero();
		return;
	}

	// seek
	SeekSteering::tick(steeringOut, delta);

	// slow down
	if (distance <= deaccelerationDistance)
	{
		double scale = distance / deaccelerationDistance;
		steeringOut *= scale;
	}
}
