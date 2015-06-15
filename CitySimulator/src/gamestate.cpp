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
//	view.setSize(static_cast<sf::Vector2f>(Constants::windowSize));
//	game->setView(view);
}

GameState::~GameState()
{
	delete world;
}

void GameState::tick(float delta)
{

}

void GameState::render(sf::RenderWindow &window)
{
	window.draw(*world);
}

void GameState::handleInput(const sf::Event &event)
{
}
