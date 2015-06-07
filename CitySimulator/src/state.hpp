#pragma once
#include <SFML/Graphics.hpp>

class Game;

class State
{
public:
	virtual ~State()
	{
	}

	enum StateType
	{
		NONE,
		MENU,
		OPTIONS,
		GAME,
		PAUSE,
		GAMEOVER
	};

	State(Game *game_, StateType screenType, bool mouse = true) : game(game_), type(screenType), showMouse(mouse)
	{
	}

	virtual void tick(float delta) = 0;
	virtual void render(sf::RenderWindow &window) = 0;
	virtual void handleInput(const sf::Event &event) = 0;

	const StateType type;
	bool showMouse;

protected:
	Game *game;
};
