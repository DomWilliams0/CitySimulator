#include "services.hpp"

CameraService::CameraService(World &world) : world(&world)
{
}

void CameraService::onEnable()
{
	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
	view.zoom(Config::getFloat("debug.zoom"));
	updateWindowView();
}

void CameraService::updateWindowView() const
{
	Locator::locate<RenderService>()->getWindow()->setView(view);
}

void CameraService::onDisable()
{
}

void CameraService::tick(float delta)
{
	if (trackedEntity != nullptr)
	{
		view.setCenter(trackedEntity->getPosition());
		updateWindowView();
	}
}

void CameraService::switchWorld(World &world)
{
	Logger::logDebug("Switched camera world");
	this->world = &world;
}

void CameraService::setTrackedEntity(EntityID entity)
{
	EntityService *es = Locator::locate<EntityService>();
	if (es->hasComponent(entity, COMPONENT_PHYSICS))
	{
		trackedEntity = es->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);
		Logger::logDebug(format("Started tracking entity %1%", std::to_string(entity)));

		Locator::locate<EventService>()->registerListener(this, EVENT_INPUT_START_MOVING);
	}
	else
		Logger::logWarning(
				format("Could not track entity %1% as it doesn't have a physics component", std::to_string(entity)));
}

void CameraService::clearPlayerEntity()
{
	trackedEntity = nullptr;
	Locator::locate<EventService>()->unregisterListener(this, EVENT_INPUT_START_MOVING);
}

void CameraService::onEvent(const Event &event)
{

}
