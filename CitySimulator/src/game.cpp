#include <SFML/Window.hpp>
#include <numeric>
#include "game.hpp"
#include "logger.hpp"
#include "gamestate.hpp"
#include "config.hpp"

void FPSCounter::tick(float delta, sf::RenderWindow &window)
{
	backlog.push_back(delta);

	if (ticker.tick(delta))
	{
		int fps(0);

		double total = accumulate(backlog.begin(), backlog.end(), 0.0);
		if (total != 0)
		{
			double average = total / backlog.size();
			if (average != 0)
				fps = static_cast<int>(1.0 / average);
		}

		fpsText.setString(std::to_string(fps) + " FPS");

		backlog.clear();
	}

	window.draw(fpsText);
}


BaseGame::BaseGame(const sf::Vector2i &windowSize, const sf::Uint32 &style, const std::string &title) :
	window(sf::RenderWindow(sf::VideoMode(windowSize.x, windowSize.y), title, style))
{
	limitFrameRate(true);

	// set icon
	setWindowIcon("icon.png");

	// load font
	if (!Constants::mainFont.loadFromFile("res/font.ttf"))
	{
		Logger::logError("Font could not be loaded");
		exit(-1);
	}

	// key bindings
	window.setKeyRepeatEnabled(false);
	input.registerBindings();

	Logger::logDebug(title + " started");
}

void BaseGame::beginGame()
{
	start();

	fps.init(0.25);

	sf::Clock clock;
	sf::Event e;

	while (window.isOpen())
	{
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			// keys
			else if (e.type == sf::Event::KeyPressed)
			{
				// escape to quit
				if (e.key.code == sf::Keyboard::Escape)
					window.close();
				else
					input.update(e.key.code, true);
			}
			else if (e.type == sf::Event::KeyReleased)
				input.update(e.key.code, false);

			else
				handleInput(e);
		}

		float delta(clock.restart().asSeconds());
		tick(delta);

		window.clear(backgroundColour);
		render();

		// overlay
		if (showFPS)
		{
			sf::View view(window.getView());

			window.setView(window.getDefaultView());
			fps.tick(delta, window);
			window.setView(view);
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


Game::Game(const sf::Vector2i &windowSize, const sf::Uint32 &style) : BaseGame(windowSize, style, "Dank Game Memes")
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
	Logger::logDebug("Shutdown cleanly");
}

void loadConfig(int &windowStyle)
{
	Config::loadConfig();

	int width, height;

	// borderless fullscreen
	if (Config::get<bool>("display-borderless-fullscreen"))
	{
		windowStyle = sf::Style::None;

		auto screenSize(sf::VideoMode::getDesktopMode());
		width = screenSize.width;
		height = screenSize.height;
	}

	// standard window
	else
	{
		windowStyle = sf::Style::Default;
		width = Config::get<int>("display-resolution-width");
		height = Config::get<int>("display-resolution-height");
	}

	Constants::setWindowSize(width, height);
}


int main()
{
	try
	{
		// create logger
		createLogger(std::cout, Logger::DEBUG);

		// load window size/style
		int style;
		loadConfig(style);

		BaseGame *game;
		game = new Game(Constants::windowSize, style);
		dynamic_cast<BaseGame*>(game)->beginGame();
	}
	catch (std::exception &e)
	{
		Logger::logError(std::string("An error occurred: ") + e.what());
	}
	catch (...)
	{
		Logger::logError("An unknown error occured");
	}
}
