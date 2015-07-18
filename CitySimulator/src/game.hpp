#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "utils.hpp"
#include "state.hpp"
#include "constants.hpp"
#include "input.hpp"

class FPSCounter
{
public:

	void init(float waitTime)
	{
		ticker.init(waitTime);

		fpsText.setFont(Constants::mainFont);
		fpsText.setCharacterSize(20);
		fpsText.setPosition(20, 20);
		fpsText.setColor(sf::Color::Red);
	}

	void tick(float delta, sf::RenderWindow &window);

private:

	std::vector<float> backlog;

	sf::Text fpsText;
	Utils::TimeTicker ticker;
};


class BaseGame
{
public:
	BaseGame(const sf::Vector2i &windowSize, const sf::Uint32 &style, const std::string &title);

	virtual ~BaseGame()
	{
	}

	void beginGame();
	virtual void end() = 0;

	inline void setView(const sf::View &view)
	{
		window.setView(view);
	}

	inline Input* getInput()
	{
		return &input;
	}

protected:
	sf::RenderWindow window;

	bool showFPS;

	virtual void start() = 0;
	virtual void tick(float delta) = 0;
	virtual void render() = 0;
	virtual void handleInput(sf::Event e) = 0;

	void limitFrameRate(bool limit);

	inline void setBackgroundColour(const sf::Uint8 &r, const sf::Uint8 &g, const sf::Uint8 &b, const sf::Uint8 &a = 255)
	{
		backgroundColour = sf::Color(r, g, b, a);
	}


private:
	void setWindowIcon(const std::string &fileName);

	sf::Color backgroundColour;
	FPSCounter fps;
	Input input;
};

class Game : public BaseGame
{
public:
	Game(const sf::Vector2i &windowSize, const sf::Uint32 &style);
	~Game();

	void end() override;

protected:
	void start() override;
	void tick(float delta) override;
	void render() override;
	void handleInput(sf::Event e) override;

public:
	void switchState(StateType newScreenType);

private:
	State *current;
	std::stack<State*> states;
	State* Game::createFromStateType(StateType type);
};
