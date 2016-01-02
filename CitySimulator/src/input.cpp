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
	Logger::logDebug(format("%1% raw key %2%", event.rawInputKey.pressed? "Pressed" : "Released", std::to_string(event.rawInputKey.key)));
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