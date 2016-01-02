#ifndef CITYSIM_EVENTS_HPP
#define CITYSIM_EVENTS_HPP

#include <SFML/Window.hpp>

enum EventType
{
	EVENT_INPUT_KEY,
	EVENT_INPUT_CLICK,

	EVENT_HUMAN_SPAWN,
	EVENT_HUMAN_DEATH,
	EVENT_HUMAN_INTERACT,

	EVENT_UNKNOWN
};

struct Event
{
	Event(EventType type);

	EventType type;
};

struct InputKeyEvent : public Event
{
	InputKeyEvent(sf::Keyboard::Key key, bool pressed);

	sf::Keyboard::Key key;
	bool pressed;
};

struct EventListener
{
	virtual void onEvent(Event *event)
	{ }
	virtual void onInputKeyEvent(InputKeyEvent *event)
	{ }
};

#endif
