#include <boost/filesystem.hpp>
#include "utils.hpp"


using namespace boost;

sf::Color Utils::darken(const sf::Color &color, int delta)
{
	auto r = color.r < delta ? 0 : color.r - delta;
	auto g = color.g < delta ? 0 : color.g - delta;
	auto b = color.b < delta ? 0 : color.b - delta;
	return sf::Color(r, g, b);
}

std::string Utils::searchForFile(const std::string &filename, const std::string &directory)
{
	filesystem::recursive_directory_iterator itr(filesystem::absolute(directory));
	filesystem::recursive_directory_iterator end;

	for (; itr != end; ++itr)
	{
		auto path = itr->path();
		if (path.filename() == filename)
			return path.string();
	}

	throw std::runtime_error(std::string("File not found: ") + filename);
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
		currentEnd = randomRange(minDuration, maxDuration);
	else
		currentEnd = maxDuration;

	current = 0;
}

