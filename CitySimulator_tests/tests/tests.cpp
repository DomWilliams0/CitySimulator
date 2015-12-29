#include "gtest/gtest.h"
#include "logger.hpp"

class TestingEnvironment : public ::testing::Environment
{

public:
	virtual void SetUp() override
	{
		Logger::createLogger(std::cerr, Logger::DEBUG);
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

