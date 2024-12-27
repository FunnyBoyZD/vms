#include "Connection.h"

#include <future>
#include <iostream>
#include <csignal>
#include <unordered_map>

#include <boost/program_options.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

#include "Vms/Net/TcpAcceptor.h"
#include "Vms/Core/Executor.h"
#include "Vms/Core/Logger.h"
#include "Utils.h"

#define _FN "Server"

namespace {
    std::promise<void> p;
    auto f{ p.get_future() };

    static std::promise<void>* gP{ &p };

    // Unified signal handler function
    void signalHandler(int signal) {
        if (gP) {
            VMS_LOG_INFO(_FN, "Signal " << signal << " received. Initiating shutdown...");
            gP->set_value();
            gP = nullptr;
        }
    }

    std::unordered_map<std::string, std::string> map;
    std::set<ConnectionPtr> clients;

    boost::asio::thread_pool hashPool(std::thread::hardware_concurrency());

    std::mutex mapMutex;
    std::mutex clientsMutex;
}

int main(int argc, char* argv[])
{
    boost::program_options::variables_map vm;
    std::uint32_t logLevel{ 4 };
    std::uint16_t ipPort{ 8081 };

    try {
        boost::program_options::options_description desc("Options");

        desc.add_options()
            ("help", "Print this help message")
            ("log-level", boost::program_options::value(&logLevel), "Log level(0 - 4), default = 4")
            ("verbose", "Use verbose logging, default = off")
            ("port", boost::program_options::value(&ipPort), "IP port (numeric), default = 8081");

        store( boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

        notify(vm);

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

    Vms::Core::Executor executor;
    auto acceptor{ std::make_shared<Vms::Net::TcpAcceptor>(executor.ioService(), boost::asio::ip::tcp::v4()) };

    boost::asio::ip::tcp::endpoint boundEndpoint;
    auto ec{ acceptor->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), ipPort),
        boundEndpoint) };
    if (ec) {
        VMS_LOG_ERROR(_FN, "Can't bind server: " << ec.message());
        return 1;
    }

    ec = acceptor->listen(10, [&](boost::asio::ip::tcp::socket s) {
        auto conn = std::make_shared<Connection>(
            std::move(s),
            [](const std::string& key, const std::string& value) {
                post(hashPool, [value, key]() {
                    // Compute the heavy hash value
                    auto hashValue{ calcHeavyHash(value) };
                    const auto& message{ key + " " + std::to_string(hashValue) + "\n" };

                    {
                        // Update the shared map under a lock
                        std::lock_guard<std::mutex> lock(mapMutex);
                        map[key] = std::to_string(hashValue);
                    }

                    // Broadcast the update to all connected clients
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (auto& client : clients) {
                        client->send(message);
                    }

                    VMS_LOG_INFO(_FN, "Client's message \"" + message +"\" processing completed");
                });
        },
        [](ConnectionPtr conn) {
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.erase(conn);
            }

            conn->close(); // Explicitly close the socket.
            VMS_LOG_INFO(_FN, "Client cleanup complete");
            });

        {
            std::lock_guard<std::mutex> lock(mapMutex);
            for (auto it = map.begin(); it != map.end(); ++it) {
                conn->send(it->first + " " + it->second + "\n");
            }
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.insert(conn);
        }

        conn->start();
    });
    if (ec) {
        VMS_LOG_ERROR(_FN, "Can't listen server: " << ec.message());
        return 1;
    }

    VMS_LOG_INFO(_FN, "Started at " << boundEndpoint);

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    f.get();

    VMS_LOG_INFO(_FN, "Shutting down...");

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& client : clients) {
            client->send("Server shutting down\n");
            client->close(); // Ensure connection is explicitly closed
        }

        clients.clear();
    }

    // Stop the thread pool and wait for all tasks to finish
    hashPool.join();

    executor.stop();

    VMS_LOG_INFO(_FN, "Stopped");

    return 0;
}
