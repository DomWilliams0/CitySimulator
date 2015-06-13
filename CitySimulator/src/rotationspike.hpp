#pragma once

#include <SFML/Graphics.hpp>

inline void generatePoints(const sf::FloatRect &rect, std::vector<sf::Vector2f> &points)
{
	points = {sf::Vector2f(rect.left, rect.top),
		sf::Vector2f(rect.left + rect.width, rect.top),
		sf::Vector2f(rect.left + rect.width, rect.top + rect.height),
		sf::Vector2f(rect.left, rect.top + rect.height)};
}

inline void rotationSpike()
{
	sf::RenderWindow window(sf::VideoMode(600, 600), "Rotation Spike");
	window.setFramerateLimit(60);

	sf::Texture spriteTexture;
	spriteTexture.loadFromFile("res/world/tileset.png", sf::IntRect(8 * 16, 0, 16, 16));

	//	sf::Sprite rectSprite(spriteTexture);
	//	rectSprite.setPosition(300, 300);
	//	rectSprite.setScale(6, 6);
	//	rectSprite.setOrigin(rectSprite.getLocalBounds().width / 2, rectSprite.getLocalBounds().height / 2);
	//
	//	auto bounds = rectSprite.getLocalBounds();
	//	std::vector<sf::Vector2f> points = {sf::Vector2f(bounds.left, bounds.top),
	//		sf::Vector2f(bounds.left + bounds.width, bounds.top),
	//		sf::Vector2f(bounds.left + bounds.width, bounds.top + bounds.height),
	//		sf::Vector2f(bounds.left, bounds.top + bounds.height)};
	//
	//	std::vector<sf::CircleShape> corners;
	//	for (int i = 0; i < 4; i++)
	//	{
	//		sf::CircleShape corner(5);
	//		corner.setFillColor(sf::Color::Yellow);
	//		corner.setOrigin(corner.getRadius() / 2, corner.getRadius() / 2);
	//		corners.push_back(corner);
	//	}

	sf::FloatRect bounds(0, 0, 160, 160);

	std::vector<sf::Vector2f> positions, texCoords;
	generatePoints(bounds, positions);
	generatePoints(sf::FloatRect(0, 0, 16, 16), texCoords);
	sf::Vector2f centre(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);

	int rotationDegrees = 160;

	sf::VertexArray quad(sf::Quads, 4);
	for (int i = 0; i < 4; ++i)
	{
		quad[i].texCoords = texCoords[i];

		sf::Vector2f prev = positions[i];
		prev.x -= centre.x;
		prev.y -= centre.y;

		sf::Vector2f newVertex(prev.x * cos(rotationDegrees) - prev.y * sin(rotationDegrees),
		                       prev.x * sin(rotationDegrees) + prev.y * cos(rotationDegrees));

		newVertex.x += centre.x;
		newVertex.y += centre.y;

		quad[i].position = newVertex;
		printf("%0f, %0f\t%0f, %0f\n", prev.x, prev.y, newVertex.x, newVertex.y);
	}


	sf::Event event;
	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				window.close();
		}

		window.clear();

		sf::RenderStates states;
		states.texture = &spriteTexture;

		window.draw(quad, states);

		window.display();
	}
}
