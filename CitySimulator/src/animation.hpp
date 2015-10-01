#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "constants.hpp"

struct Animation
{
	typedef std::vector<sf::IntRect> Sequence;

	explicit Animation(sf::Texture *animationTexture) : texture(animationTexture)
	{
	}

	Animation* addRow(const sf::Vector2i &startPosition, const sf::Vector2i &spriteDimensions, int rowLength);


	//	std::vector<Sequence>& getSequences()
	//	{
	//		return sequences;
	//	}

	sf::Texture *texture;
	std::vector<Sequence> sequences;
};

class SpriteSheet
{
public:
	SpriteSheet() : processed(false)
	{
	}

	void loadSprite(ConfigKeyValue &entityTags);
	void processAllSprites();

	Animation* getAnimation(const std::string &name)
	{
		auto anim = animations.find(name);
		if (anim == animations.end())
		FAIL("Animation '%s' not found", name);

		return &anim->second;
	}

private:
	sf::Texture texture;
	std::unordered_map<std::string, Animation> animations;

	std::map<sf::Image*, ConfigKeyValue> preProcessSpriteImages;
	bool processed;
	void checkProcessed(bool shouldBe);

	sf::Vector2i stringToVector(const std::string &s);
	void positionImages(sf::Vector2i &imageSize, std::map<sf::Image*, sf::IntRect> &imagePositions);
};

class Animator : public sf::Drawable
{
public:
	Animator()
	{
		reset();
	}

	Animator(Animation *anim, float step, DirectionType initialDirection = Direction::SOUTH, bool initiallyPlaying = false);
	void init(Animation *anim, float step, DirectionType initialDirection = Direction::SOUTH, bool initiallyPlaying = false);

	void tick(float delta);

	/// <summary>
	/// Turns in the given direction.
	/// </summary>
	void turn(DirectionType direction, bool reset = true);

	/// <summary>
	/// Plays/pauses the animation.
	/// </summary>
	void setPlaying(bool playing, bool reset = false);

	/// <summary>
	/// Toggles the pause state of the animation.
	/// </summary>
	void togglePlaying(bool resetEachTime = false);

	void reset()
	{
		init(nullptr, 0);
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;


private:
	Animation *animation;

	float frameStep;
	float currentFrameTime;

	size_t currentSequence;
	size_t currentFrame;
	bool playing;

	sf::VertexArray vertices;
	sf::Vector2f currentSize;

	DirectionType direction;


	void resizeVertices(float width, float height);
	void updateFrame();
};
