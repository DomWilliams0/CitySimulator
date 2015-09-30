#include <boost/property_tree/xml_parser.hpp>
#include "entity.hpp"
#include "config.hpp"
#include "animation.hpp"
#include "logger.hpp"
#include "utils.hpp"

void EntityFactory::loadEntities(EntityType entityType, const std::string &fileName)
{
	ConfigurationFile config(Utils::searchForFile(fileName));
	config.load();

	// get root list name
	std::string root = boost::filesystem::path(fileName).stem().string();

	// load tags
	std::vector<ConfigKeyValue> entityMapList;
	config.getMapList(root, entityMapList);

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
		allTags.insert({ name, entity });
	}

	loadedTags[entityType] = allTags;

	// external resource loading
	for (auto &entity : allTags)
	{
		// sprites
		auto sprite = entity.second.find("sprite");
		if (sprite != entity.second.end())
			Globals::spriteSheet->loadSprite(entity.second);
	}
}

void System::tick(float dt)
{
	EntityManager *manager = Globals::entityManager;

	for (size_t i = 0; i < MAX_ENTITIES; ++i)
	{
		Entity e = manager->entities[i];
		if ((e & mask) == mask)
			tickEntity(e, dt);
	}
}

void PositionComponent::reset()
{
	pos.x = pos.y = 0;
}

void RenderComponent::reset()
{
}

void VelocityComponent::reset()
{
	velocity.x = velocity.y = 0;
}


void MovementSystem::tickEntity(Entity e, float dt)
{
	
}

void RenderSystem::tickEntity(Entity e, float dt)
{

}

Entity EntityManager::createEntity()
{
	// no space
	if (entityCount == MAX_ENTITIES)
	{
		Logger::logWarning("Max number of entities reached");
		return MAX_ENTITIES;
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

BaseComponent* EntityManager::addComponent(Entity e, ComponentType type)
{
	entities[e] |= type;
	// todo: reset component?

	return getComponent(e, type);
}

void EntityManager::removeComponent(Entity e, ComponentType type)
{
	entities[e] &= ~type;
	// todo reset component?
}

bool EntityManager::hasComponent(Entity e, ComponentType type)
{
	return (entities[e] & type) != COMPONENT_NONE;
}

BaseComponent* EntityManager::getComponent(Entity e, ComponentType type)
{
	switch (type)
	{
	case COMPONENT_POSITION: return &positionComponents[e];
	case COMPONENT_VELOCITY: return &velocityComponents[e];
	case COMPONENT_RENDER: return &renderComponents[e];
	default: FAIL("Invalid component type %1%", type);
	}
}

void EntityManager::addPositionComponent(Entity e, float x, float y)
{
	PositionComponent *comp = dynamic_cast<PositionComponent*>(Globals::entityManager->addComponent(e, COMPONENT_POSITION));
	comp->pos.x = x;
	comp->pos.y = y;
}

void EntityManager::addVelocityComponent(Entity e, float x, float y)
{
	VelocityComponent *comp = dynamic_cast<VelocityComponent*>(Globals::entityManager->addComponent(e, COMPONENT_VELOCITY));
	comp->velocity.x = x;
	comp->velocity.y = y;
}


void EntityManager::addRenderComponent(Entity e, const std::string &animation, DirectionType initialDirection, bool playing)
{
	RenderComponent *comp = dynamic_cast<RenderComponent*>(Globals::entityManager->addComponent(e, COMPONENT_RENDER));
	// todo
}
