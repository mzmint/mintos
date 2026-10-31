#pragma once

#include <cstdint>
#include <string>


namespace Mint {
    namespace System {

        std::string computerName();
        std::string username();
        std::string userHomeDirectory();
        std::string cpuArchitecture();

        std::string cpuName();
        std::string cpuVendor();
        unsigned int cpuThreads();
        unsigned int cpuCores();
        double cpuUsagePercent();

        std::uint64_t totalRAM();
        std::uint64_t availableRAM();
        std::uint64_t usedRAM();
        double ramUsagePercent();

        std::uint64_t totalStorage(const std::string& drive);
        std::uint64_t availableStorage(const std::string& drive);
        std::uint64_t usedStorage(const std::string& drive);
        double storageUsagePercent(const std::string& drive);

        std::uint64_t uptimeMs();
        std::uint64_t uptimeSeconds();
        std::uint64_t uptimeMinutes();
        std::uint64_t uptimeHours();

        unsigned int screenWidth();
        unsigned int screenHeight();
        unsigned int screenRefreshRate();

        std::string currentDirectory();
        std::string temporaryDirectory();
        std::string environmentVariable(const std::string& name);

        bool hasBattery();
        double batteryPercent();
        bool isCharging();

        std::string hostname();
        std::string localIPAddress();

        bool is64Bit();
        bool isVirtualMachine();

    }
}