#include "events.hpp"
#include "service/base_service.hpp"
#include "service/event_service.hpp"
#include "service/logging_service.hpp"

void EventService::onEnable()
{
}
void EventService::onDisable()
{
}

void EventService::registerListener(EventListener *listener, EventType eventType)
{
	Logger::logDebuggiest(format("Registering listener for event %1%", _str(eventType)));
	listeners[eventType].push_front(listener);
}
void EventService::unregisterListener(EventListener *listener, EventType eventType)
{
	listeners[eventType].remove(listener);
	Logger::logDebuggiest(format("Unregistering listener for event %1%", _str(eventType)));
}



void EventService::unregisterListener(EventListener *listener)
{
	struct
	{
		EventListener *value;

		bool operator() (const EventListener *e)
		{
			return e == value;
		}
	} pred;

	pred.value = listener;

	for (auto &pair : listeners)
		pair.second.remove_if(pred);

	Logger::logDebuggiest("Unregistered listener from all events");
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
