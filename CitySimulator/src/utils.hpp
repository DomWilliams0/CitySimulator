#pragma once
#include <SFML/System.hpp>
#include "constants.hpp"
#include <random>

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

	inline float randomRange(float min, float max)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> dis(0, 1);

		return static_cast<float>(dis(gen) * (max - min) + min);
	}

	std::string searchForFile(const std::string &filename, const std::string &directory = "res");
}

namespace Debug
{
	template <class V>
	inline void printVector(const sf::Vector2<V> &v, const char *msg = "")
	{
		printf("%s%f, %f\n", msg, v.x, v.y);
	}
}