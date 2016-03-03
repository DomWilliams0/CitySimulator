#include <SFML/Window.hpp>
#include "game.hpp"
#include "state/gamestate.hpp"
#include "events.hpp"
#include "service/camera_service.hpp"
#include "service/config_service.hpp"
#include "service/event_service.hpp"
#include "service/input_service.hpp"
#include "service/logging_service.hpp"
#include "service/render_service.hpp"

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

	// position
	auto windowSize = window->getSize();
	window->setPosition({
			                    Config::getInt("display.position.x") - static_cast<int>(windowSize.x / 2),
			                    Config::getInt("display.position.y") - static_cast<int>(windowSize.y / 2)
	                    });

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

			else if (e.type == sf::Event::MouseWheelScrolled)
			{
				CameraService *camera = Locator::locate<CameraService>();
				sf::Vector2i mousePos = {e.mouseWheelScroll.x, e.mouseWheelScroll.y};

				const sf::Keyboard::Key &sprintKey = Locator::locate<InputService>()->getKey(KEY_SPRINT);
				const float increment = sf::Keyboard::isKeyPressed(sprintKey) ? 1.3f : 1.1f;
				float zoom = e.mouseWheelScroll.delta > 0 ? 1.f / increment : increment;

				camera->zoomTo(zoom, mousePos, *window);
			}

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
