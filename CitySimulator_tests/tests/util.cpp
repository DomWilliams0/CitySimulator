#include "gtest/gtest.h"
#include "utils.hpp"

TEST(UtilTests, ToPixelToTile)
{
	float x = 5.f;
	float y = -10.f;
	sf::Vector2f v(x,y);

	EXPECT_EQ(Utils::toPixel(v), sf::Vector2f(x*Constants::tileSizef, y*Constants::tileSizef));
	EXPECT_EQ(Utils::toPixel(Utils::toTile(v)), v);
}