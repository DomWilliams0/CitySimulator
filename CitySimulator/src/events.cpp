#include "services.hpp"
#include "events.hpp"

void EventService::onEnable()
{
}
void EventService::onDisable()
{
}

void EventService::registerListener(EventListener *listener, EventType eventType)
{
	Logger::logDebuggier(format("Registering listener for event %1%", std::to_string(eventType)));
	listeners[eventType].push_front(listener);
}
void EventService::unregisterListener(EventListener *listener, EventType eventType)
{
	listeners[eventType].remove(listener);
	Logger::logDebuggier(format("Unregistering listener for event %1%", std::to_string(eventType)));
}

void EventService::processQueue()
{
	// todo only process a subset according to a time limit/fixed maximum count
	for (const Event &e : pendingEvents)
	{
		auto eventListeners = listeners[e.type];
		for (EventListener *listener : eventListeners)
			listener->onEvent(e);
	}

	pendingEvents.clear();
}

void EventService::callEvent(const Event &event)
{
	pendingEvents.push_front(event);
}

InputKeyEvent::InputKeyEvent(sf::Keyboard::Key key, bool pressed) : Event(INPUT_KEY), key(key), pressed(pressed)
{
}

Event::Event(EventType type) : type(type)
{
}