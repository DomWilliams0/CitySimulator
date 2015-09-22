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

class Animator
{
public:
	Animator(Animation *anim, float step) : animation(anim), frameStep(step), currentFrameTime(0), currentSequence(0), currentFrame(0)
	{
		vertices.setPrimitiveType(sf::Quads);
		vertices.resize(4);
		updateFrame();
	}

	void tick(float delta)
	{
		currentFrameTime += delta;

		// next frame
		if (currentFrameTime >= frameStep)
		{
			currentFrameTime = fmod(currentFrameTime, frameStep);

			if (++currentFrame >= animation->sequences.size())
				currentFrame = 0;

			updateFrame();
		}
	}

private:
	Animation *animation;

	float frameStep;
	float currentFrameTime;

	size_t currentSequence;
	size_t currentFrame;

	sf::VertexArray vertices;
	sf::Vector2f currentSize;


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
		vertices[1].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);
		vertices[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
		vertices[3].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
	}

public:
	void draw(sf::RenderTarget &target, sf::RenderStates &states) const
	{
		states.texture = animation->texture;
		target.draw(vertices, states);
	}
};
