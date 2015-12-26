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

	extern const float entityScalef;
	extern const sf::Vector2f entityScale;

	extern sf::Font mainFont;

	extern std::string referenceConfigPath;
	extern std::string configPath;
}

class BaseGame;

class SpriteSheet;

class EntityManager;

class EntityFactory;

class Input;

namespace Globals
{
	extern BaseGame *game;
	extern Input *input;
	extern SpriteSheet *spriteSheet;
	extern EntityManager *entityManager;
	extern EntityFactory *entityFactory;
}

typedef int DirectionType;

namespace Direction
{
	extern const int NORTH;
	extern const int EAST;
	extern const int SOUTH;
	extern const int WEST;

	extern const int DIRECTION_COUNT;

	DirectionType random();

	DirectionType fromAngle(double degrees);
}

enum EntityType
{
	ENTITY_HUMAN,
	ENTITY_VEHICLE
};
#endif