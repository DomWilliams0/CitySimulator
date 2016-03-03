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

	std::string referenceConfigFileName("reference.json");
	std::string userConfigFileName("config.json");

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
	return static_cast<DirectionType>(Utils::random(0, static_cast<int>(DIRECTION_UNKNOWN)));
}

DirectionType Direction::fromAngle(double degrees)
{
	const static double multiple = 360.0f / DIRECTION_UNKNOWN;

	int angle = static_cast<int>(multiple * round(degrees / multiple));

	switch (angle)
	{
		case 0:
			return DIRECTION_EAST;
		case 90:
			return DIRECTION_SOUTH;
		case -90:
			return DIRECTION_NORTH;
		default:
			return DIRECTION_WEST;
	}
}
