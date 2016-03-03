#include "gtest/gtest.h"
#include "service/locator.hpp"
#include "test_helpers.hpp"

class TestingEnvironment : public ::testing::Environment
{

public:
	virtual void SetUp() override
	{
		Locator::provide(SERVICE_LOGGING, new LoggingService(std::cout, LOG_DEBUGGIEST));
		Locator::provide(SERVICE_CONFIG, new ConfigService(DATA_ROOT, "test_reference_config.json", "test_config.json"));
		Locator::provide(SERVICE_EVENT, new EventService);
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

