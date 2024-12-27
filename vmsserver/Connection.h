//
// Connection.h
//

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <deque>
#include <functional>
#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include "Vms/Core/Types.h"

/// The Connection class provides a communication link between the server and clients.
/**
 * The `Connection` class represents a TCP connection for asynchronous read and write operations.
 * It handles:
 *   - Receiving messages from the client.
 *   - Sending updates or notifications back to the client.
 *   - Notifying the server of disconnections and updates.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe with appropriate synchronization (internal mutex used).
 *
 * @par Example Usage
 * @code
 * auto conn = std::make_shared<Connection>(
 *     std::move(socket),
 *     [](const std::string& key, const std::string& value) {
 *         // Update callback logic
 *     },
 *     [](ConnectionPtr conn) {
 *         // Disconnect callback logic
 *     });
 * conn->start();
 * conn->send("Welcome to the server!\n");
 * @endcode
 *
 * @par Concepts:
 * - Asynchronous IO
 * - Thread-Safe Connection Management
 */
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    /// Type alias for the update callback.
    /**
     * This callback is invoked when a key-value pair is received from the client.
     */
    using UpdateCallback = std::function<void(const std::string&, const std::string&)>;

    /// Type alias for the disconnect callback.
    /**
     * This callback is invoked when the client disconnects unexpectedly or intentionally.
     */
    using DisconnectCallback = std::function<void(std::shared_ptr<Connection>)>;

    /// Deleted copy constructor.
    Connection(const Connection&) = delete;

    /// Deleted copy assignment operator.
    Connection& operator=(const Connection&) = delete;

    /// Connection constructor.
    /**
     * Constructs a new `Connection` object with a given TCP socket and callback handlers.
     *
     * @param s The `boost::asio::ip::tcp::socket` associated with the client connection.
     * @param onUpdate Callback invoked when a valid "key value" message is received.
     * @param onDisconnect Callback invoked when the connection is closed or disconnected.
     */
    explicit Connection(boost::asio::ip::tcp::socket s, UpdateCallback onUpdate, DisconnectCallback onDisconnect);

    /// Destructor.
    ~Connection() = default;

    /// Starts the asynchronous read operation.
    /**
     * Begins reading from the socket to process client messages.
     * This must be called after constructing the `Connection` object.
     */
    void start();

    /// Closes the connection.
    /**
    * Closes the underlying socket gracefully. Safe to call multiple times.
    * Notifies the disconnect callback if applicable.
    */
    void close();

    /// Sends a message to the client asynchronously.
    /**
     * Queues a message to be sent to the client. If there is no ongoing write operation,
     * it immediately starts writing.
     *
     * @param message The message to send to the client.
     */
    void send(const std::string& message);

private:
    /// Internal method to handle asynchronous reads.
    /**
     * Reads data from the client until a newline is encountered. Processes the message
     * and invokes the `UpdateCallback` if the input is valid.
     *
     * @param ec The error code from the last read operation.
     * @param sz The number of bytes read.
     */
    void onRead(const std::error_code& ec, std::uint32_t sz);

    /// Internal method to handle asynchronous writes.
    /**
     * Writes the queued messages to the client one by one.
     * Ensures thread-safe access to the message queue.
     */
    void write();

    /// The underlying TCP socket.
    boost::asio::ip::tcp::socket s_;

    /// The client's remote endpoint (IP address and port).
    const boost::asio::ip::tcp::endpoint ep_;

    /// Buffer for reading data from the client.
    boost::asio::streambuf b_;

    /// Callback invoked when a key-value pair is received.
    UpdateCallback onUpdate_;

    /// Callback invoked when the client disconnects.
    DisconnectCallback onDisconnect_;

    /// Queue of messages to be sent to the client.
    std::deque<std::string> writeQueue_;

    /// Mutex to ensure thread-safe access to the write queue.
    std::mutex writeMutex_;
};

/// Type alias for a shared pointer to a `Connection` object.
using ConnectionPtr = std::shared_ptr<Connection>;

#endif
