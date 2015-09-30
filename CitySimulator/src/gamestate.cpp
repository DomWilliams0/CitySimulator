#include "gamestate.hpp"
#include "game.hpp"
#include "config.hpp"
#include "entity.hpp"
#include "animation.hpp"

GameState::GameState() : State(GAME)
{
	// create globals
	Globals::entityManager = new EntityManager;
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

	float zoom;
	Config::getFloat("debug-zoom", zoom);
	view.zoom(zoom);
	Globals::game->setView(view);

	// animation test
	std::string skin;
	Config::getString("debug-human-skin", skin);
	Animation *animation = Globals::spriteSheet->getAnimation(skin);
	testAnimator = new Animator(animation, 0.18f);

	// ecs test
	Entity e1 = Globals::entityManager->createEntity();
	Globals::entityManager->addPositionComponent(e1, 50, 50);
	Globals::entityManager->addVelocityComponent(e1, 10, 100);

	Entity e2 = Globals::entityManager->createEntity();
	Globals::entityManager->addPositionComponent(e2, 10, 10);
	Globals::entityManager->addRenderComponent(e2, "hue", Direction::EAST, true);
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
}

void GameState::tempControlAnimation(float delta)
{
	Input *input = Globals::game->getInput();

	// todo: temporary animator turning
	DirectionType direction = Direction::COUNT;
	if (input->isFirstPressed(UP))
		direction = Direction::NORTH;
	else if (input->isFirstPressed(DOWN))
		direction = Direction::SOUTH;
	else if (input->isFirstPressed(LEFT))
		direction = Direction::WEST;
	else if (input->isFirstPressed(RIGHT))
		direction = Direction::EAST;

	if (direction != Direction::COUNT)
		testAnimator->turn(direction, true);

	if (input->isFirstPressed(STOP_CONTROLLING))
		testAnimator->togglePlaying(true);

	testAnimator->tick(delta);
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
	window.draw(*testAnimator);
}

void GameState::handleInput(const sf::Event &event)
{
}
