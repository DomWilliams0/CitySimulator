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

class MovementController
{
public:
	MovementController()
	{
	}

	MovementController(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed)
	{
		reset(entity, movementForce, maxWalkSpeed, maxSprintSpeed);
	}

	virtual void reset(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed);

	virtual b2Vec2 tick(float delta, float &newMaxSpeed) = 0;

	void tick(PhysicsComponent *phys, float delta);

	virtual void halt() = 0;

protected:
	EntityID entity;
	bool running;
	float movementForce, maxWalkSpeed, maxSprintSpeed;

};


class DynamicMovementController : public MovementController
{
public:

	DynamicMovementController() : MovementController()
	{ }

	DynamicMovementController(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed)
			: MovementController(entity, movementForce, maxWalkSpeed, maxSprintSpeed)
	{ }

	virtual b2Vec2 tick(float delta, float &newMaxSpeed) override;

	/**
	 * Adds the given vector to the accumulated forces, which will be applied to the entity's steering force
	 */
	void move(const sf::Vector2f &vector);

	virtual void halt() override;

private:
	b2Vec2 steering;
};

class PlayerMovementController : public MovementController, EventListener
{
public:

	PlayerMovementController()
	{
		init();
	}

	PlayerMovementController(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed)
			: MovementController(entity, movementForce, maxWalkSpeed, maxSprintSpeed)
	{
		init();
	}


	virtual ~PlayerMovementController();

	void registerListeners();

	void unregisterListeners();

	virtual b2Vec2 tick(float delta, float &newMaxSpeed) override;

	virtual void onEvent(const Event &event) override;

	virtual void halt() override;

	virtual void reset(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed) override;

private:
	void init();
	std::vector<bool> moving;
};

#endif
