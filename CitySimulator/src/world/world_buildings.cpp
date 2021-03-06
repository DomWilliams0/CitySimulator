#include "world.hpp"

void DomesticConnectionMap::addDoor(const sf::Vector2i &tile)
{
	Location loc(container->getID(), tile);
	Door d;
	d.location = loc;

	doors.insert({loc, d});
}
Door *DomesticConnectionMap::getDoorByTile(const sf::Vector2i &tile)
{
	Location loc(container->getID(), tile);
	auto it = doors.find(loc);
	return it == doors.end() ? nullptr : &it->second;
}


Building &BuildingConnectionMap::addBuilding(const sf::IntRect &bounds, WorldID insideWorld)
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



Building *BuildingConnectionMap::getBuildingByID(BuildingID id)
{
	auto it = buildings.find(id);
	return it == buildings.end() ? nullptr : &it->second;
}


void BuildingConnectionMap::getBuildingByOutsideDoorTile(const sf::Vector2i &tile,
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
