#include "gtest/gtest.h"
#include "services.hpp"

class TestingEnvironment : public ::testing::Environment
{

public:
	virtual void SetUp() override
	{
		Locator::provide(SERVICE_LOGGING, new NullLoggingService);
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

