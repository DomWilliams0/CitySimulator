#ifndef CITYSIM_AI_HPP
#define CITYSIM_AI_HPP

#include "entity.hpp"
#include "constants.hpp"
#include "services.hpp"

class EntityBrain
{
public:
	explicit EntityBrain(EntityID e)
	{
		entity = e;
		phys = Locator::locate<EntityService>()->getComponent<PhysicsComponent>(e, COMPONENT_PHYSICS);
		movementForce = Config::getFloat("debug.movement.force");
	}

	virtual ~EntityBrain()
	{
	}

	virtual void tick(float delta) = 0;

protected:
	EntityID entity;

	float movementForce;

//	void turnTowards(DirectionType direction);
//	void setMoving(bool moving);

	virtual void onEnable()
	{
	}

	virtual void onDisable()
	{
	}

	PhysicsComponent *phys;
};

class InputBrain : public EntityBrain
{
public:
	explicit InputBrain(EntityID e)
			: EntityBrain(e)
	{
	}

	void tick(float delta) override;
};

class AIBrain : public EntityBrain
{
public:
	explicit AIBrain(EntityID e)
			: EntityBrain(e)
	{
	}

	void tick(float delta) override;
};

#endif
