#include "world.hpp"


Building &BuildingMap::addBuilding(const sf::IntRect &bounds, WorldID insideWorld)
{
	static BuildingID lastID = 0;

	BuildingID id = lastID++;
	Building &b = 
		buildings.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(id),
				std::forward_as_tuple(bounds, id, container->getID(), insideWorld)
				).first->second;

	b.discoverWindows();
	return b;
}



Building *BuildingMap::getBuildingByID(BuildingID id)
{
	auto it = buildings.find(id);
	return it == buildings.end() ? nullptr : &it->second;
}
