#include "test_helpers.hpp"
#include "services.hpp"

struct ServicesTest : public ::testing::Test
{
	virtual void SetUp() override
	{
		Locator::provide(SERVICE_INPUT, new InputService);
	}

	virtual void TearDown() override
	{

	}
};

TEST_F(ServicesTest, Provide)
{
	InputService *old = Locator::locate<InputService>();
	EXPECT_NE(old, nullptr);

	InputService *newService = new InputService;
	EXPECT_NE(newService, old);

	Locator::provide(SERVICE_INPUT, newService);
	EXPECT_EQ(newService, Locator::locate<InputService>());
	EXPECT_DEATH(delete old, ".*");
}

TEST_F(ServicesTest, InputService)
{
	auto input = Locator::locate<InputService>();
	ASSERT_NE(input, nullptr);

	auto binding = InputKey::KEY_DOWN;

	// nothing pressed
	EXPECT_FALSE(input->isFirstPressed(binding));
	EXPECT_FALSE(input->isPressed(binding));

	auto key = input->getKey(binding);

	// first press
	input->update(key, true);
	EXPECT_TRUE(input->isFirstPressed(binding));
	EXPECT_TRUE(input->isPressed(binding));
	input->advance();

	// held down
	input->update(key, true);
	EXPECT_FALSE(input->isFirstPressed(binding));
	EXPECT_TRUE(input->isPressed(binding));
	input->advance();

	// let go
	input->update(key, false);
	EXPECT_FALSE(input->isFirstPressed(binding));
	EXPECT_FALSE(input->isPressed(binding));
	input->advance();
}

TEST_F(ServicesTest, KeyBindings)
{
	auto input = Locator::locate<InputService>();
	ASSERT_NE(input, nullptr);

	input->bindKey(KEY_UP, sf::Keyboard::G);
	EXPECT_EQ(input->getBinding(sf::Keyboard::G), KEY_UP);
	EXPECT_EQ(input->getKey(KEY_UP), sf::Keyboard::G);
}
