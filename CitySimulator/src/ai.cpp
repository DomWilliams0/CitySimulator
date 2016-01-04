#include "ai.hpp"


EntityBrain::EntityBrain(EntityID e) : entity(e)
{
	phys = Locator::locate<EntityService>()->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);
}

InputBrain::InputBrain(EntityID e) : EntityBrain(e), controller(e)
{
	EventService *es = Locator::locate<EventService>();
	es->registerListener(&controller, EVENT_INPUT_START_MOVING);
	es->registerListener(&controller, EVENT_INPUT_STOP_MOVING);
}

InputBrain::~InputBrain()
{
	EventService *es = Locator::locate<EventService>();
	es->unregisterListener(&controller, EVENT_INPUT_START_MOVING);
	es->unregisterListener(&controller, EVENT_INPUT_STOP_MOVING);
}

void InputBrain::tick(float delta)
{
	float acceleration = Config::getFloat("debug.movement.force");
	phys->steering = controller.tick(acceleration, delta);
}

void AIBrain::tick(float delta)
{
	// do things
}
