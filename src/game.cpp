#include <SFML/Window.hpp>
#include "game.hpp"
#include "logger.hpp"
#include "gamestate.hpp"
#include "config.hpp"
#include "input.hpp"

BaseGame::BaseGame(sf::RenderWindow &renderWindow) : window(renderWindow)
{
	limitFrameRate(60, true); // default

	// set icon
	setWindowIcon("icon.png");

	// load font
	if (!Constants::mainFont.loadFromFile("res/misc/font.ttf"))
	{
		Logger::logError("Font could not be loaded");
		exit(-1);
	}

	// key bindings
	window.setKeyRepeatEnabled(false);
	Globals::input = new Input;
	Globals::input->registerBindings();

	// set as global
	Globals::game = this;

	Logger::logDebug("Game started");
}

void BaseGame::beginGame()
{
	// initially fill screen
	window.clear(backgroundColour);
	window.display();

	start();

	fps.init(0.25);

	sf::Clock clock;
	sf::Event e;

	// todo separate physics from rendering

	while (window.isOpen())
	{
		Globals::input->advance();

		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			// keys
			else if (e.type == sf::Event::KeyPressed || e.type == sf::Event::KeyReleased)
			{
				bool pressed = e.type == sf::Event::KeyPressed;

				if (pressed)
				{
					// escape to quit
					if (e.key.code == sf::Keyboard::Escape)
						window.close();
				}
				Globals::input->update(e.key.code, pressed);
				handleInput(e);
			}

			// everything else
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
}

void BaseGame::endGame()
{
	end();
	window.close();
}

void BaseGame::limitFrameRate(int limit, bool vsync)
{
	window.setFramerateLimit(limit);
	window.setVerticalSyncEnabled(vsync);
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


Game::Game(sf::RenderWindow &window) : BaseGame(window), current(nullptr)
{
	window.setTitle("Dank Game Memes");
	showFPS = true;
	limitFrameRate(Config::getInt("display.fps-limit"), Config::getBool("display.vsync"));
}

void Game::start()
{
	switchState(StateType::GAME);
	box2DWorld = dynamic_cast<GameState*>(current)->getBox2DWorld();
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

void Game::render()
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

	window.setMouseCursorVisible(current->showMouse);
}

State* Game::createFromStateType(StateType type)
{
	switch (type)
	{
	case GAME: return new GameState;
	default:
		throw std::runtime_error("Not implemented");
	}
}
