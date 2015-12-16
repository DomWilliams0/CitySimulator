#define _USE_MATH_DEFINES

#include <boost/filesystem.hpp>
#include <boost/format/format_class.hpp>
#include <regex>
#include <boost/lexical_cast.hpp>
#include "utils.hpp"


using namespace boost;

sf::Color Utils::darken(const sf::Color &color, int delta)
{
	auto r = color.r < delta ? 0 : color.r - delta;
	auto g = color.g < delta ? 0 : color.g - delta;
	auto b = color.b < delta ? 0 : color.b - delta;
	return sf::Color(r, g, b);
}


int Utils::stringToInt(const std::string &s)
{
	static std::regex noCharRegex(".*[a-zA-Z ].*");
	if (!regex_match(s, noCharRegex))
	{
		std::stringstream ss(s);
		int testInt;

		if (ss >> testInt)
			return lexical_cast<int>(s);
	}

	// todo
//	ERROR("Could not convert '%1%' to int", s);
	return -1;
}

void Utils::validateDirectory(const std::string &directory)
{
	if (!filesystem::exists(directory))
		return;
	// todo
//		throw filenotfound_exception(str(format("Invalid directory given: %1%") % directory));
}

std::string Utils::searchForFile(const std::string &filename, const std::string &directory)
{
	validateDirectory(directory);

	filesystem::recursive_directory_iterator itr(filesystem::absolute(directory));
	filesystem::recursive_directory_iterator end;

	for (; itr != end; ++itr)
	{
		auto path = itr->path();
		if (path.filename() == filename)
			return path.make_preferred().string();
	}

	// not found
//	throw filenotfound_exception(str(format("File not found: %1%") % filename));
	// todo
	return "";
}

sf::FloatRect Utils::expandRect(const sf::FloatRect &rect, const sf::Vector2f &offset)
{
	sf::FloatRect ret(rect);
	bool horizontal = offset.x >= Math::EPSILON;
	bool negative = (horizontal ? offset.x : offset.y) < 0.f;

	if (horizontal)
	{
		ret.width += offset.x;
		if (negative)
			ret.left -= offset.x;
	}
	else
	{
		ret.height += offset.y;
		if (negative)
			ret.top -= offset.y;
	}

	return ret;
}

void Utils::TimeTicker::init(float min, float max)
{
	current = 0;

	if (max < 0)
	{
		range = false;
		maxDuration = min;
	}
	else
	{
		range = true;
		minDuration = min;
		maxDuration = max;
	}
}

bool Utils::TimeTicker::tick(float delta)
{
	current += delta;

	// finished
	if (current > currentEnd)
	{
		reset();
		return true;
	}

	return false;
}

void Utils::TimeTicker::reset()
{
	if (range)
		currentEnd = random<float>(minDuration, maxDuration);
	else
		currentEnd = maxDuration;

	current = 0;
}

namespace Math
{
	const float EPSILON = 0.000001f;
	const float degToRad = static_cast<float>(M_PI / 180.0);
	const float radToDeg = static_cast<float>(180.0 / M_PI);
}
