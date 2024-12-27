#ifndef _VMS_CORE_ASSERT_H_
#define _VMS_CORE_ASSERT_H_

#include "Vms/Core/Types.h"
#include <cassert>

namespace Vms { namespace Core
{
    void runtimeAssertImpl(const char* cond, const char* file, unsigned int lineno);
} }

#define runtime_assert(exp) (void)((exp) || (::Vms::Core::runtimeAssertImpl(#exp, __FILE__, __LINE__), 0))

#endif
