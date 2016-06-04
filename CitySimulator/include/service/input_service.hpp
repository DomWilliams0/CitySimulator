#ifndef CITYSIMULATOR_INPUT_SERVICE_HPP
#define CITYSIMULATOR_INPUT_SERVICE_HPP

#include "ai.hpp"
#include <boost/bimap.hpp>
#include "base_service.hpp"

class InputBrain;
class EntityBrain;

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

	bool hasPlayerEntity();

	// throws an exception if hasPlayerEntity returns false
	EntityID getPlayerEntity();

private:
	boost::bimap<InputKey, sf::Keyboard::Key> bindings;

	boost::optional<EntityID> playerEntity;
	boost::shared_ptr<EntityBrain> playersOldBrain;
	boost::shared_ptr<InputBrain> inputBrain;

	void handleMouseEvent(const Event &event);

	void handleKeyEvent(const Event &event);

	void handleClickedFixture(b2Fixture *fixture);

	struct WorldQueryCallback : public b2QueryCallback
	{
		b2Fixture *fixture = nullptr;

		virtual bool ReportFixture(b2Fixture *fixture) override
		{
			this->fixture = fixture;
			return false; // stop after single hit
		}
	};

	bool getClickedFixture(const sf::Vector2i &screenPos, float radius, b2Fixture **out);
};

#endif
