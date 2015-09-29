#include <regex>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include "config.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "constants.hpp"


void ConfigurationFile::load()
{
	// doesn't exist
	if (configPath.empty() || !exists(configPath))
		throw Utils::filenotfound_exception(FORMAT("Config file not found: %1%", (configPath.empty() ? "none given" : configPath.string())));


	// load all nodes
	YAML::Node root = YAML::LoadFile(configPath.string());
	std::map<std::string, std::string> config;
	loadNode(root, "", config);

	/*
	// replace all variables
	static std::regex varRegex("^=([a-zA-Z-_ ]+)=$");
	std::smatch match;
	for (auto &pair : config)
	{
		// variable value
		if (regex_search(pair.second, match, varRegex))
		{
			std::string var(match[1]);

			// find value to substitute
			auto subIt(config.find(var));
			if (subIt == config.end())
			FAIL("Variable not found: %1%", var);

			// replace
			pair.second = subIt->second;
		}
	}

	// remove temporary nodes
	for (auto it = config.begin(); it != config.end();)
	{
		if (boost::starts_with(it->first, "temp"))
			it = config.erase(it);
		else
			++it;
	}

	*/

	// store variables in config map
	parseConfig(config);

	// debug print
	// for (auto &pair : configMap)
	//	 std::cout << pair.first << " === " << pair.second.value << std::endl;
}

ValueStruct& ConfigurationFile::getValueStruct(const std::string &key, ValueType type)
{
	auto found(configMap.find(key));

	// invalid key
	if (found == configMap.end())
	FAIL_GET(key);

	ValueStruct &value = found->second;

	// invalid type
	if (value.type != type)
	FAIL2("Wrong valuetype given for %1% (%2%)", key, value.type);

	return value;
}

void splitString(const std::string &s, const char *delimiter, std::vector<std::string> &ret)
{
	boost::char_separator<char> sep(delimiter);
	boost::tokenizer<boost::char_separator<char>> tokens(s, sep);
	for (auto &t : tokens)
		ret.push_back(t);
}

void ConfigurationFile::getInt(const std::string &key, int &i)
{
	static std::regex noCharRegex(".*[a-zA-Z ].*");

	ValueStruct &value = getValueStruct(key, ValueType::SCALAR);

	if (!std::regex_match(value.value, noCharRegex))
	{
		std::stringstream ss(value.value);
		int testInt;

		if (ss >> testInt)
		{
			i = boost::lexical_cast<int>(value.value);
			return;
		}
	}

	FAIL_GET(key);
}


void ConfigurationFile::getBool(const std::string &key, bool &b)
{
	ValueStruct &value = getValueStruct(key, ValueType::SCALAR);
	b = boost::to_lower_copy(value.value) == "true";
}

void ConfigurationFile::getString(const std::string &key, std::string &s)
{
	ValueStruct &value = getValueStruct(key, ValueType::SCALAR);
	s = value.value;
}

void ConfigurationFile::getList(const std::string &key, std::vector<std::string> &l)
{
	ValueStruct &value = getValueStruct(key, ValueType::FLAT_LIST);
	splitString(boost::get<std::string>(value.value), "|", l);
}

void ConfigurationFile::getIntList(const std::string &key, std::vector<int> &l)
{
	std::vector<std::string> strings;
	getList(key, strings);

	for (auto &s : strings)
		l.push_back(stringToInt(s));
}

void ConfigurationFile::getMapList(const std::string &key, std::vector<std::map<std::string, std::string>> &ml)
{
	ValueStruct &value = getValueStruct(key, ValueType::MAP_LIST);

	// split into key=value;key=value;...
	std::vector<std::string> pairs;
	splitString(value.value, "|", pairs);

	for (std::string &pair : pairs)
	{
		// split into key=value;
		std::vector<std::string> kvPairs;
		splitString(pair, ";", kvPairs);

		std::map<std::string, std::string> map;

		for (std::string &kvPair : kvPairs)
		{
			auto equals(kvPair.find_first_of("="));

			// not found
			if (equals == std::string::npos)
			FAIL("Invalid key=value format: %1%", kvPair);

			std::string key(kvPair.substr(0, equals));
			std::string val(kvPair.substr(equals + 1, kvPair.size() - 1));

			map.insert({key, val});
		}

		ml.push_back(map);
	}
}


