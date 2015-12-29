#include <SFML/Graphics.hpp>
#include "services.hpp"

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

