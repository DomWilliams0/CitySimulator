#include <SFML/Window.hpp>
#include "game.hpp"
#include "logger.hpp"
#include "gamestate.hpp"

BaseGame::BaseGame(sf::RenderWindow &window)
{
	// graphics backend
	Locator::provide(SERVICE_RENDER, new RenderService(&window));

	// set icon
	setWindowIcon("icon.png");

	// load font
	if (!Constants::mainFont.loadFromFile(Config::getResource("font")))
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
		auto input = Locator::locate<InputService>();

		input->advance();

		while (window->pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window->close();

				// keys
			else if (e.type == sf::Event::KeyPressed || e.type == sf::Event::KeyReleased)
			{
				bool pressed = e.type == sf::Event::KeyPressed;

				if (pressed)
				{
					// escape to quit
					if (e.key.code == sf::Keyboard::Escape)
						window->close();
				}
				input->update(e.key.code, pressed);
				handleInput(e);
			}

			else
			{
				// everything else
				handleInput(e);
			}
		}

		float delta(clock.restart().asSeconds());
		tick(delta);

		window->clear(backgroundColour);
		render(*window);

		// overlay
		if (showFPS)
		{
			sf::View view(window->getView());

			window->setView(window->getDefaultView());
			fps.tick(delta, *window);
			window->setView(view);
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

void BaseGame::setWindowIcon(const std::string &fileName)
{
	sf::Image icon;
	if (!icon.loadFromFile(Utils::searchForFile(fileName)))
	{
		Logger::logWarning("Could not load icon");
		return;
	}

	auto window = Locator::locate<RenderService>()->getWindow();
	window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}


Game::Game(sf::RenderWindow &window) : BaseGame(window), current(nullptr)
{
	window.setTitle(Config::getString("debug.window-title"));
	showFPS = true;
	limitFrameRate(Config::getInt("display.fps-limit"), Config::getBool("display.vsync"));
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

void Game::handleInput(sf::Event e)
{
	current->handleInput(e);
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
