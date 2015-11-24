#pragma once
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>
#include <boost/property_tree/ptree.hpp>

#define FAIL_GET(path) FAIL("Invalid config path: %1%", path)

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

	void getIntRef(const std::string &path, int &i);
	int getInt(const std::string &path);
	void getFloatRef(const std::string &path, float &f);
	float getFloat(const std::string &path);
	void getBoolRef(const std::string &path, bool &b);
	bool getBool(const std::string &path);
	void getStringRef(const std::string &path, std::string &s);
	std::string getString(const std::string &path);

	template <class T>
	void getList(const std::string &path, std::vector<T> &l)
	{
		for (auto &item : propertyTree.get_child(path))
			l.push_back(item.second.get_value<T>());
	}

	template <class T>
	void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml)
	{
		for (auto &item : propertyTree.get_child(path))
		{
			std::map<std::string, T> map;
			for (auto &element : item.second)
				map.insert({element.first, element.second.get_value<T>()});

			ml.push_back(map);
		}
	}

	static int stringToInt(const std::string &s);
	static bool stringToBool(const std::string &s);

protected:
	std::unordered_map<std::string, ValueStruct> configMap;
	boost::filesystem::path configPath;

	friend class Config;

private:
	boost::property_tree::ptree propertyTree;

	template <class T>
	T getNumber(const std::string &path);
	ValueStruct& getValueStruct(const std::string &path, ValueType type);
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

	static int getInt(const std::string &path);
	static float getFloat(const std::string &path);
	static bool getBool(const std::string &path);
	static std::string getString(const std::string &path);

	template <class T>
	static void getList(const std::string &path, std::vector<int> &l)
	{
		getInstance().config.getList<T>(path, l);
	}

	template <class T=std::string>
	static void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml)
	{
		getInstance().config.getMapList<T>(path, ml);
	}


private:
	ConfigurationFile config;

	void ensureConfigExists();
	void createDefaultConfig();
};
