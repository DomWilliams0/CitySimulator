#include "gamestate.hpp"
#include "config.hpp"
#include "entity.hpp"
#include "services.hpp"

GameState::GameState() : State(GAME), playerControl(true)
{
	// create globals
	Globals::entityManager = new EntityManager;
	Globals::entityFactory = new EntityFactory;
	Globals::spriteSheet = new SpriteSheet;

	// load entities
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
	dynamic_cast<RenderService *>(Locator::locate(SERVICE_RENDER))->getWindow()->setView(view);

	Entity e = Globals::entityManager->createEntity();
	sf::Vector2i tilePos = {Config::getInt("debug.start-pos.x"), Config::getInt("debug.start-pos.y")};
	Globals::entityManager->addPhysicsComponent(e, &world, tilePos);
	Globals::entityManager->addRenderComponent(e, ENTITY_HUMAN, "Business Man", 0.2f, Direction::EAST, false);
	Globals::entityManager->addPlayerInputComponent(e);

	entityTracking = (PhysicsComponent *) Globals::entityManager->getComponentOfType(e, COMPONENT_PHYSICS);
}

GameState::~GameState()
{
	delete Globals::entityManager;
	delete Globals::entityFactory;
	delete Globals::spriteSheet;
}

void GameState::tempControlCamera(float delta)
{
	const static float viewSpeed = 400;

	float dx(0), dy(0);

	if (dynamic_cast<InputService*>(Locator::locate(SERVICE_INPUT))->isPressed(KEY_UP))
		dy = -delta;
	else if (dynamic_cast<InputService*>(Locator::locate(SERVICE_INPUT))->isPressed(KEY_DOWN))
		dy = delta;
	if (dynamic_cast<InputService*>(Locator::locate(SERVICE_INPUT))->isPressed(KEY_LEFT))
		dx = -delta;
	else if (dynamic_cast<InputService*>(Locator::locate(SERVICE_INPUT))->isPressed(KEY_RIGHT))
		dx = delta;

	if (dx || dy)
	{
		view.move(dx * viewSpeed, dy * viewSpeed);
		dynamic_cast<RenderService *>(Locator::locate(SERVICE_RENDER))->getWindow()->setView(view);
	}
}

void GameState::tick(float delta)
{
	world.tick(delta);

	if (dynamic_cast<InputService*>(Locator::locate(SERVICE_INPUT))->isFirstPressed(InputKey::KEY_YIELD_CONTROL))
		playerControl = !playerControl;

	// todo make sure this hack DOESN'T end up in production
	if (playerControl)
	{
		Globals::entityManager->tickSystems(delta);
		view.setCenter(Utils::toPixel(entityTracking->getPosition()));
		dynamic_cast<RenderService *>(Locator::locate(SERVICE_RENDER))->getWindow()->setView(view);
	}
	else
		tempControlCamera(delta);

}

void GameState::render(sf::RenderWindow &window)
{
	window.draw(world);

	dynamic_cast<RenderService *>(Locator::locate(SERVICE_RENDER))->renderEntities();
}

void GameState::handleInput(const sf::Event &event)
{
}

b2World *GameState::getBox2DWorld()
{
	return world.getBox2DWorld();
}