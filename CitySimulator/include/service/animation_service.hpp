#ifndef CITYSIM_ANIMATION_SERVICE_HPP
#define CITYSIM_ANIMATION_SERVICE_HPP

#include "base_service.hpp"
#include "constants.hpp"
#include "animation.hpp"

class AnimationService : public BaseService
{
public:
	virtual void onEnable() override;

	void loadSprite(ConfigKeyValue &entityTags, EntityType entityType);

	void loadGUI();

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

#endif
