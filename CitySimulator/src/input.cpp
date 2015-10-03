#include <SFML/Window/Keyboard.hpp>
#include <boost/format.hpp>
#include "input.hpp"
#include "logger.hpp"

using sf::Keyboard;

void Input::registerBindings()
{
	bindings.clear();
	bindings.insert({KEY_UP, Keyboard::Key::W});
	bindings.insert({KEY_LEFT, Keyboard::Key::A});
	bindings.insert({KEY_DOWN, Keyboard::Key::S});
	bindings.insert({KEY_RIGHT, Keyboard::Key::D});
	bindings.insert({KEY_YIELD_CONTROL, Keyboard::Key::Tab});

	// check all keys have been registered
	if (bindings.left.size() != KEY_COUNT)
	{
		auto msg = boost::format("Expected %1% key bindings, recieved %2% instead") % KEY_COUNT % bindings.left.size();

		Logger::logError(str(msg));
		throw std::runtime_error("Invalid number of key bindings");
	}
}

void Input::update(Keyboard::Key key, bool press)
{
	auto inputKey = bindings.right.find(key);
	if (inputKey != bindings.right.end())
	{
		int index = inputKey->second;
		pressed[index] = press;
	}
}


bool Input::isPressed(InputKey key)
{
	return pressed[key];
}

bool Input::isFirstPressed(InputKey key)
{
	return pressed[key] && !wasPressed[key];
}

void Input::advance()
{
	for (size_t i = 0; i < KEY_COUNT; ++i)
		wasPressed[i] = pressed[i];
}
