#pragma once
#include <boost/smart_ptr/shared_ptr.hpp>
#include <vector>
#include "animation.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "world.hpp"

#define MAX_ENTITIES 1024

class b2World;
class b2Body;
typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

class EntityFactory
{
public:
	void loadEntities(EntityType entityType, const std::string &fileName);

private:
	std::map<EntityType, EntityTags> loadedTags;
};

template <class T>
sf::Vector2<T> fromB2Vec(const b2Vec2 &v)
{
	return{static_cast<T>(v.x), static_cast<T>(v.y)};
}

template <class T>
b2Vec2 toB2Vec(const sf::Vector2<T> &v)
{
	return{static_cast<float>(v.x), static_cast<float>(v.y)};
}

// component-entity-systems

typedef int Entity;

enum ComponentType
{
	COMPONENT_NONE = 0,
	COMPONENT_PHYSICS = 1 << 0,
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

struct PhysicsComponent : BaseComponent
{
	void reset() override;
	sf::Vector2i getTilePosition() const;
	sf::Vector2f getPosition() const;
	sf::Vector2f getVelocity() const;
	sf::Vector2f getLastVelocity() const;
	void setVelocity(const sf::Vector2f& velocity);
	bool isStopped();

	b2Body *body;
	b2World *bWorld;
	b2Vec2 lastVelocity;
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

class RenderSystem : public System
{
public:
	RenderSystem() : System(COMPONENT_PHYSICS | COMPONENT_RENDER)
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

class PhysicsSystem : public System
{
public:
	explicit PhysicsSystem(): System(COMPONENT_PHYSICS)
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
		systems.push_back(new PhysicsSystem);

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
	PhysicsComponent physicsComponents[MAX_ENTITIES];
	RenderComponent renderComponents[MAX_ENTITIES];
	InputComponent inputComponents[MAX_ENTITIES];

	// component management
	void removeComponent(Entity e, ComponentType type);
	bool hasComponent(Entity e, ComponentType type);
	BaseComponent*getComponentOfType(Entity e, ComponentType type);

	template <class T>
	T* getComponent(Entity e, ComponentType type)
	{
		return dynamic_cast<T*>(getComponentOfType(e, type));
	}

	// helpers
	void addPhysicsComponent(Entity e, World *world, const sf::Vector2i &startTilePos);
	void addRenderComponent(Entity e, EntityType entityType, const std::string &animation, float step, DirectionType initialDirection, bool playing);
	void addPlayerInputComponent(Entity e);
	void addAIInputComponent(Entity e);

private:
	BaseComponent* addComponent(Entity e, ComponentType type);
	void addBrain(Entity e, bool aiBrain);
};
