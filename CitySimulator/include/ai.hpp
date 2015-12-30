#ifndef CITYSIM_AI_HPP
#define CITYSIM_AI_HPP

#include "entity.hpp"
#include "constants.hpp"
#include "services.hpp"

class EntityBrain
{
public:
	explicit EntityBrain(Entity e)
	{
		entity = e;
//		todo phys = Globals::entityManager->getComponent<PhysicsComponent>(e, COMPONENT_PHYSICS);
		phys = nullptr;
		movementForce = Config::getFloat("debug.movement.force");
	}

	virtual ~EntityBrain()
	{
	}

	virtual void tick(float delta) = 0;

protected:
	Entity entity;

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
	explicit InputBrain(Entity e)
			: EntityBrain(e)
	{
	}

	void tick(float delta) override;
};

class AIBrain : public EntityBrain
{
public:
	explicit AIBrain(Entity e)
			: EntityBrain(e)
	{
	}

	void tick(float delta) override;
};

#endif
