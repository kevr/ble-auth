#ifndef SCANNER_H
#define SCANNER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <byteswap.h>

using boost::asio::ip::tcp;
using boost::lambda::bind;
using boost::lambda::var;
using boost::lambda::_1;

namespace net
{

	class scanner
	{
	public:
		scanner()
			: resolver(io), sock(io), deadline(io)
		{
			deadline.expires_at(boost::posix_time::pos_infin);
			check_deadline();
		}

		bool scan(std::string network, unsigned short block, unsigned short port, std::string& out_host)
		{
			unsigned char bytes[4];

			auto pos = network.find(".");
			bytes[0] = (unsigned char)std::stoi(network.substr(0, pos));
			pos += 1;
			auto old = pos;
			pos = network.find(".", old);
			bytes[1] = (unsigned char)std::stoi(network.substr(old, pos - old));
			pos += 1;
			old = pos;
			pos = network.find(".", old);
			bytes[2] = (unsigned char)std::stoi(network.substr(old, pos - old));
			pos += 1;
			old = pos;
			bytes[3] = (unsigned char)std::stoi(network.substr(old, network.size() - old));

			boost::posix_time::time_duration duration = boost::posix_time::millisec(500);

			unsigned int *ip = (unsigned int *)&bytes[0];
			unsigned int big_ip = __bswap_32(*ip);
			auto bits = 32 - block;
			auto end = bits * 32;

			std::cout << (unsigned int)bytes[0] << "." << (unsigned int)bytes[1] << "." << (unsigned int)bytes[2] << "." << (unsigned int)bytes[3] << std::endl;

			// While the IP is not a full 32 bits of 1s
			while(bytes[3] <= end) {
				std::string host = std::to_string(int(bytes[0])) + "." +
					std::to_string(int(bytes[1])) + "." + std::to_string(int(bytes[2])) +
					"." + std::to_string(int(bytes[3]));
				std::cout << "Trying " << host << std::endl;
				tcp::resolver::query query(host, std::to_string(port));
				boost::system::error_code ec;
				auto it = resolver.resolve(query, ec);

				if(!ec) {
					deadline.expires_from_now(duration);
					ec = boost::asio::error::would_block;
					boost::asio::async_connect(sock, it, var(ec) = boost::lambda::_1);
					do io.run_one(); while(ec == boost::asio::error::would_block);
					if(ec || !sock.is_open()) {
						// do nothing
					} else {
						out_host = host;
						return true;
					}
				}

				*ip = __bswap_32(++big_ip);
			}

			return false;
		}

		void check_deadline()
		{
			if(deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
				boost::system::error_code ignored_ec;
				sock.close(ignored_ec);
				deadline.expires_at(boost::posix_time::pos_infin);
			}
			deadline.async_wait(boost::bind(&scanner::check_deadline, this));
		}

	private:
		boost::asio::io_service io;
		boost::asio::ip::tcp::resolver resolver;
		boost::asio::ip::tcp::socket sock;
		boost::asio::deadline_timer deadline;
	};
};

#endif
