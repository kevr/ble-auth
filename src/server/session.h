#ifndef SESSION_H
#define SESSION_H

#include <bluetooth/bluetooth.h>
#include <ctime>
#include <chrono>

namespace bluetooth
{
	class session
	{
	public:
		session(bdaddr_t peer) : peer(peer)
		{
			tp = std::chrono::steady_clock::now();
		}
	
		std::chrono::time_point<std::chrono::steady_clock> latest()
		{
			return tp;
		}

		std::chrono::time_point<std::chrono::steady_clock> update()
		{
			tp = std::chrono::steady_clock::now();
			return tp;
		}

		// Timeout in seconds
		bool timed_out(float timeout)
		{
			auto now = std::chrono::steady_clock::now() - tp;
			return now.count() >= timeout;
		}

	private:
		bdaddr_t peer;
		std::chrono::time_point<std::chrono::steady_clock> tp;
	};
};

#endif
