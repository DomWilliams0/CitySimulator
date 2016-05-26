#include "ecs.hpp"
#include "Box2D/Box2D.h"


void RenderComponent::reset()
{
	anim.reset();
}


void InputComponent::reset()
{
	brain.reset();
}

sf::Vector2f PhysicsComponent::getTilePosition() const
{
	return Utils::fromB2Vec<float>(body->GetPosition());
}

sf::Vector2f PhysicsComponent::getPosition() const
{
	b2Vec2 pos(body->GetPosition());
	return Utils::toPixel(Utils::fromB2Vec<float>(pos));
}

sf::Vector2f PhysicsComponent::getVelocity() const
{
	b2Vec2 v = body->GetLinearVelocity();
	return Utils::fromB2Vec<float>(v);
}

sf::Vector2f PhysicsComponent::getLastVelocity() const
{
	return Utils::fromB2Vec<float>(lastVelocity);
}

void PhysicsComponent::setVelocity(const sf::Vector2f &velocity)
{
	body->SetLinearVelocity(Utils::toB2Vec(velocity));
}

bool PhysicsComponent::isStopped()
{
	return Math::lengthSquared(getVelocity()) < 1;
}

bool PhysicsComponent::isSteering()
{
	return steering.x != 0.f || steering.y != 0.f;
}


void PhysicsComponent::getAABB(b2AABB &out)
{
	// merci http://gamedev.stackexchange.com/a/1373
	out.lowerBound = b2Vec2(FLT_MAX, FLT_MAX);
	out.upperBound = b2Vec2(-FLT_MAX, -FLT_MAX);
	b2Fixture *fixture = body->GetFixtureList();
	while (fixture != nullptr)
	{
		out.Combine(out, fixture->GetAABB(0));
		fixture = fixture->GetNext();
	}
}


void PhysicsComponent::reset()
{
	world = 0;
	if (body != nullptr)
	{
		bWorld->DestroyBody(body);
		body = nullptr;
	}
}
