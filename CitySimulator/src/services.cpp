#include <SFML/Graphics.hpp>
#include "services.hpp"
#include "entity.hpp"

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

RenderService::RenderService(sf::RenderWindow &renderWindow) : window(renderWindow)
{
}

sf::RenderWindow* RenderService::getWindow()
{
	return &window;
}

void RenderService::renderEntities()
{
	// todo temporary until Globals is nuked
	Globals::entityManager->renderSystems(window);
}