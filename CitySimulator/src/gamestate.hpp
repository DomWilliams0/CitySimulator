#pragma once
#include "state.hpp"
#include "world.hpp"

class GameState : public State
{
public:
	explicit GameState(BaseGame *game_);

	virtual void tick(float delta) override;
	virtual void render(sf::RenderWindow &window) override;
	virtual void handleInput(const sf::Event &event) override;

private:
	World world;
	sf::View view;

	sf::Vector2f viewOffset;
};
