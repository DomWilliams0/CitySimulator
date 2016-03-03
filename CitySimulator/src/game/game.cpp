#include <SFML/Window.hpp>
#include "game.hpp"
#include "state/gamestate.hpp"
#include "service/config_service.hpp"
#include "service/render_service.hpp"

Game::Game(sf::RenderWindow &window) : BaseGame(window), current(nullptr)
{
	window.setTitle(Config::getString("debug.window-title", "Game"));
	showFPS = true;
	limitFrameRate(Config::getInt("display.fps-limit", 60), Config::getBool("display.vsync", true));
}

void Game::start()
{
	switchState(StateType::STATE_GAME);
	box2DWorld = dynamic_cast<GameState *>(current)->getBox2DWorld();
}

Game::~Game()
{
	delete current;
}

void Game::end()
{
}

void Game::tick(float delta)
{
	current->tick(delta);
}

void Game::render(sf::RenderWindow &window)
{
	current->render(window);

	if (box2DWorld != nullptr)
		box2DWorld->DrawDebugData();
}

void Game::switchState(StateType newStateType)
{
	// destruct current
	if (newStateType == StateType::STATE_UNKNOWN)
	{
		delete current;
		states.pop();
		current = states.top();
	}

		// push new state onto stack
	else
	{
		State *newState(createFromStateType(newStateType));
		if (!newState) return;

		bool shouldDestruct = (current && current->type != STATE_GAME && newStateType != STATE_PAUSE);

		if (shouldDestruct)
		{
			states.pop();
			delete current;
		}

		current = newState;
		states.push(newState);
	}

	auto window = Locator::locate<RenderService>()->getWindow();
	window->setMouseCursorVisible(current->showMouse);
}

State *Game::createFromStateType(StateType type)
{
	switch (type)
	{
		case STATE_GAME:
			return new GameState;
		default:
			throw std::runtime_error("Not implemented");
	}
}
