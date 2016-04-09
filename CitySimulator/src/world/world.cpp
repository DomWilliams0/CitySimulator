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

	// generate tileset
	tileset.load();
	tileset.convertToTexture(loader.flippedTileGIDs);

	// load terrain
	for (auto &pair : terrainCache)
		pair.second.applyTiles(tileset);

	// transfer loaded worlds
	for (auto &lwPair : loader.loadedWorlds)
	{
		World *world = lwPair.second.world;
		worlds[world->getID()] = world;

		world->getCollisionMap()->load();
	}

	// transfer buildings
	BuildingMap &bm = getMainWorld()->getBuildingMap();
	for (auto &building : loader.buildings)
		bm.addBuilding(building.bounds, building.insideWorldID);

	Logger::popIndent();
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

World::World(WorldID id, const std::string &name, bool outside) 
: id(id), name(name), outside(outside)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);

	if (outside)
		buildingMap.reset(this);
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

BuildingMap &World::getBuildingMap()
{
	return *buildingMap;
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

bool World::isOutside() const
{
	return outside;
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
