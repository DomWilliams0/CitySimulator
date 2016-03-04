#ifndef CITYSIMULATOR_ANIMATION_HPP
#define CITYSIMULATOR_ANIMATION_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "constants.hpp"
#include "utils.hpp"

struct Animation
{
	typedef std::vector<sf::IntRect> Sequence;

	explicit Animation(sf::Texture *animationTexture) : texture(animationTexture)
	{
	}

	Animation *addRow(const sf::Vector2i &startPosition, const sf::Vector2i &spriteDimensions, int rowLength);


	//	std::vector<Sequence>& getSequences()
	//	{
	//		return sequences;
	//	}

	sf::Texture *texture;
	std::vector<Sequence> sequences;
};

class Animator : public sf::Drawable
{
public:
	Animator()
	{
		reset();
	}

	Animator(Animation *anim, float step, DirectionType initialDirection = DIRECTION_SOUTH,
			 bool initiallyPlaying = false);

	void init(Animation *anim, float step, DirectionType initialDirection = DIRECTION_SOUTH,
			  bool initiallyPlaying = false);

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

	Utils::TimeTicker ticker;

	size_t currentSequence;
	size_t currentFrame;
	bool playing;

	sf::VertexArray vertices;
	sf::Vector2f currentSize;

	DirectionType direction;


	void resizeVertices(float width, float height);

	void updateFrame();
};

#endif