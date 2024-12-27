#ifndef _VMS_CORE_STRAND_H_
#define _VMS_CORE_STRAND_H_

#include "Vms/Core/Types.h"
#include "Vms/Core/Assert.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

namespace Vms { namespace Core
{
    using StrandPtr = std::shared_ptr<boost::asio::strand<boost::asio::io_context::executor_type>>;

    inline StrandPtr makeStrand(boost::asio::io_service& ioService)
    {
        return std::make_shared<boost::asio::strand<boost::asio::io_context::executor_type>>(boost::asio::make_strand(ioService));
    }

    inline bool isStrandThread(const StrandPtr& strand)
    {
        return strand->running_in_this_thread();
    }
} }

#define strand_assert(strand) assert(::Vms::Core::isStrandThread(strand))
#define strand_runtime_assert(strand) runtime_assert(::Vms::Core::isStrandThread(strand))

#endif
