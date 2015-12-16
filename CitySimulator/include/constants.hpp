#ifndef CITYSIM_CONSTANTS_HPP
#define CITYSIM_CONSTANTS_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <boost/format.hpp>

#define FORMAT(msg, argument) str(boost::format(msg) % argument)
#define FORMAT2(msg, arg1, arg2) str(boost::format(msg) % arg1 % arg2)
#define ERROR(msg, argument) throw std::runtime_error(FORMAT(msg, argument))
#define ERROR2(msg, arg1, arg2) throw std::runtime_error(FORMAT2(msg, arg1, arg2))

typedef std::unordered_map<std::string, std::string> ConfigMap;
typedef std::map<std::string, std::string> ConfigKeyValue;

namespace Constants
{
	void setWindowSize(int x, int y);

	extern sf::Vector2i windowSize;
	extern sf::Vector2i windowCentre;

	extern const int tileSize;
	extern const float tileSizef;

	extern const sf::Vector2f entityScale;

	extern sf::Font mainFont;
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