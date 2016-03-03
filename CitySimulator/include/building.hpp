#ifndef CITYSIM_BUILDING_HPP
#define CITYSIM_BUILDING_HPP

#include <SFML/Graphics/Rect.hpp>
#include <map>

class World;

class Building
{
public:
	Building(const sf::IntRect &tileBounds, World &world);

	bool isWindowLit(const sf::Vector2i &tile);

	void setWindowLight(const sf::Vector2i &tile, bool lit);

private:
	World &world;
	std::map<sf::Vector2i, bool> windows;

};


#endif
