#include "ecs.hpp"
#include "Box2D/Dynamics/b2World.h"


void RenderComponent::reset()
{
	anim.reset();
}


void InputComponent::reset()
{
	brain.reset();
}

void PhysicsComponent::reset()
{
	if (body != nullptr)
	{
		bWorld->DestroyBody(body);
		body = nullptr;
	}
}
