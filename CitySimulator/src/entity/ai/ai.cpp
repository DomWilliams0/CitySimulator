#include "ai.hpp"
#include "service/locator.hpp"

void Brain::setEntity(EntityID e, bool stop)
{
	entity = e;

	initController(Config::getFloat("debug.movement.force"),
				   Config::getFloat("debug.movement.max-speed.walk"),
				   Config::getFloat("debug.movement.max-speed.run"));

	EntityService *es = Locator::locate<EntityService>();
	if (!es->hasComponent(entity, COMPONENT_PHYSICS))
		error("Could not create brain for entity %1% as it doesn't have a physics component", _str(entity));

	phys = es->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);

	if (stop)
		controller->halt();
}

void Brain::tick(float delta)
{
	tickBrain(delta);
	controller->tick(phys, delta);
}


EntityBrain::EntityBrain(EntityID e)
{
	setEntity(e);
}

void EntityBrain::tickBrain(float delta)
{
	// todo tick behaviours, which tick steerings
}

InputBrain::InputBrain(EntityID e)
{
	setEntity(e);
	boost::dynamic_pointer_cast<PlayerMovementController>(controller)->registerListeners();
}

InputBrain::~InputBrain()
{
	boost::dynamic_pointer_cast<PlayerMovementController>(controller)->unregisterListeners();
}

void EntityBrain::initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed)
{
	controller.reset(dynamic_cast<MovementController*>(new DynamicMovementController(entity, movementForce, maxWalkSpeed, maxSprintSpeed)));
}

void InputBrain::initController(float movementForce, float maxWalkSpeed, float maxSprintSpeed)
{
	controller.reset(dynamic_cast<MovementController*>(new PlayerMovementController(entity, movementForce, maxWalkSpeed, maxSprintSpeed)));
}
