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

  	// allocate main world building IDs
	for (auto &building : mainWorld.buildings)
	{
		Logger::logDebuggier(format("Found building %1% '%2%' in main world", 
          			_str(building.insideWorldID), building.insideWorldName));
    	Logger::pushIndent();

    	loadWorld(building.insideWorldName, true, building.insideWorldID);

    	Logger::popIndent();
		// treeRoot.children.emplace_back(buildingWorld, &treeRoot);
	}

	// transfer building IDs to doors
	for (auto &door : mainWorld.doors)
	{
		UnloadedBuilding *owningBuilding = findBuildingOwner(door, mainWorld.buildings);
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
	discoverAndLoadAllWorlds(mainWorld);

	return mainWorld.world;
}


void WorldService::WorldLoader::discoverAndLoadAllWorlds(LoadedWorld &world)
{
  	static std::set<WorldID> done;

  	if (done.find(world.world->getID()) != done.end())
      	return;
  	done.insert(world.world->getID());

  	/* Logger::logDebuggier(format("Discovering worlds in %1%", _str(world.world->getID()))); */

	// iterate all doors found in last world
	for (UnloadedDoor &door : world.doors)
	{
		// only positive IDs load worlds
		if (door.doorID <= 0)
			continue;

    	LoadedWorld *newWorld = nullptr;

		// find the other door with same world share
		// todo make sure the list of ALL doors is searched
		if (door.doorTag == DOORTAG_WORLD_SHARE)
		{
			auto otherDoor = std::find_if(world.doors.begin(), world.doors.end(),
			        [door](const UnloadedDoor &d)
			        {
				    	return d.doorTag != DOORTAG_WORLD_SHARE &&
				    	d.worldShare == door.worldShare;
			        });

			if (otherDoor == world.doors.end())
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
      		Logger::logDebuggiest(format("Door ID %1% loads world '%2%'", _str(door.doorID), door.worldName));
			LoadedWorld &loadedWorld = loadWorld(door.worldName, true);
			if (loadedWorld.failed())
			{
				Logger::logError(format("Cannot find building world '%1%', owner of door %2%",
				            door.worldName, _str(door.doorID)));
				continue;
			}

			door.worldID = loadedWorld.world->getID();
      		newWorld = &loadedWorld;
		}

		else if (door.doorTag == DOORTAG_UNKNOWN)
		{
			Logger::logError(format("Door %1% has no assigned door tag", _str(door.doorID)));
      		return;
		}

    	if (newWorld == nullptr)
      		newWorld = &loadedWorlds[door.worldID];

    	discoverAndLoadAllWorlds(*newWorld);
	}
}

WorldService::WorldLoader::LoadedWorld &WorldService::WorldLoader::loadWorld(const std::string &name, 
    	bool isBuilding)
{
  	return loadWorld(name, isBuilding, generateWorldID());
}

WorldService::WorldLoader::LoadedWorld &WorldService::WorldLoader::loadWorld(const std::string &name, 
    	bool isBuilding, WorldID worldID)
{
  	// create new LoadedWorld in place
  	LoadedWorld &loadedWorld = loadedWorlds.emplace(worldID, LoadedWorld{}).first->second;

	// load tmx
	auto path = getWorldFilePath(name, isBuilding);
	loadedWorld.tmx.load(path);
	loadedWorld.world = new World(worldID, path);
	loadedWorld.world->loadFromFile(loadedWorld.tmx);

	// find buildings and doors
	auto buildingLayer = std::find_if(loadedWorld.tmx.layers.begin(), loadedWorld.tmx.layers.end(),
	        [](const TMX::Layer &layer)
	        {
		    return layer.name == "buildings";
	        });

	// no buildings layer
	if (buildingLayer == loadedWorld.tmx.layers.end())
  	{
    	Logger::logDebuggier("No \"buildings\" layer");
		return loadedWorld;
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
      		b.insideWorldID = generateWorldID();
			loadedWorld.buildings.push_back(b);
		}

		else if (propObj.hasProperty(TMX::PROPERTY_DOOR_ID))
		{
			// doors
			UnloadedDoor d;
			d.tile.x = (int) (tile.tile.position.x / Constants::tilesetResolution);
			d.tile.y = (int) (tile.tile.position.y / Constants::tilesetResolution);
			d.doorID = boost::lexical_cast<int>(propObj.getProperty(TMX::PROPERTY_DOOR_ID));
			d.doorTag = DOORTAG_UNKNOWN;

			// preloaded
			if (propObj.hasProperty(TMX::PROPERTY_DOOR_ID))
			{
				d.doorTag = DOORTAG_WORLD_ID;
				d.worldID = boost::lexical_cast<WorldID>(propObj.getProperty(TMX::PROPERTY_DOOR_ID));
			}

			// unloaded
			if (propObj.hasProperty(TMX::PROPERTY_DOOR_WORLD))
			{
				d.doorTag = DOORTAG_WORLD_NAME;
				d.worldName = propObj.getProperty(TMX::PROPERTY_DOOR_WORLD);
			}

			// sharing
			else if (propObj.hasProperty(TMX::PROPERTY_DOOR_WORLD_SHARE_SPECIFIER))
			{
				d.doorTag = DOORTAG_WORLD_SHARE;
				d.worldShare = propObj.getProperty(TMX::PROPERTY_DOOR_WORLD_SHARE_SPECIFIER);
			}

			// share source
			if (propObj.hasProperty(TMX::PROPERTY_DOOR_WORLD_SHARE_SOURCE))
			{
				d.worldShare = propObj.getProperty(TMX::PROPERTY_DOOR_WORLD_SHARE_SOURCE);
			}

			loadedWorld.doors.push_back(d);
		}
	}

	return loadedWorld;
}

WorldID WorldService::WorldLoader::generateWorldID()
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


std::string WorldService::WorldLoader::getWorldFilePath(const std::string &name, bool isBuilding)
{
	static const std::string extension = ".tmx";

  	const std::string &root = Config::getResource(isBuilding ? "world.buildings" : "world.root");
  	return Utils::joinPaths(
      		root,
      		name + extension
      		);
}
