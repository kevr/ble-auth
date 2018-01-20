#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <string>
#include <stdexcept>

namespace cache
{
	struct last_host
	{
		static bool available()
		{
			std::ifstream ifs("/tmp/.ble-auth.cache", std::ios::in);
			if(!ifs.is_open())
				return false;
			return true;
		}

		static std::string value()
		{
			std::ifstream ifs("/tmp/.ble-auth.cache", std::ios::in);
			if(!ifs.is_open())
				throw std::domain_error("cache file is unavailable");
			std::string host;
			ifs >> host; // Sweet, ignore all whitespace
			ifs.close();
			return host;
		}

		static void set(std::string host)
		{
			std::ofstream ofs("/tmp/.ble-auth.cache", std::ios::out);
			ofs << host;
			ofs.close();
		}
	};
};

#endif
