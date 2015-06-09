#pragma once
#include <SFML/System.hpp>
#include "game.hpp"
#include "constants.hpp"

namespace Utils
{
	inline static void centre(sf::Text &text, Game *game, int y = -1)
	{
		text.setPosition(Constants::windowSize.x / 2 - text.getLocalBounds().width / 2, y < 0 ? text.getPosition().y : y);
	}

	static sf::Color darken(const sf::Color &color, int delta)
	{
		auto r = color.r < delta ? 0 : color.r - delta;
		auto g = color.g < delta ? 0 : color.g - delta;
		auto b = color.b < delta ? 0 : color.b - delta;
		return sf::Color(r, g, b);
	}

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

	std::string searchForFile(const std::string &filename, const std::string &directory = "res");

	sf::Image rotateImage(const sf::Image &image, int rotation);
}
