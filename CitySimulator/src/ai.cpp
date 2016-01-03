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

	float dx = 0.f;
	float dy = 0.f;

	DirectionType direction = start ? event.startMove.direction : event.stopMove.direction;
	switch (direction)
	{
		case DIRECTION_NORTH:
			dy = -1;
			break;
		case DIRECTION_SOUTH:
			dy = 1;
			break;
		case DIRECTION_WEST:
			dx = -1;
			break;
		case DIRECTION_EAST:
			dx = 1;
			break;
		default:
			error("Invalid direction in human %1% moving event: %2%", start ? "start" : "stop",
			      std::to_string(direction));
	}

	// todo movement should be toggled by start and stop move events

	float acceleration = Config::getFloat("debug.movement.force");
	if (!start)
		acceleration *= -1;

	sf::Vector2f impulse(dx * acceleration, dy * acceleration);

	impulse -= phys->getVelocity();
	impulse *= phys->body->GetMass();
	phys->body->ApplyForce(toB2Vec(impulse), phys->body->GetWorldCenter(), true);
}

void AIBrain::tick(float delta)
{
	// do things
}
