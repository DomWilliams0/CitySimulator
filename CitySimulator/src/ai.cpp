#include "ai.hpp"
#include "entity.hpp"

void InputBrain::tick(float delta)
{
	const static float acceleration = Config::getFloat("debug-movement-force");
	const static float decay = Config::getFloat("debug-movement-decay");

	// walking
	bool right = input->isPressed(KEY_RIGHT);
	bool left = input->isPressed(KEY_LEFT);
	bool down = input->isPressed(KEY_DOWN);
	bool up = input->isPressed(KEY_UP);

	// todo: WIP!
	sf::Vector2f impulse;

	if (right != left)
		impulse.x = right ? acceleration : -acceleration;
	else
		impulse.x = 0.f;

	if (down != up)
		impulse.y = down ? acceleration : -acceleration;
	else
		impulse.y = 0.f;

	impulse -= phys->getVelocity();
	impulse *= phys->body->GetMass();
	phys->body->ApplyForce(toB2Vec(impulse), phys->body->GetWorldCenter(), true);

	//	if (right != left)
//		phys->steeringLinear.x = right ? 1.f : -1.f;
//	else
//		phys->steeringLinear.x = 0.f;
//
//	if (down != up)
//		phys->steeringLinear.y = down ? 1.f : -1.f;
//	else
//		phys->steeringLinear.y = 0.f;
}

void AIBrain::tick(float delta)
{
	// do things
}
