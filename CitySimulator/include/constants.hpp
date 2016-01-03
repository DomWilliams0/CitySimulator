#ifndef CITYSIM_CONSTANTS_HPP
#define CITYSIM_CONSTANTS_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <boost/format.hpp>

typedef std::unordered_map<std::string, std::string> ConfigMap;
typedef std::map<std::string, std::string> ConfigKeyValue;

namespace Constants
{
	void setWindowSize(int x, int y);

	extern sf::Vector2i windowSize;
	extern sf::Vector2i windowCentre;

	extern const int tileSize;
	extern const float tileSizef;
	extern const int tilesetResolution;

	extern const float scale;
	extern const float tileScale;

	extern const float entityScalef;
	extern const sf::Vector2f entityScale;

	extern sf::Font mainFont;

	extern std::string referenceConfigFileName;
	extern std::string userConfigFileName;
}

enum DirectionType
{
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST,

	DIRECTION_COUNT
};

namespace Direction
{
	DirectionType random();

	DirectionType fromAngle(double degrees);
}

enum EntityType
{
	ENTITY_HUMAN,
	ENTITY_VEHICLE
};
#endif