#include "Connection.h"

#include <utility>

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

#include "Vms/Core/Logger.h"

#define _FN "Connection"

Connection::Connection(boost::asio::ip::tcp::socket s, UpdateCallback onUpdate, DisconnectCallback onDisconnect)
    : s_(std::move(s)),
      ep_(s_.remote_endpoint()),
      onUpdate_(std::move(onUpdate)),
      onDisconnect_(std::move(onDisconnect))
{}

void Connection::start()
{
    onRead({}, 0);
}

void Connection::close() {
    if (s_.is_open()) {
        boost::system::error_code ec;
        ec = s_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        ec = s_.close(ec);
        if (ec) {
            VMS_LOG_WARN(_FN, "Failed to close socket: " << ec.message());
        } else {
            VMS_LOG_INFO(_FN, "Socket closed gracefully for: " << ep_);
        }
    }
}

void Connection::send(const std::string& message)
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    const auto& writeInProgress{ !writeQueue_.empty() };

    writeQueue_.push_back(message);

    if (!writeInProgress) {
        write();
    }
}

void Connection::write()
{
    auto self(shared_from_this());
    async_write(s_, boost::asio::buffer(writeQueue_.front()),
        [this](const std::error_code& ec, std::size_t /*length*/) {
            std::lock_guard<std::mutex> lock(writeMutex_);

            if (ec) {
                VMS_LOG_INFO(_FN, "Send failed: " << ec.message());
                return;
            }

            writeQueue_.pop_front();

            if (!writeQueue_.empty()) {
                write();
            }
        });
}

void Connection::onRead(const std::error_code& ec, std::uint32_t sz)
{
    if (ec) {
        VMS_LOG_INFO(_FN, "Disconnected " << ep_ << " with ec: " << ec.message());
        s_.close();

        if (onDisconnect_) {
            onDisconnect_(shared_from_this());
        }
        return;
    }

    try {
        if (sz > 0) {
            std::istream is(&b_);
            std::string line;
            std::getline(is, line);

            bool format_error{};

            auto spacePos = line.find(' ');
            if (spacePos == std::string::npos || spacePos == line.length() - 1) {
                format_error = true;
            }
            else
            {
                const auto& key{ line.substr(0, spacePos) };
                const auto& value{ line.substr(spacePos + 1) };

                if( key.empty() || value.empty()) {
                    format_error = true;
                }
                else if (onUpdate_){
                    onUpdate_(key, value);
                }
            }

            if( format_error ) {
                VMS_LOG_WARN(_FN, "Malformed input received: " << line);
                send("Error: Malformed input. Correct format: key value\n");
            }
        }
    } catch (const std::exception& ex) {
        VMS_LOG_ERROR(_FN, "Exception during read: " << ex.what());
        close();
        if (onDisconnect_) {
            onDisconnect_(shared_from_this());
        }
    }

    async_read_until(s_, b_, "\n",
    [self = shared_from_this()](const std::error_code& ec, std::size_t sz) {
        self->onRead(ec, sz);
    });
}
