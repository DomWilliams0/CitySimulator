#include "world.hpp"
#include "service/logging_service.hpp"


void InteractionMap::load(const TMX::TileMap &tileMap)
{
	auto buildingLayer = std::find_if(tileMap.layers.begin(), tileMap.layers.end(),
	                                  [](const TMX::Layer *layer)
	                                  {
		                                  return layer->name == "building";
	                                  });

	if (buildingLayer == tileMap.layers.end())
	{
		Logger::logWarning("No \"building\" layer was found in InteractionMap::load");
		return;
	}

	TMX::Layer *layer = *buildingLayer;

	std::string buildingWorld(layer->getProperty(TMX::PROPERTY_BUILDING_WORLD));
	Logger::logDebug(buildingWorld);

}
