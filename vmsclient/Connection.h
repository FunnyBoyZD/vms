#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "Vms/Core/Types.h"
#include <boost/asio/ip/tcp.hpp>

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    using DoneFn = std::function<void (const std::error_code&)>;

    explicit Connection(boost::asio::ip::tcp::socket s);
    ~Connection() = default;

    void start(DoneFn doneCb);

    bool writeSync(const std::string& str);

private:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    void onRead(const std::error_code& ec, std::size_t sz);

    void done(const std::error_code& ec);

    boost::asio::ip::tcp::socket s_;
    DoneFn doneCb_;

    std::array<char, 4096> readBuff_;
};

using ConnectionPtr = std::shared_ptr<Connection>;

#endif
