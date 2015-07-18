#define _USE_MATH_DEFINES
#include "constants.hpp"

namespace Constants
{
	sf::Vector2i windowSize(-1, -1);
	sf::Vector2i windowCentre(-1, -1);

	const int tileSize(16);
	const float tileSizef(tileSize);

	sf::Font mainFont;

	const float degToRad = static_cast<float>(M_PI / 180.0);

	void setWindowSize(int x, int y)
	{
		windowSize.x = x;
		windowSize.y = y;

		windowCentre.x = x / 2;
		windowCentre.y = y / 2;
	}
}

namespace Globals
{
	BaseGame *game;
	EntityManager *entityManager;
	EntityFactory *entityFactory;
	SpriteSheet *spriteSheet;
}