#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include "state.hpp"

class BaseGame
{
public:
	BaseGame(const sf::Vector2i &windowSize, const sf::Uint32 &style, const std::string &title);

	virtual ~BaseGame()
	{
	}
	void beginGame();

protected:
	sf::RenderWindow window;
	sf::Color backgroundColor;
	
	bool showFPS;

	virtual void start() = 0;
	virtual void tick(float delta) = 0;
	virtual void render() = 0;
	virtual void end() = 0;
	virtual void handleInput(sf::Event e) = 0;

	void limitFrameRate(bool limit);


private:
	void setWindowIcon(const std::string &fileName);
	void initFPSDisplay(sf::Text &fps);
	sf::Text fps;
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
