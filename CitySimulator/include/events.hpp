#ifndef CITYSIM_EVENTS_HPP
#define CITYSIM_EVENTS_HPP

#include <SFML/Window.hpp>

enum EventType
{
	EVENT_RAW_INPUT_KEY,
	EVENT_RAW_INPUT_CLICK,

	EVENT_HUMAN_SPAWN,
	EVENT_HUMAN_DEATH,
	EVENT_HUMAN_INTERACT,

	EVENT_UNKNOWN
};

struct Event
{
	// event types
	struct RawInputKeyEvent
	{
		sf::Keyboard::Key key;
		bool pressed;
	};

	EventType type;

	union
	{
		RawInputKeyEvent rawInputKey;
	};
};


struct EventListener
{
	virtual void onEvent(const Event &event) = 0;
};

#endif
