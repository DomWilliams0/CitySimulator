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


DirectionType Direction::parseString(const std::string &s)
{
	if (s == "N" || s == "n")
		return DIRECTION_NORTH;

	if (s == "E" || s == "e")
		return DIRECTION_EAST;

	if (s == "S" || s == "s")
		return DIRECTION_SOUTH;

	if (s == "W" || s == "w")
		return DIRECTION_WEST;

	return DIRECTION_UNKNOWN;
}


void Direction::toVector(DirectionType direction, sf::Vector2f &out)
{
	float x, y;
	toVector(direction, x, y);
	out.x = x;
	out.y = y;
}

void Direction::toVector(DirectionType direction, float &xOut, float &yOut)
{
switch (direction)
	{
		case DIRECTION_NORTH:
			xOut = 0;
			yOut = -1;
			break;
		case DIRECTION_EAST:
			xOut = 1;
			yOut = 0;
			break;
		case DIRECTION_SOUTH:
			xOut = 0;
			yOut = 1;
			break;
		case DIRECTION_WEST:
			xOut = -1;
			yOut = 0;
			break;
		default:
			error("An unknown direction cannot be converted to a vector");

	}
}


bool ::Direction::isHorizontal(DirectionType direction)
{
	return direction == DIRECTION_EAST || direction == DIRECTION_WEST;
}