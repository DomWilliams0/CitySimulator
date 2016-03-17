#include "service/config_service.hpp"
#include "service/logging_service.hpp"
#include "service/world_service.hpp"

WorldService::WorldLoader::WorldLoader(WorldTreeNode &treeRoot) : lastWorldID(0), treeRoot(treeRoot)
{
}

World *WorldService::WorldLoader::loadWorlds(const std::string &mainWorldName, Tileset &tileset)
{
	this->treeRoot = treeRoot;

	// load main world
	World *mainWorld = loadMainWorld(mainWorldName, tileset);
	if (mainWorld == nullptr)
	{
		Logger::logError("Failed to load main world");
		return nullptr;
	}

	// load all buildings from main world
	findBuildings(mainWorld);


	return mainWorld;
}


World *WorldService::WorldLoader::loadMainWorld(const std::string &name, Tileset &tileset)
{
	auto worldPath = Utils::joinPaths(Config::getResource("world.root"), name) + ".tmx";

	World *mainWorld = new World(0);
	TMX::TileMap tmx;
	tmx.load(worldPath);
	mainWorld->loadFromFile(worldPath, flippedTileGIDs, tmx, &tileset);

	return mainWorld;
}


void WorldService::WorldLoader::findBuildings(World *world)
{
	// todo
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