#define _USE_MATH_DEFINES
#include "constants.hpp"

namespace Constants
{
	const sf::Vector2i windowSize(1080, 728);
	const sf::Vector2i windowCentre(windowSize.x/2, windowSize.y/2);

	const int tilesetResolution(16);
	const int tileSize(32);
	const int tileScale(tileSize / tilesetResolution);
	const int realTileSize(tileSize * tileScale);

	sf::Font mainFont;

	const float degToRad = M_PI / 180;
}
