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
	virtual void onEnable()
	{
	}


	virtual void onDisable()
	{
	}

};

#endif
