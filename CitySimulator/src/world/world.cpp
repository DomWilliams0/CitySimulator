#include "world.hpp"
#include "service/locator.hpp"

WorldService::WorldService(const std::string &mainWorldPath, const std::string &tilesetPath)
		: tileset(tilesetPath), mainWorldName(mainWorldPath), entityTransferListener(this)
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
	}

	// transfer buildings and connections
	BuildingConnectionMap *bm = getMainWorld()->getBuildingConnectionMap();
	for (WorldLoader::LoadedBuilding &building : loader.buildings)
	{
		Building &b = bm->addBuilding(building.bounds, building.insideWorldID);
		for (WorldLoader::LoadedDoor &door : building.doors)
			b.addDoor(Location(b.getOutsideWorld()->getID(), door.tile), door.doorID);
	}

	for (auto &pair : loader.loadedWorlds)
	{
		if (pair.second.world->isOutside())
			continue;

		DomesticConnectionMap *cm = pair.second.world->getDomesticConnectionMap();
		for (WorldLoader::LoadedDoor &door : pair.second.doors)
			cm->addDoor(door.tile);

	}

	// load collisions
	for (auto &pair : terrainCache)
		pair.second.loadBlockData();

	Logger::popIndent();

	// register listener
	Locator::locate<EventService>()->registerListener(
			&entityTransferListener, EVENT_HUMAN_SWITCH_WORLD);
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

bool WorldService::getConnectionDestination(const Location &src, Location &out)
{
	auto dst = connectionLookup.find(src);
	if (dst == connectionLookup.end())
		return false;

	out = dst->second;
	return true;
}

void WorldService::tickActiveWorlds(float delta)
{
	for (auto &pair : worlds)
	{
		World *world = pair.second;
		if (!world->isEmpty())
			world->tick(delta);
	}
}

WorldService::EntityTransferListener::EntityTransferListener(WorldService *ws) : ws(ws)
{
}

void WorldService::EntityTransferListener::onEvent(const Event &event)
{
	World *newWorld = ws->getWorld(event.humanSwitchWorld.newWorld);
	b2World *newBWorld = newWorld->getBox2DWorld();
	// todo nullptr should never be returned, throw exception instead

	EntityService *es = Locator::locate<EntityService>();
	PhysicsComponent *phys = es->getComponent<PhysicsComponent>(event.entityID, COMPONENT_PHYSICS); // todo never return null
	b2World *oldBWorld = phys->bWorld;

	sf::Vector2f newPosition;
	newPosition.x = event.humanSwitchWorld.spawnX;
	newPosition.y = event.humanSwitchWorld.spawnY;

	// todo temporary fix to prevent teleporting back and forth due to spawning in the door
	if (!newWorld->isOutside())
		newPosition.y -= 1;

	// clone body and add to new world
	b2Body *oldBody = phys->body;
	b2Body *newBody = es->createBody(newBWorld, oldBody, newPosition);

	// remove from old world
	oldBWorld->DestroyBody(oldBody);

	// update component
	phys->body = newBody;
	phys->bWorld = newBWorld;
	phys->world = newWorld->getID();

	// camera target
	CameraService *cs = Locator::locate<CameraService>();
	if (phys == cs->getTrackedEntity())
	{
		Event e;
		e.type = EVENT_CAMERA_SWITCH_WORLD;
		e.cameraSwitchWorld.newWorld = event.humanSwitchWorld.newWorld;
		e.cameraSwitchWorld.centreX = event.humanSwitchWorld.spawnX;
		e.cameraSwitchWorld.centreY = event.humanSwitchWorld.spawnY;

		Locator::locate<EventService>()->callEvent(e);
	}
}

World::World(WorldID id, const std::string &name, bool outside) 
: id(id), name(name), outside(outside)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
	if (outside)
		connectionMap = dynamic_cast<ConnectionMap *>(new BuildingConnectionMap(this));
	else
		connectionMap = dynamic_cast<ConnectionMap *>(new DomesticConnectionMap(this));
}

void World::setTerrain(WorldTerrain &terrain)
{
	this->terrain = &terrain;
}

WorldTerrain *World::getTerrain()
{
	return terrain;
}

CollisionMap *World::getCollisionMap() const
{
	return terrain == nullptr ? nullptr : terrain->getCollisionMap();
}

ConnectionMap *World::getConnectionMap()
{
	return connectionMap;
}

BuildingConnectionMap *World::getBuildingConnectionMap()
{
	if (!outside)
		error("Cannot get building connection map for non-outside world");
	return dynamic_cast<BuildingConnectionMap *>(connectionMap);
}

DomesticConnectionMap *World::getDomesticConnectionMap()
{
	if (outside)
		error("Cannot get domestic connection map for outside world");
	return dynamic_cast<DomesticConnectionMap *>(connectionMap);
}


b2World *World::getBox2DWorld() const
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

bool World::isEmpty()
{
	return getBox2DWorld()->GetBodyCount() == 1; // just block collision body
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
	Locator::locate<EntityService>()->renderSystems(id);

	// overterrain
	terrain->render(target, states, true);

	// box2d debug
	if (Config::getBool("debug.render-physics"))
		getBox2DWorld()->DrawDebugData();

}
