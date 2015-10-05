#include "constants.hpp"
#include "utils.hpp"

namespace Constants
{
	sf::Vector2i windowSize(-1, -1);
	sf::Vector2i windowCentre(-1, -1);

	const int tileSize(16);
	const float tileSizef(tileSize);
	const sf::Vector2f entityScale(0.5, 0.5);

	sf::Font mainFont;

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
	return Utils::random(0, Direction::DIRECTION_COUNT);
}

DirectionType Direction::fromAngle(float degrees)
{
	const static float multiple = 360.0f / DIRECTION_COUNT;

	int angle = static_cast<DirectionType>(multiple * roundf(degrees / multiple));

	switch (angle)
	{
	case 0: return EAST;
	case 90: return SOUTH;
	case -90: return NORTH;
	default: return WEST;
	}
}

namespace Globals
{
	BaseGame *game;
	Input *input;
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

	const int DIRECTION_COUNT = 4;
}
