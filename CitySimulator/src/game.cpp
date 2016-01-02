#include <SFML/Window.hpp>
#include "game.hpp"
#include "gamestate.hpp"

BaseGame::BaseGame(sf::RenderWindow &window)
{
	// graphics backend
	Locator::provide(SERVICE_RENDER, new RenderService(&window));

	// set icon
	setWindowIcon(Config::getResource("misc.icon"));

	// load font
	if (!Constants::mainFont.loadFromFile(Config::getResource("misc.font")))
	{
		Logger::logError("Font could not be loaded");
		exit(-1);
	}


	// key bindings
	window.setKeyRepeatEnabled(false);
	Locator::provide(SERVICE_INPUT, new InputService);

	Logger::logInfo("Game started");
}

void BaseGame::beginGame()
{
	sf::RenderWindow *window = Locator::locate<RenderService>()->getWindow();
	
	// initially fill screen
	window->clear(backgroundColour);
	window->display();

	start();

	fps.init(0.25);

	sf::Clock clock;
	sf::Event e;

	// todo separate physics from rendering

	while (window->isOpen())
	{
		window = Locator::locate<RenderService>()->getWindow();
		auto event = Locator::locate<EventService>();

		// process OS events
		while (window->pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window->close();

			else if (e.type == sf::Event::KeyPressed || e.type == sf::Event::KeyReleased)
				event->callRawInputKeyEvent(e.key.code, e.type == sf::Event::KeyPressed);
		}

		// process game events
		event->processQueue();

		// tick
		float delta(clock.restart().asSeconds());
		tick(delta);

		// render
		window->clear(backgroundColour);
		render(*window);

		// overlay
		if (showFPS)
		{
			// restore to default for gui display
			window->setView(window->getDefaultView());
			fps.tick(delta, *window);
		}

		window->display();
	}
}

void BaseGame::endGame()
{
	end();
	Locator::locate<RenderService>()->getWindow()->close();
}

void BaseGame::limitFrameRate(int limit, bool vsync)
{
	auto window = Locator::locate<RenderService>()->getWindow();

	window->setFramerateLimit(limit);
	window->setVerticalSyncEnabled(vsync);
}

void BaseGame::setWindowIcon(const std::string &path)
{
	sf::Image icon;
	if (!icon.loadFromFile(path))
	{
		Logger::logWarning("Could not load icon");
		return;
	}

	auto window = Locator::locate<RenderService>()->getWindow();
	window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}


Game::Game(sf::RenderWindow &window) : BaseGame(window), current(nullptr)
{
	window.setTitle(Config::getString("debug.window-title", "Game"));
	showFPS = true;
	limitFrameRate(Config::getInt("display.fps-limit", 60), Config::getBool("display.vsync", true));
}

void Game::start()
{
	switchState(StateType::GAME);
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
	if (newStateType == StateType::NONE)
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

		bool shouldDestruct = (current && current->type != GAME && newStateType != PAUSE);

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
		case GAME:
			return new GameState;
		default:
			throw std::runtime_error("Not implemented");
	}
}
