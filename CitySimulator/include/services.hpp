#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <unordered_map>

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

// services

class BaseService
{
public:
	virtual void onEnable();
	virtual void onDisable();
};

#endif
