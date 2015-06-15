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

void reverseRange(sf::Uint8 *buffer, int left, int right)
{
	while (left < right)
	{
		sf::Uint8 temp = buffer[left];
		buffer[left++] = buffer[right];
		buffer[right--] = temp;
	}
}

sf::Image Utils::rotateImage(const sf::Image &image, int rotation)
{
	auto size = image.getSize();
	const sf::Uint8 *ptr = image.getPixelsPtr();
	std::vector<sf::Uint8> pixels(ptr, ptr + size.x * size.y * 4);

	// transpose
	for (size_t x = 0; x < size.x; x++)
	{
		for (size_t y = 0; y < size.y; y++)
		{
			auto i = x + (y * size.x);
			auto j = y + (x * size.x);

			pixels[i] = pixels[j];
		}
	}

	// +90 reverse rows
	if (rotation == 90)
	{
		for (size_t y = 0; y < size.y; y++)
		{
			int row = y * size.x;
			reverseRange(&pixels[row], 0, size.x - 1);
		}
	}
	// -90 reverse columns
	else if (rotation == -90)
	{
		for (size_t x = 0; x < size.x; x++)
		{
			int col = x * size.y;
			reverseRange(&pixels[col], 0, size.y - 1);
		}
	}
	else
		throw std::exception("Invalid rotation");


	sf::Image rotated;
	rotated.create(size.x, size.y, &pixels[0]);
	return rotated;
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

