#pragma once
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>
#include <boost/variant.hpp>

#define FAIL_GET(key) FAIL("Invalid config key: %1%", key)

enum ValueType
{
	SCALAR,
	FLAT_LIST,
	MAP_LIST
};

struct ValueStruct
{
	std::string value;
	ValueType type;
};

class ConfigurationFile
{
public:
	ConfigurationFile()
	{
	}

	// no search will be attempted
	explicit ConfigurationFile(const std::string &fileName) : configPath(fileName)
	{
	}

	void load();

	void getInt(const std::string &key, int &i);
	void getFloat(const std::string &key, float &f);
	void getBool(const std::string &key, bool &b);
	void getString(const std::string &key, std::string &s);
	void getList(const std::string &key, std::vector<std::string> &l);
	void getIntList(const std::string &key, std::vector<int> &l);
	void getMapList(const std::string &key, std::vector<std::map<std::string, std::string>> &ml);

	static int stringToInt(const std::string &s);
	static bool stringToBool(const std::string &s);

protected:
	std::unordered_map<std::string, ValueStruct> configMap;
	boost::filesystem::path configPath;

	friend class Config;

private:
	// path: variable name
	//	std::vector<std::pair<std::string, std::string>> variablesToProcess;

	template <class T> T getNumber(const std::string& key);
	ValueStruct& getValueStruct(const std::string &key, ValueType type);
	void parseConfig(std::map<std::string, std::string> &config);

	void loadNode(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config);
	void loadScalar(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config);
	void loadMap(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config);
	void loadSequence(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config);
	void loadOther(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config);
};

class Config
{
public:
	static void loadConfig();

	static Config& getInstance()
	{
		static Config instance;
		return instance;
	}

	static void getInt(const std::string &key, int &i);
	static void getFloat(const std::string &key, float &f);
	static void getBool(const std::string &key, bool &b);
	static void getString(const std::string &key, std::string &s);
	static void getList(const std::string &key, std::vector<std::string> &l);
	static void getIntList(const std::string &key, std::vector<int> &l);
	static void getMapList(const std::string &key, std::vector<std::map<std::string, std::string>> &ml);



private:
	ConfigurationFile config;

	void ensureConfigExists();
	void createDefaultConfig();
};
