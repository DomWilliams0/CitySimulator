#include <SFML/Window/Keyboard.hpp>
#include <boost/format.hpp>
#include "input.hpp"
#include "logger.hpp"

using sf::Keyboard;

void Input::registerBindings()
{
	bindings.clear();
	bindings.insert({UP, Keyboard::Key::W});
	bindings.insert({LEFT, Keyboard::Key::A});
	bindings.insert({DOWN, Keyboard::Key::S});
	bindings.insert({RIGHT, Keyboard::Key::D});
	bindings.insert({ STOP_CONTROLLING, Keyboard::Key::Tab });

	// check all keys have been registered
	if (bindings.left.size() != COUNT)
	{
		auto msg = boost::format("Expected %1% key bindings, recieved %2% instead") % COUNT % bindings.left.size();

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
	for (size_t i = 0; i < COUNT; ++i)
		wasPressed[i] = pressed[i];
}
