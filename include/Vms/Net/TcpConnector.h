#ifndef _VMS_NET_TCPCONNECTOR_H_
#define _VMS_NET_TCPCONNECTOR_H_

#include "Vms/Core/Strand.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Vms { namespace Net
{
    class TcpConnector
    {
    public:
        using ConnectFn = std::function<void (boost::asio::ip::tcp::socket, const std::error_code&)>;

        TcpConnector(boost::asio::io_service& ioService,
            boost::asio::ip::tcp::endpoint endpoint,
            std::chrono::steady_clock::duration timeout);
        ~TcpConnector() = default;

        inline boost::asio::io_service& ioService() { return ioService_; }
        inline const boost::asio::io_service& ioService() const { return ioService_; }

        // Callbacks from a single TcpConnector are never called concurrently.
        void connect(ConnectFn cb);

    private:
        TcpConnector(const TcpConnector&) = delete;
        TcpConnector& operator=(const TcpConnector&) = delete;

        boost::asio::io_service& ioService_;
        const boost::asio::ip::tcp::endpoint endpoint_;
        const std::chrono::steady_clock::duration timeout_;

        Core::StrandPtr strand_;
    };
} }

#endif
