#include "events.hpp"
#include "service/base_service.hpp"
#include "service/event_service.hpp"
#include "service/logging_service.hpp"

std::string eventToString(EventType eventType)
{
	switch (eventType)
	{
		case EVENT_RAW_INPUT_KEY:
			return "EVENT_RAW_INPUT_KEY";
		case EVENT_RAW_INPUT_CLICK:
			return "EVENT_RAW_INPUT_CLICK";
		case EVENT_INPUT_SPRINT:
			return "EVENT_INPUT_SPRINT";
		case EVENT_INPUT_START_MOVING:
			return "EVENT_INPUT_START_MOVING";
		case EVENT_INPUT_STOP_MOVING:
			return "EVENT_INPUT_STOP_MOVING";
		case EVENT_INPUT_YIELD_CONTROL:
			return "EVENT_INPUT_YIELD_CONTROL";
		case EVENT_CAMERA_SWITCH_WORLD:
			return "EVENT_CAMERA_SWITCH_WORLD";
		case EVENT_HUMAN_SWITCH_WORLD:
			return "EVENT_HUMAN_SWITCH_WORLD";
		case EVENT_HUMAN_DEATH:
			return "EVENT_HUMAN_DEATH";
		case EVENT_HUMAN_INTERACT:
			return "EVENT_HUMAN_INTERACT";
		default:
			return format("Unknown (%1%)", _str(eventType));
	}
}


void EventService::onEnable()
{
}

void EventService::onDisable()
{
}

void EventService::registerListener(EventListener *listener, EventType eventType)
{
	Logger::logDebuggiest(format("Registering listener \"%1%\" for %2%",
	                             listener->identifier, eventToString(eventType)));
	listeners[eventType].push_front(listener);
}

void EventService::unregisterListener(EventListener *listener, EventType eventType)
{
	Logger::logDebuggiest(format("Unregistering listener \"%1%\" for %2%",
	                             listener->identifier, eventToString(eventType)));
	listeners[eventType].remove(listener);
}


void EventService::unregisterListener(EventListener *listener)
{
	struct
	{
		EventListener *value;

		bool operator()(const EventListener *e)
		{
			return e == value;
		}
	} pred;

	pred.value = listener;

	for (auto &pair : listeners)
		pair.second.remove_if(pred);

	Logger::logDebuggiest(format("Unregistered listener \"%1%\" from all events", listener->identifier));
}


void EventService::processQueue()
{
	// todo only process a subset according to a time limit/fixed maximum count

	size_t size = std::distance(pendingEvents.cbegin(), pendingEvents.cend());

	while (size--)
	{
		Event e = pendingEvents.front();
		pendingEvents.pop_front();

		auto eventListeners = listeners[e.type];
		for (EventListener *listener : eventListeners)
			listener->onEvent(e);

	}
}

void EventService::callEvent(const Event &event)
{
	pendingEvents.push_front(event);
}
