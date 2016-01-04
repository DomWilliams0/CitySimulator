#include "services.hpp"

CameraService::CameraService(World &world) : world(&world)
{
	controller = new SimpleMovementController(INVALID_ENTITY);
}

CameraService::~CameraService()
{
	delete controller;
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
	else
	{
		b2Vec2 movement(controller->tick(Config::getFloat("debug.movement.camera-speed"), delta));
		view.move(movement.x, movement.y);
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

		Locator::locate<EventService>()->unregisterListener(controller, EVENT_INPUT_START_MOVING);
		Locator::locate<EventService>()->unregisterListener(controller, EVENT_INPUT_STOP_MOVING);
	}
	else
		Logger::logWarning(
				format("Could not track entity %1% as it doesn't have a physics component", std::to_string(entity)));
}

void CameraService::clearPlayerEntity()
{
	trackedEntity = nullptr;
	Locator::locate<EventService>()->registerListener(controller, EVENT_INPUT_START_MOVING);
	Locator::locate<EventService>()->registerListener(controller, EVENT_INPUT_STOP_MOVING);
}