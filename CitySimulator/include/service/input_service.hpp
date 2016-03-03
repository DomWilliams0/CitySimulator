#ifndef CITYSIM_INPUT_SERVICE_HPP
#define CITYSIM_INPUT_SERVICE_HPP

#include "ai.hpp"
#include <boost/bimap.hpp>
#include "base_service.hpp"

class InputBrain;

class InputService : public BaseService, public EventListener
{
public:
	virtual void onEnable() override;

	virtual void onDisable() override;

	void bindKey(InputKey binding, sf::Keyboard::Key key);

	sf::Keyboard::Key getKey(InputKey binding);

	InputKey getBinding(sf::Keyboard::Key key);

	virtual void onEvent(const Event &event) override;

	void setPlayerEntity(EntityID entity);

	void clearPlayerEntity();

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
	boost::shared_ptr<EntityBrain> playersOldBrain;
	boost::shared_ptr<InputBrain> inputBrain;

	void handleMouseEvent(const Event &event);

	void handleKeyEvent(const Event &event);

	boost::optional<EntityIdentifier *> getClickedEntity(const sf::Vector2i &screenPos, float radius = 0.25f);
};

#endif
