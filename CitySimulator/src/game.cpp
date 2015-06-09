#include <SFML/Window.hpp>
#include "game.hpp"
#include "gamestate.hpp"
#include "constants.hpp"
#include "logger.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "maploader.hpp"

Game::Game(const sf::Vector2i &windowSize, const sf::Uint32 &style) :
	window(sf::RenderWindow(sf::VideoMode(windowSize.x, windowSize.y), "Dank C++ Memes", style))
{
	// create window
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	// set icon
	setWindowIcon();

	// create logger
	Logger::createLogger(std::cout, Logger::DEBUG);

	// load font
	if (!Constants::mainFont.loadFromFile("res/font.ttf"))
	{
		Logger::logError("Font could not be loaded");
		exit(-1);
	}

	Logger::logDebug("Game started");
}

Game::~Game()
{
	delete current;
}

void Game::start()
{
	// create initial state
	switchState(State::StateType::GAME);

	sf::Clock clock;
	sf::Event event;
	
	sf::Text fps;
	initFPSDisplay(fps);

	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				window.close();
			else
				current->handleInput(event);
		}

		float delta(clock.restart().asSeconds());

		current->tick(delta);

		window.clear();
		current->render(window);
		
		fps.setString(std::to_string(static_cast<int>(1 / delta)));
		window.draw(fps);

		window.display();
	}
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

void Game::setWindowIcon()
{
	sf::Image icon;
	if (!icon.loadFromFile(Utils::searchForFile("icon.png")))
	{
		Logger::logWarning("Could not load icon");
		return;
	}

	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}

void Game::initFPSDisplay(sf::Text &fps)
{
	fps.setFont(Constants::mainFont);
	fps.setCharacterSize(20);
	fps.setPosition(20, 20);
	fps.setColor(sf::Color::Red);
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
		Game g(Constants::windowSize, sf::Style::Default);
		g.start();
	}
	catch (std::exception e)
	{
		Logger::logError(std::string("An error occurred: ") + e.what());
	}
}
