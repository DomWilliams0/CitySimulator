#ifndef CITYSIMULATOR_GAME_HPP
#define CITYSIMULATOR_GAME_HPP

#include <SFML/Graphics.hpp>
#include <stack>
#include "utils.hpp"
#include "state/state.hpp"

class b2World;

class FPSCounter
{
public:

	void init(float waitTime);

	void tick(float delta, sf::RenderWindow &window);

private:

	std::vector<float> backlog;

	sf::Text fpsText;
	Utils::TimeTicker ticker;
};


class BaseGame
{
public:
	BaseGame(sf::RenderWindow &window);

	virtual ~BaseGame()
	{
	}

	void beginGame();

	void endGame();

protected:
	bool showFPS;

	virtual void start() = 0;

	virtual void end() = 0;

	virtual void tick(float delta) = 0;

	virtual void render(sf::RenderWindow &window) = 0;

	void limitFrameRate(int limit, bool vsync);

	void setBackgroundColour(const sf::Uint8 &r, const sf::Uint8 &g, const sf::Uint8 &b, const sf::Uint8 &a = 255)
	{
		backgroundColour = sf::Color(r, g, b, a);
	}


private:
	void setWindowIcon(const std::string &path);

	sf::Color backgroundColour;
	FPSCounter fps;
};

class Game : public BaseGame
{
public:
	Game(sf::RenderWindow &window);

	~Game();

protected:
	void start() override;

	void end() override;

	void tick(float delta) override;

	void render(sf::RenderWindow &window) override;

public:
	void switchState(StateType newScreenType);

private:
	b2World *box2DWorld;
	State *current;
	std::stack<State *> states;

	State *createFromStateType(StateType type);
};

#endif