#ifndef SOCKET_H
#define SOCKET_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

namespace net
{
	class socket;

	void default_on_read(socket&, std::string);

	class socket
	{
	public:
		socket(boost::asio::io_service& io)
			: sock(io), resolver(io)
		{ }

		void on_read(std::function<void(socket&, std::string)> f)
		{
			on_read_fn = f;
		}

		void run(std::string host, unsigned short port)
		{
			sock.close();
			boost::asio::ip::tcp::resolver::query query(host, std::to_string(port));
			resolver.async_resolve(query,
				boost::bind(&socket::async_on_resolve, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::iterator));
		}

		bool connected() const
		{
			return m_connected;
		}

	private:
		void async_on_resolve(const boost::system::error_code& ec,
			boost::asio::ip::tcp::resolver::iterator iter)
		{
			if(!ec) {
				auto ep = *iter;
				sock.async_connect(ep,
					boost::bind(&socket::async_on_connect, this,
						boost::asio::placeholders::error,
						++iter));
			} else {
				std::cerr << "unable to resolve host" << std::endl;
			}
		}

		void async_on_connect(const boost::system::error_code& ec,
			boost::asio::ip::tcp::resolver::iterator iter)
		{
			if(!ec) {
				std::cout << "connected" << std::endl;
				m_connected = true;
				boost::asio::async_read_until(sock, input_stream, "\r\n",
					boost::bind(&socket::async_on_read, this,
						boost::asio::placeholders::error));
			} else if(iter != tcp::resolver::iterator()) {
				std::cerr << "error: " << ec.message() << ", trying next" << std::endl;
				auto ep = *iter;
				sock.close();
				sock.async_connect(ep,
					boost::bind(&socket::async_on_connect, this,
						boost::asio::placeholders::error,
						++iter));
			} else {
				std::cerr << "error: " << ec.message() << std::endl;
			}
		}

		void async_on_read(const boost::system::error_code& ec)
		{
			if(!ec) {
				std::istream is(&input_stream);
				std::string str;
				std::getline(is, str);
				if(str.back() == '\r')
					str.pop_back();
				on_read_fn(*this, std::move(str));
				boost::asio::async_read_until(sock, input_stream, "\r\n",
					boost::bind(&socket::async_on_read, this,
						boost::asio::placeholders::error));
			} else if(ec == boost::asio::error::eof) {
				std::cout << "socket eof on read, closing" << std::endl;
			} else {
				std::cout << "socket error on read, closing" << std::endl;
			}
		}

		void async_on_write(const boost::system::error_code& ec,
			std::size_t bytes)
		{
			if(!ec) {
			} else {
			}
		}

	private:
		boost::asio::ip::tcp::socket sock;
		boost::asio::ip::tcp::resolver resolver;
		boost::asio::streambuf input_stream;

		std::function<void(socket&, std::string)>
			on_read_fn = default_on_read;

		bool m_connected = false;
	};
};

#endif
