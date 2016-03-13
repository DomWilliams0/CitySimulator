#include <boost/lexical_cast.hpp>
#include "world.hpp"
#include "service/logging_service.hpp"
#include "building.hpp"


void BuildingMap::gatherBuildings(std::map<int, Building> buildings, TMX::Layer *buildingLayer)
{
	for (TMX::Tile *tile : buildingLayer->items)
	{
		if (tile->getTileType() != TMX::TILE_PROPERTY_SHAPE)
			continue;

		TMX::PropertyObject *propObj = dynamic_cast<TMX::PropertyObject *>(tile);

		if (!propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD))
			continue;

		std::string buildingWorld(propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD));
		int buildingID = boost::lexical_cast<int>(propObj->getProperty(TMX::PROPERTY_BUILDING_ID));

		sf::IntRect bounds(
				(int) (propObj->position.x / Constants::tileScale),
				(int) (propObj->position.y / Constants::tileScale),
				(int) (propObj->dimensions.x / Constants::tileScale),
				(int) (propObj->dimensions.y / Constants::tileScale)
		);


		Building b(*container, bounds, buildingID, buildingWorld);
		buildings.insert({buildingID, b});

		Logger::logDebuggiest(format("Found building %1% at (%2%, %3%)",
									 _str(buildingID), _str(bounds.left), _str(bounds.top)));
	}
}


void BuildingMap::load(const TMX::TileMap &tileMap, std::vector<std::string> &worldsToLoad)
{
	auto buildingLayer = std::find_if(tileMap.layers.begin(), tileMap.layers.end(),
	                                  [](const TMX::Layer *layer)
	                                  {
		                                  return layer->name == "buildings";
	                                  });

	if (buildingLayer == tileMap.layers.end())
	{
		Logger::logWarning("No \"buildings\" layer was found in BuildingMap::load");
		return;
	}

	TMX::Layer *layer = *buildingLayer;


	std::map<int, Building> buildings;
	gatherBuildings(buildings, layer);

	// entrances
	for (TMX::Tile *tile : layer->items)
	{
		if (tile->getTileType() != TMX::TILE_PROPERTY_SHAPE)
			continue;

		TMX::PropertyObject *propObj = dynamic_cast<TMX::PropertyObject *>(tile);

		if (!propObj->hasProperty(TMX::PROPERTY_BUILDING_DOOR))
			continue;


		int buildingID = boost::lexical_cast<int>(propObj->getProperty(TMX::PROPERTY_BUILDING_ID));
		int doorID = boost::lexical_cast<int>(propObj->getProperty(TMX::PROPERTY_BUILDING_DOOR));

		auto bFind = buildings.find(buildingID);
		if (bFind == buildings.end())
		{
			Logger::logWarning(format("Entrance at (%1%, %2%) has an unknown building ID '%3%'",
			                          _str(propObj->position.x), _str(propObj->position.y), _str(buildingID)));
			continue;
		}

		bFind->second.addDoor(doorID, tile->position, container);
		Logger::logDebuggiest(format("Found door %1% to building %2%", _str(doorID), _str(buildingID)));
	}
}
