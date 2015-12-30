#ifndef CITYSIM_ENTITY_HPP
#define CITYSIM_ENTITY_HPP

#include <boost/smart_ptr/shared_ptr.hpp>
#include <vector>
#include "animation.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "world.hpp"


class b2World;

class b2Body;

typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

class EntityFactory
{
public:
	void loadEntitiesFromFile(const std::string &fileName);

private:
	std::map<EntityType, EntityTags> loadedTags;

	void loadEntities(ConfigurationFile &config, EntityType entityType, const std::string &sectionName);
};

template<class T>
sf::Vector2<T> fromB2Vec(const b2Vec2 &v)
{
	return {static_cast<T>(v.x), static_cast<T>(v.y)};
}

template<class T>
b2Vec2 toB2Vec(const sf::Vector2<T> &v)
{
	return {static_cast<float>(v.x), static_cast<float>(v.y)};
}

// component-entity-systems

typedef unsigned int Entity;

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

	void setVelocity(const sf::Vector2f &velocity);

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
	InputSystem() : System(COMPONENT_INPUT)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

class PhysicsSystem : public System
{
public:
	explicit PhysicsSystem() : System(COMPONENT_PHYSICS)
	{
	}

	void tickEntity(Entity e, float dt) override;
};

#endif