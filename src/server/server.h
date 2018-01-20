#ifndef SERVER_H
#define SERVER_H

#include "endpoint.h"
#include <vector>
#include <mutex>

namespace net
{
	class server
	{
	public:
		server(boost::asio::io_service& io, unsigned short port);

		boost::shared_ptr<endpoint> make_endpoint()
		{
			boost::shared_ptr<endpoint> ep(new endpoint(io));

			ep->on_read([&](boost::shared_ptr<endpoint> ep, std::string str) {
				std::cout << "got message: " << str << std::endl;
			});

			ep->on_close([&](boost::shared_ptr<endpoint> ep) {
				remove_endpoint(ep);
			});

			return ep;
		}

		void run()
		{
			auto ep = make_endpoint();
			acceptor.async_accept(ep->socket(),
				boost::bind(&server::async_on_accept, this,
					boost::asio::placeholders::error,
					ep));
		}

		std::vector<boost::shared_ptr<endpoint>> endpoints()
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			return m_endpoints;
		}

	private:

		void add_endpoint(boost::shared_ptr<endpoint> ep)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_endpoints.push_back(ep);
		}

		void remove_endpoint(boost::shared_ptr<endpoint> ep)
		{
			// Erase endpoint by pointer value
			std::lock_guard<std::mutex> guard(m_mutex);
			for(std::size_t i = 0; i < m_endpoints.size(); ++i) {
				if(m_endpoints[i] == ep) {
					m_endpoints.erase(m_endpoints.begin() + i);
					break;
				}
			}
		}

		void async_on_accept(const boost::system::error_code& ec,
			boost::shared_ptr<endpoint> ep)
		{
			if(!ec) {
				std::cout << "adding endpoint" << std::endl;
				add_endpoint(ep);
				ep->run();
			} else {
				std::cout << "error accepting endpoint" << std::endl;
			}

			auto new_ep = make_endpoint();
			acceptor.async_accept(new_ep->socket(),
				boost::bind(&server::async_on_accept, this,
					boost::asio::placeholders::error,
					new_ep));
		}

	private:
		boost::asio::io_service& io;
		boost::asio::ip::tcp::acceptor acceptor;
		std::vector<boost::shared_ptr<endpoint>> m_endpoints;
		std::mutex m_mutex;
	};
};

#endif
