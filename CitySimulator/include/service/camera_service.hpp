#ifndef CITYSIMULATOR_CAMERA_SERVICE_HPP
#define CITYSIMULATOR_CAMERA_SERVICE_HPP

#include "ecs.hpp"
#include "base_service.hpp"
#include "world.hpp"
#include "input.hpp"

struct PhysicsComponent;


class CameraService : public BaseService
{
public:
	CameraService(World &world);

	virtual void onEnable() override;

	virtual void onDisable() override;

	void tick(float delta);

	World *getCurrentWorld();

	void switchWorld(WorldID world, const sf::Vector2f &centredTile);

	void setTrackedEntity(EntityID entity);

	void clearPlayerEntity();

	PhysicsComponent *getTrackedEntity() const;

	void updateViewSize(unsigned int width, unsigned int height);

	// merci: https://github.com/SFML/SFML/wiki/Source:-Zoom-View-At-(specified-pixel)
	void zoomTo(float delta, const sf::Vector2i &pixel, sf::RenderWindow &window);

private:
	World *world;
	PhysicsComponent *trackedEntity;
	sf::View view;
	float zoom;

	PlayerMovementController controller;

	struct WorldChangeListener : public EventListener
	{
		CameraService *cs;

		WorldChangeListener(CameraService *cs);

		virtual void onEvent(const Event &event) override;

	} worldChangeListener;
};

#endif
