#include <boost/property_tree/xml_parser.hpp>
#include "ai.hpp"
#include "world.hpp"

void EntityService::onEnable()
{
	// load entities from file
	auto fileName(Config::getResource("entities.config"));
	ConfigurationFile config(fileName);
	config.load();

	loadEntities(config, ENTITY_HUMAN, "human");
	loadEntities(config, ENTITY_VEHICLE, "vehicle");


	// init entities
	for (size_t i = 0; i < MAX_ENTITIES; ++i)
		entities[i] = COMPONENT_NONE;

	// init systems in correct order
	systems.push_back(new InputSystem);
	systems.push_back(new PhysicsSystem);

	auto render = new RenderSystem;
	systems.push_back(render);
	renderSystem = render;
}

void EntityService::loadEntities(ConfigurationFile &config, EntityType entityType, const std::string &sectionName)
{
	std::vector<std::map<std::string, std::string>> entities;
	config.getMapList<std::string>(sectionName, entities);

	// load tags
	std::vector<ConfigKeyValue> entityMapList;
	config.getMapList(sectionName, entityMapList);

	Logger::pushIndent();

	AnimationService *as = Locator::locate<AnimationService>();
	EntityTags allTags;
	for (auto &entity : entityMapList)
	{
		auto nameIt(entity.find("name"));
		if (nameIt == entity.end())
		{
			Logger::logWarning(format("No name found for entity of type %1%, skipping", std::to_string(entityType)));
			continue;
		}

		// prototype
		auto prototypeIt(entity.find("prototype"));
		if (prototypeIt != entity.end())
		{
			auto prototypeEntity = allTags.find(prototypeIt->second);
			if (prototypeEntity == allTags.end())
			{
				Logger::logWarning(format("Entity prototype '%1%' not found, skipping entity %2%",
				                          prototypeIt->second, nameIt->second));
				continue;
			}

			// inherit all
			entity.insert(prototypeEntity->second.begin(), prototypeEntity->second.end());
		}


		allTags.insert({nameIt->second, entity});
	}

	loadedTags[entityType] = allTags;

	// external resource loading
	for (auto &entity : allTags)
	{
		// sprites
		auto sprite = entity.second.find("sprite");
		if (sprite != entity.second.end())
			as->loadSprite(entity.second, entityType);
	}

	Logger::popIndent();
}

void EntityService::onDisable()
{
	for (System *system : systems)
		delete system;
}

unsigned int EntityService::getEntityCount() const
{
	return entityCount;
}

EntityID EntityService::createEntity()
{
	// no space
	if (entityCount == MAX_ENTITIES)
		error("Max number of entities reached (%1%)", std::to_string(MAX_ENTITIES));

	// todo: use a memory pool instead to avoid iterating the entire array each time
	for (EntityID e = 0; e < MAX_ENTITIES; ++e)
		if (!isAlive(e))
		{
			entityCount++;
			return e;
		}

	return MAX_ENTITIES;
}

void EntityService::killEntity(EntityID e)
{
	if (isAlive(e))
		entityCount--;

	entities[e] = COMPONENT_NONE;
}

bool EntityService::isAlive(EntityID e) const
{
	return entities[e] != COMPONENT_NONE;
}

void EntityService::tickSystems(float delta)
{
	for (System *system : systems)
		system->tick(this, delta);
}

void EntityService::renderSystems(sf::RenderWindow &window)
{
	renderSystem->render(this, window);
}

BaseComponent *EntityService::addComponent(EntityID e, ComponentType type)
{
	entities[e] |= type;

	auto comp = getComponentOfType(e, type);
	comp->reset();
	return comp;
}

void EntityService::removeComponent(EntityID e, ComponentType type)
{
	entities[e] &= ~type;
}

bool EntityService::hasComponent(EntityID e, ComponentType type) const
{
	return (entities[e] & type) != COMPONENT_NONE;
}

BaseComponent *EntityService::getComponentOfType(EntityID e, ComponentType type)
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

void EntityService::addPhysicsComponent(EntityID e, World *world, const sf::Vector2i &startTilePos, float maxSpeed, float damping)
{
	PhysicsComponent *phys = dynamic_cast<PhysicsComponent *>(addComponent(e, COMPONENT_PHYSICS));

	phys->maxSpeed = maxSpeed;
	phys->damping = damping;

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
	aabb.SetAsBox(
			scale * (28.f / 32.f), // width: 2px off each side
			scale * 0.5f, // height: just bottom half
			b2Vec2(0, scale * 0.75f), // centred over bottom half
			0.f
	);

	b2FixtureDef fixDef;
	fixDef.friction = 0.5f;
	fixDef.density = 985.f;
	fixDef.shape = &aabb;

	phys->body->CreateFixture(&fixDef);
}


void EntityService::addRenderComponent(EntityID e, EntityType entityType, const std::string &animation, float step,
                                       DirectionType initialDirection, bool playing)
{
	RenderComponent *comp = dynamic_cast<RenderComponent *>(addComponent(e, COMPONENT_RENDER));

	AnimationService *as = Locator::locate<AnimationService>();
	Animation *anim = as->getAnimation(entityType, animation);
	comp->anim.init(anim, step, initialDirection, playing);
}

void EntityService::addBrain(EntityID e, bool aiBrain)
{
	InputComponent *comp = dynamic_cast<InputComponent *>(addComponent(e, COMPONENT_INPUT));

	if (aiBrain)
		comp->brain.reset(new AIBrain(e));
	else
		comp->brain.reset(new InputBrain(e));
}

void EntityService::addPlayerInputComponent(EntityID e)
{
	addBrain(e, false);
}

void EntityService::addAIInputComponent(EntityID e)
{
	addBrain(e, true);
}

