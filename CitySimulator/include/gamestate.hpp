#pragma once
#include "state.hpp"
#include "world.hpp"

class Animator;

class GameState : public State
{
public:
	GameState();
	~GameState();
	virtual void tick(float delta) override;
	virtual void render(sf::RenderWindow &window) override;
	virtual void handleInput(const sf::Event &event) override;
	b2World* getBox2DWorld();
private:
	World world;
	sf::View view;

	sf::Vector2f viewOffset;

	void tempControlCamera(float delta);
};
