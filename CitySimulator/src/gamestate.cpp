#include "gamestate.hpp"
#include "game.hpp"
#include "config.hpp"
#include "entity.hpp"
#include "animation.hpp"
#include "input.hpp"

GameState::GameState() : State(GAME)
{
	// create globals
	Globals::entityManager = new EntityManager;
	Globals::entityFactory = new EntityFactory;
	Globals::spriteSheet = new SpriteSheet;

	// load entities
	Globals::entityFactory->loadEntities(ENTITY_HUMAN, "humans.yml");
	Globals::entityFactory->loadEntities(ENTITY_VEHICLE, "vehicles.yml");

	// load sprites
	Globals::spriteSheet->processAllSprites();

	// load world
	std::string worldName;
	Config::getString("debug-world-name", worldName);
	world.loadFromFile(worldName);

	// camera view
	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
	view.setCenter(world.getPixelSize().x / 2.f, world.getPixelSize().y / 2.f);

	float zoom;
	Config::getFloat("debug-zoom", zoom);
	view.zoom(zoom);
	Globals::game->setView(view);

	Entity e = Globals::entityManager->createEntity();
	Globals::entityManager->addPositionComponent(e, 100.0f, 100.0f);
	Globals::entityManager->addRenderComponent(e, ENTITY_HUMAN, "Business Man", 0.18f, Direction::EAST, false);
	Globals::entityManager->addVelocityComponent(e, 0.0f, 0.0f);
	Globals::entityManager->addPlayerInputComponent(e);
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

	if (Globals::input->isPressed(KEY_UP))
		dy = -delta;
	else if (Globals::input->isPressed(KEY_DOWN))
		dy = delta;
	if (Globals::input->isPressed(KEY_LEFT))
		dx = -delta;
	else if (Globals::input->isPressed(KEY_RIGHT))
		dx = delta;

	if (dx || dy)
	{
		view.move(dx * viewSpeed, dy * viewSpeed);
		Globals::game->setView(view);
	}
}

void GameState::tick(float delta)
{
	world.tick(delta);

	Globals::entityManager->tickSystems(delta);

	tempControlCamera(delta);
}

void GameState::render(sf::RenderWindow &window)
{
	window.draw(world);

	Globals::entityManager->renderSystems(window);
}

void GameState::handleInput(const sf::Event &event)
{
	// temporary input test
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
	{
		if (Globals::entityManager->hasComponent(0, COMPONENT_INPUT))
			Globals::entityManager->removeComponent(0, COMPONENT_INPUT);
		else
			Globals::entityManager->addPlayerInputComponent(0);
	}
}
