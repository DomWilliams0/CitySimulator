#include "ecs.hpp"
#include "ai.hpp"
#include "world.hpp"
#include "service/locator.hpp"

CameraService::CameraService(World &world) : world(&world), trackedEntity(nullptr), worldChangeListener(this)
{
	float speed = Config::getFloat("debug.movement.camera-speed");
	controller.reset(CAMERA_ENTITY, speed, speed, speed);
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

	// register
	worldChangeListener.identifier = "camera world change listener";
	Locator::locate<EventService>()->registerListener(
			&worldChangeListener, EVENT_CAMERA_SWITCH_WORLD
	);
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
		b2Vec2 movement(controller.tick(delta, speed));
		view.move(movement.x * delta, movement.y * delta);
	}
}


World *CameraService::getCurrentWorld()
{
	return world;
}

void CameraService::setTrackedEntity(EntityID entity)
{
	EntityService *es = Locator::locate<EntityService>();
	if (es->hasComponent(entity, COMPONENT_PHYSICS))
	{
		trackedEntity = es->getComponent<PhysicsComponent>(entity, COMPONENT_PHYSICS);
		Logger::logDebug(format("Started tracking entity %1%", _str(entity)));

		controller.unregisterListeners();
	}
	else
		Logger::logWarning(
				format("Could not track entity %1% as it doesn't have a physics component", _str(entity)));
}

void CameraService::clearPlayerEntity()
{
	trackedEntity = nullptr;
	controller.registerListeners();
}


PhysicsComponent *CameraService::getTrackedEntity() const
{
	return trackedEntity;
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

CameraService::WorldChangeListener::WorldChangeListener(CameraService *cs) : cs(cs)
{ }

void CameraService::WorldChangeListener::onEvent(const Event &event)
{
	WorldService *ws = Locator::locate<WorldService>();
	World *newWorld = ws->getWorld(event.cameraSwitchWorld.newWorld); // todo never return null

	cs->world = newWorld;
	cs->view.setCenter((float) event.cameraSwitchWorld.centreX, (float) event.cameraSwitchWorld.centreY);

	Logger::logDebug(format("Switched camera world to %1%", _str(newWorld->getID())));
}


