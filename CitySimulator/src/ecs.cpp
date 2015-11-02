#include "entity.hpp"
#include "ai.hpp"

// systems

void System::tick(float dt)
{
	EntityManager *manager = Globals::entityManager;

	for (size_t e = 0; e < MAX_ENTITIES; ++e)
	{
		if ((manager->entities[e] & mask) == mask)
			tickEntity(e, dt);
	}
}

void System::render(sf::RenderWindow &window)
{
	EntityManager *manager = Globals::entityManager;

	for (size_t e = 0; e < MAX_ENTITIES; ++e)
	{
		if ((manager->entities[e] & mask) == mask)
			renderEntity(e, window);
	}
}

template <class T>
T* get(Entity e, ComponentType type)
{
	return Globals::entityManager->getComponent<T>(e, type);
}

void RenderSystem::tickEntity(Entity e, float dt)
{
	auto *render = get<RenderComponent>(e, COMPONENT_RENDER);
	auto *physics = get<PhysicsComponent>(e, COMPONENT_PHYSICS);

	// set playing
	bool stopped = Math::lengthSquared(physics->getVelocity()) == 0;
	render->anim.setPlaying(!stopped, stopped);

	// change animation direction
	// todo get this from orientation instead of movement
	sf::Vector2f directionVector = stopped ? physics->getLastVelocity() : physics->getVelocity();
	float angleDeg = atan2(directionVector.y, directionVector.x) * Math::radToDeg;
	DirectionType direction = Direction::fromAngle(angleDeg);
	render->anim.turn(direction, false);

	// advance animation frame
	render->anim.tick(dt);
}

void InputSystem::tickEntity(Entity e, float dt)
{
	get<InputComponent>(e, COMPONENT_INPUT)->brain->tick(dt);
}

void PhysicsSystem::tickEntity(Entity e, float dt)
{
	auto *physics = get<PhysicsComponent>(e, COMPONENT_PHYSICS);

	// store current velocity for next step
	auto vel(physics->getVelocity());
	if (vel.x != 0.f || vel.y != 0.f)
		physics->lastVelocity = toB2Vec(physics->getVelocity());
}

void tempDrawVector(PhysicsComponent *physics, const sf::Vector2f vector, sf::Color colour, sf::RenderWindow &window)
{
	sf::RectangleShape r;
	r.rotate(atan2(vector.y, vector.x) * Math::radToDeg);
	r.move(physics->getPosition() + sf::Vector2f(8.0f, 8.0f));
	r.setSize(sf::Vector2f(Math::length(vector), 1.0f));
	r.setFillColor(colour);
	window.draw(r);
}

void RenderSystem::renderEntity(Entity e, sf::RenderWindow &window)
{
	auto render = get<RenderComponent>(e, COMPONENT_RENDER);
	auto *physics = get<PhysicsComponent>(e, COMPONENT_PHYSICS);

	sf::RenderStates states;
	sf::Transform transform;

	transform.translate(physics->getPosition());
	transform.scale(Constants::entityScale);

	states.transform *= transform;
	render->anim.draw(window, states);

	// debug
	tempDrawVector(physics, physics->getVelocity(), sf::Color::Green, window);
}

// components

sf::Vector2i PhysicsComponent::getTilePosition() const
{
	sf::Vector2i tile;
	auto position(getPosition());
	tile.x = static_cast<int>(position.x);
	tile.y = static_cast<int>(position.y) + 1;
	return Utils::toTile(tile);
}

sf::Vector2f PhysicsComponent::getPosition() const
{
	auto pos(body->GetPosition());
	return sf::Vector2f(pos.x, pos.y);
}

sf::Vector2f PhysicsComponent::getVelocity() const
{
	auto vel(body->GetLinearVelocity());
	return sf::Vector2f(vel.x, vel.y);
}

sf::Vector2f PhysicsComponent::getLastVelocity() const
{
	return fromB2Vec<float>(lastVelocity);
}

void RenderComponent::reset()
{
	anim.reset();
}


void InputComponent::reset()
{
	brain.reset();
}

void PhysicsComponent::reset()
{
	if (body != nullptr)
	{
		bWorld->DestroyBody(body);
		body = nullptr;
	}
}
