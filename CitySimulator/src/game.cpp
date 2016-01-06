#include <SFML/Window.hpp>
#include "game.hpp"
#include "gamestate.hpp"
#include "services.hpp"

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

	if (Config::getBool("display.centre"))
	{
		auto desktop = sf::VideoMode::getDesktopMode();
		auto windowSize = window->getSize();
		window->setPosition({
				                    static_cast<int>(desktop.width / 2) - static_cast<int>(windowSize.x / 2),
				                    static_cast<int>(desktop.height / 2) - static_cast<int>(windowSize.y / 2)
		                    });
	}

	// initially fill screen
	window->clear(backgroundColour);
	window->display();

	start();

	fps.init(Config::getFloat("debug.fps-tick-rate"));

	sf::Clock clock;
	sf::Event e;

	// todo separate physics from rendering

	while (window->isOpen())
	{
		window = Locator::locate<RenderService>()->getWindow();
		EventService *es = Locator::locate<EventService>();

		// process OS events
		while (window->pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window->close();

			else if (e.type == sf::Event::Resized)
				Locator::locate<CameraService>()->updateViewSize(e.size.width, e.size.height);

			else if (e.type == sf::Event::KeyPressed || e.type == sf::Event::KeyReleased)
			{
				Event event;
				event.type = EVENT_RAW_INPUT_KEY;
				event.rawInputKey.key = e.key.code;
				event.rawInputKey.pressed = e.type == sf::Event::KeyPressed;
				es->callEvent(event);
			}

			else if (e.type == sf::Event::MouseButtonPressed || e.type == sf::Event::MouseButtonReleased)
			{
				Event event;
				event.type = EVENT_RAW_INPUT_CLICK;
				event.rawInputClick.button = e.mouseButton.button;
				event.rawInputClick.x = e.mouseButton.x;
				event.rawInputClick.y = e.mouseButton.y;
				event.rawInputClick.pressed = e.type == sf::Event::MouseButtonPressed;
				es->callEvent(event);
			}
		}

		// process game events
		es->processQueue();

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
			auto windowSize = window->getSize();
			window->setView(sf::View(sf::FloatRect(0, 0, windowSize.x, windowSize.y)));
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
