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
	WorldLoader loader(worldTree);
	mainWorld = loader.loadWorlds(mainWorldName, tileset, connectionLookup);
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
	mainWorld = nullptr;
}


World &WorldService::getWorld()
{
	return *mainWorld;
}

bool isCollidable(BlockType blockType)
{
	static const std::set<BlockType> collidables(
			{BLOCK_WATER, BLOCK_TREE, BLOCK_BUILDING_WALL, BLOCK_BUILDING_EDGE, BLOCK_BUILDING_ROOF,
			 BLOCK_BUILDING_ROOF_CORNER});
	return collidables.find(blockType) != collidables.end();
}

bool isInteractable(BlockType blockType)
{
	static const std::set<BlockType> interactables(
			{BLOCK_SLIDING_DOOR});
	return interactables.find(blockType) != interactables.end();
}

LayerType layerTypeFromString(const std::string &s)
{
	if (s == "underterrain")
		return LAYER_UNDERTERRAIN;
	if (s == "terrain")
		return LAYER_TERRAIN;
	if (s == "overterrain")
		return LAYER_OVERTERRAIN;
	if (s == "objects")
		return LAYER_OBJECTS;
	if (s == "collisions")
		return LAYER_COLLISIONS;
	if (s == "buildings")
		return LAYER_BUILDINGS;

	Logger::logWarning("Unknown LayerType: " + s);
	return LAYER_UNKNOWN;
}

bool isTileLayer(const LayerType &layerType)
{
	return layerType == LAYER_UNDERTERRAIN || layerType == LAYER_TERRAIN || layerType == LAYER_OVERTERRAIN;
}

bool isOverLayer(const LayerType &layerType)
{
	return layerType == LAYER_OVERTERRAIN;
}

World::World(int id, const std::string &fullPath) :
		terrain(this), collisionMap(this), buildingMap(this), id(id), filePath(fullPath)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
}

void World::loadFromFile(TMX::TileMap &tmx)
{
	// todo all of this, eek
//	tmx.load(filePath);
//	resize(tmx.size);

//	terrain.load(tmx, flippedGIDs, tileset);
}

void World::finishLoading(TMX::TileMap *tmx)
{
	terrain.loadLayers(tmx->layers);
	collisionMap.load();
}

void World::resize(sf::Vector2i size)
{
	tileSize = size;
	pixelSize = Utils::toPixel(size);
}

WorldTerrain &World::getTerrain()
{
	return terrain;
}

CollisionMap &World::getCollisionMap()
{
	return collisionMap;
}

BuildingMap &World::getBuildingMap()
{
	return buildingMap;
}

b2World *World::getBox2DWorld()
{
	return &collisionMap.world;
}

sf::Vector2i World::getPixelSize() const
{
	return pixelSize;
}

sf::Vector2i World::getTileSize() const
{
	return tileSize;
}

sf::Transform World::getTransform() const
{
	return transform;
}

BlockType World::getBlockAt(const sf::Vector2i &tile, LayerType layer)
{
	int index = terrain.getBlockIndex(tile, layer);
	return terrain.blockTypes[index];
}

int World::getID() const
{
	return id;
}

void World::tick(float delta)
{
	// todo fixed time step
	getBox2DWorld()->Step(delta, 6, 2);
}

void World::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform *= transform;

	// terrain
	terrain.render(target, states, false);

	// entities
	Locator::locate<EntityService>()->renderSystems();

	// overterrain
	terrain.render(target, states, true);

}
