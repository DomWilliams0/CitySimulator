#include "gamestate.hpp"

GameState::GameState() : State(GAME)
{
	// load art service for queueing
	auto animationService = new AnimationService;
	Locator::provide(SERVICE_ANIMATION, animationService);
	
	// load entities
	auto entityService = new EntityService;
	Locator::provide(SERVICE_ENTITY, entityService);

	// load art
	animationService->processQueuedSprites();

	// load world
	world.loadFromFile(Config::getString("debug.world-name"),
	                   Config::getResource("world.tileset"));

	// camera view
	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
	view.setCenter(world.getPixelSize().x / 2.f, world.getPixelSize().y / 2.f);

	view.zoom(Config::getFloat("debug.zoom"));
	Locator::locate<RenderService>()->getWindow()->setView(view);

	EntityID e = entityService->createEntity();
	sf::Vector2i tilePos = {Config::getInt("debug.start-pos.x"), Config::getInt("debug.start-pos.y")};
	entityService->addPhysicsComponent(e, &world, tilePos);
	entityService->addRenderComponent(e, ENTITY_HUMAN, "Business Man", 0.2f, DIRECTION_EAST, false);
	entityService->addPlayerInputComponent(e);

	Locator::locate<InputService>()->setPlayerEntity(e);
}

void GameState::tick(float delta)
{
	world.tick(delta);

	Locator::locate<EntityService>()->tickSystems(delta);
}

void GameState::render(sf::RenderWindow &window)
{
	Locator::locate<RenderService>()->render(world);
}

b2World *GameState::getBox2DWorld()
{
	return world.getBox2DWorld();
}