#include "gamestate.hpp"
#include "config.hpp"
#include "entity.hpp"
#include "services.hpp"

GameState::GameState() : State(GAME), playerControl(true)
{
	// create globals
	// todo SOON TO BE VANQUISHED
	Globals::entityFactory = new EntityFactory;
	Globals::spriteSheet = new SpriteSheet;

	// load entities
	auto entityService = new EntityService;
	Locator::provide(SERVICE_ENTITY, entityService);
	Globals::entityFactory->loadEntitiesFromFile("entities.json");

	// load sprites
	Globals::spriteSheet->processAllSprites();

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
	entityService->addRenderComponent(e, ENTITY_HUMAN, "Business Man", 0.2f, Direction::EAST, false);
	entityService->addPlayerInputComponent(e);

	entityTracking = (PhysicsComponent *) entityService->getComponentOfType(e, COMPONENT_PHYSICS);
}

GameState::~GameState()
{
	delete Globals::entityFactory;
	delete Globals::spriteSheet;
}

void GameState::tempControlCamera(float delta)
{
	const static float viewSpeed = 400;

	float dx(0), dy(0);

	if (Locator::locate<InputService>()->isPressed(KEY_UP))
		dy = -delta;
	else if (Locator::locate<InputService>()->isPressed(KEY_DOWN))
		dy = delta;
	if (Locator::locate<InputService>()->isPressed(KEY_LEFT))
		dx = -delta;
	else if (Locator::locate<InputService>()->isPressed(KEY_RIGHT))
		dx = delta;

	if (dx || dy)
	{
		view.move(dx * viewSpeed, dy * viewSpeed);
		Locator::locate<RenderService>()->getWindow()->setView(view);
	}
}

void GameState::tick(float delta)
{
	world.tick(delta);

	if (Locator::locate<InputService>()->isFirstPressed(InputKey::KEY_YIELD_CONTROL))
		playerControl = !playerControl;

	// todo make sure this hack DOESN'T end up in production
	if (playerControl)
	{
		Locator::locate<EntityService>()->tickSystems(delta);
		if (entityTracking)
			view.setCenter(Utils::toPixel(entityTracking->getPosition()));
		Locator::locate<RenderService>()->getWindow()->setView(view);
	}
	else
		tempControlCamera(delta);

}

void GameState::render(sf::RenderWindow &window)
{
	window.draw(world);

	Locator::locate<RenderService>()->renderEntities();
}

void GameState::handleInput(const sf::Event &event)
{
}

b2World *GameState::getBox2DWorld()
{
	return world.getBox2DWorld();
}