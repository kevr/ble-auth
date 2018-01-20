#include "endpoint.h"
using namespace net;

void net::default_on_read(boost::shared_ptr<endpoint> ep, std::string str)
{
	std::cout << "default_on_read: " << str << std::endl;
}

void net::default_on_close(boost::shared_ptr<endpoint> ep)
{
	std::cout << "socket closed" << std::endl;
}

endpoint::endpoint(boost::asio::io_service& io)
	: sock(io)
{ /* io_service initialization */ }

void endpoint::on_read(std::function<void(boost::shared_ptr<endpoint>, std::string)> f)
{
	on_read_fn = f;
}

void endpoint::on_close(std::function<void(boost::shared_ptr<endpoint>)> f)
{
	on_close_fn = f;
}

void endpoint::send(std::string data)
{
	data.append("\r\n");
	boost::asio::async_write(sock, boost::asio::buffer(data),
		boost::bind(&endpoint::async_on_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void endpoint::run()
{
	boost::asio::async_read_until(sock, input_stream, "\r\n",
		boost::bind(&endpoint::async_on_read, shared_from_this(),
			boost::asio::placeholders::error));
}

boost::asio::ip::tcp::socket& endpoint::socket()
{
	return sock;
}

void endpoint::async_on_read(const boost::system::error_code& ec)
{
	if(!ec) {
		std::istream is(&input_stream);
		std::string str;
		std::getline(is, str);
		if(str.back() == '\r')
			str.pop_back();
		on_read_fn(shared_from_this(), std::move(str));
		boost::asio::async_read_until(sock, input_stream, "\r\n",
			boost::bind(&endpoint::async_on_read, shared_from_this(),
				boost::asio::placeholders::error));
	} else if(ec == boost::asio::error::eof) {
		on_close_fn(shared_from_this());
		std::cout << "closing endpoint connection" << std::endl;
	} else {
		std::cerr << "error reading data from endpoint" << std::endl;
		std::cout << "closing endpoint connection" << std::endl;
		on_close_fn(shared_from_this());
	}
}

void endpoint::async_on_write(const boost::system::error_code& ec,
	std::size_t bytes)
{
	if(!ec) {
		std::cout << "wrote " << bytes << " bytes" << std::endl;
	} else {
		std::cerr << "error writing to endpoint" << std::endl;
	}
}

