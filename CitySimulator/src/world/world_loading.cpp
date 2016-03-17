#include "service/config_service.hpp"
#include "service/logging_service.hpp"
#include "service/world_service.hpp"

WorldService::WorldLoader::WorldLoader(WorldTreeNode &treeRoot) : lastWorldID(0), treeRoot(treeRoot)
{
}

World *WorldService::WorldLoader::loadWorlds(const std::string &mainWorldName, Tileset &tileset)
{
	// load main world
	TMX::TileMap mainTMX;
	World *mainWorld = loadMainWorld(mainWorldName, tileset, mainTMX);
	if (mainWorld == nullptr)
	{
		Logger::logError("Failed to load main world");
		return nullptr;
	}

	// find all buildings from main world and allocate them IDs
	std::vector<std::string> buildingWorldNames;
	findBuildings(mainTMX, buildingWorldNames);

	for (auto &buildingName : buildingWorldNames)
	{
		Logger::logDebuggier(format("Found building '%1%' in main world", buildingName));
		World *buildingWorld = new World(++lastWorldID, buildingName);
		treeRoot.children.emplace_back(buildingWorld, &treeRoot);
	}


	return mainWorld;
}


World *WorldService::WorldLoader::loadMainWorld(const std::string &name, Tileset &tileset, TMX::TileMap &tmx)
{
	auto worldPath = Utils::joinPaths(Config::getResource("world.root"), name) + ".tmx";

	World *mainWorld = new World(0, worldPath);
	tmx.load(worldPath);
	mainWorld->loadFromFile(tmx);

	return mainWorld;
}


void WorldService::WorldLoader::findBuildings(TMX::TileMap &tmx, std::vector<std::string> &buildingWorldNames)
{
	auto buildingLayerIterator = std::find_if(tmx.layers.begin(), tmx.layers.end(),
											  [](const TMX::Layer *layer)
									  {
										  return layer->name == "buildings";
									  });

	if (buildingLayerIterator == tmx.layers.end())
	{
		Logger::logInfo("No \"buildings\" layer was found in main world");
		return;
	}

	TMX::Layer *buildingLayer = *buildingLayerIterator;

	for (TMX::Tile *tile : buildingLayer->items)
	{
		if (tile->getTileType() != TMX::TILE_PROPERTY_SHAPE)
			continue;

		TMX::PropertyObject *propObj = dynamic_cast<TMX::PropertyObject *>(tile);

		if (propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD))
			buildingWorldNames.push_back(propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD));
	}

}

std::string WorldService::WorldLoader::getBuildingFilePath(const std::string &name)
{
	static const std::string extension = ".tmx";

	return Utils::joinPaths(
			Utils::joinPaths(
					Config::getResource("world.root"),
					Config::getResource("world.buildings")),
			name + extension);
}