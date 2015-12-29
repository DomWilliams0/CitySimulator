#include "logger.hpp"
#include "utils.hpp"
#include "services.hpp"

void Locator::provide(ServiceType type, BaseService *service)
{
	std::string verb("Provided new");

	// delete old
	auto old = locate(type);
	if (old != nullptr)
	{
		verb = "Replaced";
		delete old;
	}

	services[type] = service;
	Logger::logDebug(format("%1% service for service type %2%", verb, std::to_string(type)));
}

BaseService *Locator::locate(ServiceType type)
{
	auto service = services.find(type);
	return service == services.end() ? nullptr : service;
}