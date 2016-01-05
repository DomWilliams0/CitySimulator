#ifndef CITYSIM_EVENTS_HPP
#define CITYSIM_EVENTS_HPP

#include <SFML/Window.hpp>
#include "utils.hpp"
#include "entity.hpp"
#include "constants.hpp"

enum EventType
{
	EVENT_RAW_INPUT_KEY,
	EVENT_RAW_INPUT_CLICK,

	EVENT_INPUT_SPRINT,
	EVENT_INPUT_START_MOVING,
	EVENT_INPUT_STOP_MOVING,
	EVENT_INPUT_YIELD_CONTROL,

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

	struct RawInputClickEvent
	{
		sf::Mouse::Button button;
		int x;
		int y;
		bool pressed;
	};

	struct InputStartMoveEvent
	{
		DirectionType direction;
	};

	struct InputStopMoveEvent
	{
		DirectionType direction;
	};

	struct InputSprintEvent
	{
		bool start;
	};

	EventType type;
	EntityID entityID;

	union
	{
		RawInputKeyEvent rawInputKey;
		RawInputClickEvent rawInputClick;
		InputStartMoveEvent startMove;
		InputStopMoveEvent stopMove;
		InputSprintEvent sprintToggle;
	};
};


struct EventListener
{
	virtual void onEvent(const Event &event) = 0;
};

#endif
