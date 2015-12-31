#include "test_helpers.hpp"
#include "services.hpp"

struct ConfigTest : ::testing::Test
{
	ConfigTest() : config(std::string(DATA_ROOT) + "/test_reference_config.json", std::string(DATA_ROOT) + "/test_config.json")
	{
	}

	virtual void SetUp() override
	{
		config.load();
	}

	virtual void TearDown() override
	{
	}

	ConfigurationFile config;
};

TEST(ConfigLoading, FileNotFound)
{
	ConfigurationFile noFile("no_existerino.json");
	EXPECT_THROW(noFile.load();, Utils::filenotfound_exception);

	ConfigurationFile noRoot("config.json");
	EXPECT_THROW(noRoot.load(), Utils::filenotfound_exception);
}

TEST_F(ConfigTest, SimpleValueGetting)
{
	int i;
	config.getIntRef("acorn", i);
	EXPECT_EQ(config.getInt("acorn"), 1);
	EXPECT_EQ(i, 1);

	std::string s;
	config.getStringRef("butterfly", s);
	EXPECT_EQ(config.getString("butterfly"), "muffin");
	EXPECT_EQ(s, "muffin");

	bool b;
	config.getBoolRef("car-battery.inner", b);
	EXPECT_EQ(config.getBool("car-battery.inner"), true);
	EXPECT_EQ(b, true);

	float f;
	config.getFloatRef("override-me.pegasus", f);
	EXPECT_EQ(config.getFloat("override-me.pegasus"), f);
	EXPECT_EQ(f, 5.5f);

	EXPECT_EQ(config.getString("car-battery.deepah.cheetah"), "woof");
}

TEST_F(ConfigTest, ComplexValueGetting)
{
	using namespace std;

	vector<int> ints;
	config.getList<int>("eggs.numbahs", ints);
	EXPECT_EQ(ints.size(), 4);
	EXPECT_EQ(ints, vector<int>({1, 2, 3, -60}));

	vector<string> strings;
	config.getList<string>("eggs.strangs", strings);
	EXPECT_EQ(strings.size(), 5);
	EXPECT_EQ(strings, vector<string>({"rudolph", "was", "a", "bad", "boy"}));

	vector<map<string, string>> map;
	config.getMapList("eggs.swimmers", map);
	EXPECT_EQ(map.size(), 2);
	EXPECT_EQ(map[0]["name"], "Boris");
	EXPECT_EQ(map[1]["swimming"], "painful to watch");
}

TEST_F(ConfigTest, Overwriting)
{
	EXPECT_NE(config.getFloat("override-me.pegasus"), 100.f);

	config.loadOnTop();

	EXPECT_EQ(config.getFloat("override-me.pegasus"), 100.f);
	EXPECT_EQ(config.getString("dog"), "cornichon");
}

TEST(ConfigServiceTest, ServiceAccess)
{
	auto service = new ConfigService(DATA_ROOT, "test_reference_config.json");
	Locator::provide(SERVICE_CONFIG, service);

	EXPECT_EQ(service->getString("butterfly"), "muffin");
	EXPECT_EQ(service->getBool("car-battery.inner"), true);
}

TEST(ConfigServiceTest, GlobalConfig)
{
	Locator::provide(SERVICE_CONFIG, new ConfigService(DATA_ROOT, "test_reference_config.json"));

	EXPECT_EQ(Config::getString("butterfly"), "muffin");
	EXPECT_EQ(Config::getBool("car-battery.inner"), true);
}

TEST(ConfigServiceTest, GlobalConfigOverride)
{
	Locator::provide(SERVICE_CONFIG,
	                 new ConfigService(DATA_ROOT, "test_reference_config.json", "test_config.json"));
	EXPECT_EQ(Config::getFloat("override-me.pegasus"), 100.f);
}

