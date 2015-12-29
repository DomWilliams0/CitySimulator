#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <unordered_map>
#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>

enum ServiceType
{
	SERVICE_INPUT
};

class BaseService;

class Locator
{
public:
	void provide(ServiceType type, BaseService *service);

	BaseService* locate(ServiceType type);

private:
	std::unordered_map<ServiceType, BaseService*, std::hash<int>> services;

};

// services

class BaseService
{
public:
	virtual void onEnable();
	virtual void onDisable();
};

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
	boost::dynamic_bitset<> pressed, wasPressed;
};

#endif
