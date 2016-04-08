#include "world.hpp"


void BuildingMap::addBuilding(const sf::IntRect &bounds, WorldID insideWorld)
{
	static BuildingID lastID = 0;

	BuildingID id = lastID++;
	Building &b = 
		buildings.emplace(id, Building{bounds, id, container->getID(), insideWorld}).first->second;
}



Building *BuildingMap::getBuildingByID(BuildingID id)
{
	auto it = buildings.find(id);
	return it == buildings.end() ? nullptr : &it->second;
}
