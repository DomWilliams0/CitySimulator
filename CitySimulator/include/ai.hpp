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
	virtual ~Brain()
	{
	}

	/**
	 * Changes the entity that this brain is controlling
	 * @param e The new entity to control
	 * @param stop If the entity should stop in his tracks
	 */
	void setEntity(EntityID e, bool stop = true);

	void tick(float delta);


protected:
	EntityID entity;
	PhysicsComponent *phys;
	boost::shared_ptr<MovementController> controller;

	virtual void initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed) = 0;

	virtual void tickBrain(float delta)
	{
	}

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

protected:
	virtual void initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed);

	void tickBrain(float delta) override;
};

/**
 * An empty brain that delegates all player inputs to the movement controller
 */
class InputBrain : public Brain
{
public:
	InputBrain(EntityID e);

	virtual ~InputBrain();

protected:
	virtual void initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed);
};

// steering behaviours

/**
 * An interface for steering behaviours
 */
class BaseSteering
{
public:
	BaseSteering(PhysicsComponent &entity) : entity(entity)
	{
	}

	virtual ~BaseSteering()
	{
	}

	virtual void tick(b2Vec2 &steeringOut, float delta) = 0;

	PhysicsComponent &getEntity() const;

	void setEntity(PhysicsComponent &entity);

protected:
	PhysicsComponent &entity;
};

/**
 * A steering behaviour with a target position
 */
class BaseTargetedSteering : public BaseSteering
{

public:
	BaseTargetedSteering(PhysicsComponent &entity, const sf::Vector2f &target) :
			BaseSteering(entity), target(target)
	{ }

private:
	virtual ~BaseTargetedSteering()
	{
	}

	virtual void tick(b2Vec2 &steeringOut, float delta) = 0;

	const sf::Vector2f &getTarget() const;

	void setTarget(const sf::Vector2f &target);

protected:
	sf::Vector2f target;
};

/**
 * A steering behaviour to seek directly to the target
 */
class SeekSteering : public BaseTargetedSteering
{

public:
	SeekSteering(PhysicsComponent &entity, const sf::Vector2f &target) : BaseTargetedSteering(entity, target)
	{ }

	virtual void tick(b2Vec2 &steeringOut, float delta);
};

/**
 * A steering behaviour to seek to and stop at the target
 */
class ArriveSteering : public BaseTargetedSteering
{

public:
	ArriveSteering(PhysicsComponent &entity, const sf::Vector2f &target) : BaseTargetedSteering(entity, target)
	{ }

	virtual void tick(b2Vec2 &steeringOut, float delta);
};

#endif