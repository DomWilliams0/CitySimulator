#pragma once
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

typedef boost::variant<int, bool, std::string> ConfigValue;
typedef std::unordered_map<std::string, ConfigValue> ConfigMap;

class Config
{
public:


	Config() : configPath(boost::filesystem::absolute("res/config/config.yml"))
	{
	}


	static void loadConfig();

	template <class T>
	static T get(const std::string &key)
	{
		auto value(getInstance().configMap.find(key));

		// not found
		if (value == getInstance().configMap.end())
			throw std::runtime_error(str(boost::format("Invalid config key: %1%") % key));

		return boost::get<T>(value->second);
	}


	static Config &getInstance()
	{
		static Config instance;
		return instance;
	}

	boost::filesystem::path configPath;

private:

	void loadNode(const YAML::Node &node, const std::string &prefix);
	void ensureConfigExists();
	void createDefaultConfig();

	ConfigMap configMap;
};
