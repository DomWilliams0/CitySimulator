#ifndef CITYSIM_SERVICES_HPP
#define CITYSIM_SERVICES_HPP

#include <boost/bimap.hpp>
#include <typeindex>
#include <typeinfo>
#include <forward_list>
#include "entity.hpp"
#include "events.hpp"
#include <unordered_map>
#include "utils.hpp"
#include "constants.hpp"
#include "animation.hpp"
#include "config.hpp"

enum ServiceType
{
	SERVICE_ANIMATION,
	SERVICE_CAMERA,
	SERVICE_CONFIG,
	SERVICE_ENTITY,
	SERVICE_EVENT,
	SERVICE_INPUT,
	SERVICE_LOGGING,
	SERVICE_RENDER,
	SERVICE_UNKNOWN
};

class BaseService
{
public:
	virtual void onEnable();

	virtual void onDisable();
};

class AnimationService;
class CameraService;
class ConfigService;
class EntityService;
class EventService;
class InputService;
class LoggingService;
class RenderService;

class World;
class PhysicsComponent;
class SimpleMovementController;

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service);

	static std::string serviceToString(ServiceType type);

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

class CameraService : public BaseService
{
public:
	CameraService(World &world);
	~CameraService();

	virtual void onEnable() override;
	virtual void onDisable() override;

	void tick(float delta);

	void switchWorld(World &world);
	void setTrackedEntity(EntityID entity);
	void clearPlayerEntity();

	inline World* getWorld()
	{
		return world;
	}

	inline sf::View getView() const
	{
		return view;
	}

private:
	World *world;
	PhysicsComponent *trackedEntity;
	sf::View view;

	SimpleMovementController *controller;
};

class ConfigService : public BaseService
{
public:
	ConfigService(const std::string &directory,
	              const std::string &appConfigPath, const std::string &userConfigPath = "");

	virtual void onEnable() override;

	int getInt(const std::string &path);
	int getInt(const std::string &path, int defaultValue);
	float getFloat(const std::string &path);
	float getFloat(const std::string &path, float defaultValue);
	bool getBool(const std::string &path);
	bool getBool(const std::string &path, bool defaultValue);
	std::string getString(const std::string &path);
	std::string getString(const std::string &path, const std::string &defaultValue);
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

	void setRootDirectory(const std::string &directory)
	{
		rootDirectory = directory;
	}

private:
	ConfigurationFile config;
	boost::filesystem::path rootDirectory;

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

	void addPhysicsComponent(EntityID e, World *world, const sf::Vector2i &startTilePos, float maxSpeed, float damping);

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

typedef void(EventListener::*EventCallback)(Event &);

class EventService : public BaseService
{
public:
	virtual void onEnable() override;
	virtual void onDisable() override;

	void registerListener(EventListener *listener, EventType eventType);
	void unregisterListener(EventListener *listener, EventType eventType);

	void processQueue();

	void callEvent(const Event &event);

private:
	std::forward_list<Event> pendingEvents;
	std::unordered_map<EventType, std::forward_list<EventListener *>, std::hash<int>> listeners;
};

enum InputKey
{
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,

	KEY_YIELD_CONTROL,
	KEY_SPRINT,

	KEY_EXIT,

	KEY_COUNT
};

class SimpleMovementController : public EventListener
{
public:
	SimpleMovementController(EntityID entity, float movementForce, float maxWalkSpeed, float maxSprintSpeed)
			: entity(entity), moving(DIRECTION_COUNT, false),
			  running(false), wasRunning(false),
			  movementForce(movementForce), maxSpeed(maxWalkSpeed), maxSprintSpeed(maxSprintSpeed)

	{
		moving.shrink_to_fit();
	}

	~SimpleMovementController()
	{
		unregisterListeners();
	}

	void registerListeners();
	void unregisterListeners();

	b2Vec2 tick(float delta, float &newMaxSpeed);
	void tick(PhysicsComponent *phys, float delta);

	virtual void onEvent(const Event &event) override;

private:
	EntityID entity;

	std::vector<bool> moving;
	bool running, wasRunning;
	float movementForce, maxSprintSpeed, maxSpeed;

};

class InputService : public BaseService, public EventListener
{
public:
	virtual void onEnable() override;

	void bindKey(InputKey binding, sf::Keyboard::Key key);
	sf::Keyboard::Key getKey(InputKey binding);
	InputKey getBinding(sf::Keyboard::Key key);

	virtual void onEvent(const Event &event) override;

	inline void setPlayerEntity(EntityID entity)
	{
		playerEntity = entity;
		Locator::locate<CameraService>()->setTrackedEntity(entity);
	}

	inline void clearPlayerEntity()
	{
		playerEntity.reset();
		Locator::locate<CameraService>()->clearPlayerEntity();
	}

	inline bool hasPlayerEntity()
	{
		return playerEntity.is_initialized();
	}

	// throws an exception if hasPlayerEntity returns false
	inline EntityID getPlayerEntity()
	{
		return playerEntity.get();
	}


private:
	boost::bimap<InputKey, sf::Keyboard::Key> bindings;
	boost::optional<EntityID> playerEntity;
};

enum LogLevel
{
	LOG_DEBUGGIEST,
	LOG_DEBUGGIER,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,

	LOG_UNKNOWN
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

	// no change if invalid
	void setLogLevel(const std::string &s);

private:
	LogLevel level;
	std::ostream &stream;
	std::string prefix;

	std::unordered_map<LogLevel, std::string, std::hash<int>> levels;

	virtual void log(const std::string &msg, LogLevel level);
	LogLevel logLevelFromString(const std::string &s);


};

class NullLoggingService : public LoggingService
{

public:
	NullLoggingService();

private:
	virtual void log(const std::string &msg, LogLevel level) override
	{
	}
};

class RenderService : public BaseService
{
public:
	RenderService(sf::RenderWindow *renderWindow);

	virtual void onEnable() override;
	void render(const World &world);

	sf::RenderWindow *getWindow();

	inline void setView(sf::View &view)
	{
		this->view = &view;
	}

private:
	sf::RenderWindow *window;
	sf::View *view;
};

// helpers

namespace Config
{
	int getInt(const std::string &path);
	int getInt(const std::string &path, int defaultValue);
	float getFloat(const std::string &path);
	float getFloat(const std::string &path, float defaultValue);
	bool getBool(const std::string &path);
	bool getBool(const std::string &path, bool defaultValue);
	std::string getString(const std::string &path);
	std::string getString(const std::string &path, const std::string &defaultValue);
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
