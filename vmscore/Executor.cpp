#include "Vms/Core/Executor.h"
#include "Vms/Core/Logger.h"
#include "Vms/Core/Assert.h"

#define _FN "Core"

namespace Vms { namespace Core
{
    Executor::Executor()
    : work_(new boost::asio::io_service::work(ioService_)),
      thread_([this]() {
          boost::system::error_code ec;
          ioService().run(ec);
          if (ec) {
              VMS_LOG_ERROR(_FN, "Executor: " << ec.message());
          }
      })
    {
    }

    Executor::~Executor()
    {
        stop(true);
    }

    void Executor::stop(bool force)
    {
        if (!thread_.joinable()) {
            return;
        }

        if (force) {
            ioService().stop();
        } else {
            work_.reset();
        }

        runtime_assert((std::this_thread::get_id() != thread_.get_id()) && "Executor: self join detected!");
        thread_.join();
    }
} }
