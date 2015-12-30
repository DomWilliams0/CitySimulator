#include "entity.hpp"
#include "ai.hpp"
#include "services.hpp"

// systems

void System::tick(EntityService *es, float dt)
{
	for (EntityID e = 0; e < MAX_ENTITIES; ++e)
	{
		if ((es->getComponentMask(e) & mask) == mask)
			tickEntity(es, e, dt);
	}
}

void System::render(EntityService *es, sf::RenderWindow &window)
{
	for (EntityID e = 0; e < MAX_ENTITIES; ++e)
	{
		if ((es->getComponentMask(e) & mask) == mask)
			renderEntity(es, e, window);
	}
}

void RenderSystem::tickEntity(EntityService *es, EntityID e, float dt)
{
	auto *render = es->getComponent<RenderComponent>(e, COMPONENT_RENDER);
	auto *physics = es->getComponent<PhysicsComponent>(e, COMPONENT_PHYSICS);

	// set playing
	bool stopped = physics->isStopped();
	render->anim.setPlaying(!stopped, stopped);

	// change animation direction
	// todo get this from orientation instead of movement
	sf::Vector2f directionVector = stopped ? physics->getLastVelocity() : physics->getVelocity();
	double angleDeg = atan2(directionVector.y, directionVector.x) * Math::radToDeg;
	DirectionType direction = Direction::fromAngle(angleDeg);
	render->anim.turn(direction, false);

	// advance animation frame
	render->anim.tick(dt);
}

void InputSystem::tickEntity(EntityService *es, EntityID e, float dt)
{
	es->getComponent<InputComponent>(e, COMPONENT_INPUT)->brain->tick(dt);
}

void PhysicsSystem::tickEntity(EntityService *es, EntityID e, float dt)
{
	auto *physics = es->getComponent<PhysicsComponent>(e, COMPONENT_PHYSICS);

	// maximum speed
	float maxSpeed = Config::getFloat("debug.movement.max-speed");
	if (Math::lengthSquared(physics->getVelocity()) > maxSpeed * maxSpeed)
	{
		physics->setVelocity(Math::truncate(physics->getVelocity(), maxSpeed));

		// remove damping
		physics->body->SetLinearDamping(0.f);
	}
	else
		physics->body->SetLinearDamping(Config::getFloat("debug.movement.stop-decay"));

	// stop
	if (physics->isStopped())
		physics->body->SetLinearVelocity({0.f, 0.f});

	// store current velocity for next step
	auto vel(physics->getVelocity());
	if (vel.x != 0.f || vel.y != 0.f)
		physics->lastVelocity = toB2Vec(physics->getVelocity());
}

void tempDrawVector(PhysicsComponent *physics, const sf::Vector2f vector, sf::Color colour, sf::RenderWindow &window)
{
	sf::RectangleShape r;
	r.rotate(atan2(vector.y, vector.x) * Math::radToDeg);
	r.move(Utils::toPixel(physics->getPosition()));
	r.setSize(sf::Vector2f(Math::length(vector) * Constants::tileSizef / 2, 1.0f));
	r.setFillColor(colour);
	window.draw(r);
}

void RenderSystem::renderEntity(EntityService *es, EntityID e, sf::RenderWindow &window)
{
	auto render = es->getComponent<RenderComponent>(e, COMPONENT_RENDER);
	auto *physics = es->getComponent<PhysicsComponent>(e, COMPONENT_PHYSICS);

	sf::RenderStates states;
	sf::Transform transform;

	auto offsetPosition = physics->getPosition();
	offsetPosition.x -= 0.5f;
	offsetPosition.y -= 0.5f;

	transform.translate(Utils::toPixel(offsetPosition));
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

void PhysicsComponent::setVelocity(const sf::Vector2f &velocity)
{
	body->SetLinearVelocity(toB2Vec(velocity));
}

bool PhysicsComponent::isStopped()
{
	return Math::lengthSquared(getVelocity()) < 1;
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
