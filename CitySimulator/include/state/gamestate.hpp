#ifndef CITYSIMULATOR_GAMESTATE_HPP
#define CITYSIMULATOR_GAMESTATE_HPP

#include "state.hpp"
#include "world.hpp"

class Animator;

struct PhysicsComponent;

class GameState : public State
{
public:
	GameState();

	virtual void tick(float delta) override;

	virtual void render(sf::RenderWindow &window) override;

	b2World *getBox2DWorld();

private:
	World *mainWorld;
};

#endif
