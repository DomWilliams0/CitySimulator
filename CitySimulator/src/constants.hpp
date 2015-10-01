#pragma once
#include <SFML/Graphics.hpp>
#include <boost/format.hpp>
#include <unordered_map>

#define FORMAT(msg, argument) str(boost::format(msg) % argument)
#define FORMAT2(msg, arg1, arg2) str(boost::format(msg) % arg1 % arg2)
#define FAIL(msg, argument) throw std::runtime_error(FORMAT(msg, argument))
#define FAIL2(msg, arg1, arg2) throw std::runtime_error(str(boost::format(msg) % arg1 % arg2))

typedef std::unordered_map<std::string, std::string> ConfigMap;
typedef std::map<std::string, std::string> ConfigKeyValue;

namespace Constants
{
	void setWindowSize(int x, int y);

	extern sf::Vector2i windowSize;
	extern sf::Vector2i windowCentre;

	extern const int tileSize;
	extern const float tileSizef;

	extern const sf::Vector2f humanScale;

	extern sf::Font mainFont;

	extern const float degToRad;
}

class BaseGame;
class SpriteSheet;
class EntityManager;
class EntityFactory;

namespace Globals
{
	extern BaseGame *game;
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

	extern const int COUNT;
	DirectionType random();
}
