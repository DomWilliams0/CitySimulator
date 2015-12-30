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

	Logger::pushIndent();

	EntityTags allTags;
	for (auto &entity : entityMapList)
	{
		auto nameIt(entity.find("name"));

		// no name
		if (nameIt == entity.end())
		{
			Logger::logWarning(format("No name found for entity of type %1%, skipping", std::to_string(entityType)));
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

	Logger::popIndent();
}

Entity EntityService::createEntity()
{
	// no space
	if (entityCount == MAX_ENTITIES)
		error("Max number of entities reached (%1%)", std::to_string(MAX_ENTITIES));

	// todo: use a memory pool instead to avoid iterating the entire array each time
	for (Entity e = 0; e < MAX_ENTITIES; ++e)
		if (!isAlive(e))
		{
			entityCount++;
			return e;
		}

	return MAX_ENTITIES;
}

void EntityService::killEntity(Entity e)
{
	if (isAlive(e))
		entityCount--;

	entities[e] = COMPONENT_NONE;
}

bool EntityService::isAlive(Entity e)
{
	return entities[e] != COMPONENT_NONE;
}

void EntityService::tickSystems(float delta)
{
	for (System *system : systems)
		system->tick(delta);
}

void EntityService::renderSystems(sf::RenderWindow &window)
{
	renderSystem->render(window);
}

BaseComponent *EntityService::addComponent(Entity e, ComponentType type)
{
	entities[e] |= type;

	auto comp = getComponentOfType(e, type);
	comp->reset();
	return comp;
}

void EntityService::removeComponent(Entity e, ComponentType type)
{
	entities[e] &= ~type;
}

bool EntityService::hasComponent(Entity e, ComponentType type)
{
	return (entities[e] & type) != COMPONENT_NONE;
}

BaseComponent *EntityService::getComponentOfType(Entity e, ComponentType type)
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
			error("Invalid component type %1%", std::to_string(type));
	}
}

void EntityService::addPhysicsComponent(Entity e, World *world, const sf::Vector2i &startTilePos)
{
	PhysicsComponent *phys = dynamic_cast<PhysicsComponent *>(addComponent(e, COMPONENT_PHYSICS));
	b2World *bWorld = world->getBox2DWorld();

	phys->bWorld = bWorld;

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.position.Set(static_cast<float>(startTilePos.x), static_cast<float>(startTilePos.y));
	phys->body = bWorld->CreateBody(&def);
	phys->body->SetFixedRotation(true);

	// basic full body aabb
	b2PolygonShape aabb;

	const auto scale = Constants::entityScalef / 2;
	aabb.SetAsBox(scale, scale);
	aabb.m_centroid.Set(scale, scale);

	b2FixtureDef fixDef;
	fixDef.density = 985.f;
	fixDef.shape = &aabb;

	phys->body->CreateFixture(&fixDef);
}


void EntityService::addRenderComponent(Entity e, EntityType entityType, const std::string &animation, float step,
                                       DirectionType initialDirection, bool playing)
{
	RenderComponent *comp = dynamic_cast<RenderComponent *>(addComponent(e, COMPONENT_RENDER));

	Animation *anim = Globals::spriteSheet->getAnimation(entityType, animation);
	comp->anim.init(anim, step, initialDirection, playing);
}

void EntityService::addBrain(Entity e, bool aiBrain)
{
	InputComponent *comp = dynamic_cast<InputComponent *>(addComponent(e, COMPONENT_INPUT));

	if (aiBrain)
		comp->brain.reset(new AIBrain(e));
	else
		comp->brain.reset(new InputBrain(e));
}

void EntityService::addPlayerInputComponent(Entity e)
{
	addBrain(e, false);
}

void EntityService::addAIInputComponent(Entity e)
{
	addBrain(e, true);
}

