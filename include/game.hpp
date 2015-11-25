#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "utils.hpp"
#include "state.hpp"
#include "constants.hpp"
#include <Box2D/Dynamics/b2World.h>

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

	void setView(const sf::View &view)
	{
		window.setView(view);
	}

	sf::RenderWindow& getWindow()
	{
		return window;
	}

	void beginGame();
	void endGame();

protected:
	sf::RenderWindow &window;

	bool showFPS;

	virtual void start() = 0;
	virtual void end() = 0;
	virtual void tick(float delta) = 0;
	virtual void render() = 0;
	virtual void handleInput(sf::Event e) = 0;

	void limitFrameRate(bool limit);

	void setBackgroundColour(const sf::Uint8 &r, const sf::Uint8 &g, const sf::Uint8 &b, const sf::Uint8 &a = 255)
	{
		backgroundColour = sf::Color(r, g, b, a);
	}


private:
	void setWindowIcon(const std::string &fileName);

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
	void render() override;
	void handleInput(sf::Event e) override;

public:
	void switchState(StateType newScreenType);

private:
	b2World *box2DWorld;
	State *current;
	std::stack<State*> states;
	State* createFromStateType(StateType type);
};
