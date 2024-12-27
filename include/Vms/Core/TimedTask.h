#ifndef _VMS_CORE_TIMEDTASK_H_
#define _VMS_CORE_TIMEDTASK_H_

#include "Vms/Core/Strand.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>

namespace Vms { namespace Core
{
    class TimedTask : public std::enable_shared_from_this<TimedTask>
    {
    public:
        explicit TimedTask(boost::asio::io_service& ioService, const StrandPtr& strand = StrandPtr())
        : timer_(ioService), strand_(strand)
        {
        }

        ~TimedTask() = default;

        inline const Core::StrandPtr& strand() const { return strand_; }

        template <class T>
        inline void schedule(T&& task, std::chrono::steady_clock::duration timeout)
        {
            timer_.expires_from_now(timeout);
            auto that = shared_from_this();
            std::uint32_t c = ++cancelCounter_;
            MoveHelper<T> helper(std::forward<T>(task));
            if (strand_) {
                timer_.async_wait(boost::asio::bind_executor(*strand_, [that, helper, c](const boost::system::error_code& ec) {
                    if ((ec != boost::asio::error::operation_aborted) && (that->cancelCounter_ == c)) {
                        helper.data();
                    }
                }));
            } else {
                timer_.async_wait([that, helper, c](const boost::system::error_code& ec) {
                    if ((ec != boost::asio::error::operation_aborted) && (that->cancelCounter_ == c)) {
                        helper.data();
                    }
                });
            }
        }

        inline void cancel()
        {
            // When steady_timer is expired it cannot be canceled, this can
            // cause problems with serialized operations. i.e. a single strand
            // is servicing the queue, a handler is in progress and it wants to cancel
            // a timer, but the timer is already expired, thus, the cancel will fail
            // and the timer handler will get executed after current handler returns.
            // That's not what one expects when doing serialized operations, thus, we
            // need to use a flag to prevent such cases...
            ++cancelCounter_;

            boost::system::error_code e;
            timer_.cancel(e);
        }

    private:
        template <class T>
        struct MoveHelper
        {
            MoveHelper(T&& data) : data(std::forward<T>(data)) {}

            MoveHelper(MoveHelper& other) : data(std::move(other.data)) {}
            MoveHelper& operator=(MoveHelper& other)
            {
                if (this != &other) {
                    data = std::move(other.data);
                }
                return *this;
            }

            MoveHelper(MoveHelper&& other) : data(std::move(other.data)) {}
            MoveHelper& operator=(MoveHelper&& other)
            {
                if (this != &other) {
                    data = std::move(other.data);
                }
                return *this;
            }

            T data;
        };

        TimedTask(const TimedTask&) = delete;
        TimedTask& operator=(const TimedTask&) = delete;

        boost::asio::steady_timer timer_;
        StrandPtr strand_;
        std::atomic<std::uint32_t> cancelCounter_{0};
    };

    using TimedTaskPtr = std::shared_ptr<TimedTask>;
} }

#endif
