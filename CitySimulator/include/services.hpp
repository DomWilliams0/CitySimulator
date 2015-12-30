#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <boost/bimap.hpp>
#include "logger.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "entity.hpp"

enum ServiceType
{
	SERVICE_INPUT,
	SERVICE_RENDER,
	SERVICE_CONFIG,

	SERVICE_COUNT
};

class BaseService
{
public:
	virtual void onEnable();

	virtual void onDisable();
};

class InputService;
class RenderService;
class ConfigService;

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service)
	{
		std::string verb("Provided new");

		// delete old
		auto old = getInstance().services[type];
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

	// helper
	template <class T>
	static T* locate(bool errorOnFail = true)
	{
		ServiceType type = SERVICE_COUNT;

		if (typeid(T) == typeid(InputService))
			type = SERVICE_INPUT;
		else if (typeid(T) == typeid(RenderService))
			type = SERVICE_RENDER;
		else if (typeid(T) == typeid(ConfigService))
			type = SERVICE_CONFIG;

		if (type == SERVICE_COUNT)
			error("Invalid service type given");

		auto ret = dynamic_cast<T *>(getInstance().services[type]);
		if (errorOnFail && ret == nullptr)
			error("Service could not be located");
		return ret;
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

	void bindKey(InputKey binding, sf::Keyboard::Key key);

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

	sf::Keyboard::Key getKey(InputKey binding);

	InputKey getBinding(sf::Keyboard::Key key);

private:
	boost::bimap<InputKey, sf::Keyboard::Key> bindings;
	std::vector<bool> pressed, wasPressed;
	// dynamic bitset is being a pain, but this won't have much overhead
};

class RenderService : public BaseService
{
public:
	RenderService(sf::RenderWindow *renderWindow);

	sf::RenderWindow* getWindow();

	void renderEntities();

private:
	sf::RenderWindow *window;
};

class ConfigService : public BaseService
{
public:
	ConfigService(const std::string &path, const std::string &overridingPath = "");

	virtual void onEnable() override;

	int getInt(const std::string &path);

	float getFloat(const std::string &path);

	bool getBool(const std::string &path);

	std::string getString(const std::string &path);

	std::string getResource(const std::string &path);

	template<class T>
	void getList(const std::string &path, std::vector<int> &l)
	{
		config.getList<T>(path, l);
	}

	template<class T=std::string>
	void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml)
	{
		config.getMapList<T>(path, ml);
	}

private:
	ConfigurationFile config;

	void ensureConfigExists();
};

const unsigned int MAX_ENTITIES = 1024;
typedef unsigned int EntityID;

class EntityService : public BaseService
{
public:
	virtual void onEnable() override;

	virtual void onDisable() override;

	// entity management
	EntityID createEntity();

	void killEntity(EntityID e);

	bool isAlive(EntityID e);

	// systems
	void tickSystems(float delta);

	void renderSystems(sf::RenderWindow &window);

private:
	Entity entities[MAX_ENTITIES];
	size_t entityCount;

	// components
	PhysicsComponent physicsComponents[MAX_ENTITIES];
	RenderComponent renderComponents[MAX_ENTITIES];
	InputComponent inputComponents[MAX_ENTITIES];

	// component management
	void removeComponent(Entity e, ComponentType type);

	bool hasComponent(Entity e, ComponentType type);

	BaseComponent *getComponentOfType(Entity e, ComponentType type);

	template<class T>
	T *getComponent(Entity e, ComponentType type)
	{
		return dynamic_cast<T *>(getComponentOfType(e, type));
	}

	// systems
	std::vector<System *> systems;
	RenderSystem *renderSystem;

	// helpers
	void addPhysicsComponent(Entity e, World *world, const sf::Vector2i &startTilePos);

	void addRenderComponent(Entity e, EntityType entityType, const std::string &animation,
	                        float step, DirectionType initialDirection, bool playing);

	void addPlayerInputComponent(Entity e);

	void addAIInputComponent(Entity e);

	BaseComponent *addComponent(Entity e, ComponentType type);

	void addBrain(Entity e, bool aiBrain);
};

// helpers

namespace Config
{
	int getInt(const std::string &path);

	float getFloat(const std::string &path);

	bool getBool(const std::string &path);

	std::string getString(const std::string &path);

	std::string getResource(const std::string &path);

	template<class T>
	void getList(const std::string &path, std::vector<int> &l)
	{
		Locator::locate<ConfigService>()->getList<T>(path, l);
	}

	template<class T=std::string>
	void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml)
	{
		Locator::locate<ConfigService>()->getMapList<T>(path, ml);
	}
}

#endif
