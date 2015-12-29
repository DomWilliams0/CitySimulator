#include <SFML/Graphics.hpp>
#include "services.hpp"
#include "entity.hpp"

void BaseService::onEnable()
{
}

void BaseService::onDisable()
{
}

Locator::Locator()
{
}

Locator::~Locator()
{
	for (auto &pair : services)
		delete pair.second;
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