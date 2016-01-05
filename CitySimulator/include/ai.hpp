#ifndef CITYSIM_AI_HPP
#define CITYSIM_AI_HPP

#include "entity.hpp"
#include "services.hpp"

struct PhysicsComponent;

class EntityBrain
{
public:
	EntityBrain(EntityID e);

	virtual ~EntityBrain();

	virtual void tick(float delta);

protected:
	EntityID entity;
	SimpleMovementController controller;
	PhysicsComponent *phys;


	virtual void onEnable()
	{
	}

	virtual void onDisable()
	{
	}

//	void turnTowards(DirectionType direction);
//	void setMoving(bool moving);
	void setMoving(bool moving, DirectionType direction);

};

class InputBrain : public EntityBrain
{
public:
	InputBrain(EntityID e) : EntityBrain(e)
	{
		controller.registerListeners();
	}

	virtual ~InputBrain()
	{
		controller.unregisterListeners();
	}
};

class AIBrain : public EntityBrain
{
public:
	AIBrain(EntityID e) : EntityBrain(e)
	{
	}

//	void tick(float delta) override;
};

#endif
