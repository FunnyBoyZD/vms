#include "Connection.h"
#include "Vms/Net/TcpConnector.h"
#include "Vms/Core/Executor.h"
#include "Vms/Core/Logger.h"
#include <boost/program_options.hpp>
#include <future>
#include <iostream>

#define _FN "Client"

int main(int argc, char* argv[])
{
    boost::program_options::variables_map vm;
    std::uint32_t logLevel = 4;
    std::string ipAddressStr = "127.0.0.1";
    std::uint16_t ipPort = 8081;
    std::uint32_t connectTimeoutMs = 5000;

    try {
        boost::program_options::options_description desc("Options");

        desc.add_options()
            ("help", "Print this help message")
            ("log-level", boost::program_options::value(&logLevel), "Log level(0 - 4), default = 4")
            ("verbose", "Use verbose logging, default = off")
            ("ip-address", boost::program_options::value(&ipAddressStr), "IP address (numeric), default = 127.0.0.1")
            ("port", boost::program_options::value(&ipPort), "IP port (numeric), default = 8081")
            ("connect-timeout-ms", boost::program_options::value(&connectTimeoutMs), "Connect timeout (ms), default = 5000");

        boost::program_options::store(
            boost::program_options::command_line_parser(
                argc, argv).options(desc).run(), vm);

        boost::program_options::notify(vm);

        if (vm.count("help") > 0) {
            std::cout << desc << std::endl;
            return 1;
        }
    } catch (const boost::program_options::error& e) {
        VMS_LOG_ERROR(_FN, "Invalid command line arguments: " << e.what());
        return 1;
    } catch (const std::exception& e) {
        VMS_LOG_ERROR(_FN, "Error: " << e.what());
        return 1;
    }

    if (logLevel > Vms::Core::LogLevelOFF) {
        VMS_LOG_ERROR(_FN, "Bad log-level " << logLevel);
        return 1;
    }

    Vms::Core::logger.setLevel(static_cast<Vms::Core::LogLevel>(Vms::Core::LogLevelOFF - logLevel));
    Vms::Core::logger.setVerbose(vm.count("verbose") > 0);

    boost::system::error_code ec;
    auto ipAddress = boost::asio::ip::address::from_string(ipAddressStr, ec);
    if (ec) {
        VMS_LOG_ERROR(_FN, "Cannot parse IP address: " << ec.message());
        return 1;
    }

    Vms::Core::Executor executor;

    Vms::Net::TcpConnector connector(executor.ioService(),
        boost::asio::ip::tcp::endpoint(ipAddress, ipPort), std::chrono::milliseconds(connectTimeoutMs));

    VMS_LOG_INFO(_FN, "Started");

    std::promise<ConnectionPtr> connP;
    std::promise<void> doneP;
    auto connF = connP.get_future();
    auto doneF = doneP.get_future();

    connector.connect([&connP, &doneP](boost::asio::ip::tcp::socket s, const std::error_code& ec) {
        if (ec) {
            VMS_LOG_ERROR(_FN, "Failed to connect: " << ec.message());
            connP.set_value({});
            doneP.set_value();
            return;
        }

        VMS_LOG_INFO(_FN, "Connected!");

        auto conn = std::make_shared<Connection>(std::move(s));
        conn->start([&doneP](const std::error_code& ec) {
            VMS_LOG_INFO(_FN, "Connection done: " << ec.message());
            doneP.set_value();
        });

        connP.set_value(conn);
    });

    auto conn = connF.get();

    if (conn) {
        VMS_LOG_INFO(_FN, "Type \"exit\" to disconnect");

        // No simple portable way to read stdin asynchronously, so do it synchronously here
        // and feed into connection also synchronously until it's done.
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "exit") {
                break;
            }
            line += '\n';
            if (!conn->writeSync(line)) {
                break;
            }
        }
        if (!std::cin) {
            // Even if stdin is done we still want to listen for server data...
            doneF.get();
        }
    }

    VMS_LOG_INFO(_FN, "Shutting down...");

    executor.stop();

    VMS_LOG_INFO(_FN, "Stopped");

    return 0;
}
