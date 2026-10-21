#pragma once

#include <string>
#include <cstdint>

namespace Mint {

    std::string computerName();
    std::string username();
    std::string cpuArchitecture();

    unsigned int cpuThreads();

    std::uint64_t totalRAM();
    std::uint64_t availableRAM();
    double ramUsagePercent();

    std::uint64_t uptimeMs();

}
