#pragma once
#include <boost/smart_ptr/shared_ptr.hpp>
#include "state.hpp"
#include "world.hpp"

class GameState : public State
{
public:
	explicit GameState(Game *game_);

	virtual void tick(float delta) override;
	virtual void render(sf::RenderWindow &window) override;
	virtual void handleInput(const sf::Event &event) override;

private:
	boost::shared_ptr<BaseWorld> world;
};
