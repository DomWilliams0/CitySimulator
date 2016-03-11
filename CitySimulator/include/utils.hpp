#ifndef CITYSIMULATOR_UTILS_HPP
#define CITYSIMULATOR_UTILS_HPP

#include <SFML/Graphics.hpp>
#include <random>

#define _str std::to_string


std::string format(const std::string &s, const std::string &arg1);

std::string format(const std::string &s, const std::string &arg1, const std::string &arg2);

std::string format(const std::string &s, const std::string &arg1, const std::string &arg2, const std::string &arg3);

void error(const std::string &msg);

void error(const std::string &msg, const std::string &arg1);

void error(const std::string &msg, const std::string &arg1, const std::string &arg2);

void error(const std::string &msg, const std::string &arg1, const std::string &arg2, const std::string &arg3);

namespace Constants
{
	extern const int tileSize;
	extern const float tileSizef;
}

namespace Utils
{
	class TimeTicker
	{
	public:
		TimeTicker()
		{
			setMinAndMax(1.f);
		}

		TimeTicker(float min)
		{
			setMinAndMax(min);
		}

		TimeTicker(float min, float max)
		{
			setMinAndMax(min, max);
		}

		// if max is not -1, the time value is generated randomly between
		// min and max on each reset
		void setMinAndMax(float min, float max = -1);

		bool tick(float delta);

	private:
		void reset();

		float minDuration, maxDuration;
		bool range;

		float current;
		float currentEnd;
	};

	sf::Color darken(const sf::Color &color, int delta);

	template<class V>
	sf::Vector2<V> toPixel(const sf::Vector2<V> &v)
	{
		return sf::Vector2<V>(v.x * Constants::tileSize, v.y * Constants::tileSize);
	}

	template<class V>
	sf::Vector2<V> toPixel(V x, V y)
	{
		return sf::Vector2<V>(x * Constants::tileSize, y * Constants::tileSize);
	}

	template<class V>
	sf::Vector2<V> toTile(const sf::Vector2<V> &v)
	{
		return sf::Vector2<V>(v.x / Constants::tileSize, v.y / Constants::tileSize);
	}

	template<class V>
	sf::Vector2<V> toTile(V x, V y)
	{
		return sf::Vector2<V>(x / Constants::tileSize, y / Constants::tileSize);
	}

	/// <summary>
	/// Generates a random number between min and max-1
	/// </summary>
	template<class T=int>
	T random(T min, T max)
	{
		static std::random_device rd;
		static std::mt19937 gen(50);
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

	int stringToInt(const std::string &s);

	void validateDirectory(const std::string &directory);

	std::string searchForFile(const std::string &filename, const std::string &directory = "");

	std::string joinPaths(const std::string &root, const std::string &path);

	int roundToMultiple(double x, int multiple);

	template<class T>
	sf::Rect<T> scaleToBox2D(const sf::Rect<T> &rect)
	{
		return sf::Rect<T>(rect.left / Constants::tileSizef, rect.top / Constants::tileSizef,
						   rect.width / Constants::tileSizef, rect.height / Constants::tileSizef);
	}
}

namespace Math
{
	extern const double EPSILON;
	extern const double degToRad;
	extern const double radToDeg;

	template<class V=float>
	float lengthSquared(const sf::Vector2<V> &v)
	{
		return v.x * v.x + v.y * v.y;
	}

	template<class V=float>
	float length(const sf::Vector2<V> &v)
	{
		return sqrtf(lengthSquared(v));
	}

	template<class V=float>
	sf::Vector2<V> normalize(const sf::Vector2<V> &v)
	{
		float length = Math::length(v);
		if (length <= EPSILON)
			return v;

		sf::Vector2<V> ret;
		ret.x = v.x / length;
		ret.y = v.y / length;

		return ret;
	}

	template<class V=float>
	sf::Vector2<V> multiply(const sf::Vector2<V> &v, V scalar)
	{
		sf::Vector2<V> ret(v);
		ret.x *= scalar;
		ret.y *= scalar;
		return ret;
	}

	template<class V=float>
	sf::Vector2<V> multiply(const sf::Vector2<V> &v, const sf::Vector2<V> &v2)
	{
		sf::Vector2<V> ret(v);
		ret.x *= v2.x;
		ret.y *= v2.y;
		return ret;
	}

	template<class V=float>
	sf::Vector2f truncate(const sf::Vector2f &v, const float newLength)
	{
		float l(length(v));
		if (l != 0.f)
			return multiply(v, newLength / l);

		return sf::Vector2<V>(v);
	}
}

namespace Debug
{
	template<class V>
	inline void printVector(const sf::Vector2<V> &v, const char *msg = "")
	{
		printf("%s%f, %f\n", msg, v.x, v.y);
	}
}

namespace sf
{
	// comparisons of rectangles by position only
	template<class T>
	inline bool operator<(const Rect<T> &a, const Rect<T> &b)
	{
		return a.left < b.left || (!(b.left < a.left) && a.top < b.top);
	}

	template<class T>

	inline bool operator>(const Rect<T> &lhs, const Rect<T> &rhs)
	{
		return operator<(rhs, lhs);
	}

	template<class T>

	inline bool operator<=(const Rect<T> &lhs, const Rect<T> &rhs)
	{
		return !operator>(lhs, rhs);
	}

	template<class T>
	inline bool operator>=(const Rect<T> &lhs, const Rect<T> &rhs)
	{
		return !operator<(lhs, rhs);
	}

	// points
	template<class T>
	inline bool operator<(const Vector2<T> &a, const Vector2<T> &b)
	{
		return a.x < b.x || (!(b.x < a.x) && a.y < b.y);
	}

	template<class T>

	inline bool operator>(const Vector2<T> &lhs, const Vector2<T> &rhs)
	{
		return operator<(rhs, lhs);
	}

	template<class T>

	inline bool operator<=(const Vector2<T> &lhs, const Vector2<T> &rhs)
	{
		return !operator>(lhs, rhs);
	}

	template<class T>
	inline bool operator>=(const Vector2<T> &lhs, const Vector2<T> &rhs)
	{
		return !operator<(lhs, rhs);
	}
}
#endif