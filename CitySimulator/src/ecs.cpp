#include "entity.hpp"
#include "ai.hpp"
#include "config.hpp"

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
	static float acceleration = 20.0f;

	auto motion = get<MotionComponent>(e, COMPONENT_MOTION);

	motion->velocity += Math::multiply<float>(Math::normalize<float>(motion->steeringLinear), acceleration);
	motion->velocity *= movementDecay;

	if (Math::lengthSquared(motion->velocity) < minSpeed * minSpeed)
		motion->velocity.x = motion->velocity.y = 0.0f;

	motion->position += motion->velocity * dt;

	//	motion->orientation += motion->steeringAngular * dt;
	//	motion->orientation += motion->rotation * dt;
}

void RenderSystem::tickEntity(Entity e, float dt)
{
	auto *render = get<RenderComponent>(e, COMPONENT_RENDER);

	render->anim.tick(dt);
}

void InputSystem::tickEntity(Entity e, float dt)
{
	get<InputComponent>(e, COMPONENT_INPUT)->brain->tick(dt);
}

void tempDrawVector(MotionComponent *motion, const sf::Vector2f vector, sf::Color colour, sf::RenderWindow &window)
{
	sf::RectangleShape r;
	r.rotate(atan2(vector.y, vector.x) * Math::radToDeg);
	r.move(motion->position + sf::Vector2f(8.0f, 8.0f));
	r.setSize(sf::Vector2f(Math::length<float>(vector), 1.0f));
	r.setFillColor(colour);
	window.draw(r);
}

void RenderSystem::renderEntity(Entity e, sf::RenderWindow &window)
{
	auto render = get<RenderComponent>(e, COMPONENT_RENDER);
	auto motion = get<MotionComponent>(e, COMPONENT_MOTION);

	sf::RenderStates states;
	sf::Transform transform;

	transform.translate(motion->position);
	transform.scale(Constants::entityScale);

	states.transform *= transform;
	render->anim.draw(window, states);

	// debug
	tempDrawVector(motion, motion->steeringLinear, sf::Color::Red, window);
	tempDrawVector(motion, motion->velocity, sf::Color::Green, window);
}

// components

void MotionComponent::reset()
{
	position.x = position.y = 0.0f;
	orientation = 0.0f;
	velocity.x = velocity.y = 0.0f;
	rotation = 0.0f;
	steeringLinear.x = steeringLinear.y = 0.0f;
	steeringAngular = 0.0f;
}

void RenderComponent::reset()
{
	anim.reset();
}


void InputComponent::reset()
{
	brain.reset();
}
