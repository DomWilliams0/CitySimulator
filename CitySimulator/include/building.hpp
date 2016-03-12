#ifndef CITYSIMULATOR_BUILDING_HPP
#define CITYSIMULATOR_BUILDING_HPP

#include <SFML/Graphics/Rect.hpp>
#include <map>

class World;

/**
 * A building entrance/exit, either inside or outside
 */
struct Entrance
{
	World &world;
	sf::Vector2f localTilePos;
	int id;
};

class Building
{
public:
	Building(World &world, const sf::IntRect &tileBounds, int id, std::string buildingWorldName);

	bool isWindowLit(const sf::Vector2i &tile);

	void setWindowLight(const sf::Vector2i &tile, bool lit);

private:
	World *outsideWorld;
	World *insideWorld;
	int buildingID;
	std::string insideWorldName;

	std::map<sf::Vector2i, bool> windows;
	std::vector<Entrance> doors;



};


#endif
