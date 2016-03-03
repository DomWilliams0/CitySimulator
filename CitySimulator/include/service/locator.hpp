#ifndef CITYSIM_LOCATOR_HPP
#define CITYSIM_LOCATOR_HPP

#include "base_service.hpp"

class Locator
{
public:

	static void provide(ServiceType type, BaseService *service);

	static std::string serviceToString(ServiceType type);

	template<class T>
	static T *locate(bool errorOnFail = true);

private:
	Locator();

	~Locator();

	static Locator &getInstance()
	{
		static Locator instance;
		return instance;
	}

	std::vector<BaseService *> services;
};


#include "locator.ipp"

#endif
