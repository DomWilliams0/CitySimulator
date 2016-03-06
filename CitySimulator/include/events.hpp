#ifndef CITYSIMULATOR_EVENTS_HPP
#define CITYSIMULATOR_EVENTS_HPP

#include <SFML/Window.hpp>
#include "utils.hpp"
#include "ecs.hpp"
#include "constants.hpp"

enum EventType
{
	EVENT_RAW_INPUT_KEY,
	EVENT_RAW_INPUT_CLICK,

	EVENT_INPUT_SPRINT,
	EVENT_INPUT_MOVE,
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

	struct InputMoveEvent
	{
		bool halt;
		float x;
		float y;
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
		InputMoveEvent move;
		InputSprintEvent sprintToggle;
	};
};


struct EventListener
{
	virtual void onEvent(const Event &event) = 0;
};

#endif
