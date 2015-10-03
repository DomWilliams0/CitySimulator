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

void MovementSystem::tickEntity(Entity e, float dt)
{
	auto pos = get<PositionComponent>(e, COMPONENT_POSITION);
	auto vel = get<VelocityComponent>(e, COMPONENT_VELOCITY);

	pos->pos += vel->velocity * dt;
}

void RenderSystem::tickEntity(Entity e, float dt)
{
	auto *render = get<RenderComponent>(e, COMPONENT_RENDER);

	render->anim.tick(dt);
}

void InputSystem::tickEntity(Entity e, float dt)
{
	Globals::entityManager->getComponent<InputComponent>(e, COMPONENT_INPUT)->brain->tick(dt);
}

void RenderSystem::renderEntity(Entity e, sf::RenderWindow &window)
{
	auto render = get<RenderComponent>(e, COMPONENT_RENDER);
	auto pos = get<PositionComponent>(e, COMPONENT_POSITION);

	sf::RenderStates states;
	sf::Transform transform;

	transform.translate(pos->pos);
	transform.scale(Constants::entityScale);

	states.transform *= transform;
	render->anim.draw(window, states);
}

// components

void PositionComponent::reset()
{
	pos.x = pos.y = 0;
}

void RenderComponent::reset()
{
	anim.reset();
}

void VelocityComponent::reset()
{
	velocity.x = velocity.y = 0;
}

void InputComponent::reset()
{
	brain.reset();
}
