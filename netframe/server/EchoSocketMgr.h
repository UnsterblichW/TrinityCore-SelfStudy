
#pragma once

#include "EchoSession.h"
#include "NetworkThread.h"
#include "SocketMgr.h"
#include "AsyncAcceptor.h"
#include <boost/asio/io_context.hpp>
#include <utility>

class EchoSocketMgr : public SocketMgr<EchoSession>
{
    typedef SocketMgr<EchoSession> BaseSocketMgr;
public:
    static EchoSocketMgr & Instance()
    {
        static EchoSocketMgr instance;
        return instance;
    }

    bool StartNetwork(boost::asio::io_context& ioContext, std::string const& bindIp, uint16_t port, int threadCount=1) override
    {
        if (!BaseSocketMgr::StartNetwork(ioContext, bindIp, port, threadCount))
        {
            return false;
        }
        _acceptor->AsyncAcceptWithCallback<&EchoSocketMgr::OnSocketAccept>();
        return true;
    }

protected:
    NetworkThread<EchoSession>* CreateThreads() const override
    {
        return new NetworkThread<EchoSession>[1];
    }

    static void OnSocketAccept(tcp::socket&& socket, uint32_t threadIndex)
    {
        Instance().OnSocketOpen(std::forward<tcp::socket>(socket), threadIndex);
    }
};

#define sEchoSocketMgr EchoSocketMgr::Instance()