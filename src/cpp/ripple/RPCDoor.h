#ifndef RIPPLE_RPCDOOR_H
#define RIPPLE_RPCDOOR_H

#include "RPCServer.h"

/*
Handles incoming connections from people making RPC Requests
*/

class RPCDoor
{
public:
	explicit RPCDoor (boost::asio::io_service& io_service);
	~RPCDoor ();

private:
	boost::asio::ip::tcp::acceptor	mAcceptor;
	boost::asio::deadline_timer		mDelayTimer;

	void startListening();
	void handleConnect(RPCServer::pointer new_connection,
		const boost::system::error_code& error);

	bool isClientAllowed(const std::string& ip);
};

#endif