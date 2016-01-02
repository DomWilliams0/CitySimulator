#ifndef CITYSIM_GAMESTATE_HPP
#define CITYSIM_GAMESTATE_HPP

#include "state.hpp"
#include "world.hpp"

class Animator;

class PhysicsComponent;

class GameState : public State
{
public:
	GameState();

	virtual void tick(float delta) override;

	virtual void render(sf::RenderWindow &window) override;

	b2World *getBox2DWorld();

private:
	World world;
	sf::View view;
	PhysicsComponent *entityTracking;

	sf::Vector2f viewOffset;
	bool playerControl;

	void tempControlCamera(float delta);
};

#endif