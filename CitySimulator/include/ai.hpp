#ifndef CITYSIMULATOR_AI_HPP
#define CITYSIMULATOR_AI_HPP

#include "input.hpp"
#include "ecs.hpp"
#include "service/input_service.hpp"

struct PhysicsComponent;

/**
 * An interface for steering behaviours
 */
class BaseSteering
{
public:
	BaseSteering()
	{
	}

	virtual ~BaseSteering()
	{
	}

	virtual void tick(b2Vec2 &steeringOut, float delta) = 0;

	PhysicsComponent *getEntity() const;

	void setEntity(PhysicsComponent *entity);

protected:
	PhysicsComponent *entity;
};

/**
 * A steering behaviour with a target position
 */
class BaseTargetedSteering : public BaseSteering
{
public:
	BaseTargetedSteering() : BaseSteering()
	{
	}

	virtual ~BaseTargetedSteering()
	{
	}

	virtual void tick(b2Vec2 &steeringOut, float delta) = 0;

	const sf::Vector2f &getTarget() const;

	void setTarget(const sf::Vector2f &target);

	double getDistanceSqrd(const sf::Vector2f &entityPos) const;

protected:
	sf::Vector2f target;
};

/**
 * A steering behaviour to seek directly to the target
 */
class SeekSteering : public BaseTargetedSteering
{
public:
	SeekSteering() : BaseTargetedSteering()
	{
	}

	virtual void tick(b2Vec2 &steeringOut, float delta);
};

/**
 * A steering behaviour to seek to and stop at the target
 */
class ArriveSteering : public SeekSteering
{
public:
	ArriveSteering() : SeekSteering()
	{
		arrivalThreshold = 0.25f;
		deaccelerationDistance = 1.f;
	}

	virtual void tick(b2Vec2 &steeringOut, float delta);

	void setArrivalThreshold(float arrivalThreshold)
	{
		this->arrivalThreshold = arrivalThreshold * arrivalThreshold;
	}

	void setDeaccelerationDistance(float deaccelerationDistance)
	{
		this->deaccelerationDistance = deaccelerationDistance * deaccelerationDistance;
	}

private:
	float arrivalThreshold;
	float deaccelerationDistance;
};

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
	//	boost::shared_ptr<MovementController> controller;

	virtual void initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed) = 0;

	virtual MovementController* getController() = 0;

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

// brains

/**
 * A brain with behaviours
 */
class EntityBrain : public Brain
{
public:
	EntityBrain(EntityID e);

protected:
	virtual void initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed) override;

	virtual MovementController *getController() override
	{
		return controller.get();
	}


	void tickBrain(float delta) override;

private:
	boost::shared_ptr<DynamicMovementController> controller;
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

	virtual MovementController *getController()
	{
		return controller.get();
	}


private:
	boost::shared_ptr<PlayerMovementController> controller;
};

#endif
