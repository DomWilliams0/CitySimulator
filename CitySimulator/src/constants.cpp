#define _USE_MATH_DEFINES
#include "constants.hpp"
#include "utils.hpp"

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

DirectionType Direction::random()
{
	return Utils::random(0, Direction::COUNT);
}

namespace Globals
{
	BaseGame *game;
	EntityManager *entityManager;
	EntityFactory *entityFactory;
	SpriteSheet *spriteSheet;
}

namespace Direction
{
	const int NORTH = 0;
	const int EAST = 1;
	const int SOUTH = 2;
	const int WEST = 3;

	const int COUNT = 4;
}
