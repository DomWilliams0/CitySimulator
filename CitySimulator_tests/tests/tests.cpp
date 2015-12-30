#include "gtest/gtest.h"
#include "services.hpp"

class TestingEnvironment : public ::testing::Environment
{

public:
	virtual void SetUp() override
	{
		// todo null logger
		Locator::provide(SERVICE_LOGGING, new LoggingService(std::cout, LOG_DEBUG));
	}

	virtual void TearDown() override
	{
	}
};

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);

	::testing::AddGlobalTestEnvironment(new TestingEnvironment);

	return RUN_ALL_TESTS();
}

