#ifndef CITYSIMULATOR_STATE_HPP
#define CITYSIMULATOR_STATE_HPP

namespace sf
{
	class RenderWindow;
}

enum StateType
{
	STATE_MENU,
	STATE_OPTIONS,
	STATE_GAME,
	STATE_PAUSE,

	STATE_UNKNOWN
};

class State
{
public:
	virtual ~State()
	{
	}


	State(StateType screenType, bool mouse = true) : type(screenType), showMouse(mouse)
	{
	}

	virtual void tick(float delta) = 0;

	virtual void render(sf::RenderWindow &window) = 0;

	const StateType type;
	bool showMouse;
};

#endif