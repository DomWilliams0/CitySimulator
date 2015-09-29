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
	STOP_CONTROLLING,

	COUNT
};


class Input
{
public:

	Input() : pressed(COUNT), wasPressed(COUNT)
	{
	}

	void registerBindings();


	/// <summary>
	/// Updates the specified key with the given pressed value
	/// </summary>
	/// <param name="key">The key.</param>
	/// <param name="press">if set to <c>true</c> [press].</param>
	void update(sf::Keyboard::Key key, bool press);

	/// <summary>
	/// Determines whether the specified key is pressed.
	/// </summary>
	/// <param name="key">The key.</param>
	/// <returns></returns>
	bool isPressed(InputKey key);
	
	/// <summary>
	/// Determines whether the specified key has just been pressed.
	/// </summary>
	/// <param name="key">The key.</param>
	/// <returns></returns>
	bool isFirstPressed(InputKey key);


	/// <summary>
	/// Moves forward a timestep.
	/// </summary>
	void advance();
private:
	boost::bimap<InputKey, sf::Keyboard::Key> bindings;
	boost::dynamic_bitset<> pressed, wasPressed;
};
