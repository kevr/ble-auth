#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <string>
#include <functional>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace net
{
	class endpoint;

	void default_on_read(boost::shared_ptr<endpoint>, std::string);
	void default_on_close(boost::shared_ptr<endpoint>);

	class endpoint : public boost::enable_shared_from_this<endpoint>
	{
	public:
		endpoint(boost::asio::io_service&);
		void on_read(std::function<void(boost::shared_ptr<endpoint>, std::string)>);
		void on_close(std::function<void(boost::shared_ptr<endpoint>)>);
		void send(std::string data);
		void run();
		boost::asio::ip::tcp::socket& socket();

	private: /* Handler methods */
		void async_on_read(const boost::system::error_code&);
		void async_on_write(const boost::system::error_code&, std::size_t);

	private:
		boost::asio::ip::tcp::socket sock;
		boost::asio::streambuf input_stream;

		std::function<void(boost::shared_ptr<endpoint>, std::string)>
			on_read_fn = default_on_read;

		std::function<void(boost::shared_ptr<endpoint>)>
			on_close_fn = default_on_close;
	};
};

#endif
