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

	void setEntity(EntityID e, bool stop = true);

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
		setEntity(e);
		controller.registerListeners();
	}

	virtual ~InputBrain()
	{
		controller.unregisterListeners();
	}
};

class StupidAIBrain : public EntityBrain
{
public:
	StupidAIBrain(EntityID e) : EntityBrain(e), direction(DIRECTION_SOUTH)
	{
		ticker.init(0.05f, 0.25f);
	}

	void tick(float delta) override;

private:
	Utils::TimeTicker ticker;
	DirectionType direction;
};

#endif
