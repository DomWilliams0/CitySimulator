#include "utils.hpp"
#include "test_helpers.hpp"

TEST(UtilTests, Format)
{
	EXPECT_EQ(format("Single %1% argument", "great"), "Single great argument");
	EXPECT_EQ(format("%1% entire %2%", "Two", "arguments"), "Two entire arguments");
	EXPECT_EQ(format("%1% damn, %2% %3%!", "Hot", "three", "arguments"), "Hot damn, three arguments!");

	EXPECT_EQ(format("This is a %1%, and a %3% %2% one", "test", "good", "very"),
	          "This is a test, and a very good one");
}

TEST(UtilTests, Error)
{
	EXPECT_ERROR_MESSAGE(error("Once upon a %1%, an %3% was %2%", "time", "thrown", "error");,
	                     "Once upon a time, an error was thrown")
}


TEST(UtilTests, ToPixelToTile)
{
	float x = 5.f;
	float y = -10.f;
	sf::Vector2f v(x, y);

	EXPECT_EQ(Utils::toPixel(v), sf::Vector2f(x * Constants::tileSizef, y * Constants::tileSizef));
	EXPECT_EQ(Utils::toPixel(Utils::toTile(v)), v);
}

TEST(UtilTests, StringToInt)
{
	EXPECT_EQ(Utils::stringToInt("5"), 5);
	EXPECT_EQ(Utils::stringToInt("20000"), 20000);
	EXPECT_EQ(Utils::stringToInt("-10"), -10);

	EXPECT_THROW(Utils::stringToInt("potato");, std::runtime_error);
}

TEST(UtilTests, Random)
{
	EXPECT_EQ(Utils::random<int>(0, 1), 0);

	for (int i = 0; i < 500; ++i)
	{
		auto rand = Utils::random<float>(5, 10);
		EXPECT_TRUE(rand >= 5 && rand < 10);
	}
}

TEST(UtilTests, RoundDownToMultiple)
{
	EXPECT_EQ(Utils::roundToMultiple(8., 5), 10);
	EXPECT_EQ(Utils::roundToMultiple(11., 5), 10);
	
	EXPECT_EQ(Utils::roundToMultiple(7.51, 5), 10);
	EXPECT_EQ(Utils::roundToMultiple(7.49, 5), 5);

	EXPECT_EQ(Utils::roundToMultiple(5.1, 10), 10);
}
