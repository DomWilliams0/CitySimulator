#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <string>
#include <vector>
#include "utils.hpp"

enum ServiceType
{
	SERVICE_ANIMATION,
	SERVICE_CAMERA,
	SERVICE_CONFIG,
	SERVICE_ENTITY,
	SERVICE_EVENT,
	SERVICE_INPUT,
	SERVICE_LOGGING,
	SERVICE_RENDER,
	SERVICE_WORLD,

	SERVICE_UNKNOWN
};

class BaseService
{
public:
	virtual void onEnable();

	virtual void onDisable();
};

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service);

	static std::string serviceToString(ServiceType type);

	template<class T>
	static T *locate(bool errorOnFail = true);

private:
	Locator();

	~Locator();

	static Locator &getInstance()
	{
		static Locator instance;
		return instance;
	}

	std::vector<BaseService *> services;
};

#include "service/locator.ipp"

#endif
