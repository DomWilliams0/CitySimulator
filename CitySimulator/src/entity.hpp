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
	COMPONENT_POSITION = 1 << 0,
	COMPONENT_VELOCITY = 1 << 1,
	COMPONENT_RENDER = 1 << 2
};

// component-entity-systems

typedef int Entity;

// components
struct BaseComponent
{
	virtual void reset() = 0;
};

struct PositionComponent : public BaseComponent
{
	void reset() override;
	sf::Vector2f pos;
};

struct VelocityComponent : public BaseComponent
{
	void reset() override;

	sf::Vector2f velocity;
};

struct RenderComponent : public BaseComponent
{
	void reset() override;
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
	MovementSystem() : System(COMPONENT_POSITION | COMPONENT_VELOCITY)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

class RenderSystem : public System
{
public:
	RenderSystem() : System(COMPONENT_POSITION | COMPONENT_RENDER)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

class EntityManager
{
public:
	EntityManager() : entityCount(0)
	{
		// init entities
		for (size_t i = 0; i < MAX_ENTITIES; ++i)
			entities[i] = COMPONENT_NONE;

		// init systems
		systems.push_back(new MovementSystem);
		systems.push_back(new RenderSystem);

	}

	~EntityManager()
	{
		for (System *system : systems)
			delete system;
	}

	Entity entities[MAX_ENTITIES];
	size_t entityCount;

	// entity
	Entity createEntity();
	void deleteEntity(Entity e);
	bool isAlive(Entity e);

	// systems
	std::vector<System*> systems;
	void tickSystems(float delta);

	// components
	PositionComponent positionComponents[MAX_ENTITIES];
	VelocityComponent velocityComponents[MAX_ENTITIES];
	RenderComponent renderComponents[MAX_ENTITIES];

	// component management
	BaseComponent* addComponent(Entity e, ComponentType type);
	void removeComponent(Entity e, ComponentType type);
	bool hasComponent(Entity e, ComponentType type);
	BaseComponent* getComponent(Entity e, ComponentType type);

	// helpers
	void addPositionComponent(Entity e, float x, float y);
	void addVelocityComponent(Entity e, float x, float y);
	void addRenderComponent(Entity e, const std::string& animation, DirectionType initialDirection, bool playing);
};

