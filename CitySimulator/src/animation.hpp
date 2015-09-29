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
		return &animations.at(name);
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
	Animator(Animation *anim, float step);

	void tick(float delta);

	/// <summary>
	/// Turns in the given direction.
	/// </summary>
	void turn(int direction, bool reset = true);
	
	/// <summary>
	/// Plays/pauses the animation.
	/// </summary>
	void setPlaying(bool playing, bool reset = false);

	/// <summary>
	/// Toggles the pause state of the animation.
	/// </summary>
	void togglePlaying(bool resetEachTime = false);

private:
	Animation *animation;

	float frameStep;
	float currentFrameTime;

	size_t currentSequence;
	size_t currentFrame;
	bool playing;

	sf::VertexArray vertices;
	sf::Vector2f currentSize;

	int direction;


	void resizeVertices(float width, float height)
	{
		if (width == currentSize.x && height == currentSize.y)
			return;

		vertices[0].position = sf::Vector2f(0, 0);
		vertices[1].position = sf::Vector2f(width, 0);
		vertices[2].position = sf::Vector2f(width, height);
		vertices[3].position = sf::Vector2f(0, height);

		currentSize.x = width;
		currentSize.y = height;
	}

	void updateFrame()
	{
		Animation::Sequence sequence(animation->sequences[currentSequence]);
		sf::FloatRect rect(sequence[currentFrame]);

		resizeVertices(rect.width, rect.height);

		vertices[0].texCoords = sf::Vector2f(rect.left, rect.top);
		vertices[1].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
		vertices[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
		vertices[3].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);
	}

protected:

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		states.texture = animation->texture;
		target.draw(vertices, states);
	}
};
