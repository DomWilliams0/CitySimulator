#include <SFML/Window/Keyboard.hpp>
#include "services.hpp"

void InputService::onEnable()
{
	bindings.clear();

	bindKey(KEY_UP, sf::Keyboard::W);
	bindKey(KEY_LEFT, sf::Keyboard::A);
	bindKey(KEY_DOWN, sf::Keyboard::S);
	bindKey(KEY_RIGHT, sf::Keyboard::D);
	bindKey(KEY_YIELD_CONTROL, sf::Keyboard::Tab);
	bindKey(KEY_SPRINT, sf::Keyboard::Space);
	bindKey(KEY_EXIT, sf::Keyboard::Escape);
	// todo load from config

	// check all keys have been registered
	if (bindings.left.size() != KEY_COUNT)
	{
		Logger::logError(format("Expected %1% key bindings, received %2% instead",
		                        std::to_string(KEY_COUNT), std::to_string(bindings.left.size())));

		error("Invalid number of key bindings");
	}

	// listen to input events
	auto events = Locator::locate<EventService>();
	events->registerListener(this, EVENT_RAW_INPUT_KEY);

	playerEntity = -1;
}

void InputService::bindKey(InputKey binding, sf::Keyboard::Key key)
{
	std::string verb("Set");

	// remove existing
	auto existing = bindings.left.find(binding);
	if (existing != bindings.left.end())
	{
		verb = "Replaced existing";
		bindings.left.replace_data(existing, key);
	}

	bindings.left.insert({binding, key});
	Logger::logDebuggier(format("%1% binding for key %2%: %3%", verb, std::to_string(key), std::to_string(binding)));
}

void InputService::onEvent(const Event &event)
{
	InputKey binding(getBinding(event.rawInputKey.key));

	// unrecognized key
	if (binding == KEY_COUNT)
		return;

	// quit
	if (binding == KEY_EXIT)
	{
		Logger::logDebug("Exit key pressed, quitting");
		Locator::locate<RenderService>()->getWindow()->close();
		return;
	}

	EventService *es = Locator::locate<EventService>();
	bool hasEntity = hasPlayerEntity();

	Event e;

	// assign entity id
	e.entityID = hasEntity ? *playerEntity : CAMERA_ENTITY;

	// yield entity control
	if (binding == KEY_YIELD_CONTROL)
	{
		if (hasEntity)
		{
			e.type = EVENT_INPUT_YIELD_CONTROL;
			clearPlayerEntity();
			es->callEvent(e);
		}
		return;
	}

	// sprint
	if (binding == KEY_SPRINT)
	{
		e.type = EVENT_INPUT_SPRINT;
		e.sprintToggle.start = event.rawInputKey.pressed;
		es->callEvent(e);
		return;
	}

	// input
	bool startMoving = event.rawInputKey.pressed;
	e.type = startMoving ? EVENT_INPUT_START_MOVING : EVENT_INPUT_STOP_MOVING;

	DirectionType direction;

	switch (binding)
	{
		case KEY_UP:
			direction = DIRECTION_NORTH;
			break;
		case KEY_LEFT:
			direction = DIRECTION_WEST;
			break;
		case KEY_DOWN:
			direction = DIRECTION_SOUTH;
			break;
		case KEY_RIGHT:
			direction = DIRECTION_EAST;
			break;
		default:
			error("An invalid movement key slipped through InputService's onEvent: %1%",
			      std::to_string(binding));
			return;
	}

	if (startMoving)
		e.startMove.direction = direction;
	else
		e.stopMove.direction = direction;

	es->callEvent(e);
}

sf::Keyboard::Key InputService::getKey(InputKey binding)
{
	auto result = bindings.left.find(binding);
	return result == bindings.left.end() ? sf::Keyboard::Unknown : result->second;
}

InputKey InputService::getBinding(sf::Keyboard::Key key)
{

	auto result = bindings.right.find(key);
	return result == bindings.right.end() ? InputKey::KEY_COUNT : result->second;
}


void SimpleMovementController::registerListeners()
{
	Locator::locate<EventService>()->registerListener(this, EVENT_INPUT_START_MOVING);
	Locator::locate<EventService>()->registerListener(this, EVENT_INPUT_STOP_MOVING);
	Locator::locate<EventService>()->registerListener(this, EVENT_INPUT_SPRINT);
}

void SimpleMovementController::unregisterListeners()
{
	Locator::locate<EventService>()->unregisterListener(this, EVENT_INPUT_START_MOVING);
	Locator::locate<EventService>()->unregisterListener(this, EVENT_INPUT_STOP_MOVING);
	Locator::locate<EventService>()->unregisterListener(this, EVENT_INPUT_SPRINT);
}

b2Vec2 SimpleMovementController::tick(float delta, float &newMaxSpeed)
{
	bool north = moving[DIRECTION_NORTH];
	bool south = moving[DIRECTION_SOUTH];
	bool east = moving[DIRECTION_EAST];
	bool west = moving[DIRECTION_WEST];

	float x, y;

	if (east != west)
		x = east ? movementForce : -movementForce;
	else
		x = 0.f;

	if (south != north)
		y = south ? movementForce : -movementForce;
	else
		y = 0.f;

	newMaxSpeed = running ? maxSprintSpeed : maxSpeed;
	return {x, y};
}


void SimpleMovementController::tick(PhysicsComponent *phys, float delta)
{
	float maxSpeed;
	b2Vec2 steering(tick(delta, maxSpeed));
	phys->steering.Set(steering.x, steering.y);
	phys->maxSpeed = maxSpeed;

	wasRunning = running;
}

void SimpleMovementController::onEvent(const Event &event)
{
	// todo remove check and unregister listener instead
	if (event.entityID != entity)
		return;

	// sprinting
	if (event.type == EVENT_INPUT_SPRINT)
	{
		running = event.sprintToggle.start;
		return;
	}

	bool start = event.type == EVENT_INPUT_START_MOVING;
	DirectionType direction = start ? event.startMove.direction : event.stopMove.direction;
	moving[direction] = start;
}
