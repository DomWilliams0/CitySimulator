#include <SFML/Graphics.hpp>
#include "animation.hpp"


Animation *Animation::addRow(const sf::Vector2i &startPosition, const sf::Vector2i &spriteDimensions, int rowLength)
{
	Sequence seq;

	sf::IntRect rect(startPosition, spriteDimensions);
	for (int i = 0; i < rowLength; i++)
	{
		seq.push_back(rect);
		rect.left += spriteDimensions.x;
	}

	sequences.push_back(seq);
	return this;
}

Animator::Animator(Animation *anim, float step, DirectionType initialDirection, bool initiallyPlaying)
{
	init(anim, step, initialDirection, initiallyPlaying);
}

void Animator::init(Animation *anim, float step, DirectionType initialDirection, bool initiallyPlaying)
{
	animation = anim;
	ticker.init(step);
	currentSequence = 0;
	currentFrame = 0;
	playing = initiallyPlaying;
	direction = initialDirection;

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(4);

	if (anim != nullptr)
	{
		direction = Direction::DIRECTION_COUNT;
		turn(initialDirection);
	}
}

void Animator::tick(float delta)
{
	if (!playing)
		return;

	// next frame
	if (ticker.tick(delta))
	{
		if (++currentFrame >= animation->sequences.size())
			currentFrame = 0;

		updateFrame();
	}
}

void Animator::turn(DirectionType direction, bool reset)
{
	if (direction == this->direction)
		return;

	this->direction = direction;

	// convert direction to sequence
	if (direction == Direction::NORTH)
		currentSequence = 3;
	else if (direction == Direction::EAST)
		currentSequence = 2;
	else if (direction == Direction::WEST)
		currentSequence = 1;
	else if (direction == Direction::SOUTH)
		currentSequence = 0;

	if (reset)
		currentFrame = 0;

	updateFrame();
}

void Animator::setPlaying(bool playing, bool reset)
{
	this->playing = playing;

	if (reset)
	{
		currentFrame = 0;
		updateFrame();
	}
}

void Animator::togglePlaying(bool resetEachTime)
{
	setPlaying(!playing, resetEachTime);
}

void Animator::resizeVertices(float width, float height)
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

void Animator::updateFrame()
{
	Animation::Sequence sequence(animation->sequences[currentSequence]);
	sf::FloatRect rect(sequence[currentFrame]);

	resizeVertices(rect.width, rect.height);

	vertices[0].texCoords = sf::Vector2f(rect.left, rect.top);
	vertices[1].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
	vertices[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
	vertices[3].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);
}

void Animator::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.texture = animation->texture;
	target.draw(vertices, states);
}
