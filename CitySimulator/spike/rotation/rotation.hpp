#pragma once
#include "../../src/game.hpp"
#include "../../src/world.hpp"

#define TILE_SIZE 256

class RotationGame : public BaseGame
{
protected:
	void start() override;

	void tick(float delta) override;

	void render() override;

	void end() override;

	void handleInput(sf::Event e) override;

public:
	RotationGame(const sf::Vector2i &windowSize, const sf::Uint32 &style);

private:
	Tileset tileset;
	sf::VertexArray vertices;

	sf::RectangleShape reference;
};
