#ifndef _VMS_NET_TCPACCEPTOR_H_
#define _VMS_NET_TCPACCEPTOR_H_

#include "Vms/Core/Strand.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Vms { namespace Net
{
    class TcpAcceptor : public std::enable_shared_from_this<TcpAcceptor>
    {
    public:
        using AcceptFn = std::function<void (boost::asio::ip::tcp::socket)>;

        TcpAcceptor(boost::asio::io_service& ioService, const boost::asio::ip::tcp& protocol);
        ~TcpAcceptor() = default;

        inline boost::asio::io_service& ioService() { return ioService_; }
        inline const boost::asio::io_service& ioService() const { return ioService_; }

        std::error_code bind(const boost::asio::ip::tcp::endpoint& endpoint, boost::asio::ip::tcp::endpoint& boundEndpoint);

        // Callbacks from a single TcpAcceptor are never called concurrently.
        std::error_code listen(std::uint32_t backlog, AcceptFn cb);

        void stop();

    private:
        TcpAcceptor(const TcpAcceptor&) = delete;
        TcpAcceptor& operator=(const TcpAcceptor&) = delete;

        void onAccept(const std::error_code& err, boost::asio::ip::tcp::socket socket, AcceptFn cb);

        boost::asio::io_service& ioService_;

        Core::StrandPtr strand_;
        boost::asio::ip::tcp::acceptor acceptor_;
    };
} }

#endif
