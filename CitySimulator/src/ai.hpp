#pragma once
#include "entity.hpp"
#include "constants.hpp"
#include "input.hpp"

class EntityBrain
{
public:
	explicit EntityBrain(Entity e)
	{
		entity = e;
		pos = Globals::entityManager->getComponent<PositionComponent>(e, COMPONENT_POSITION);
		vel = Globals::entityManager->getComponent<VelocityComponent>(e, COMPONENT_VELOCITY);
		ren = Globals::entityManager->getComponent<RenderComponent>(e, COMPONENT_RENDER);
	}

	virtual ~EntityBrain()
	{
	}

	virtual void tick(float delta) = 0;

protected:
	Entity entity;

	void turnTowards(DirectionType direction);
	void setMoving(bool moving);

	bool isMoving() const
	{
		static sf::Vector2f zero;
		return vel->velocity != zero;
	}

	virtual void onEnable()
	{
	}

	virtual void onDisable()
	{
	}

	//	void init(Entity e);
	//	void reset();

private:
	PositionComponent *pos;
	VelocityComponent *vel;
	RenderComponent *ren;
	// todo direction -> position, start/stop -> velocity, remove render from brain
};

class InputBrain : public EntityBrain
{
public:
	explicit InputBrain(Entity e)
		: EntityBrain(e), input(Globals::input)
	{
	}

	void tick(float delta) override
	{
		// temporary movement toggling
		if (input->isFirstPressed(KEY_YIELD_CONTROL))
			setMoving(!isMoving());
	}

private:
	Input *input;
};

class AIBrain : public EntityBrain
{
public:
	explicit AIBrain(Entity e)
		: EntityBrain(e)
	{
	}

	void tick(float delta) override
	{
		// do things
	}
};
