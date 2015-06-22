#include "game.hpp"
#include "state.hpp"
#include "gamestate.hpp"
#include "config.hpp"

GameState::GameState(BaseGame *game_) : State(game_, StateType::GAME)
{
	world.loadFromFile(Config::get<std::string>("debug-world-name"));

	// camera view
	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
	view.setCenter(world.getPixelSize().x / 2.f, world.getPixelSize().y / 2.f);
	view.zoom(0.5);
	game->setView(view);
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
	window.draw(world.getTerrain());
}

void GameState::handleInput(const sf::Event &event)
{
}
