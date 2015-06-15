#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "utils.hpp"
#include "state.hpp"
#include "constants.hpp"

class FPSCounter
{
public:

	inline void init(float waitTime)
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
	
	inline void setView(const sf::View &view)
	{
		window.setView(view);
	}

protected:
	sf::RenderWindow window;

	bool showFPS;

	virtual void start() = 0;
	virtual void tick(float delta) = 0;
	virtual void render() = 0;
	virtual void end() = 0;
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
};

class Game : public BaseGame
{
public:
	Game(const sf::Vector2i &windowSize, const sf::Uint32 &style);
	~Game();


protected:
	void start() override;
	void end() override;
	void tick(float delta) override;
	void render() override;
	void handleInput(sf::Event e) override;

public:
	void switchState(State::StateType newScreenType);

private:
	State *current;
	std::stack<State*> states;
	State* Game::createFromStateType(State::StateType type);
};
