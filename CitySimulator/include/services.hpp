#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <boost/bimap.hpp>
#include <typeindex>
#include <typeinfo>
#include <iostream>
#include "entity.hpp"
#include "utils.hpp"
#include "config.hpp"

enum ServiceType
{
	SERVICE_INPUT,
	SERVICE_RENDER,
	SERVICE_CONFIG,
	SERVICE_ENTITY,
	SERVICE_ANIMATION,
	SERVICE_LOGGING,

	SERVICE_UNKNOWN
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

class EntityService;

class AnimationService;

class LoggingService;

class World;

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service);

	static std::string serviceToString(ServiceType type);

	// helper
	template<class T>
	static T *locate(bool errorOnFail = true)
	{
		auto possibleType = getInstance().types.find(typeid(T));

		if (possibleType == getInstance().types.end())
			error("Invalid service type given. Has its type been registered?");

		ServiceType type = possibleType->second;
		T *ret = dynamic_cast<T *>(getInstance().services[type]);
		if (errorOnFail && ret == nullptr)
			error("Could not locate service '%1%'", serviceToString(type));
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

	std::vector<BaseService *> services;
	std::unordered_map<std::type_index, ServiceType> types;

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

	sf::RenderWindow *getWindow();

	void renderEntities();

private:
	sf::RenderWindow *window;
};

class ConfigService : public BaseService
{
public:
	ConfigService(const std::string &appConfigPath, const std::string &userConfigPath = "");

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
typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

class EntityService : public BaseService
{
public:
	virtual void onEnable() override;

	virtual void onDisable() override;

	unsigned int getEntityCount() const;

	EntityID createEntity();

	void killEntity(EntityID e);

	bool isAlive(EntityID e) const;

	inline EntityID getComponentMask(EntityID e) const
	{
		if (e < 0 || e >= MAX_ENTITIES)
			error("EntityID %1% out of range in getComponentMask", std::to_string(e));

		return entities[e];
	}

	// systems
	void tickSystems(float delta);

	void renderSystems(sf::RenderWindow &window);

	// component management
	void removeComponent(EntityID e, ComponentType type);

	bool hasComponent(EntityID e, ComponentType type) const;

	BaseComponent *getComponentOfType(EntityID e, ComponentType type);

	template<class T>
	T *getComponent(EntityID e, ComponentType type)
	{
		return dynamic_cast<T *>(getComponentOfType(e, type));
	}

	void addPhysicsComponent(EntityID e, World *world, const sf::Vector2i &startTilePos);

	void addRenderComponent(EntityID e, EntityType entityType, const std::string &animation,
	                        float step, DirectionType initialDirection, bool playing);

	void addPlayerInputComponent(EntityID e);

	void addAIInputComponent(EntityID e);

private:
	EntityID entities[MAX_ENTITIES];
	EntityID entityCount;

	// loading
	std::map<EntityType, EntityTags> loadedTags;

	void loadEntities(ConfigurationFile &config, EntityType entityType, const std::string &sectionName);

	// components
	PhysicsComponent physicsComponents[MAX_ENTITIES];
	RenderComponent renderComponents[MAX_ENTITIES];
	InputComponent inputComponents[MAX_ENTITIES];

	// systems
	std::vector<System *> systems;
	RenderSystem *renderSystem;

	// helpers
	BaseComponent *addComponent(EntityID e, ComponentType type);

	void addBrain(EntityID e, bool aiBrain);
};

class AnimationService : public BaseService
{
public:
	virtual void onEnable() override;

	void loadSprite(ConfigKeyValue &entityTags, EntityType entityType);

	Animation *getAnimation(EntityType entityType, const std::string &name);

	std::string getRandomAnimationName(EntityType entityType);

	void processQueuedSprites();

private:
	sf::Texture texture;
	std::map<EntityType, std::unordered_map<std::string, Animation>> animations;

	std::map<sf::Image *, std::pair<ConfigKeyValue, EntityType>> *preProcessImageData;
	bool processed;

	void checkProcessed(bool shouldBe);

	sf::Vector2i stringToVector(const std::string &s);

	void positionImages(sf::Vector2i &imageSize, std::map<sf::Image *, sf::IntRect> &imagePositions);
};

enum LogLevel
{
	LOG_DEBUGGIEST,
	LOG_DEBUGGIER,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

class LoggingService : public BaseService
{
public:
	LoggingService(std::ostream &stream, const LogLevel &level) : stream(stream), level(level)
	{
	}

	virtual void onEnable() override;

	inline void logDebuggier(const std::string &msg)
	{
		log(msg, LOG_DEBUGGIER);
	}

	inline void logDebuggiest(const std::string &msg)
	{
		log(msg, LOG_DEBUGGIEST);
	}

	inline void logDebug(const std::string &msg)
	{
		log(msg, LOG_DEBUG);
	}

	inline void logInfo(const std::string &msg)
	{
		log(msg, LOG_INFO);
	}

	inline void logWarning(const std::string &msg)
	{
		log(msg, LOG_WARNING);
	}

	inline void logError(const std::string &msg)
	{
		log(msg, LOG_ERROR);
	}

	void pushIndent();

	void popIndent();

private:
	LogLevel level; // todo unused
	std::ostream &stream;
	std::string prefix;

	std::unordered_map<LogLevel, std::string, std::hash<int>> levels;

	virtual void log(const std::string &msg, LogLevel level);

};

class NullLoggingService : public LoggingService
{

public:
	NullLoggingService() : LoggingService(std::cerr, LOG_INFO)
	{
	}

private:
	virtual void log(const std::string &msg, LogLevel level) override
	{
	}
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

namespace Logger
{
	inline void logDebuggiest(const std::string &msg)
	{
		Locator::locate<LoggingService>()->logDebuggiest(msg);
	}

	inline void logDebuggier(const std::string &msg)
	{
		Locator::locate<LoggingService>()->logDebuggier(msg);
	}

	inline void logDebug(const std::string &msg)
	{
		Locator::locate<LoggingService>()->logDebug(msg);
	}

	inline void logInfo(const std::string &msg)
	{
		Locator::locate<LoggingService>()->logInfo(msg);
	}

	inline void logWarning(const std::string &msg)
	{
		Locator::locate<LoggingService>()->logWarning(msg);
	}

	inline void logError(const std::string &msg)
	{
		Locator::locate<LoggingService>()->logError(msg);
	}

	inline void pushIndent()
	{
		Locator::locate<LoggingService>()->pushIndent();
	}

	inline void popIndent()
	{
		Locator::locate<LoggingService>()->popIndent();
	}
}

#endif
