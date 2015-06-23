#pragma once
#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include <unordered_map>
#include "gamestate.hpp"
#include "logger.hpp"

class SpriteSheet
{
public:
	static void loadSprite(const std::string &fileName);

private:
	sf::Texture texture;
	std::unordered_map<std::string, sf::IntRect> sheets;

	std::vector<sf::Image> preProcessSpriteImages;

};

inline void SpriteSheet::loadSprite(const std::string &fileName)
{
	// placeholder
}