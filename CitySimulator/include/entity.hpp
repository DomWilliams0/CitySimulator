#ifndef CITYSIM_ENTITY_HPP
#define CITYSIM_ENTITY_HPP

typedef unsigned int EntityID;

#include <boost/smart_ptr/shared_ptr.hpp>
#include <vector>
#include "animation.hpp"
#include "config.hpp"
#include "constants.hpp"

class b2World;

class b2Body;

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

	inline sf::Vector2f getTilePosition() const
	{
		return fromB2Vec<float>(body->GetPosition());
	}

	inline sf::Vector2f getPosition() const
	{
		b2Vec2 pos(body->GetPosition());
		return Utils::toPixel(fromB2Vec<float>(pos));
	}

	inline sf::Vector2f getVelocity() const
	{
		b2Vec2 v = body->GetLinearVelocity();
		return fromB2Vec<float>(v);
	}

	inline sf::Vector2f getLastVelocity() const
	{
		return fromB2Vec<float>(lastVelocity);
	}

	inline void setVelocity(const sf::Vector2f &velocity)
	{
		body->SetLinearVelocity(toB2Vec(velocity));
	}

	inline bool isStopped()
	{
		return Math::lengthSquared(getVelocity()) < 1;
	}

	inline bool isSteering()
	{
		return steering.x != 0.f || steering.y != 0.f;
	}

	b2Body *body;
	b2World *bWorld;
	b2Vec2 lastVelocity;

	b2Vec2 steering;
};

class EntityService;

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

	void tick(EntityService *es, float dt);

	void render(EntityService *es, sf::RenderWindow &window);

	virtual void tickEntity(EntityService *es, EntityID e, float dt) = 0;

	virtual void renderEntity(EntityService *es, EntityID e, sf::RenderWindow &window)
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

	void tickEntity(EntityService *es, EntityID e, float dt) override;

	void renderEntity(EntityService *es, EntityID e, sf::RenderWindow &window) override;
};

class InputSystem : public System
{
public:
	InputSystem() : System(COMPONENT_INPUT)
	{
	}

	void tickEntity(EntityService *es, EntityID e, float dt) override;
};

class PhysicsSystem : public System
{
public:
	explicit PhysicsSystem() : System(COMPONENT_PHYSICS)
	{
	}

	void tickEntity(EntityService *es, EntityID e, float dt) override;
};

#endif