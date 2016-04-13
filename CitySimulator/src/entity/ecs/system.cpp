#include "ecs.hpp"
#include "ai.hpp"
#include "service/entity_service.hpp"
#include "service/config_service.hpp"

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
	bool stopAnimation = !physics->isSteering() && physics->isStopped();

	render->anim.setPlaying(!stopAnimation, stopAnimation);

	// change animation direction
	// todo get this from orientation instead of movement
	sf::Vector2f directionVector = physics->isStopped() ? physics->getLastVelocity() : physics->getVelocity();
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

void PhysicsSystem::tickEntity(EntityService *es, EntityID e, float /* dt */)
{
	auto *physics = es->getComponent<PhysicsComponent>(e, COMPONENT_PHYSICS);

	// move
	physics->body->SetLinearVelocity(
			physics->body->GetLinearVelocity() + physics->steering);

	// maximum speed
	float maxSpeed = physics->maxSpeed;
	if (Math::lengthSquared(physics->getVelocity()) > maxSpeed * maxSpeed)
	{
		physics->setVelocity(Math::truncate(physics->getVelocity(), maxSpeed));

		// remove damping
		physics->body->SetLinearDamping(0.f);
	}
	else
		physics->body->SetLinearDamping(physics->damping);

	// stop
	if (physics->isStopped())
		physics->body->SetLinearVelocity({0.f, 0.f});

	// store current velocity for next step
	auto vel(physics->getVelocity());
	if (vel.x != 0.f || vel.y != 0.f)
		physics->lastVelocity = Utils::toB2Vec(physics->getVelocity());
}

void tempDrawVector(PhysicsComponent *physics, const sf::Vector2f vector, sf::Color colour, sf::RenderWindow &window)
{
	sf::RectangleShape r;
	r.rotate(atan2(vector.y, vector.x) * Math::radToDeg);
	r.move(physics->getPosition());
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

	sf::Vector2f offsetPosition = physics->getTilePosition();
	const float offset = 0.5f * Constants::entityScalef;
	offsetPosition.x -= offset;
	offsetPosition.y -= offset;

	const float scale = Constants::entityScalef * Constants::scale / 2.f;
	transform.translate(Utils::toPixel(offsetPosition));
	transform.scale(scale, scale);

	states.transform *= transform;
	render->anim.draw(window, states);

	// debug
	if (Config::getBool("debug.render-physics", false))
		tempDrawVector(physics, physics->getVelocity(), sf::Color::Green, window);
}
