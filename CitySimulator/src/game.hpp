#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "state.hpp"

class Game
{
public:
	Game(const sf::Vector2i &windowSize, const sf::Uint32 &style);
	~Game();

	void start();
	void end();
	void switchState(State::StateType newScreenType);

private:
	sf::RenderWindow window;

	State *current;
	std::stack<State*> states;
	State* Game::createFromStateType(State::StateType type);

	sf::Color backgroundColor;
	
	void setWindowIcon();
};
