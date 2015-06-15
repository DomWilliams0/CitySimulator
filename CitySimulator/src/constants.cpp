#define _USE_MATH_DEFINES
#include "constants.hpp"

namespace Constants
{
	const sf::Vector2i windowSize(1080, 728);
	const sf::Vector2i windowCentre(windowSize.x/2, windowSize.y/2);

	const int tileSize(16);
	const float tileSizef(tileSize);

	sf::Font mainFont;

	const float degToRad = static_cast<float>(M_PI / 180.0);
}
