#include <SFML/Window/Keyboard.hpp>
#include "services.hpp"

void InputService::onEnable()
{
	bindings.clear();

	bindKey(KEY_UP, sf::Keyboard::W);
	bindKey(KEY_LEFT, sf::Keyboard::A);
	bindKey(KEY_DOWN, sf::Keyboard::S);
	bindKey(KEY_RIGHT, sf::Keyboard::D);
	bindKey(KEY_YIELD_CONTROL, sf::Keyboard::Tab);
	bindKey(KEY_EXIT, sf::Keyboard::Escape);
	// todo load from config

	// check all keys have been registered
	if (bindings.left.size() != KEY_COUNT)
	{
		Logger::logError(format("Expected %1% key bindings, received %2% instead",
		                        std::to_string(KEY_COUNT), std::to_string(bindings.left.size())));

		error("Invalid number of key bindings");
	}

	// listen to input events
	auto events = Locator::locate<EventService>();
	events->registerListener(this, EVENT_RAW_INPUT_KEY);
}

void InputService::bindKey(InputKey binding, sf::Keyboard::Key key)
{
	std::string verb("Set");

	// remove existing
	auto existing = bindings.left.find(binding);
	if (existing != bindings.left.end())
	{
		verb = "Replaced existing";
		bindings.left.replace_data(existing, key);
	}

	bindings.left.insert({binding, key});
	Logger::logDebuggier(format("%1% binding for key %2%: %3%", verb, std::to_string(key), std::to_string(binding)));
}

void InputService::onEvent(const Event &event)
{
	InputKey binding(getBinding(event.rawInputKey.key));
	if (binding == KEY_COUNT)
		return;

	EntityID controlledEntity = 0; // todo get from EntityService

	// quit
	if (binding == KEY_EXIT)
	{
		Logger::logDebug("Exit key pressed, quitting");
		Locator::locate<RenderService>()->getWindow()->close();
		return;
	}

	// an entity is being controlled
	if (controlledEntity != -1)
	{
		EventService *es = Locator::locate<EventService>();

		Event e;
		e.entityID = controlledEntity;


		if (binding == KEY_YIELD_CONTROL)
		{
			e.type = EVENT_HUMAN_YIELD_CONTROL;
		}

		else
		{
			bool startMoving = event.rawInputKey.pressed;
			e.type = startMoving ? EVENT_HUMAN_START_MOVING : EVENT_HUMAN_STOP_MOVING;

				DirectionType direction;

				switch (binding)
				{
					case KEY_UP:
						direction = DIRECTION_NORTH;
						break;
					case KEY_LEFT:
						direction = DIRECTION_WEST;
						break;
					case KEY_DOWN:
						direction = DIRECTION_SOUTH;
						break;
					case KEY_RIGHT:
						direction = DIRECTION_EAST;
						break;
					default:
						error("An invalid movement key slipped through InputService's onEvent: %1%",
						      std::to_string(binding));
						return;
				}

			if (startMoving)
				e.startMove.direction = direction;
			else
				e.stopMove.direction = direction;
		}

		es->callEvent(e);
	}
}

sf::Keyboard::Key InputService::getKey(InputKey binding)
{
	auto result = bindings.left.find(binding);
	return result == bindings.left.end() ? sf::Keyboard::Unknown : result->second;
}

InputKey InputService::getBinding(sf::Keyboard::Key key)
{

	auto result = bindings.right.find(key);
	return result == bindings.right.end() ? InputKey::KEY_COUNT : result->second;
}