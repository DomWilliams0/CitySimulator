#ifndef CITYSIM_CONFIG_HPP
#define CITYSIM_CONFIG_HPP

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <map>

#define FAIL_GET(path) ERROR("Invalid config path: %1%", path)

class ConfigurationFile
{
public:
	ConfigurationFile() : reloadFromFile(false)
	{
	}

	explicit ConfigurationFile(const std::string &fileName, const std::string &overridingPath = "") :
			configPath(fileName), overwriteConfigPath(overridingPath), reloadFromFile(false)
	{
	}

	void load();

	void loadOnTop();

	void recurseAndOverwrite(boost::property_tree::ptree &tree, std::string prefix);

	void getIntRef(const std::string &path, int &i);

	int getInt(const std::string &path);

	void getFloatRef(const std::string &path, float &f);

	float getFloat(const std::string &path);

	void getBoolRef(const std::string &path, bool &b);

	bool getBool(const std::string &path);

	void getStringRef(const std::string &path, std::string &s);

	std::string getString(const std::string &path);

	template<class T>
	void getList(const std::string &path, std::vector<T> &l)
	{
		for (auto &item : propertyTree.get_child(path))
			l.push_back(item.second.get_value<T>());
	}

	template<class T>
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

	void setReloadFromFile(bool reload);

	void reload();

	void setConfigPath(const std::string &path);

	void setOverridingConfigPath(const std::string &path);

	std::string getConfigPath() const;

	std::string getOverwriteConfigPath() const;

private:
	boost::property_tree::ptree propertyTree;

	bool reloadFromFile;
	std::time_t lastModification;

	boost::filesystem::path configPath;
	boost::filesystem::path overwriteConfigPath;
};

#endif