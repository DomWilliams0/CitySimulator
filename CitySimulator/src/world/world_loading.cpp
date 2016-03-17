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
	// todo promote to fields
	std::vector<UnloadedBuilding> buildingsToLoad;
	std::vector<UnloadedDoor> doorsToLoad;
	findBuildingsAndDoors(mainTMX, buildingsToLoad, doorsToLoad);

	for (auto &building : buildingsToLoad)
	{
		Logger::logDebuggier(format("Found building '%1%' in main world", building.insideWorldName));
		int id = ++lastWorldID;
		building.insideWorldID = id;
		World *buildingWorld = new World(id, building.insideWorldName);

		treeRoot.children.emplace_back(buildingWorld, &treeRoot);
	}

	// transfer building IDs to doors
	for (auto &door : doorsToLoad)
	{
		UnloadedBuilding *owningBuilding = findBuildingOwner(door, buildingsToLoad);
		if (owningBuilding == nullptr)
		{
			Logger::logError(format("A door at (%1%, %2%) is not in any buildings!",
									_str(door.tile.x), _str(door.tile.y)));
			return nullptr;
		}

		door.worldID = owningBuilding->insideWorldID;
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

WorldService::WorldLoader::UnloadedBuilding *WorldService::WorldLoader::findBuildingOwner(const WorldService::WorldLoader::UnloadedDoor &door,
															   std::vector<WorldService::WorldLoader::UnloadedBuilding> &buildings)
{
	const sf::Vector2i &tile = door.tile;

	for (auto &building : buildings)
	{
		const sf::IntRect &bounds = building.bounds;
		if (bounds.left <= tile.x && bounds.left + bounds.width >= tile.x &&
			bounds.top <= tile.y && bounds.top + bounds.height >= tile.y)
			return &building;
	}

	return nullptr;
}

void WorldService::WorldLoader::findBuildingsAndDoors(TMX::TileMap tmx,
													  std::vector<UnloadedBuilding> &buildingsToLoad,
													  std::vector<UnloadedDoor> &doorsToLoad)
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
		{
			// buildings
			sf::IntRect bounds(
					(int) (propObj->position.x / Constants::tilesetResolution),
					(int) (propObj->position.y / Constants::tilesetResolution),
					(int) (propObj->dimensions.x / Constants::tilesetResolution),
					(int) (propObj->dimensions.y / Constants::tilesetResolution)
			);

			UnloadedBuilding b;
			b.bounds = bounds;
			b.insideWorldName = propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD);
			buildingsToLoad.push_back(b);
		}

		else if (propObj->hasProperty(TMX::PROPERTY_BUILDING_DOOR))
		{
			// doors
			UnloadedDoor d;
			d.tile.x = (int) (propObj->position.x / Constants::tilesetResolution);
			d.tile.y = (int) (propObj->position.y / Constants::tilesetResolution);
			doorsToLoad.push_back(d);
		}
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