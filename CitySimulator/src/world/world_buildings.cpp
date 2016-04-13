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


void BuildingMap::getBuildingByOutsideDoorTile(const sf::Vector2i &tile,
		boost::optional<std::pair<BuildingID, DoorID>> &out)
{
	for (auto &buildingPair : buildings)
	{
		Building &building = buildingPair.second;
		DoorID doorCount = building.getDoorCount();
		for (DoorID doorID = 0; doorID <= doorCount; ++doorID)
		{
			Door *door = building.getDoor(doorID);
			if (door == nullptr)
				continue;

			Location &loc = door->location;
			if (loc.world == building.getOutsideWorld()->getID() &&
				loc.x == tile.x && loc.y == tile.y)
			{
				out = {building.getID(), doorID};
				return;
			}
		}

	}


}
