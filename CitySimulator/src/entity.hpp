#pragma once
#include <vector>
#include "animation.hpp"
#include "constants.hpp"
#include "world.hpp"

#define COMPONENT_COUNT 3
#define MAX_ENTITIES 1024
#include "config.hpp"

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

// component-entity-systems

typedef int Entity;

enum ComponentType
{
	COMPONENT_NONE = 0,
	COMPONENT_MOTION = 1 << 0,
	COMPONENT_RENDER = 1 << 1,
	COMPONENT_INPUT = 1 << 2
};

// components
struct BaseComponent
{
	virtual ~BaseComponent()
	{
	}

	virtual void reset()
	{
	}
};

struct MotionComponent : BaseComponent
{
	void reset() override;
	sf::Vector2i getTilePosition() const;

	sf::Vector2f position;
	float orientation;
	
	sf::Vector2f velocity;
	sf::Vector2f lastVelocity;
	float rotation;

	sf::Vector2f steeringLinear;
	float steeringAngular;

	World *world;
};

struct RenderComponent : BaseComponent
{
	void reset() override;
	Animator anim;
};

class EntityBrain;

struct InputComponent : BaseComponent
{
	void reset() override;
	boost::shared_ptr<EntityBrain> brain;
};

struct CollisionComponent : BaseComponent
{
	void reset() override;
	sf::FloatRect aabb;
};


// systems
class System
{
public:
	explicit System(int componentMask) : mask(componentMask)
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

protected:
	int mask;
};

class MovementSystem : public System
{
public:
	MovementSystem() : System(COMPONENT_MOTION)
	{
		movementDecay = Config::getFloat("debug-movement-decay");
		minSpeed = Config::getFloat("debug-min-speed");
	}

	void tickEntity(Entity e, float dt) override;

private:
	float movementDecay;
	float minSpeed;
};

class RenderSystem : public System
{
public:
	RenderSystem() : System(COMPONENT_MOTION | COMPONENT_RENDER)
	{
	}

	void tickEntity(Entity e, float dt) override;
	void renderEntity(Entity e, sf::RenderWindow &window) override;
};

class InputSystem : public System
{
public:
	InputSystem(): System(COMPONENT_INPUT)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

class WorldCollisionSystem : public System
{
public:
	explicit WorldCollisionSystem(): System(COMPONENT_MOTION)
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

		// init systems in correct order
		systems.push_back(new InputSystem);
		systems.push_back(new MovementSystem);
		// todo: collision

		auto render = new RenderSystem;
		systems.push_back(render);
		renderSystem = render;
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
	RenderSystem *renderSystem;
	void renderSystems(sf::RenderWindow &window);
	void tickSystems(float delta);

	// components
	MotionComponent motionComponents[MAX_ENTITIES];
	RenderComponent renderComponents[MAX_ENTITIES];
	InputComponent inputComponents[MAX_ENTITIES];

	// component management
	void removeComponent(Entity e, ComponentType type);
	bool hasComponent(Entity e, ComponentType type);
	BaseComponent* getComponent(Entity e, ComponentType type);
	template <class T>
	T* getComponent(Entity e, ComponentType type);

	// helpers
	void addMotionComponent(Entity e, World *world, sf::Vector2i initialTile);
	void addRenderComponent(Entity e, EntityType entityType, const std::string &animation, float step, DirectionType initialDirection, bool playing);
	void addPlayerInputComponent(Entity e);
	void addAIInputComponent(Entity e);
	
private:
	BaseComponent* addComponent(Entity e, ComponentType type);
	void addBrain(Entity e, bool aiBrain);
};

template <class T>
T* EntityManager::getComponent(Entity e, ComponentType type)
{
	return dynamic_cast<T*>(getComponent(e, type));
}
