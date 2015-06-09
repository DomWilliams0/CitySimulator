#include "state.hpp"
#include "gamestate.hpp"
#include "logger.hpp"
#include <boost/smart_ptr/shared_ptr.hpp>


BaseWorld* createMainWorld()
{
	return BaseWorld::loadWorld("small.tmx");
}

GameState::GameState(Game *game_) : State(game_, StateType::GAME), world(createMainWorld())
{
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
