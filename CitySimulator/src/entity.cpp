#include <boost/property_tree/xml_parser.hpp>
#include "ai.hpp"
#include "world.hpp"

EntityID EntityService::createEntity()
{
	// no space
	if (entityCount == MAX_ENTITIES)
		error("Max number of entities reached (%1%)", std::to_string(MAX_ENTITIES));

	// todo: use a memory pool instead to avoid iterating the entire array each time
	for (EntityID e = 0; e < MAX_ENTITIES; ++e)
		if (!isAlive(e))
		{
			entityCount++;
			return e;
		}

	return MAX_ENTITIES;
}

void EntityService::killEntity(EntityID e)
{
	if (isAlive(e))
		entityCount--;

	entities[e] = COMPONENT_NONE;
}

bool EntityService::isAlive(EntityID e)
{
	return entities[e] != COMPONENT_NONE;
}

void EntityService::tickSystems(float delta)
{
	for (System *system : systems)
		system->tick(this, delta);
}

void EntityService::renderSystems(sf::RenderWindow &window)
{
	renderSystem->render(this, window);
}

BaseComponent *EntityService::addComponent(EntityID e, ComponentType type)
{
	entities[e] |= type;

	auto comp = getComponentOfType(e, type);
	comp->reset();
	return comp;
}

void EntityService::removeComponent(EntityID e, ComponentType type)
{
	entities[e] &= ~type;
}

bool EntityService::hasComponent(EntityID e, ComponentType type)
{
	return (entities[e] & type) != COMPONENT_NONE;
}

BaseComponent *EntityService::getComponentOfType(EntityID e, ComponentType type)
{
	switch (type)
	{
		case COMPONENT_PHYSICS:
			return &physicsComponents[e];
		case COMPONENT_RENDER:
			return &renderComponents[e];
		case COMPONENT_INPUT:
			return &inputComponents[e];
		default:
			error("Invalid component type %1%", std::to_string(type));
	}
}

void EntityService::addPhysicsComponent(EntityID e, World *world, const sf::Vector2i &startTilePos)
{
	PhysicsComponent *phys = dynamic_cast<PhysicsComponent *>(addComponent(e, COMPONENT_PHYSICS));
	b2World *bWorld = world->getBox2DWorld();

	phys->bWorld = bWorld;

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.position.Set(static_cast<float>(startTilePos.x), static_cast<float>(startTilePos.y));
	phys->body = bWorld->CreateBody(&def);
	phys->body->SetFixedRotation(true);

	// basic full body aabb
	b2PolygonShape aabb;

	const auto scale = Constants::entityScalef / 2;
	aabb.SetAsBox(scale, scale);
	aabb.m_centroid.Set(scale, scale);

	b2FixtureDef fixDef;
	fixDef.density = 985.f;
	fixDef.shape = &aabb;

	phys->body->CreateFixture(&fixDef);
}


void EntityService::addRenderComponent(EntityID e, EntityType entityType, const std::string &animation, float step,
                                       DirectionType initialDirection, bool playing)
{
	RenderComponent *comp = dynamic_cast<RenderComponent *>(addComponent(e, COMPONENT_RENDER));

	Animation *anim = Globals::spriteSheet->getAnimation(entityType, animation);
	comp->anim.init(anim, step, initialDirection, playing);
}

void EntityService::addBrain(EntityID e, bool aiBrain)
{
	InputComponent *comp = dynamic_cast<InputComponent *>(addComponent(e, COMPONENT_INPUT));

	if (aiBrain)
		comp->brain.reset(new AIBrain(e));
	else
		comp->brain.reset(new InputBrain(e));
}

void EntityService::addPlayerInputComponent(EntityID e)
{
	addBrain(e, false);
}

void EntityService::addAIInputComponent(EntityID e)
{
	addBrain(e, true);
}

