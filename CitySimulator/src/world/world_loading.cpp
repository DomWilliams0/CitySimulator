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

	std::vector<UnloadedBuilding> buildingsToLoad;
	std::vector<UnloadedDoor> doorsToLoad;

	// find all buildings from main world and allocate them IDs
	findBuildingsAndDoors(mainWorld.tmx, buildingsToLoad, doorsToLoad);

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
		UnloadedBuilding *owningBuilding = findBuildingOwner(door, buildingsToLoad);
		if (owningBuilding == nullptr)
		{
			Logger::logError(format("A door at (%1%, %2%) is not in any buildings!",
			                        _str(door.tile.x), _str(door.tile.y)));
			return nullptr;
		}

		door.doorTag = DOORTAG_WORLD_ID;
		door.worldID = owningBuilding->insideWorldID;
	}

	// load all worlds recursively without connecting doors
	discoverAndLoadAllWorlds(buildingsToLoad, doorsToLoad);

	return mainWorld.world;
}


void WorldService::WorldLoader::discoverAndLoadAllWorlds(
		std::vector<UnloadedBuilding> &buildings,
		std::vector<UnloadedDoor> &doors)
{
	std::vector<UnloadedDoor> nextDoors;
	std::vector<UnloadedBuilding> nextBuildings;

	// iterate all doors found in last world
	for (UnloadedDoor &door : doors)
	{
		// only positive IDs load worlds
		if (door.doorID <= 0)
			continue;

		// find the other door with same world share
		// todo make sure the list of ALL doors is searched
		if (door.doorTag == DOORTAG_WORLD_SHARE)
		{

			auto otherDoor = std::find_if(doors.begin(), doors.end(),
			                              [door](const UnloadedDoor &d)
			                              {
				                              return d.doorTag != DOORTAG_WORLD_SHARE &&
				                                     d.worldShare == door.worldShare;
			                              });

			if (otherDoor == doors.end())
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

			// add loaded buildings and doors to next recurse
			findBuildingsAndDoors(loadedWorld.tmx, nextBuildings, nextDoors);

		}
		else if (door.doorTag == DOORTAG_UNKNOWN)
		{
			Logger::logError(format("Door %1% has no assigned door tag", _str(door.doorID)));
			continue;
		}
	}

	if (!doors.empty())
		discoverAndLoadAllWorlds(nextBuildings, nextDoors);

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


WorldService::WorldLoader::UnloadedBuilding *WorldService::WorldLoader::findBuildingOwner(UnloadedDoor &door,
                                                               std::vector<UnloadedBuilding> &buildings)
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


void WorldService::WorldLoader::findBuildingsAndDoors(TMX::TileMap &tmx,
                                                      std::vector<UnloadedBuilding> &buildings,
                                                      std::vector<UnloadedDoor> &doors)
{
	auto buildingLayer = std::find_if(tmx.layers.begin(), tmx.layers.end(),
	                                  [](const TMX::Layer &layer)
	                                  {
		                                  return layer.name == "buildings";
	                                  });

	if (buildingLayer == tmx.layers.end())
	{
		Logger::logInfo("No \"buildings\" layer was found in main world");
		return;
	}

	for (const TMX::TileWrapper &tile : buildingLayer->items)
	{
		if (tile.type != TMX::TILE_PROPERTY_SHAPE)
			continue;

		const TMX::PropertyObject &propObj = tile.property;

		if (propObj.hasProperty(TMX::PROPERTY_BUILDING_WORLD))
		{
			// buildings
			sf::IntRect bounds(
					(int) (tile.tile.position.x / Constants::tilesetResolution),
					(int) (tile.tile.position.y / Constants::tilesetResolution),
					(int) (propObj.dimensions.x / Constants::tilesetResolution),
					(int) (propObj.dimensions.y / Constants::tilesetResolution)
			);

			UnloadedBuilding b;
			b.bounds = bounds;
			b.insideWorldName = propObj.getProperty(TMX::PROPERTY_BUILDING_WORLD);
			buildings.push_back(b);
		}

		else if (propObj.hasProperty(TMX::PROPERTY_BUILDING_DOOR))
		{
			// doors
			UnloadedDoor d;
			d.tile.x = (int) (tile.tile.position.x / Constants::tilesetResolution);
			d.tile.y = (int) (tile.tile.position.y / Constants::tilesetResolution);
			d.doorID = boost::lexical_cast<int>(propObj.getProperty(TMX::PROPERTY_BUILDING_DOOR));
			d.doorTag = DOORTAG_UNKNOWN;

			if (propObj.hasProperty(TMX::PROPERTY_BUILDING_WORLD_ID))
			{
				d.doorTag = DOORTAG_WORLD_ID;
				d.worldID = boost::lexical_cast<WorldID>(propObj.getProperty(TMX::PROPERTY_BUILDING_WORLD_ID));
			}
			else if (propObj.hasProperty(TMX::PROPERTY_BUILDING_WORLD))
			{
				d.doorTag = DOORTAG_WORLD_NAME;
				d.worldName = propObj.getProperty(TMX::PROPERTY_BUILDING_WORLD);
			}

			if (propObj.hasProperty(TMX::PROPERTY_BUILDING_WORLD_SHARE))
			{
				// only set if not already
				if (d.doorTag == DOORTAG_UNKNOWN)
					d.doorTag = DOORTAG_WORLD_SHARE;
				d.worldShare = propObj.getProperty(TMX::PROPERTY_BUILDING_WORLD_SHARE);
			}

			doors.push_back(d);
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