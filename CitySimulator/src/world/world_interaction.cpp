#include <boost/lexical_cast.hpp>
#include "world.hpp"
#include "service/logging_service.hpp"
#include "building.hpp"


void BuildingMap::gatherBuildings(std::vector<Building> &buildings, TMX::Layer *buildingLayer)
{
	for (TMX::Tile *tile : buildingLayer->items)
	{
		if (tile->getTileType() != TMX::TILE_PROPERTY_SHAPE)
			continue;

		TMX::PropertyObject *propObj = dynamic_cast<TMX::PropertyObject *>(tile);

		if (!propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD))
			continue;

		std::string buildingWorld(propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD));
		std::string buildingID(propObj->getProperty(TMX::PROPERTY_BUILDING_ID));

		sf::IntRect bounds(
				(int) (propObj->position.x / Constants::tileScale),
				(int) (propObj->position.y / Constants::tileScale),
				(int) (propObj->dimensions.x / Constants::tileScale),
				(int) (propObj->dimensions.y / Constants::tileScale)
		);


		buildings.emplace_back(*container, bounds, boost::lexical_cast<int>(buildingID), buildingWorld);
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


		std::vector<Building> buildings;
		gatherBuildings(buildings, layer);

		// entrances
		for (TMX::Tile *tile : layer->items)
		{
			if (tile->getTileType() != TMX::TILE_PROPERTY_SHAPE)
				continue;

			TMX::PropertyObject *propObj = dynamic_cast<TMX::PropertyObject *>(tile);

			if (!propObj->hasProperty(TMX::PROPERTY_BUILDING_DOOR))
				continue;


			std::string buildingID(propObj->getProperty(TMX::PROPERTY_BUILDING_ID));
			std::string doorID(propObj->getProperty(TMX::PROPERTY_BUILDING_DOOR));
			// todo


		}
	}
