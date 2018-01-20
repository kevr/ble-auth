#ifndef SCANNER_H
#define SCANNER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

using boost::asio::ip::tcp;
using boost::lambda::bind;
using boost::lambda::var;
using boost::lambda::_1;

namespace net
{

	class scanner
	{
	public:
		scanner(boost::asio::io_service& io)
			: io(io), resolver(io), sock(io), deadline(io)
		{
			deadline.expires_at(boost::posix_time::pos_infin);
			check_deadline();
		}

		bool scan(std::string network, unsigned short block, unsigned short port, std::string& out_host)
		{
			std::vector<unsigned char> byte(4);

			auto pos = network.find(".");
			byte[0] = (unsigned char)std::stoi(network.substr(0, pos));
			pos += 1;
			auto old = pos;
			pos = network.find(".", old);
			byte[1] = (unsigned char)std::stoi(network.substr(old, pos - old));
			pos += 1;
			old = pos;
			pos = network.find(".", old);
			byte[2] = (unsigned char)std::stoi(network.substr(old, pos - old));
			pos += 1;
			old = pos;
			byte[3] = (unsigned char)std::stoi(network.substr(old, network.size() - old));

			std::cout << (unsigned int)byte[0] << "." << (unsigned int)byte[1] << "." << (unsigned int)byte[2] << "." << (unsigned int)byte[3] << std::endl;

			auto bits = 32 - block;
			boost::posix_time::time_duration duration = boost::posix_time::millisec(250);

			std::cout << "scanning\n";
			for(int i = int(block / 8); i < 4; ++i) {
				std::cout << "processing section " << i << std::endl;
				auto& section = byte[i];
				section = ((bits % 8) * 32) + 1; // Set start address
				std::cout << (unsigned int)(section) << std::endl;
				while((unsigned int)section < (unsigned int)256) {
					std::string host = std::to_string(int(byte[0])) + "." +
						std::to_string(int(byte[1])) + "." + std::to_string(int(byte[2])) +
						"." + std::to_string(int(byte[3]));

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

					if((unsigned int)section == (unsigned int)255)
						break;
					++section;
				}
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
		boost::asio::io_service& io;
		boost::asio::ip::tcp::resolver resolver;
		boost::asio::ip::tcp::socket sock;
		boost::asio::deadline_timer deadline;
	};
};

#endif
