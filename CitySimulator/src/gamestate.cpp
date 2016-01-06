#include "services.hpp"
#include "gamestate.hpp"

EntityIdentifier & createTestHuman(World &world, int x, int y, const std::string &skin, DirectionType direction)
{
	EntityService *es = Locator::locate<EntityService>();

	EntityIdentifier *entity = es->createEntity(ENTITY_HUMAN);

	es->addPhysicsComponent(*entity, &world, {x, y}, Config::getFloat("debug.movement.max-speed.walk"),
	                        Config::getFloat("debug.movement.stop-decay"));

	es->addRenderComponent(*entity, skin, 0.2f, direction, false);
	es->addAIInputComponent(entity->id);

	return *entity;
}

GameState::GameState() : State(STATE_GAME)
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

	// load camera
	Locator::provide(SERVICE_CAMERA, new CameraService(world));

	// create some humans
	const int count = 5;

	for (int i = 0; i < count; ++i)
	{
		int x = Utils::random(0, world.getTileSize().x);
		int y = Utils::random(0, world.getTileSize().y);

		createTestHuman(world, x, y, animationService->getRandomAnimationName(ENTITY_HUMAN), Direction::random());
	}
}

void GameState::tick(float delta)
{
	world.tick(delta);

	Locator::locate<CameraService>()->tick(delta);
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