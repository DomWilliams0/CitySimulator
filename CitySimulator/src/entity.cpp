#include <boost/property_tree/xml_parser.hpp>
#include "ai.hpp"
#include "logger.hpp"

void EntityFactory::loadEntitiesFromFile(const std::string &fileName)
{
	ConfigurationFile config(Utils::searchForFile(fileName));
	config.load();

	loadEntities(config, ENTITY_HUMAN, "human");
	loadEntities(config, ENTITY_VEHICLE, "vehicle");
}

void EntityFactory::loadEntities(ConfigurationFile &config, EntityType entityType, const std::string &sectionName)
{
	std::vector<std::map<std::string, std::string>> entities;
	config.getMapList<std::string>(sectionName, entities);

	// load tags
	std::vector<ConfigKeyValue> entityMapList;
	config.getMapList(sectionName, entityMapList);

	EntityTags allTags;
	for (auto &entity : entityMapList)
	{
		auto nameIt(entity.find("name"));

		// no name
		if (nameIt == entity.end())
		{
			Logger::logWarning(FORMAT("No name found for entity of type %1%, skipping", entityType));
			continue;
		}

		std::string name(nameIt->second);
		allTags.insert({name, entity});
	}

	loadedTags[entityType] = allTags;

	// external resource loading
	for (auto &entity : allTags)
	{
		// sprites
		auto sprite = entity.second.find("sprite");
		if (sprite != entity.second.end())
			Globals::spriteSheet->loadSprite(entity.second, entityType);
	}
}

Entity EntityManager::createEntity()
{
	// no space
	if (entityCount == MAX_ENTITIES)
	{
		ERROR("Max number of entities reached (%1%)", MAX_ENTITIES);
	}

	// todo: use a memory pool instead to avoid iterating the entire array each time
	for (size_t e = 0; e < MAX_ENTITIES; ++e)
		if (!isAlive(e))
		{
			entityCount++;
			return e;
		}

	return MAX_ENTITIES;
}

Entity EntityManager::createEntityWithComponents(const std::initializer_list<ComponentType> &components)
{
	Entity e = createEntity();
	for (ComponentType type : components)
		addComponent(e, type);

	return e;
}

void EntityManager::deleteEntity(Entity e)
{
	if (isAlive(e))
		entityCount--;

	entities[e] = COMPONENT_NONE;
}

bool EntityManager::isAlive(Entity e)
{
	return entities[e] != COMPONENT_NONE;
}

void EntityManager::tickSystems(float delta)
{
	for (System *system : systems)
		system->tick(delta);
}

void EntityManager::renderSystems(sf::RenderWindow &window)
{
	renderSystem->render(window);
}

BaseComponent *EntityManager::addComponent(Entity e, ComponentType type)
{
	entities[e] |= type;
	getComponentOfType(e, type)->reset();

	return getComponentOfType(e, type);
}

void EntityManager::removeComponent(Entity e, ComponentType type)
{
	entities[e] &= ~type;
}

bool EntityManager::hasComponent(Entity e, ComponentType type)
{
	return (entities[e] & type) != COMPONENT_NONE;
}

BaseComponent *EntityManager::getComponentOfType(Entity e, ComponentType type)
{
	switch (type)
	{
		case COMPONENT_PHYSICS:
			return &physicsComponents[e];
		case COMPONENT_RENDER:
			return &renderComponents[e];
		case COMPONENT_INPUT:
			return &inputComponents[e];
		default:
			ERROR("Invalid component type %1%", type);
	}
}

void EntityManager::addPhysicsComponent(Entity e, World *world, const sf::Vector2i &startTilePos)
{
	PhysicsComponent *phys = dynamic_cast<PhysicsComponent *>(addComponent(e, COMPONENT_PHYSICS));
	sf::Vector2i startPos = Utils::toPixel(startTilePos);
	b2World *bWorld = world->getBox2DWorld();

	phys->bWorld = bWorld;

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.position.Set(static_cast<float>(startPos.x), static_cast<float>(startPos.y));
	phys->body = bWorld->CreateBody(&def);

	// basic full body aabb
	const static float aabbSize = 0.5f;
	b2PolygonShape aabb;
	aabb.SetAsBox(aabbSize, aabbSize);
	aabb.m_centroid.Set(aabbSize, aabbSize);

	b2FixtureDef fixDef;
	fixDef.density = 985.f;
	fixDef.shape = &aabb;

	phys->body->CreateFixture(&fixDef);
}


void EntityManager::addRenderComponent(Entity e, EntityType entityType, const std::string &animation, float step,
                                       DirectionType initialDirection, bool playing)
{
	RenderComponent *comp = dynamic_cast<RenderComponent *>(addComponent(e, COMPONENT_RENDER));

	Animation *anim = Globals::spriteSheet->getAnimation(entityType, animation);
	comp->anim.init(anim, step, initialDirection, playing);
}

void EntityManager::addBrain(Entity e, bool aiBrain)
{
	InputComponent *comp = dynamic_cast<InputComponent *>(addComponent(e, COMPONENT_INPUT));

	if (aiBrain)
		comp->brain.reset(new AIBrain(e));
	else
		comp->brain.reset(new InputBrain(e));
}

void EntityManager::addPlayerInputComponent(Entity e)
{
	addBrain(e, false);
}

void EntityManager::addAIInputComponent(Entity e)
{
	addBrain(e, true);
}

