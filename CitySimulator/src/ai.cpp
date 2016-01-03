#include "ai.hpp"

void InputBrain::tick(float delta)
{
	float acceleration = Config::getFloat("debug.movement.force");
	auto input = Locator::locate<InputService>();

	// walking
/*	bool right = input->isPressed(KEY_RIGHT);
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
	phys->body->ApplyForce(toB2Vec(impulse), phys->body->GetWorldCenter(), true);*/

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

void InputBrain::onEvent(const Event &event)
{
	bool start = event.type == EVENT_HUMAN_START_MOVING;
	Logger::logDebug(format("Input brain should %1% moving%2%", start ? "start" : "stop",
	                        start ? " in direction " + std::to_string(event.startMove.direction) : ""));
}

void AIBrain::tick(float delta)
{
	// do things
}
