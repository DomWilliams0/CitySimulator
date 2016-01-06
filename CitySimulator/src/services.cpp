#include <iostream>
#include "services.hpp"

void BaseService::onEnable()
{
}

void BaseService::onDisable()
{
}

Locator::Locator() : services(SERVICE_UNKNOWN)
{
}

Locator::~Locator()
{
	for (auto &service : services)
		delete service;
}


void Locator::provide(ServiceType type, BaseService *service)
{
	std::string verb("Provided new");

	// delete old
	auto old = getInstance().services[type];
	if (old != nullptr)
	{
		verb = "Replaced";
		old->onDisable();
		delete old;
	}

	getInstance().services[type] = service;
	service->onEnable();
	Logger::logDebug(format("%1% service for service '%2%'", verb, serviceToString(type)));
}

std::string Locator::serviceToString(ServiceType type)
{
	switch (type)
	{
		case SERVICE_ANIMATION:
			return "Animation";
		case SERVICE_CAMERA:
			return "Camera";
		case SERVICE_CONFIG:
			return "Config";
		case SERVICE_ENTITY:
			return "Entity";
		case SERVICE_EVENT:
			return "Event";
		case SERVICE_INPUT:
			return "Input";
		case SERVICE_LOGGING:
			return "Logging";
		case SERVICE_RENDER:
			return "Render";
		default:
			return format("Unknown (%1%)", _str(type));
	}
}
NullLoggingService::NullLoggingService() : LoggingService(std::cerr, LOG_INFO)
{
}