#include "constants.hpp"
#include "utils.hpp"

namespace Constants
{
	sf::Vector2i windowSize(-1, -1);
	sf::Vector2i windowCentre(-1, -1);

	const int tileSize(32);
	const float tileSizef(tileSize);
	const int tilesetResolution(16);

	const float scale(tileSizef / tilesetResolution);
	const float tileScale(tileSizef * scale);
	
	const float entityScalef(1.f);
	const sf::Vector2f entityScale(entityScalef, entityScalef);

	sf::Font mainFont;

	std::string referenceConfigPath("res/reference.json");
	std::string configPath("res/config.json");

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

DirectionType Direction::fromAngle(double degrees)
{
	const static double multiple = 360.0f / DIRECTION_COUNT;

	int angle = static_cast<DirectionType>(multiple * round(degrees / multiple));

	switch (angle)
	{
		case 0:
			return EAST;
		case 90:
			return SOUTH;
		case -90:
			return NORTH;
		default:
			return WEST;
	}
}

namespace Globals
{
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
