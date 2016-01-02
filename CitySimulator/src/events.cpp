#include "services.hpp"
#include "events.hpp"

void EventService::onEnable()
{
}
void EventService::onDisable()
{
}

void EventService::registerListener(EventListener *listener, EventCallback callback, EventType eventType)
{
	Logger::logDebuggier(format("Registering listener for event %1%", std::to_string(eventType)));
	listeners[eventType].push_front({listener, callback});
}
void EventService::unregisterListener(EventListener *listener, EventCallback callback, EventType eventType)
{
	listeners[eventType].remove({listener, callback});
	Logger::logDebuggier(format("Unregistering listener for event %1%", std::to_string(eventType)));
}

void EventService::processQueue()
{
	// todo only process a subset according to a time limit/fixed maximum count
	for (Event &e : pendingEvents)
	{
		auto eventListeners = listeners[e.type];
		for (auto &pair : eventListeners)
		{
			EventListener *listener = pair.first;
			EventCallback callback = pair.second;
			(listener->*callback)(&e);

		}
	}

	pendingEvents.clear();
}

void EventService::callEvent(const Event &event)
{
	pendingEvents.push_front(event);
}

InputKeyEvent::InputKeyEvent(sf::Keyboard::Key key, bool pressed) : Event(EVENT_INPUT_KEY), key(key), pressed(pressed)
{
}

Event::Event(EventType type) : type(type)
{
}