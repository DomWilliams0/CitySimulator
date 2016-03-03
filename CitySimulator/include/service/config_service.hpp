#ifndef CITYSIM_CONFIG_SERVICE_HPP
#define CITYSIM_CONFIG_SERVICE_HPP

#include "locator.hpp"
#include "base_service.hpp"
#include <boost/filesystem.hpp>

class ConfigService : public BaseService {
public:
	ConfigService(const std::string &directory,
				  const std::string &appConfigPath, const std::string &userConfigPath = "");

	virtual void onEnable() override;

	int getInt(const std::string &path);

	int getInt(const std::string &path, int defaultValue);

	float getFloat(const std::string &path);

	float getFloat(const std::string &path, float defaultValue);

	bool getBool(const std::string &path);

	bool getBool(const std::string &path, bool defaultValue);

	std::string getString(const std::string &path);

	std::string getString(const std::string &path, const std::string &defaultValue);

	std::string getResource(const std::string &path);

	template<class T>
	void getList(const std::string &path, std::vector<int> &l) {
		config.getList<T>(path, l);
	}

	template<class T=std::string>
	void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml) {
		config.getMapList<T>(path, ml);
	}

	void setRootDirectory(const std::string &directory) {
		rootDirectory = directory;
	}

private:
	ConfigurationFile config;
	boost::filesystem::path rootDirectory;

	void ensureConfigExists();
};
namespace Config {
	int getInt(const std::string &path);

	int getInt(const std::string &path, int defaultValue);

	float getFloat(const std::string &path);

	float getFloat(const std::string &path, float defaultValue);

	bool getBool(const std::string &path);

	bool getBool(const std::string &path, bool defaultValue);

	std::string getString(const std::string &path);

	std::string getString(const std::string &path, const std::string &defaultValue);

	std::string getResource(const std::string &path);

	template<class T>
	void getList(const std::string &path, std::vector<int> &l) {
		Locator::locate<ConfigService>()->getList<T>(path, l);
	}

	template<class T=std::string>
	void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml) {
		Locator::locate<ConfigService>()->getMapList<T>(path, ml);
	}
}
#endif