#ifndef CITYSIMULATOR_INPUT_HPP
#define CITYSIMULATOR_INPUT_HPP

#include "events.hpp"


enum InputKey
{
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,

	KEY_YIELD_CONTROL,
	KEY_SPRINT,

	KEY_EXIT,

	KEY_UNKNOWN
};

class MovementController : public EventListener
{
public:
	MovementController(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed)
	{
		reset(entity, movementForce, maxWalkSpeed, maxSprintSpeed);
	}

	~MovementController()
	{
		unregisterListeners();
	}

	void registerListeners();

	void unregisterListeners();

	b2Vec2 tick(float delta, float &newMaxSpeed);

	void tick(PhysicsComponent *phys, float delta);

	virtual void onEvent(const Event &event) override;

	void reset(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed);

	void halt();

private:
	EntityID entity;

	b2Vec2 steering;
	bool running;
	float movementForce, maxSprintSpeed, maxSpeed;

};


#endif
