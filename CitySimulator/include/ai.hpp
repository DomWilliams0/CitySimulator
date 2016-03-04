#ifndef CITYSIMULATOR_AI_HPP
#define CITYSIMULATOR_AI_HPP

#include "input.hpp"
#include "ecs.hpp"
#include "service/input_service.hpp"

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
	StupidAIBrain(EntityID e) : EntityBrain(e), direction(DIRECTION_SOUTH),
								random(Utils::random(0.f, 1.f) < 0.5f), ticker(0.05f, 0.25f)
	{
	}

	void tick(float delta) override;

private:
	Utils::TimeTicker ticker;
	DirectionType direction;

	bool random;
};

#endif
