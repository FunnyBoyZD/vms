#include "Vms/Core/Assert.h"
#include "Vms/Core/Logger.h"
#include <signal.h>

#define _FN "Core"

namespace Vms { namespace Core
{
    void runtimeAssertImpl(const char* cond, const char* file, unsigned int lineno)
    {
        VMS_LOG_ERROR(_FN, "ASSERTION FAILED at " << file << "(" << lineno << "): " << cond);
        ::raise(SIGABRT);
    }
} }
