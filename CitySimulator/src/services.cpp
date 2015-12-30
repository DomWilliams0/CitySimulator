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
	// todo temporary until Globals is nuked
	Globals::entityManager->renderSystems(*window);
}

void EntityService::onEnable()
{
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

void EntityService::onDisable()
{
	for (System *system : systems)
		delete system;
}
