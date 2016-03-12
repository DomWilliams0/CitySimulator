#include "world.hpp"
#include "service/logging_service.hpp"


void InteractionMap::load(const TMX::TileMap &tileMap)
{
	auto buildingLayer = std::find_if(tileMap.layers.begin(), tileMap.layers.end(),
	                                  [](const TMX::Layer *layer)
	                                  {
		                                  return layer->name == "buildings";
	                                  });

	if (buildingLayer == tileMap.layers.end())
	{
		Logger::logWarning("No \"buildings\" layer was found in InteractionMap::load");
		return;
	}

	TMX::Layer *layer = *buildingLayer;

	for (TMX::Tile *tile : layer->items) {
		if (tile->getTileType() != TMX::TILE_PROPERTY_SHAPE)
			continue;

		TMX::PropertyObject *propObj = dynamic_cast<TMX::PropertyObject*>(tile);

		// building entrances
		if (propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD))
		{
			std::string buildingWorld(propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD));
			std::string buildingID(propObj->getProperty(TMX::PROPERTY_BUILDING_ID));
			std::string doorID(propObj->getProperty(TMX::PROPERTY_BUILDING_DOOR));
			// todo

		}


	}




}
