#pragma once
#include <vector>
#include "animation.hpp"
#include "constants.hpp"

#define COMPONENT_COUNT 3
#define MAX_ENTITIES 1024

typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

enum EntityType
{
	HUMAN,
	VEHICLE
};

class EntityFactory
{
public:
	void loadEntities(EntityType entityType, const std::string &fileName);

private:
	std::map<EntityType, EntityTags> loadedTags;
};


enum ComponentType
{
	COMPONENT_NONE = 0,
	POSITION = 1 << 0,
	VELOCITY = 1 << 1,
	RENDER = 1 << 2
};

// component-entity-systems

typedef int Entity;

// components
struct BaseComponent
{
};

struct PositionComponent : public BaseComponent
{
	sf::Vector2f pos;
};

struct VelocityComponent : public BaseComponent
{
	sf::Vector2f velocity;
};

struct RenderComponent : public BaseComponent
{
	Animator *anim;
};

// systems
class System
{
public:
	System(int componentMask) : mask(componentMask)
	{
	}

	void tick(float dt);

	virtual void tickEntity(Entity e, float dt) = 0;

protected:
	int mask;

};

class MovementSystem : public System
{
public:
	MovementSystem() : System(POSITION | VELOCITY)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

class RenderSystem : public System
{
public:
	RenderSystem() : System(POSITION | RENDER)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

class EntityManager
{
public:
	EntityManager::EntityManager() : entityCount(0)
	{
		for (size_t i = 0; i < MAX_ENTITIES; ++i)
			entities[i] = COMPONENT_NONE;
	}

	Entity entities[MAX_ENTITIES];
	size_t entityCount;

	// components
	PositionComponent positionComponents[MAX_ENTITIES];
	VelocityComponent velocityComponents[MAX_ENTITIES];
	RenderComponent renderComponents[MAX_ENTITIES];

	// creation/deletion
	Entity createEntity();
	void deleteEntity(Entity e);

	// component management
	BaseComponent* addComponent(Entity e, ComponentType type);
	void removeComponent(Entity e, ComponentType type);
	bool hasComponent(Entity e, ComponentType type);
	BaseComponent* getComponent(Entity e, ComponentType type);
};

