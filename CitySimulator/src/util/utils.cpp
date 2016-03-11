#define _USE_MATH_DEFINES

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <regex>
#include <boost/lexical_cast.hpp>
#include "utils.hpp"

std::string format(const std::string &s, const std::string &arg1)
{
	return boost::str(boost::format(s) % arg1);
}

std::string format(const std::string &s, const std::string &arg1, const std::string &arg2)
{
	return boost::str(boost::format(s) % arg1 % arg2);
}

std::string format(const std::string &s, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
	return boost::str(boost::format(s) % arg1 % arg2 % arg3);
}

void error(const std::string &msg)
{
	throw std::runtime_error(msg);
}

void error(const std::string &msg, const std::string &arg1)
{
	throw std::runtime_error(format(msg, arg1));
}

void error(const std::string &msg, const std::string &arg1, const std::string &arg2)
{
	throw std::runtime_error(format(msg, arg1, arg2));
}

void error(const std::string &msg, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
	throw std::runtime_error(format(msg, arg1, arg2, arg3));
}


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
			return boost::lexical_cast<int>(s);
	}

	error("Could not convert '%1%' to int", s);
	return -1;
}

void Utils::validateDirectory(const std::string &directory)
{
	if (!boost::filesystem::exists(directory))
		throw filenotfound_exception(format("Invalid directory given: '%1%'", directory));
}

std::string Utils::searchForFile(const std::string &filename, const std::string &directory)
{
	using namespace boost;

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
	throw filenotfound_exception("File not found: " + filename);
}

std::string Utils::joinPaths(const std::string &root, const std::string &path)
{
	return (boost::filesystem::path(root) / path).string();
}

int Utils::roundToMultiple(double x, int multiple)
{
	return static_cast<int>(multiple * round(x / multiple));
}

void Utils::TimeTicker::setMinAndMax(float min, float max)
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

	reset();
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
	const double EPSILON = 0.000001f;
	const double degToRad = static_cast<float>(M_PI / 180.0);
	const double radToDeg = static_cast<float>(180.0 / M_PI);
}
