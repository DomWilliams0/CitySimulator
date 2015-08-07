#include "gamestate.hpp"
#include "game.hpp"
#include "config.hpp"
#include "entity.hpp"
#include "animation.hpp"

GameState::GameState() : State(GAME)
{
	// create globals
	//	Globals::entityManager = new EntityManager;
	Globals::entityFactory = new EntityFactory;
	Globals::spriteSheet = new SpriteSheet;

	// load entities
	Globals::entityFactory->loadEntities(HUMAN, "humans.yml");
	Globals::entityFactory->loadEntities(VEHICLE, "vehicles.yml");

	// load sprites
	Globals::spriteSheet->processAllSprites();

	// load world
	std::string worldName;
	Config::getString("debug-world-name", worldName);
	world.loadFromFile(worldName);

	// camera view
	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
	view.setCenter(world.getPixelSize().x / 2.f, world.getPixelSize().y / 2.f);
	view.zoom(0.5);
	Globals::game->setView(view);
}

GameState::~GameState()
{
	//	delete Globals::entityManager;
	delete Globals::entityFactory;
	delete Globals::spriteSheet;
}

void GameState::tick(float delta)
{
	const static float viewSpeed = 400;

	Input *input = Globals::game->getInput();
	float dx(0), dy(0);

	if (input->isPressed(UP))
		dy = -delta;
	else if (input->isPressed(DOWN))
		dy = delta;
	if (input->isPressed(LEFT))
		dx = -delta;
	else if (input->isPressed(RIGHT))
		dx = delta;

	if (dx || dy)
	{
		view.move(dx * viewSpeed, dy * viewSpeed);
		Globals::game->setView(view);
	}

	world.tick(delta);
}

void GameState::render(sf::RenderWindow &window)
{
	window.draw(world);
}

void GameState::handleInput(const sf::Event &event)
{
}
