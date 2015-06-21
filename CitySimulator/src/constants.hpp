#pragma once
#include <SFML/Graphics.hpp>

namespace Constants
{
	void setWindowSize(int x, int y);

	extern sf::Vector2i windowSize;
	extern sf::Vector2i windowCentre;

	extern const int tileSize;
	extern const float tileSizef;

	extern sf::Font mainFont;

	extern const float degToRad;
}