int ConfigurationFile::stringToInt(const std::string &s)
{
	static std::regex noCharRegex(".*[a-zA-Z ].*");
	if (!regex_match(s, noCharRegex))
	{
		std::stringstream ss(s);
		int testInt;

		if (ss >> testInt)
			return boost::lexical_cast<int>(s);
	}

	FAIL("Could not convert '%1%' to int", s);
}

bool ConfigurationFile::stringToBool(const std::string &s)
{
	return boost::to_lower_copy(s) == "true";
}

void ConfigurationFile::parseConfig(std::map<std::string, std::string> &config)
{
	static std::regex listRegex("^(.+)-(#{1,2})(\\d+)-?(.+)?");
	static std::smatch match;

	struct ListData
	{
		int index;

		int nonSimpleCurrentCount;
		std::vector<int> nonSimpleCounts;

		std::string key;
		std::list<std::string> list;
		std::string next;

		bool simple;

		ListData()
		{
			reset();
		}

		void reset()
		{
			index = 0;
			list.clear();
			simple = false;
			nonSimpleCurrentCount = 0;
			nonSimpleCounts.clear();
			next.clear();
			key.clear();
		}

		void storeCount(bool plusOne = true)
		{
			nonSimpleCounts.push_back(plusOne ? nonSimpleCurrentCount : nonSimpleCurrentCount - 1);
			nonSimpleCurrentCount = 1;
		}
	};

	struct ListData currentList;
	bool endList(false);

	int configIndex(0);
	const int origSize(config.size());
	for (auto it(config.begin()); it != config.end();)
	{
		auto pair = *it;
		endList = configIndex++ == origSize - 1;

		// lists
		if (std::regex_search(pair.first, match, listRegex))
		{
			it = config.erase(it);

			std::string listPath(match[1]);
			bool simple = match[2] == "#";
			int index(std::stoi(match[3]));

			int currentIndex(currentList.index);

			// index decremented: new list
			if (index < currentIndex)
			{
				endList = true;
				currentList.next = pair.second;

				if (!currentList.simple)
					currentList.storeCount();
			}

			// index incremented: same list
			else if (index >= currentIndex)
			{
				// key=value;...
				if (!simple)
				{
					currentList.list.push_back(match[4].str() + "=" + pair.second + ";");

					++currentList.nonSimpleCurrentCount;

					// next element
					if (index != currentIndex)
					{
						// remember count of non simple lists
						currentList.storeCount(false);
					}
				}
				else
					currentList.list.push_back(pair.second);
			}

			if (currentList.next.empty())
			{
				currentList.index = index;
				currentList.simple = simple;

				if (currentList.key.empty())
					currentList.key = listPath;
			}
		}

		// not a list
		else
		{
			++it;

			// store scalar value
			ValueStruct val;
			val.type = SCALAR;
			val.value = pair.second;
			configMap.insert({pair.first, val});

			// ending current list
			if (!currentList.list.empty())
				endList = true;
		}

		// list ended
		if (endList)
		{
			ValueStruct val;
			std::stringstream output;

			// create flat list
			if (currentList.simple)
			{
				val.type = FLAT_LIST;

				for (auto &x : currentList.list)
					output << x << "|";
			}
			else
			{
				val.type = MAP_LIST;

				std::stringstream current;
				currentList.storeCount();

				// append one more to wrap around the last element
				currentList.list.push_back("");

				// division counter
				auto division(currentList.nonSimpleCounts.begin());

				int i(0);
				for (auto &s : currentList.list)
				{
					// division reached
					if (i == *division)
					{
						if (division != currentList.nonSimpleCounts.end())
							++division;

						i = -1;

						std::string element(current.str());
						output << element.substr(0, element.size() - 1) << "|";
						current.str("");
					}

					// append to current
					else
						current << s;

					++i;
				}
			}

			// store
			std::string full(output.str());
			val.value = full.substr(0, full.size() - 1);
			configMap.insert({currentList.key, val});

			// reset and possibly add first item of next list
			std::string next = currentList.next;
			currentList.reset();

			if (!next.empty())
			{
				currentList.list.push_back(next);
				if (!currentList.simple)
					++currentList.nonSimpleCurrentCount;
			}
		}
	}
}

