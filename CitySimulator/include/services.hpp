#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <boost/bimap.hpp>
#include "logger.hpp"
#include "utils.hpp"

enum ServiceType
{
	SERVICE_INPUT,
	SERVICE_RENDER,

	SERVICE_COUNT
};

class BaseService
{
public:
	virtual void onEnable();

	virtual void onDisable();
};

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service)
	{
		std::string verb("Provided new");

		// delete old
		auto old = locate(type);
		if (old != nullptr)
		{
			verb = "Replaced";
			old->onDisable();
			delete old;
		}

		getInstance().services[type] = service;
		service->onEnable();
		Logger::logDebug(format("%1% service for service type %2%", verb, std::to_string(type)));
	}

	static BaseService *locate(ServiceType type)
	{
		return getInstance().services[type];
	}

private:
	Locator();

	~Locator();

	static Locator &getInstance()
	{
		static Locator instance;
		return instance;
	}

	std::vector<BaseService*> services;
};

// services

enum InputKey
{
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_YIELD_CONTROL,

	KEY_COUNT
};

class InputService : public BaseService
{
public:
	virtual void onEnable() override;

	/// <summary>
	/// Updates the specified key with the given pressed value
	/// </summary>
	/// <param name="key">The key.</param>
	/// <param name="press">if set to <c>true</c> [press].</param>
	void update(sf::Keyboard::Key key, bool press);

	/// <summary>
	/// Determines whether the specified key is pressed.
	/// </summary>
	/// <param name="key">The key.</param>
	/// <returns></returns>
	bool isPressed(InputKey key);

	/// <summary>
	/// Determines whether the specified key has just been pressed.
	/// </summary>
	/// <param name="key">The key.</param>
	/// <returns></returns>
	bool isFirstPressed(InputKey key);

	/// <summary>
	/// Moves forward a timestep.
	/// </summary>
	void advance();

private:
	boost::bimap<InputKey, sf::Keyboard::Key> bindings;
	std::vector<bool> pressed, wasPressed;
	// dynamic bitset is being a pain, but this won't have much overhead
};

class RenderService : public BaseService
{
public:
	RenderService(sf::RenderWindow &renderWindow);

	sf::RenderWindow* getWindow();

	void renderEntities();


private:
	sf::RenderWindow &window;
};

#endif
