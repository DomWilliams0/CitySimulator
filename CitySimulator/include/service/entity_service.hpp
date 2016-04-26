#ifndef CITYSIMULATOR_ENTITY_SERVICE_HPP
#define CITYSIMULATOR_ENTITY_SERVICE_HPP

#include "base_service.hpp"
#include "ecs.hpp"
#include "world.hpp"

const EntityID MAX_ENTITIES = 1024;
typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

class EntityService : public BaseService
{
public:
	virtual void onEnable() override;

	virtual void onDisable() override;

	unsigned int getEntityCount() const;

	EntityID createEntity();

	EntityIdentifier *createEntity(EntityType type);

	void killEntity(EntityID e);

	bool isAlive(EntityID e) const;

	inline EntityID getComponentMask(EntityID e) const
	{
		if (e < 0 || e >= MAX_ENTITIES)
			error("EntityID %1% out of range in getComponentMask", _str(e));

		return entities[e];
	}

	boost::optional<EntityIdentifier *> getEntityIDFromBody(const b2Body &body);

	// systems
	void tickSystems(float delta);

	void renderSystems();

	// component management
	void removeComponent(EntityID e, ComponentType type);

	bool hasComponent(EntityID e, ComponentType type) const;

	BaseComponent *getComponentOfType(EntityID e, ComponentType type);

	template<class T>
	T *getComponent(EntityID e, ComponentType type)
	{
		return dynamic_cast<T *>(getComponentOfType(e, type));
	}

	void addPhysicsComponent(EntityIdentifier &entity, World *world, const sf::Vector2i &startTilePos,
							 float maxSpeed, float damping);

	void addRenderComponent(const EntityIdentifier &entity, const std::string &animation, float step,
							DirectionType initialDirection, bool playing);

	void addPlayerInputComponent(EntityID e);

	void addAIInputComponent(EntityID e);

	/**
	 * @return A new body with new BodyData for the given entity
	 */
	b2Body *createBody(b2World *world, EntityIdentifier &entity, const sf::Vector2f &pos);

	/**
	 * @return A new body with the same BodyData and position as the given body
	 */
	b2Body *createBody(b2World *world, b2Body *clone);

private:
	EntityID entities[MAX_ENTITIES];
	EntityIdentifier identifiers[MAX_ENTITIES];

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
};

#endif
