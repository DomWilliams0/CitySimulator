#include <boost/lexical_cast.hpp>
#include "world.hpp"
#include "service/logging_service.hpp"


void BuildingMap::gatherBuildings(TMX::Layer *buildingLayer, std::set<std::string> &worldsToLoad)
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
				(int) (propObj->position.x / Constants::tilesetResolution),
				(int) (propObj->position.y / Constants::tilesetResolution),
				(int) (propObj->dimensions.x / Constants::tilesetResolution),
				(int) (propObj->dimensions.y / Constants::tilesetResolution)
		);


		Building b(*container, bounds, buildingID, buildingWorld);
		buildings.insert({buildingID, b});

		worldsToLoad.insert("buildings/" + buildingWorld + ".tmx"); // todo have a getWorldPath function

		Logger::logDebuggiest(format("Found building %1% at (%2%, %3%)",
									 _str(buildingID), _str(bounds.left), _str(bounds.top)));
	}
}


void BuildingMap::load(const TMX::TileMap &tileMap, std::set<std::string> &worldsToLoad)
{
	Logger::logDebug("Loading buildings");
	Logger::pushIndent();

	auto buildingLayer = std::find_if(tileMap.layers.begin(), tileMap.layers.end(),
									  [](const TMX::Layer *layer)
									  {
										  return layer->name == "buildings";
									  });

	if (buildingLayer == tileMap.layers.end())
	{
		Logger::logWarning("No \"buildings\" layer was found in BuildingMap::load");
		Logger::popIndent();
		return;
	}

	TMX::Layer *layer = *buildingLayer;

	gatherBuildings(layer, worldsToLoad);

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

		sf::Vector2i doorPos = (sf::Vector2i) Math::multiply(propObj->position, 1.f / Constants::tilesetResolution);


		auto bFind = buildings.find(buildingID);
		if (bFind == buildings.end())
		{
			Logger::logWarning(format("Entrance at (%1%, %2%) has an unknown building ID %3%",
									  _str(doorPos.x), _str(doorPos.y), _str(buildingID)));
			continue;
		}

		bFind->second.addDoor(doorID, doorPos, container);
		Logger::logDebuggiest(format("Found door %1% to building %2%", _str(doorID), _str(buildingID)));
	}

	// windows
	for (auto &pair : buildings)
		pair.second.discoverWindows();

	Logger::popIndent();
	Logger::logDebug(format("Found %1% buildings", _str(buildings.size())));
}

void BuildingMap::getBuildingByOutsideDoorTile(const sf::Vector2i &tile,
											   boost::optional<std::pair<Building *, Door *>> &out)
{
	for (auto &it : buildings)
	{
		Door *door = it.second.getDoorByTile(tile);
		if (door != nullptr)
		{
			std::pair<Building *, Door *> pair = {&it.second, door};
			out = pair;
			return;
		}

	}

	Logger::logWarning(format("Could not find building from door at (%1%, %2%)", _str(tile.x), _str(tile.y)));
}

Building *BuildingMap::getBuildingByID(int id)
{
	auto it = buildings.find(id);
	return it == buildings.end() ? nullptr : &it->second;
}
