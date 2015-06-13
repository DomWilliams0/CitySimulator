#include <SFML/Window.hpp>
#include "game.hpp"
#include "gamestate.hpp"
#include "constants.hpp"
#include "logger.hpp"
#include "state.hpp"
#include "utils.hpp"

BaseGame::BaseGame(const sf::Vector2i &windowSize, const sf::Uint32 &style, const std::string &title) :
	window(sf::RenderWindow(sf::VideoMode(windowSize.x, windowSize.y), title, style))
{
	limitFrameRate(true);

	// set icon
	setWindowIcon("icon.png");

	// create logger
	Logger::createLogger(std::cout, Logger::DEBUG);

	// load font
	if (!Constants::mainFont.loadFromFile("res/font.ttf"))
	{
		Logger::logError("Font could not be loaded");
		exit(-1);
	}

	// load fps counter
	initFPSDisplay(fps);

	Logger::logDebug("Game started");
}

void BaseGame::beginGame()
{
	start();

	sf::Clock clock;
	sf::Event e;

	while (window.isOpen())
	{
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed || (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape))
				window.close();
			else
				handleInput(e);
		}

		float delta(clock.restart().asSeconds());
		tick(delta);

		window.clear();
		render();

		if (showFPS)
		{
			fps.setString(std::to_string(static_cast<int>(1 / delta)));
			window.draw(fps);
		}

		window.display();
	}

	end();
}

void BaseGame::limitFrameRate(bool limit)
{
	window.setFramerateLimit(limit ? 60 : 0);
	window.setVerticalSyncEnabled(limit);
}

void BaseGame::setWindowIcon(const std::string &fileName)
{
	sf::Image icon;
	if (!icon.loadFromFile(Utils::searchForFile(fileName)))
	{
		Logger::logWarning("Could not load icon");
		return;
	}

	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}

void BaseGame::initFPSDisplay(sf::Text &fps)
{
	fps.setFont(Constants::mainFont);
	fps.setCharacterSize(20);
	fps.setPosition(20, 20);
	fps.setColor(sf::Color::Red);
}

Game::Game(const sf::Vector2i &windowSize, const sf::Uint32 &style) : BaseGame(windowSize, style, "Dank C++ Memes")
{
	showFPS = true;
	limitFrameRate(false);
}

Game::~Game()
{
	delete current;
}

void Game::start()
{
	switchState(State::StateType::GAME);
}

void Game::tick(float delta)
{
	current->tick(delta);
}

void Game::render()
{
	current->render(window);
}

void Game::handleInput(sf::Event e)
{
	current->handleInput(e);
}

void Game::switchState(State::StateType newStateType)
{
	// destruct current
	if (newStateType == State::StateType::NONE)
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

		bool shouldDestruct = (current && current->type != State::GAME && newStateType != State::PAUSE);

		if (shouldDestruct)
		{
			states.pop();
			delete current;
		}

		current = newState;
		states.push(newState);
	}

	window.setMouseCursorVisible(current->showMouse);
}

State* Game::createFromStateType(State::StateType type)
{
	switch (type)
	{
	case State::GAME: return new GameState(this);
	default:
		throw std::runtime_error("Not implemented");
	}
}

void Game::end()
{
	window.close();
	Logger::logDebug("Game ended through game.end()");
}

int main()
{
	try
	{
		const sf::Vector2i windowSize = Constants::windowSize;
		const auto style = sf::Style::Default;

		Game *game = new Game(windowSize, style);
		dynamic_cast<BaseGame*>(game)->beginGame();
	}
	catch (std::exception e)
	{
		Logger::logError(std::string("An error occurred: ") + e.what());
	}
}
