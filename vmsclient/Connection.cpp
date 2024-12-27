#include "Connection.h"
#include "Vms/Core/Logger.h"
#include "Vms/Core/Assert.h"
#include <boost/asio/write.hpp>
#include <future>
#include <iostream>

#define _FN "Connection"

Connection::Connection(boost::asio::ip::tcp::socket s)
: s_(std::move(s))
{
}

void Connection::start(DoneFn doneCb)
{
    doneCb_ = std::move(doneCb);
    onRead({}, 0);
}

bool Connection::writeSync(const std::string& str)
{
    std::promise<bool> p;
    auto f = p.get_future();

    // We want this to be executed on ioService, i.e. serialized with read code.
    boost::asio::async_write(s_, boost::asio::buffer(str), [this, &p](const std::error_code& ec, std::size_t sz) {
        if (!doneCb_) {
            p.set_value(false);
            return;
        }

        if (ec) {
            VMS_LOG_DEBUG(_FN, "onWrite(): " << ec.message());
            done(ec);
            p.set_value(false);
            return;
        }

        VMS_LOG_DEBUG(_FN, "onWrite(" << sz << ")");

        p.set_value(true);
    });

    return f.get();
}

void Connection::onRead(const std::error_code& ec, std::size_t sz)
{
    if (!doneCb_) {
        return;
    }

    if (ec) {
        VMS_LOG_DEBUG(_FN, "onRead(): " << ec.message());
        done(ec);
        return;
    }

    VMS_LOG_DEBUG(_FN, "onRead(" << sz << ")");

    if (sz > 0) {
        std::cout.write(readBuff_.data(), sz);
        std::cout.flush();
    }

    s_.async_read_some(boost::asio::buffer(readBuff_),
        std::bind(&Connection::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void Connection::done(const std::error_code& ec)
{
    runtime_assert(doneCb_);

    boost::system::error_code err;
    s_.close(err);

    DoneFn cb;
    std::swap(doneCb_, cb);
    cb(ec);
}
