#include "server.h"
using boost::asio::ip::tcp;
using namespace net;

server::server(boost::asio::io_service& io, unsigned short port)
	: io(io), acceptor(io, tcp::endpoint(tcp::v4(), port))
{ /* Must initialize with io_service */ }


