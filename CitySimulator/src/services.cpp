#include <SFML/Graphics.hpp>
#include "logger.hpp"
#include "utils.hpp"
#include "services.hpp"

void BaseService::onEnable()
{
}

void BaseService::onDisable()
{
}

Locator::Locator()
{
}

Locator::~Locator()
{
	for (auto &pair : services)
		delete pair.second;
}

// input

void InputService::onEnable()
{
	bindings.clear();
	bindings.left.insert({KEY_UP, sf::Keyboard::Key::W});
	bindings.left.insert({KEY_LEFT, sf::Keyboard::Key::A});
	bindings.left.insert({KEY_DOWN, sf::Keyboard::Key::S});
	bindings.left.insert({KEY_RIGHT, sf::Keyboard::Key::D});
	bindings.left.insert({KEY_YIELD_CONTROL, sf::Keyboard::Key::Tab});
	// todo load from config

	// check all keys have been registered
	if (bindings.left.size() != KEY_COUNT)
	{

		Logger::logError(format("Expected %1% key bindings, received %2% instead",
		                        std::to_string(KEY_COUNT), std::to_string(bindings.left.size())));

		error("Invalid number of key bindings");
	}
}

void InputService::update(sf::Keyboard::Key key, bool press)
{
	auto inputKey = bindings.right.find(key);
	if (inputKey != bindings.right.end())
	{
		int index = inputKey->second;
		pressed[index] = press;
	}
}

bool InputService::isPressed(InputKey key)
{
	return pressed[key];
}

bool InputService::isFirstPressed(InputKey key)
{
	return pressed[key] && !wasPressed[key];
}

void InputService::advance()
{
	for (size_t i = 0; i < KEY_COUNT; ++i)
		wasPressed[i] = pressed[i];
}


