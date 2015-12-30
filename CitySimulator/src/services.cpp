#include <SFML/Graphics.hpp>
#include "services.hpp"

void BaseService::onEnable()
{
}

void BaseService::onDisable()
{
}

Locator::Locator() : services(SERVICE_COUNT, nullptr)
{
}

Locator::~Locator()
{
	for (auto &service : services)
		delete service;
}

RenderService::RenderService(sf::RenderWindow *renderWindow) : window(renderWindow)
{
}

sf::RenderWindow* RenderService::getWindow()
{
	return window;
}

void RenderService::renderEntities()
{
	Locator::locate<EntityService>()->renderSystems(*window);
}

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

void EntityService::onDisable()
{
	for (System *system : systems)
		delete system;
}
