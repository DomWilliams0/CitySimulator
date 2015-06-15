#pragma once
#include <SFML/System.hpp>
#include "constants.hpp"

namespace Utils
{
	class TimeTicker
	{
	public:
		void init(float min, float max = -1);
		bool tick(float delta);

	private:
		void reset();

		float minDuration, maxDuration;
		bool range;

		float current;
		float currentEnd;
	};

	sf::Color darken(const sf::Color &color, int delta);

	template <class V>
	inline sf::Vector2<V> toPixel(const sf::Vector2<V> &v)
	{
		return sf::Vector2<V>(v.x * Constants::tileSize, v.y * Constants::tileSize);
	}

	template <class V>
	inline sf::Vector2<V> toTile(const sf::Vector2<V> &v)
	{
		return sf::Vector2<V>(v.x / Constants::tileSize, v.y / Constants::tileSize);
	}

	inline int randomRange(int min, int max)
	{
		return (rand() % ((max - min) + 1)) + min;
	}

	std::string searchForFile(const std::string &filename, const std::string &directory = "res");

	sf::Image rotateImage(const sf::Image &image, int rotation);
}
