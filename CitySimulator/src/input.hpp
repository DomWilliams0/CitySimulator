#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>

enum InputKey
{
	UP,
	LEFT,
	DOWN,
	RIGHT,

	COUNT
};


class Input
{
public:

	Input() : pressed(COUNT)
	{
	}

	void registerBindings();
	void update(sf::Keyboard::Key key, bool press);
	bool isPressed(InputKey key);

private:
	boost::bimap<InputKey, sf::Keyboard::Key> bindings;
	boost::dynamic_bitset<> pressed;
};
