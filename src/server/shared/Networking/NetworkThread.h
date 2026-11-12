////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef NetworkThread_h__
#define NetworkThread_h__

#include "Define.h"
#include "DeadlineTimer.h"
#include "Errors.h"
#include "IoContext.h"
#include "Log.h"
#include <boost/asio/ip/tcp.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

template<class SocketType>
class NetworkThread
{
public:
    NetworkThread() : _connections(0), _stopped(false), _thread(nullptr), _ioContext(1),
        _acceptSocket(_ioContext), _updateTimer(_ioContext)
    {
    }

    virtual ~NetworkThread()
    {
        Stop();
        if (_thread)
        {
            Wait();
            delete _thread;
        }
    }

    void Stop()
    {
        _stopped = true;
        _ioContext.stop();
    }

    bool Start()
    {
        if (_thread)
            return false;

        _thread = new std::thread(&NetworkThread::Run, this);
        return true;
    }

    void Wait()
    {
        ASSERT(_thread);

        _thread->join();
        delete _thread;
        _thread = nullptr;
    }

    int32 GetConnectionCount() const
    {
        return _connections;
    }

    void AddSocket(std::shared_ptr<SocketType> sock)
    {
        std::lock_guard<std::mutex> lock(_newSocketsLock);

        ++_connections;
        _newSockets.push_back(sock);
        SocketAdded(sock);
    }

    boost::asio::ip::tcp::socket* GetSocketForAccept() { return &_acceptSocket; }

protected:
    virtual void SocketAdded(std::shared_ptr<SocketType> /*sock*/) { }
    virtual void SocketRemoved(std::shared_ptr<SocketType> /*sock*/) { }

    void AddNewSockets()
    {
        std::lock_guard<std::mutex> lock(_newSocketsLock);

        if (_newSockets.empty())
            return;

        for (std::shared_ptr<SocketType> sock : _newSockets)
        {
            if (!sock->IsOpen())
            {
                SocketRemoved(sock);
                --_connections;
            }
            else
                _sockets.push_back(sock);
        }

        _newSockets.clear();
    }

    void Run()
    {
        TC_LOG_DEBUG("misc", "Network Thread Starting");

        _updateTimer.expires_from_now(boost::posix_time::milliseconds(1));
        _updateTimer.async_wait([this](boost::system::error_code const&) { Update(); });

        // an exception out of a socket handler would end the whole process: log it and keep serving the others
        for (;;)
        {
            try
            {
                _ioContext.run();
                break;
            }
            catch (std::exception const& e)
            {
                TC_LOG_ERROR("network", "Network thread: exception in a socket handler: %s", e.what());
            }
        }

        TC_LOG_DEBUG("misc", "Network Thread exits");
        _newSockets.clear();
        _sockets.clear();
    }

    void Update()
    {
        if (_stopped)
            return;

        _updateTimer.expires_from_now(boost::posix_time::milliseconds(1));
        _updateTimer.async_wait([this](boost::system::error_code const&) { Update(); });

        AddNewSockets();

        _sockets.erase(std::remove_if(_sockets.begin(), _sockets.end(), [this](std::shared_ptr<SocketType> sock)
        {
            if (!sock->Update())
            {
                if (sock->IsOpen())
                    sock->CloseSocket();

                this->SocketRemoved(sock);

                --this->_connections;
                return true;
            }

            return false;
        }), _sockets.end());
    }

private:
    typedef std::vector<std::shared_ptr<SocketType>> SocketContainer;

    std::atomic<int32> _connections;
    std::atomic<bool> _stopped;

    std::thread* _thread;

    SocketContainer _sockets;

    std::mutex _newSocketsLock;
    SocketContainer _newSockets;

    Trinity::Asio::IoContext _ioContext;
    boost::asio::ip::tcp::socket _acceptSocket;
    Trinity::Asio::DeadlineTimer _updateTimer;
};

#endif // NetworkThread_h__
