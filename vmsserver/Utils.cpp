#include "Utils.h"
#include <boost/crc.hpp>

std::uint32_t calcHeavyHash(const std::string& str)
{
    // CRC32 is not heavy, but let's assume we're doing something really CPU-intensive here...
    boost::crc_32_type crc32;
    crc32.process_bytes(str.data(), str.size());
    return crc32.checksum();
}
