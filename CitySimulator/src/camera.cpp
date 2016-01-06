#include "services.hpp"

CameraService::CameraService(World &world) : world(&world), trackedEntity(nullptr)
{
	float speed = Config::getFloat("debug.movement.camera-speed");
	controller = new SimpleMovementController(CAMERA_ENTITY, speed, speed, speed);
}

CameraService::~CameraService()
{
	delete controller;
}

void CameraService::onEnable()
{
	sf::Vector2f size = static_cast<sf::Vector2f>(Constants::windowSize);

	view.setSize(size);
	view.reset(sf::FloatRect(-size.x / 4, -size.y / 4, size.x, size.y));
	zoom = Config::getFloat("debug.zoom");
	view.zoom(zoom);
	Locator::locate<RenderService>()->setView(view);

	clearPlayerEntity();
}

void CameraService::onDisable()
{
}

void CameraService::tick(float delta)
{
	if (trackedEntity != nullptr)
	{
		view.setCenter(trackedEntity->getPosition());
	}
	else
	{
		float speed; // todo currently unused
		b2Vec2 movement(controller->tick(delta, speed));
		view.move(movement.x * delta, movement.y * delta);
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
		Logger::logDebug(format("Started tracking entity %1%", _str(entity)));

		controller->unregisterListeners();
	}
	else
		Logger::logWarning(
				format("Could not track entity %1% as it doesn't have a physics component", _str(entity)));
}

void CameraService::clearPlayerEntity()
{
	trackedEntity = nullptr;
	controller->registerListeners();
}

void CameraService::updateViewSize(unsigned int width, unsigned int height)
{
	view.setSize(width, height);
	view.zoom(zoom);
}

void CameraService::zoomTo(float delta, const sf::Vector2i &pixel, sf::RenderWindow &window)
{
	zoom *= delta;
	window.setView(view);

	const sf::Vector2f beforeCoord{window.mapPixelToCoords(pixel)};
	view.zoom(delta);
	window.setView(view);

	const sf::Vector2f afterCoord{window.mapPixelToCoords(pixel)};
	const sf::Vector2f offsetCoords{beforeCoord - afterCoord};
	view.move(offsetCoords);
}
