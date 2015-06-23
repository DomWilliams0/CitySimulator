#pragma once
#include <SFML/Graphics.hpp>
#include <boost/variant.hpp>
#include <unordered_map>

#define FAIL(msg, argument) throw std::runtime_error(str(boost::format(msg) % argument));

typedef boost::variant<int, bool, std::string> ConfigValue;
typedef std::unordered_map<std::string, ConfigValue> ConfigMap;
typedef std::unordered_map<std::string, std::vector<std::pair<std::string, ConfigValue>>> EntityTags;

namespace Constants
{
	void setWindowSize(int x, int y);

	extern sf::Vector2i windowSize;
	extern sf::Vector2i windowCentre;

	extern const int tileSize;
	extern const float tileSizef;

	extern sf::Font mainFont;

	extern const float degToRad;
}
