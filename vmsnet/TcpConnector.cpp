#include "Vms/Net/TcpConnector.h"
#include "Vms/Core/TimedTask.h"
#include <boost/asio/bind_executor.hpp>

namespace Vms { namespace Net
{
    namespace
    {
        class ConnectOperation : public std::enable_shared_from_this<ConnectOperation>
        {
        public:
            ConnectOperation(boost::asio::io_service& ioService,
                const Core::StrandPtr& strand,
                TcpConnector::ConnectFn cb)
            : ioService_(ioService),
              timedTask_(std::make_shared<Core::TimedTask>(ioService_, strand)),
              connection_(ioService_),
              cb_(std::move(cb))
            {
            }

            void start(const boost::asio::ip::tcp::endpoint& endpoint, std::chrono::steady_clock::duration timeout)
            {
                timedTask_->schedule(std::bind(&ConnectOperation::onTimeout, shared_from_this()), timeout);
                connection_.async_connect(endpoint,
                    boost::asio::bind_executor(*timedTask_->strand(), std::bind(&ConnectOperation::onConnect,
                        shared_from_this(), std::placeholders::_1)));
            }

        private:
            void onConnect(const std::error_code& err)
            {
                if (!timedTask_) {
                    return;
                }

                strand_runtime_assert(timedTask_->strand());

                timedTask_->cancel();
                timedTask_.reset();

                if (err) {
                    boost::system::error_code ec;
                    connection_.close(ec);
                }

                TcpConnector::ConnectFn cb;
                std::swap(cb, cb_);
                cb(std::move(connection_), err);
            }

            void onTimeout()
            {
                if (!timedTask_) {
                    return;
                }

                strand_runtime_assert(timedTask_->strand());

                timedTask_.reset();

                boost::system::error_code ec;
                connection_.close(ec);

                TcpConnector::ConnectFn cb;
                std::swap(cb, cb_);
                cb(std::move(connection_), boost::system::error_code(boost::asio::error::timed_out));
            }

            boost::asio::io_service& ioService_;
            Core::TimedTaskPtr timedTask_;
            boost::asio::ip::tcp::socket connection_;
            TcpConnector::ConnectFn cb_;
        };
    }

    TcpConnector::TcpConnector(boost::asio::io_service& ioService,
        boost::asio::ip::tcp::endpoint endpoint,
        std::chrono::steady_clock::duration timeout)
    : ioService_(ioService),
      endpoint_(std::move(endpoint)),
      timeout_(timeout),
      strand_(Core::makeStrand(ioService_))
    {
    }

    void TcpConnector::connect(ConnectFn cb)
    {
        auto op = std::make_shared<ConnectOperation>(ioService_, strand_, std::move(cb));
        op->start(endpoint_, timeout_);
    }
} }
