#include "world.hpp"
#include "service/locator.hpp"

WorldService::WorldService(const std::string &mainWorldPath, const std::string &tilesetPath)
		: mainWorldName(mainWorldPath), tileset(tilesetPath)
{
}

void WorldService::onEnable()
{
	Logger::logDebug("Starting to load worlds");
	Logger::pushIndent();
	
	// load and connect all worlds
	WorldLoader loader(connectionLookup, terrainCache);
	loader.loadWorlds(mainWorldName);

	// transfer loaded worlds
	for (auto &lwPair : loader.loadedWorlds)
	{
		World *world = lwPair.second.world;
		worlds[world->getID()] = world;

	}
	// generate tileset
	tileset.load();
	tileset.convertToTexture(loader.flippedTileGIDs);



	// load terrain
	for (auto &pair : terrainCache)
		pair.second.applyTiles(tileset);

	// transfer buildings
	BuildingMap *bm = getMainWorld()->getBuildingMap();
	for (auto &building : loader.buildings)
		bm->addBuilding(building.bounds, building.insideWorldID);


	Logger::popIndent();

	/*
	 * todo: building loading
	 * use a tree for worlds loaded in worlds
	 * share terrain/collisionmap across same world maps: easy peasy! just share worldterrain etc.
	 * assign IDs incrementally
	 *
	 * collect all building bounds and WORLD tags in main world
	 * load all these building worlds and allocate IDs
	 *
	 * for every door in main world:
	 * 	find containing building
	 * 	set world ID tag to WORLD_ID from building
	 *
	 * now all doors with positive door numbers have a WORLD or WORLD_ID tag
	 * time to recurse
	 *
	 * sort doors so all with WORLD_ID and WORLD_SHARE tag are first
	 *
	 * for every door:
	 * 	read DOOR
	 *		positive: goes inside/to a child node
	 * 		negative: goes outside/up to parent node
	 * 		0: illegal, indexing starts at 1
	 *
	 * 	if positive:
	 * 		if door has a WORLD_ID tag:
	 * 			WORLD_INSTANCE = already loaded world with id WORLD_ID
	 *
	 * 		else if door has a WORLD_SHARE tag:
	 * 			find a door with same WORLD_SHARE and WORLD_ID tag
	 * 			it should already be loaded due to sorting above
	 * 			WORLD_ID = other door WORLD_ID
	 * 			WORLD_INSTANCE = other door WORLD_INSTANCE
	 *
	 * 		else no already loaded world tag:
	 * 			read WORLD tag
	 * 			WORLD_ID = newly allocated ID
	 * 			WORLD_INSTANCE = create new instance and save with new id WORLD_ID
	 *
	 * 	recurse on all new child nodes
	 *	END
	 *
	 * now all worlds are loaded, connections are added
	 * recursion time again
	 *
	 * for every door:
	 * 	if positive:
	 * 		get world from door WORLD_ID in children
	 * 		from this world, get door with negative DOOR_ID
	 * 		add connection between this door and that door
	 * 	else:
	 * 		get door with positive DOOR_ID in parent
	 * 		add connection between this door and that door
	 *
	 * recurse on all children
	 * END
	 */
}

void WorldService::onDisable()
{
	Logger::logDebug("Deleting all loaded worlds");
	for (auto &pair : worlds)
		delete pair.second;
}


World *WorldService::getMainWorld()
{
	return getWorld(0);
}

World *WorldService::getWorld(WorldID id)
{
	auto world = worlds.find(id);
	return world == worlds.end() ? nullptr : world->second;
}

World::World(WorldID id, const std::string &name) : id(id), name(name), buildingMap(this)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
}

void World::setTerrain(WorldTerrain &terrain)
{
	this->terrain = &terrain;
}

WorldTerrain *World::getTerrain()
{
	return terrain;
}

CollisionMap *World::getCollisionMap()
{
	return terrain == nullptr ? nullptr : terrain->getCollisionMap();
}

BuildingMap *World::getBuildingMap()
{
	return &buildingMap;
}

b2World *World::getBox2DWorld()
{
	return &getCollisionMap()->world;
}

sf::Vector2i World::getPixelSize() const
{
	return Utils::toPixel(getTileSize());
}

sf::Vector2i World::getTileSize() const
{
	return terrain->size;
}

sf::Transform World::getTransform() const
{
	return transform;
}

BlockType World::getBlockAt(const sf::Vector2i &tile, LayerType layer)
{
	int index = terrain->getBlockIndex(tile, layer);
	return terrain->blockTypes[index];
}

WorldID World::getID() const
{
	return id;
}

std::string World::getName() const
{
	return name;
}

void World::tick(float delta)
{
	// todo fixed time step
	getBox2DWorld()->Step(delta, 6, 2);
}

// todo move to world_rendering
void World::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform *= transform;

	// terrain
	terrain->render(target, states, false);

	// entities
	Locator::locate<EntityService>()->renderSystems();

	// overterrain
	terrain->render(target, states, true);

}
