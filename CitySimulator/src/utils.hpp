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
	sf::Vector2<V> toPixel(const sf::Vector2<V> &v)
	{
		return sf::Vector2<V>(v.x * Constants::tileSize, v.y * Constants::tileSize);
	}

	template <class V>
	sf::Vector2<V> toTile(const sf::Vector2<V> &v)
	{
		return sf::Vector2<V>(v.x / Constants::tileSize, v.y / Constants::tileSize);
	}

	/// <summary>
	/// Generates a random number between min and max-1
	/// </summary>
	template <class T=int>
	T random(T min, T max)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> dis(0, 1);

		return static_cast<T>(dis(gen) * (max - min) + min);
	}


	struct filenotfound_exception : std::runtime_error
	{
		explicit filenotfound_exception(const char *msg)
			: runtime_error(msg)
		{
		}


		explicit filenotfound_exception(const std::string &msg)
			: runtime_error(msg)
		{
		}
	};

	void validateDirectory(const std::string &directory);
	std::string searchForFile(const std::string &filename, const std::string &directory = "res");
}

namespace Math
{
	template <class V>
	float length(const sf::Vector2<V> &v)
	{
		return sqrtf(lengthSquared(v));
	}

	template <class V>
	float lengthSquared(const sf::Vector2<V> &v)
	{
		return v.x * v.x + v.y * v.y;
	}
}

namespace Debug
{
	template <class V>
	inline void printVector(const sf::Vector2<V> &v, const char *msg = "")
	{
		printf("%s%f, %f\n", msg, v.x, v.y);
	}
}
