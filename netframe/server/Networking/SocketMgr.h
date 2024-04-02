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

#ifndef SocketMgr_h__
#define SocketMgr_h__

#include "AsyncAcceptor.h"
// #include "Errors.h"
#include "NetworkThread.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <assert.h>

using boost::asio::ip::tcp;

template<class SocketType>
class SocketMgr
{
public:
    virtual ~SocketMgr()
    {
        // __assert(!_threads && !_acceptor && !_threadCount, "StopNetwork must be called prior to SocketMgr destruction");
    }

    virtual bool StartNetwork(boost::asio::io_context& ioContext, std::string const& bindIp, uint16_t port, int threadCount)
    {
        // __assert(threadCount > 0);

        AsyncAcceptor* acceptor = nullptr;
        try
        {
            acceptor = new AsyncAcceptor(ioContext, bindIp, port);
        }
        catch (boost::system::system_error const& err)
        {
            std::cout << "SocketMgr.StartNetwork " << err.what() << std::endl;
            return false;
        }

        if (!acceptor->Bind())
        {
            std::cout << "StartNetwork failed to bind socket acceptor " << std::endl;
            delete acceptor;
            return false;
        }

        _acceptor = acceptor;
        _threadCount = threadCount;
        _threads = CreateThreads();

        assert(_threads);

        for (int32_t i = 0; i < _threadCount; ++i)
            _threads[i].Start();

        _acceptor->SetSocketFactory([this]() { return GetSocketForAccept(); });

        return true;
    }

    virtual void StopNetwork()
    {
        _acceptor->Close();

        if (_threadCount != 0)
            for (int32_t i = 0; i < _threadCount; ++i)
                _threads[i].Stop();

        Wait();

        delete _acceptor;
        _acceptor = nullptr;
        delete[] _threads;
        _threads = nullptr;
        _threadCount = 0;
    }

    void Wait()
    {
        if (_threadCount != 0)
            for (int32_t i = 0; i < _threadCount; ++i)
                _threads[i].Wait();
    }

    virtual void OnSocketOpen(tcp::socket&& sock, uint32_t threadIndex)
    {
        try
        {
            std::shared_ptr<SocketType> newSocket = std::make_shared<SocketType>(std::move(sock));
            newSocket->Start();

            _threads[threadIndex].AddSocket(newSocket);
        }
        catch (boost::system::system_error const& err)
        {
            // TC_LOG_WARN("network", "Failed to retrieve client's remote address %s", err.what());
        }
    }

    int32_t GetNetworkThreadCount() const { return _threadCount; }

    uint32_t SelectThreadWithMinConnections() const
    {
        uint32_t min = 0;

        for (int32_t i = 1; i < _threadCount; ++i)
            if (_threads[i].GetConnectionCount() < _threads[min].GetConnectionCount())
                min = i;

        return min;
    }

    std::pair<tcp::socket*, uint32_t> GetSocketForAccept()
    {
        uint32_t threadIndex = SelectThreadWithMinConnections();
        return std::make_pair(_threads[threadIndex].GetSocketForAccept(), threadIndex);
    }

protected:
    SocketMgr() : _acceptor(nullptr), _threads(nullptr), _threadCount(0)
    {
    }

    virtual NetworkThread<SocketType>* CreateThreads() const = 0;

    AsyncAcceptor* _acceptor;
    NetworkThread<SocketType>* _threads;
    int32_t _threadCount;
};

#endif // SocketMgr_h__
