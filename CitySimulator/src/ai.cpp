#include "ai.hpp"


EntityBrain::EntityBrain(EntityID e) : entity(e)
{
	phys = Locator::locate<EntityService>()->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);
}

InputBrain::InputBrain(EntityID e) : EntityBrain(e), controller(e)
{
	controller.registerListeners();
}

InputBrain::~InputBrain()
{
	controller.unregisterListeners();
}

void InputBrain::tick(float delta)
{
	float acceleration = Config::getFloat("debug.movement.force");
	phys->steering = controller.tick(acceleration, delta);

	controller.doSprintSwitcharoo(phys, Config::getFloat("debug.movement.max-speed.run"));
}

void AIBrain::tick(float delta)
{
	// do things
}
