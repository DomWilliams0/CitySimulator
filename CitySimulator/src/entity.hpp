#pragma once
#include <vector>
#include "animation.hpp"
#include "constants.hpp"

#define COMPONENT_COUNT 3
#define MAX_ENTITIES 1024
#include "world.hpp"

typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

enum EntityType
{
	ENTITY_HUMAN,
	ENTITY_VEHICLE
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
	virtual ~BaseComponent()
	{
	}

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
	Animator anim;
};

// systems
class System
{
public:
	System(int componentMask, bool renders_ = false) : mask(componentMask), renders(renders_)
	{
	}

	virtual ~System()
	{
	}

	void tick(float dt);
	void render(sf::RenderWindow &window);

	virtual void tickEntity(Entity e, float dt) = 0;

	virtual void renderEntity(Entity e, sf::RenderWindow &window)
	{
	}

	bool doesRender()
	{
		return renders;
	}

protected:
	int mask;
	bool renders;
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
	RenderSystem() : System(COMPONENT_POSITION | COMPONENT_RENDER, true)
	{
	}

	void tickEntity(Entity e, float dt) override;
	void renderEntity(Entity e, sf::RenderWindow &window) override;
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
	Entity createEntityWithComponents(const std::initializer_list<ComponentType> &components);
	void deleteEntity(Entity e);
	bool isAlive(Entity e);

	// systems
	std::vector<System*> systems;
	void renderSystems(sf::RenderWindow &window);
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

	template <class T>
	T* getComponent(Entity e, ComponentType type);

	// helpers
	void addPositionComponent(Entity e, float x, float y);
	void addVelocityComponent(Entity e, float x, float y);
	void addRenderComponent(Entity e, EntityType entityType, const std::string &animation, float step, DirectionType initialDirection, bool playing);
};

template <class T>
T* EntityManager::getComponent(Entity e, ComponentType type)
{
	return dynamic_cast<T*>(getComponent(e, type));
}
