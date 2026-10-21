#include "MintLibSystem.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Lmcons.h>

#include <stdexcept>
#include <vector>

namespace Mint {

    std::string computerName() {
        DWORD size = 0;
        GetComputerNameExA(ComputerNameDnsHostname,
                           nullptr, &size);

        if (size == 0)
            throw std::runtime_error("Cannot get computer name");

        std::vector<char> buffer(size);

        if (!GetComputerNameExA(
                ComputerNameDnsHostname,
                buffer.data(), &size)) {
            throw std::runtime_error("Cannot get computer name");
        }

        return std::string(buffer.data(), size);
    }

    std::string username() {
        char buffer[UNLEN + 1]{};
        DWORD size = sizeof(buffer);

        if (!GetUserNameA(buffer, &size))
            throw std::runtime_error("Cannot get username");

        return buffer;
    }

    std::string cpuArchitecture() {
        SYSTEM_INFO info{};
        GetNativeSystemInfo(&info);

        switch (info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            return "x64";

        case PROCESSOR_ARCHITECTURE_ARM64:
            return "ARM64";

        case PROCESSOR_ARCHITECTURE_INTEL:
            return "x86";

        default:
            return "Unknown";
        }
    }

    unsigned int cpuThreads() {
        SYSTEM_INFO info{};
        GetNativeSystemInfo(&info);

        return info.dwNumberOfProcessors;
    }

    std::uint64_t totalRAM() {
        MEMORYSTATUSEX info{};
        info.dwLength = sizeof(info);

        if (!GlobalMemoryStatusEx(&info))
            throw std::runtime_error("Cannot get RAM information");

        return info.ullTotalPhys;
    }

    std::uint64_t availableRAM() {
        MEMORYSTATUSEX info{};
        info.dwLength = sizeof(info);

        if (!GlobalMemoryStatusEx(&info))
            throw std::runtime_error("Cannot get RAM information");

        return info.ullAvailPhys;
    }

    double ramUsagePercent() {
        MEMORYSTATUSEX info{};
        info.dwLength = sizeof(info);

        if (!GlobalMemoryStatusEx(&info))
            throw std::runtime_error("Cannot get RAM information");

        return static_cast<double>(info.dwMemoryLoad);
    }

    std::uint64_t uptimeMs() {
        return GetTickCount64();
    }

}
