#ifndef _VMS_CORE_EXECUTOR_H_
#define _VMS_CORE_EXECUTOR_H_

#include "Vms/Core/Types.h"
#include <boost/asio/io_service.hpp>
#include <thread>

namespace Vms { namespace Core
{
    class Executor
    {
    public:
        Executor();
        ~Executor();

        inline boost::asio::io_service& ioService() { return ioService_; }
        inline const boost::asio::io_service& ioService() const { return ioService_; }

        // 'force' means stop any outstanding I/O on ioService and return immediately.
        // If you set it to false it'll wait until ALL I/O handlers are done. Note that using
        // force = false may hang forever if some other thread keeps posting new handlers to
        // ioService.
        // This function is typically called once, if it's not called explicitly then it's called
        // automatically on Executor's destruction as stop(true).
        void stop(bool force = true);

    private:
        Executor(const Executor&) = delete;
        Executor& operator=(const Executor&) = delete;

        boost::asio::io_service ioService_;
        std::unique_ptr<boost::asio::io_service::work> work_;

        std::thread thread_;
    };
} }

#endif
