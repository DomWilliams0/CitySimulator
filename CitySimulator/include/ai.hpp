#ifndef CITYSIM_AI_HPP
#define CITYSIM_AI_HPP

#include "entity.hpp"
#include "constants.hpp"
#include "services.hpp"

class EntityBrain
{
public:
	explicit EntityBrain(EntityID e);

	virtual ~EntityBrain()
	{
	}

	virtual void tick(float delta) = 0;

protected:
	EntityID entity;
	PhysicsComponent *phys;

	virtual void onEnable()
	{
	}

	virtual void onDisable()
	{
	}

};

class InputBrain : public EntityBrain, EventListener
{
public:
	InputBrain(EntityID e);

private:
	virtual void onEvent(const Event &event) override;
	void tick(float delta) override;

	std::vector<bool> moving;
};

class AIBrain : public EntityBrain
{
public:
	AIBrain(EntityID e) : EntityBrain(e)
	{
	}

	void tick(float delta) override;
};

#endif
