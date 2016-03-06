#ifndef CITYSIMULATOR_AI_HPP
#define CITYSIMULATOR_AI_HPP

#include "input.hpp"
#include "ecs.hpp"
#include "service/input_service.hpp"

struct PhysicsComponent;

/**
 * A wrapper around a movement controller that can be controlled either by AI or the player
 */
class Brain
{
public:
	Brain(EntityID e);

	virtual ~Brain();

	/**
	 * Changes the entity that this brain is controlling
	 * @param e The new entity to control
	 * @param stop If the entity should stop in his tracks
	 */
	void setEntity(EntityID e, bool stop = true);

	virtual void tick(float delta);

protected:
	EntityID entity;
	PhysicsComponent *phys;
	MovementController controller;

	virtual void onEnable()
	{
	}

	virtual void onDisable()
	{
	}
};

/**
 * A brain with behaviours
 */
class EntityBrain : public Brain
{
public:
	EntityBrain(EntityID e);

	void tick(float delta) override;
};

/**
 * An empty brain that delegates all player inputs to the movement controller
 */
class InputBrain : public Brain
{
public:
	InputBrain(EntityID e) : Brain(e)
	{
		setEntity(e);
		controller.registerListeners();
	}

	virtual ~InputBrain()
	{
		controller.unregisterListeners();
	}
};

#endif
