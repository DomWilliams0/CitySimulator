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

void System::tick(float dt)
{
	EntityManager *manager = Globals::entityManager;

	for (size_t e = 0; e < MAX_ENTITIES; ++e)
	{
		if ((manager->entities[e] & mask) == mask)
			tickEntity(e, dt);
	}
}

void System::render(sf::RenderWindow &window)
{
	EntityManager *manager = Globals::entityManager;

	for (size_t e = 0; e < MAX_ENTITIES; ++e)
	{
		if ((manager->entities[e] & mask) == mask)
			renderEntity(e, window);
	}
}

void PositionComponent::reset()
{
	pos.x = pos.y = 0;
}

void RenderComponent::reset()
{
	anim.reset();
}

void VelocityComponent::reset()
{
	velocity.x = velocity.y = 0;
}

template <class T>
T* get(Entity e, ComponentType type)
{
	return Globals::entityManager->getComponent<T>(e, type);
}

void MovementSystem::tickEntity(Entity e, float dt)
{
	auto pos = get<PositionComponent>(e, COMPONENT_POSITION);
	auto vel = get<VelocityComponent>(e, COMPONENT_VELOCITY);

	pos->pos += vel->velocity * dt;
}

void RenderSystem::tickEntity(Entity e, float dt)
{
	auto *render = get<RenderComponent>(e, COMPONENT_RENDER);

	render->anim.tick(dt);
}

void RenderSystem::renderEntity(Entity e, sf::RenderWindow &window)
{
	auto render = get<RenderComponent>(e, COMPONENT_RENDER);
	auto pos = get<PositionComponent>(e, COMPONENT_POSITION);

	sf::RenderStates states;
	sf::Transform transform;

	transform.translate(pos->pos);
	transform.scale(Constants::humanScale);

	states.transform *= transform;
	render->anim.draw(window, states);
}

Entity EntityManager::createEntity()
{
	// no space
	if (entityCount == MAX_ENTITIES)
	{
		FAIL("Max number of entities reached (%1%)", MAX_ENTITIES);
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
	for (System *system : systems)
		if (system->doesRender())
			system->render(window);
}

BaseComponent* EntityManager::addComponent(Entity e, ComponentType type)
{
	entities[e] |= type;

	return getComponent(e, type);
}

void EntityManager::removeComponent(Entity e, ComponentType type)
{
	entities[e] &= ~type;
	getComponent(e, type)->reset();
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


void EntityManager::addRenderComponent(Entity e, EntityType entityType, const std::string &animation, float step, DirectionType initialDirection, bool playing)
{
	RenderComponent *comp = dynamic_cast<RenderComponent*>(Globals::entityManager->addComponent(e, COMPONENT_RENDER));

	Animation *anim = Globals::spriteSheet->getAnimation(entityType, animation);
	comp->anim.init(anim, step, initialDirection, playing);
}
