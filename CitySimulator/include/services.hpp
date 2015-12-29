#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <unordered_map>

class BaseService
{
	// todo any required shared helpers
};

enum ServiceType
{
	SERVICE_INPUT
};

class Locator
{
public:
	void provide(ServiceType type, BaseService *service);

	BaseService* locate(ServiceType type);

private:
	std::unordered_map<ServiceType, BaseService*> services;

};

#endif
