/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ASYNCACCEPT_H_
#define __ASYNCACCEPT_H_

// #include "IoContext.h"
// #include "IpAddress.h"
// #include "Log.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <atomic>
#include <functional>
#include <iostream>

using boost::asio::ip::tcp;

#define TRINITY_MAX_LISTEN_CONNECTIONS boost::asio::socket_base::max_connections

class AsyncAcceptor
{
public:
    typedef void(*AcceptCallback)(tcp::socket&& newSocket, uint32_t threadIndex);

    AsyncAcceptor(boost::asio::io_context& ioContext, std::string const& bindIp, uint16_t port) :
        _acceptor(ioContext), _endpoint(boost::asio::ip::address::from_string(bindIp), port),
        _socket(ioContext), _closed(false), _socketFactory(std::bind(&AsyncAcceptor::DefeaultSocketFactory, this))
    {
    }

    template<class T>
    void AsyncAccept();

    template<AcceptCallback acceptCallback>
    void AsyncAcceptWithCallback()
    {
        tcp::socket* socket;
        uint32_t threadIndex;
        std::tie(socket, threadIndex) = _socketFactory();
        _acceptor.async_accept(*socket, [this, socket, threadIndex](boost::system::error_code error)
            {
                if (!error)
                {
                    try
                    {
                        socket->non_blocking(true);

                        acceptCallback(std::move(*socket), threadIndex);
                    }
                    catch (boost::system::system_error const& err)
                    {
                        std::cout << "Failed to initialize client's socket" << std::endl;
                    }
                }

                if (!_closed)
                    this->AsyncAcceptWithCallback<acceptCallback>();
            });
    }

    bool Bind()
    {
        boost::system::error_code errorCode;
        _acceptor.open(_endpoint.protocol(), errorCode);
        if (errorCode)
        {
            std::cout << "Failed to open acceptor" << std::endl;
            return false;
        }

#if defined (_WIN64) || defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#else
        _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), errorCode);
        if (errorCode)
        {
            std::cout << "Failed to set reuse_address option on acceptor" << std::endl;
            return false;
        }
#endif

        _acceptor.bind(_endpoint, errorCode);
        if (errorCode)
        {
            std::cout << "Could not bind" << std::endl;
            return false;
        }

        _acceptor.listen(TRINITY_MAX_LISTEN_CONNECTIONS, errorCode);
        if (errorCode)
        {
            std::cout << "Failed to start listening" << std::endl;
            return false;
        }

        return true;
    }

    void Close()
    {
        if (_closed.exchange(true))
            return;

        boost::system::error_code err;
        _acceptor.close(err);
    }

    void SetSocketFactory(std::function<std::pair<tcp::socket*, uint32_t>()> func) { _socketFactory = func; }

private:
    std::pair<tcp::socket*, uint32_t> DefeaultSocketFactory() { return std::make_pair(&_socket, 0); }

    tcp::acceptor _acceptor;
    tcp::endpoint _endpoint;
    tcp::socket _socket;
    std::atomic<bool> _closed;
    std::function<std::pair<tcp::socket*, uint32_t>()> _socketFactory;
};

template<class T>
void AsyncAcceptor::AsyncAccept()
{
    _acceptor.async_accept(_socket, [this](boost::system::error_code error)
        {
            if (!error)
            {
                try
                {
                    // this-> is required here to fix an segmentation fault in gcc 4.7.2 - reason is lambdas in a templated class
                    std::make_shared<T>(std::move(this->_socket))->Start();
                }
                catch (boost::system::system_error const& err)
                {
                    std::cout << "Failed to retrieve client's remote address " << err.what() << std::endl;
                }
            }

            // lets slap some more this-> on this so we can fix this bug with gcc 4.7.2 throwing internals in yo face
            if (!_closed)
                this->AsyncAccept<T>();
        });
}

#endif /* __ASYNCACCEPT_H_ */