void ConfigurationFile::loadNode(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	switch (node.Type())
	{
	case YAML::NodeType::Map:
		loadMap(node, prefix, config);
		break;
	case YAML::NodeType::Scalar:
		loadScalar(node, prefix, config);
		break;
	case YAML::NodeType::Sequence:
		loadSequence(node, prefix, config);
		break;
	default:
		loadOther(node, prefix, config);
		break;
	}
}

void ConfigurationFile::loadScalar(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	static std::regex reservedChars(".*[|;].*");

	auto realPrefix(prefix.substr(0, prefix.size() - 1));
	auto value(node.as<std::string>());

	// reserved symbols
	if (std::regex_match(value, reservedChars) || std::regex_match(realPrefix, reservedChars))
	FAIL2("Reserved characters used in key %1%: %2%", realPrefix, value);

	config.insert({realPrefix, value});
}

void ConfigurationFile::loadMap(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	for (auto &pair : node)
		loadNode(pair.second, prefix + pair.first.as<std::string>() + "-", config);
}

void ConfigurationFile::loadSequence(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	int i(0);
	std::string listChar = node.begin()->IsScalar() ? "#" : "##";

	for (auto &value : node)
		loadNode(value, prefix + listChar + std::to_string(i++) + "-", config);
}

void ConfigurationFile::loadOther(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	FAIL("Unhandled config node: %1%", prefix);
}

void Config::loadConfig()
{
	getInstance().config.configPath = boost::filesystem::absolute("res/config/config.yml");
	getInstance().ensureConfigExists();
	getInstance().config.load();
}


void Config::ensureConfigExists()
{
	// create default config
	if (!exists(getInstance().config.configPath))
	{
		Logger::logInfo("Config not found, loading default");
		createDefaultConfig();
	}
	else
		Logger::logInfo("Found config");
}

void Config::createDefaultConfig()
{
	using namespace YAML;
	Emitter out;

	// display
	out << BeginMap << Key << "display";
	out << Value << BeginMap;
	out << Key << "borderless-fullscreen" << Value << false;

	// resolution
	out << Key << "resolution";
	out << Value << BeginMap;
	out << Key << "width" << Value << 1080;
	out << Key << "height" << Value << 768;
	out << EndMap;

	// end display
	out << EndMap;

	// debug
	out << Key << "debug";
	out << Value << BeginMap;
	out << Key << "world-name" << Value << "small.tmx";
	out << Key << "limit-fps" << Value << false;
	out << EndMap;

	// end debug
	out << EndMap;

	// end config
	out << EndMap;


	// write to config file
	std::ofstream configStream;
	configStream.open(config.configPath.string());
	configStream << out.c_str();
	configStream.close();
}

void Config::getInt(const std::string &key, int &i)
{
	getInstance().config.getInt(key, i);
}

void Config::getBool(const std::string &key, bool &b)
{
	getInstance().config.getBool(key, b);
}

void Config::getString(const std::string &key, std::string &s)
{
	getInstance().config.getString(key, s);
}

void Config::getList(const std::string &key, std::vector<std::string> &l)
{
	getInstance().config.getList(key, l);
}

void Config::getIntList(const std::string &key, std::vector<int> &l)
{
	getInstance().config.getIntList(key, l);
}

void Config::getMapList(const std::string &key, std::vector<std::map<std::string, std::string>> &ml)
{
	getInstance().config.getMapList(key, ml);
}
