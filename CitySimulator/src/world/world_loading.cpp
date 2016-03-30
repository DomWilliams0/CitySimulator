#include <boost/lexical_cast.hpp>
#include "service/config_service.hpp"
#include "service/logging_service.hpp"
#include "service/world_service.hpp"

WorldService::WorldLoader::WorldLoader(WorldTreeNode &treeRoot) : lastWorldID(0), treeRoot(treeRoot)
{
}

World *WorldService::WorldLoader::loadWorlds(const std::string &mainWorldName, Tileset &tileset)
{
	// load main world
	LoadedWorld &mainWorld = loadWorld(mainWorldName, false);
	if (mainWorld.failed())
	{
		Logger::logError("Failed to load main world");
		return nullptr;
	}

	// find all buildings from main world and allocate them IDs
	// todo pass collections to fill
	findBuildingsAndDoors(mainWorld.tmx);

	for (auto &building : buildingsToLoad)
	{
		Logger::logDebuggier(format("Found building '%1%' in main world", building.insideWorldName));
		building.insideWorldID = generateBuildingID();
		World *buildingWorld = new World(building.insideWorldID, building.insideWorldName);

		treeRoot.children.emplace_back(buildingWorld, &treeRoot);
	}

	// transfer building IDs to doors
	for (auto &door : doorsToLoad)
	{
		UnloadedBuilding *owningBuilding = findBuildingOwner(door);
		if (owningBuilding == nullptr)
		{
			Logger::logError(format("A door at (%1%, %2%) is not in any buildings!",
			                        _str(door.tile.x), _str(door.tile.y)));
			return nullptr;
		}

		door.doorTag = DOORTAG_WORLD_ID;
		door.worldID = owningBuilding->insideWorldID;
	}

	// recurse
	recurseOnDoors();

	return mainWorld.world;
}


void WorldService::WorldLoader::recurseOnDoors()
{
	for (UnloadedDoor &door : doorsToLoad)
	{
		// only positive IDs load worlds
		if (door.doorID <= 0)
			continue;

		// find the other door with same world share
		// todo make sure the list of ALL doors is searched
		if (door.doorTag == DOORTAG_WORLD_SHARE)
		{

			auto otherDoor = std::find_if(doorsToLoad.begin(), doorsToLoad.end(),
			                              [door](const UnloadedDoor &d)
			                              {
				                              return d.doorTag != DOORTAG_WORLD_SHARE &&
				                                     d.worldShare == door.worldShare;
			                              });

			if (otherDoor == doorsToLoad.end())
			{
				Logger::logError(format("Door %1% has an unknown world share tag '%2%'",
				                        _str(door.doorID), door.worldShare));
				continue;
			}


			// share world ID
			door.worldID = otherDoor->worldID;
		}

		// load world
		else if (door.doorTag == DOORTAG_WORLD_NAME)
		{
			LoadedWorld &loadedWorld = loadWorld(door.worldName, true);
			if (loadedWorld.failed())
			{
				Logger::logError(format("Cannot find building world '%1%', owner of door %2%",
				                        door.worldName, _str(door.doorID)));
				continue;
			}

			door.worldID = loadedWorld.world->getID();
		}
		else if (door.doorTag == DOORTAG_UNKNOWN)
		{
			Logger::logError(format("Door %1% has no assigned door tag", _str(door.doorID)));
			continue;
		}


	}


}
WorldService::WorldLoader::LoadedWorld &WorldService::WorldLoader::loadWorld(const std::string &name, bool isBuilding)
{
	WorldID id = generateBuildingID();
	loadedWorlds.emplace_back();
	LoadedWorld &loadedWorld = loadedWorlds[id];

	auto path = getWorldFilePath(name, isBuilding);

	loadedWorld.tmx.load(path);

	loadedWorld.world = new World(generateBuildingID(), path);
	loadedWorld.world->loadFromFile(loadedWorld.tmx);

	Logger::logDebuggier(format("World '%1%' has been allocated ID %2%", name, _str(id)));

	return loadedWorld;
}

WorldID WorldService::WorldLoader::generateBuildingID()
{
	return lastWorldID++;
}


WorldService::WorldLoader::UnloadedBuilding *WorldService::WorldLoader::findBuildingOwner(
		const WorldService::WorldLoader::UnloadedDoor &door)
{
	const sf::Vector2i &tile = door.tile;

	for (auto &building : buildingsToLoad)
	{
		const sf::IntRect &bounds = building.bounds;
		if (bounds.left <= tile.x && bounds.left + bounds.width >= tile.x &&
		    bounds.top <= tile.y && bounds.top + bounds.height >= tile.y)
			return &building;
	}

	return nullptr;
}

void WorldService::WorldLoader::findBuildingsAndDoors(TMX::TileMap tmx)
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
			d.doorID = boost::lexical_cast<int>(propObj->getProperty(TMX::PROPERTY_BUILDING_DOOR));
			d.doorTag = DOORTAG_UNKNOWN;

			if (propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD_ID))
			{
				d.doorTag = DOORTAG_WORLD_ID;
				d.worldID = boost::lexical_cast<WorldID>(propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD_ID));
			}
			else if (propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD))
			{
				d.doorTag = DOORTAG_WORLD_NAME;
				d.worldName = propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD);
			}

			if (propObj->hasProperty(TMX::PROPERTY_BUILDING_WORLD_SHARE))
			{
				// only set if not already
				if (d.doorTag == DOORTAG_UNKNOWN)
					d.doorTag = DOORTAG_WORLD_SHARE;
				d.worldShare = propObj->getProperty(TMX::PROPERTY_BUILDING_WORLD_SHARE);
			}

			doorsToLoad.push_back(d);
		}
	}

}

std::string WorldService::WorldLoader::getWorldFilePath(const std::string &name, bool isBuilding)
{
	static const std::string extension = ".tmx";

	if (isBuilding)
		return Utils::joinPaths(
				Utils::joinPaths(
						Config::getResource("world.root"),
						Config::getResource("world.buildings")),
				name + extension);
	else
		return Utils::joinPaths(
				Config::getResource("world.root"),
				name + extension);

}