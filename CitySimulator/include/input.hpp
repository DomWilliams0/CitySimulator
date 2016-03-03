#ifndef CITYSIM_INPUT_HPP
#define CITYSIM_INPUT_HPP

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

class SimpleMovementController : public EventListener
{
public:
	SimpleMovementController(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed)
	{
		reset(entity, movementForce, maxWalkSpeed, maxSprintSpeed);
	}

	~SimpleMovementController()
	{
		unregisterListeners();
	}

	void registerListeners();

	void unregisterListeners();

	b2Vec2 tick(float delta, float &newMaxSpeed);

	void tick(PhysicsComponent *phys, float delta);

	virtual void onEvent(const Event &event) override;

	void reset(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed);

private:
	EntityID entity;

	std::vector<bool> moving;
	bool running, wasRunning;
	float movementForce, maxSprintSpeed, maxSpeed;

};


#endif
