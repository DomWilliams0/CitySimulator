#include <typeinfo>
#include "animation_service.hpp"
#include "camera_service.hpp"
#include "config_service.hpp"
#include "entity_service.hpp"
#include "event_service.hpp"
#include "input_service.hpp"
#include "logging_service.hpp"
#include "render_service.hpp"
#include "world_service.hpp"
#include "locator.hpp"

template<class T>
T *Locator::locate(bool errorOnFail)
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

