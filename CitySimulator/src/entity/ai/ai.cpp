#include "ai.hpp"
#include "service/locator.hpp"

Brain::Brain(EntityID e) : entity(e), controller(e, 0.f, 0.f, 0.f) // dummy values
{
	setEntity(e);
}

Brain::~Brain()
{
}

void Brain::setEntity(EntityID e, bool stop)
{
	entity = e;

	// todo pass in as parameters
	controller.reset(e, Config::getFloat("debug.movement.force"),
					 Config::getFloat("debug.movement.max-speed.walk"),
					 Config::getFloat("debug.movement.max-speed.run"));

	EntityService *es = Locator::locate<EntityService>();
	if (!es->hasComponent(entity, COMPONENT_PHYSICS))
		error("Could not create brain for entity %1% as it doesn't have a physics component", _str(entity));

	phys = es->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);

	if (stop)
		controller.halt();
}

void Brain::tick(float delta)
{
	controller.tick(phys, delta);
}


EntityBrain::EntityBrain(EntityID e) : Brain(e)
{
}

void EntityBrain::tick(float delta)
{
	// todo tick behaviours, which tick steerings
}
