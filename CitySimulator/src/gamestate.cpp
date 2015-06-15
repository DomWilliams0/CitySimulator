#include "game.hpp"
#include "state.hpp"
#include "gamestate.hpp"


BaseWorld* createMainWorld()
{
	return BaseWorld::loadWorld("small.tmx");
}

GameState::GameState(BaseGame *game_) : State(game_, StateType::GAME), world(createMainWorld())
{
	// camera view
	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
//	view.zoom(1);

	game->setView(view);
}

GameState::~GameState()
{
	delete world;
}

void GameState::tick(float delta)
{
	const static float viewSpeed = 400;

	Input *input = game->getInput();
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
		game->setView(view);
	}

}

void GameState::render(sf::RenderWindow &window)
{
	window.draw(*world);
}

void GameState::handleInput(const sf::Event &event)
{
}
