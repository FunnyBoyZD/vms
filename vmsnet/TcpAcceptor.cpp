#include "Vms/Net/TcpAcceptor.h"
#include "Vms/Core/Logger.h"
#include <boost/asio/bind_executor.hpp>

#define _FN "Net"

namespace Vms { namespace Net
{
    TcpAcceptor::TcpAcceptor(boost::asio::io_service& ioService, const boost::asio::ip::tcp& protocol)
    : ioService_(ioService),
      strand_(Core::makeStrand(ioService_)),
      acceptor_(ioService_, protocol)
    {
    }

    std::error_code TcpAcceptor::bind(const boost::asio::ip::tcp::endpoint& endpoint, boost::asio::ip::tcp::endpoint& boundEndpoint)
    {
        boost::system::error_code ec;
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if (ec) {
            return ec;
        }

        acceptor_.bind(endpoint, ec);
        if (ec) {
            return ec;
        }

        boundEndpoint = acceptor_.local_endpoint();

        return std::error_code{};
    }

    std::error_code TcpAcceptor::listen(std::uint32_t backlog, AcceptFn cb)
    {
        boost::system::error_code ec;
        acceptor_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
        if (ec) {
            VMS_LOG_WARN(_FN, "TcpAcceptor: cannot set TCP_NODELAY: " << ec.message());
        }

        acceptor_.listen(backlog, ec);
        if (ec) {
            return ec;
        }

        acceptor_.async_accept(boost::asio::bind_executor(*strand_,
            std::bind(&TcpAcceptor::onAccept, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(cb))));

        return std::error_code{};
    }

    void TcpAcceptor::stop()
    {
        auto sharedThis = shared_from_this();
        boost::asio::dispatch(*strand_, [sharedThis]() {
            strand_assert(sharedThis->strand_);

            boost::system::error_code ec;
            sharedThis->acceptor_.close(ec);
        });
    }

    void TcpAcceptor::onAccept(const std::error_code& err, boost::asio::ip::tcp::socket socket, AcceptFn cb)
    {
        strand_runtime_assert(strand_);

        if (err) {
            if (err != boost::system::error_code(boost::asio::error::operation_aborted)) {
                VMS_LOG_ERROR(_FN, "TcpAcceptor: accept failed: " << err.message());
            } else {
                // TcpAcceptor stopped, don't accept again.
                return;
            }
        } else {
            boost::system::error_code ec;
            socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
            if (ec) {
                VMS_LOG_WARN(_FN, "TcpAcceptor: cannot set TCP_NODELAY: " << ec.message());
            }

            cb(std::move(socket));
        }

        acceptor_.async_accept(boost::asio::bind_executor(*strand_,
            std::bind(&TcpAcceptor::onAccept, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(cb))));
    }
} }
