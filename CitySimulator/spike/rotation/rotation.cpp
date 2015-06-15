#include "rotation.hpp"

inline void generatePoints(const sf::FloatRect &rect, std::vector<sf::Vector2f> &points)
{
	points = {sf::Vector2f(rect.left, rect.top),
		sf::Vector2f(rect.left + rect.width, rect.top),
		sf::Vector2f(rect.left + rect.width, rect.top + rect.height),
		sf::Vector2f(rect.left, rect.top + rect.height)};
}

inline void rotate(sf::Vertex *quad, float degrees, const sf::Vector2i &pos)
{
	sf::Vector2f centre(static_cast<float>(pos.x + TILE_SIZE / 2), static_cast<float>(pos.y + TILE_SIZE / 2));
	float radians = degrees * Constants::degToRad;

	for (int i = 0; i < 4; ++i)
	{
		sf::Vector2f prev = quad[i].position;
		prev.x -= centre.x;
		prev.y -= centre.y;

		sf::Vector2f newVertex(prev.x * cos(radians) - prev.y * sin(radians),
		                       prev.x * sin(radians) + prev.y * cos(radians));

		newVertex.x += centre.x;
		newVertex.y += centre.y;

		quad[i].position = newVertex;
	}
}


void RotationGame::start()
{
	const sf::Vector2f pos(250, 250);

	// reference
	reference.setPosition(pos.x, pos.y);
	reference.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	reference.setFillColor(sf::Color(0, 50, 0));

	// vertices
	std::vector<sf::Vector2f> positions;
	generatePoints(sf::FloatRect(pos.x, pos.y, TILE_SIZE, TILE_SIZE), positions);

	for (int i = 0; i < 4; ++i)
		vertices[i].position = positions[i];
	tileset.textureQuad(&vertices[0], TREE, 0, 0);
	tileset.convertToTexture(std::vector<int>());

	// rotation
	rotate(&vertices[0], 13, static_cast<sf::Vector2i>(pos));
}

void RotationGame::tick(float delta)
{
}

void RotationGame::render()
{
	window.draw(reference);

	sf::RenderStates states;
	states.texture = tileset.getTexture();
	window.draw(vertices, states);
}

void RotationGame::end()
{
}

void RotationGame::handleInput(sf::Event e)
{
}

RotationGame::RotationGame(const sf::Vector2i &windowSize, const sf::Uint32 &style)
	: BaseGame(windowSize, style, "Rotation Test"), tileset("tileset.png"), vertices(sf::Quads)
{
	vertices.resize(4);
}
