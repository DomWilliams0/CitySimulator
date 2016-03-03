#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <string>
#include <vector>
#include <typeinfo>
#include <typeindex>
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

class AnimationService;
class CameraService;
class ConfigService;
class EntityService;
class EventService;
class InputService;
class LoggingService;
class RenderService;
class WorldService;

class World;

class PhysicsComponent;

class SimpleMovementController;

class InputBrain;

class b2Body;

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service);

	static std::string serviceToString(ServiceType type);

	template<class T>
	static T *locate(bool errorOnFail = true)
	{
		ServiceType type = SERVICE_UNKNOWN;

		if (typeid(T) == typeid(AnimationService))
			type = SERVICE_ANIMATION;
		else if (typeid(T) == typeid(CameraService))
			type = SERVICE_CAMERA;
		else if (typeid(T) == typeid(ConfigService))
			type = SERVICE_CONFIG;
		else if (typeid(T) == typeid(EntityService))
			type = SERVICE_ENTITY;
		else if (typeid(T) == typeid(EventService))
			type = SERVICE_EVENT;
		else if (typeid(T) == typeid(InputService))
			type = SERVICE_INPUT;
		else if (typeid(T) == typeid(LoggingService))
			type = SERVICE_LOGGING;
		else if (typeid(T) == typeid(RenderService))
			type = SERVICE_RENDER;
		else if (typeid(T) == typeid(WorldService))
			type = SERVICE_WORLD;
		else
			error("Invalid service type given. Has its type been registered?");

		T *ret = dynamic_cast<T *>(getInstance().services[type]);
		if (errorOnFail && ret == nullptr)
			error("Could not locate service '%1%'", serviceToString(type));
		return ret;
	}

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

#endif
