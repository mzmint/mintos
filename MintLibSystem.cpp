#include "MintLibSystem.h"

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <lmcons.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wininet.lib")

namespace Mint {
    namespace System {

        // ==========================================
        // Computer identification
        // ==========================================

        std::string computerName() {
            char buffer[MAX_COMPUTERNAME_LENGTH + 1]{};
            DWORD size = sizeof(buffer);

            if (!GetComputerNameA(buffer, &size))
                return "";

            return buffer;
        }

        std::string username() {
            char buffer[UNLEN + 1]{};
            DWORD size = sizeof(buffer);

            if (!GetUserNameA(buffer, &size))
                return "";

            return buffer;
        }

        std::string userHomeDirectory() {
            char buffer[MAX_PATH]{};

            DWORD size = GetEnvironmentVariableA(
                "USERPROFILE", buffer, MAX_PATH
            );

            if (size == 0 || size >= MAX_PATH)
                return "";

            return buffer;
        }

        std::string cpuArchitecture() {
            SYSTEM_INFO info{};
            GetNativeSystemInfo(&info);

            switch (info.wProcessorArchitecture) {
                case PROCESSOR_ARCHITECTURE_AMD64:
                    return "x86_64";

                case PROCESSOR_ARCHITECTURE_INTEL:
                    return "x86";

                case PROCESSOR_ARCHITECTURE_ARM64:
                    return "ARM64";

                case PROCESSOR_ARCHITECTURE_ARM:
                    return "ARM";

                default:
                    return "Unknown";
            }
        }

        // ==========================================
        // CPU information
        // ==========================================

        std::string cpuName() {
            HKEY key;

            if (RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                0,
                KEY_READ,
                &key
            ) != ERROR_SUCCESS)
                return "";

            char buffer[256]{};
            DWORD size = sizeof(buffer);

            LONG result = RegQueryValueExA(
                key,
                "ProcessorNameString",
                nullptr,
                nullptr,
                reinterpret_cast<BYTE*>(buffer),
                &size
            );

            RegCloseKey(key);

            if (result != ERROR_SUCCESS)
                return "";

            return buffer;
        }

        std::string cpuVendor() {
            int info[4]{};
            __cpuid(info, 0);

            char vendor[13]{};

            *reinterpret_cast<int*>(vendor) = info[1];
            *reinterpret_cast<int*>(vendor + 4) = info[3];
            *reinterpret_cast<int*>(vendor + 8) = info[2];

            return vendor;
        }

        unsigned int cpuThreads() {
            return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
        }

        unsigned int cpuCores() {
            DWORD size = 0;

            GetLogicalProcessorInformationEx(
                RelationProcessorCore,
                nullptr,
                &size
            );

            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                return 0;

            std::vector<BYTE> buffer(size);

            auto info =
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                    buffer.data()
                );

            if (!GetLogicalProcessorInformationEx(
                RelationProcessorCore,
                info,
                &size
            ))
                return 0;

            unsigned int cores = 0;
            DWORD offset = 0;

            while (offset < size) {
                auto entry =
                    reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                        buffer.data() + offset
                    );

                if (entry->Relationship == RelationProcessorCore)
                    cores++;

                offset += entry->Size;
            }

            return cores;
        }

        double cpuUsagePercent() {
            static ULARGE_INTEGER previousIdle{};
            static ULARGE_INTEGER previousKernel{};
            static ULARGE_INTEGER previousUser{};

            static double usage = 0.0;
            static bool initialized = false;

            static ULONGLONG lastUpdate = 0;

            ULONGLONG now = GetTickCount64();

            if (initialized && now - lastUpdate < 1000)
                return usage;

            FILETIME idleTime{}, kernelTime{}, userTime{};

            if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
                return usage;

            ULARGE_INTEGER idle{}, kernel{}, user{};

            idle.LowPart = idleTime.dwLowDateTime;
            idle.HighPart = idleTime.dwHighDateTime;

            kernel.LowPart = kernelTime.dwLowDateTime;
            kernel.HighPart = kernelTime.dwHighDateTime;

            user.LowPart = userTime.dwLowDateTime;
            user.HighPart = userTime.dwHighDateTime;

            if (initialized) {
                auto idleDiff = idle.QuadPart - previousIdle.QuadPart;
                auto kernelDiff = kernel.QuadPart - previousKernel.QuadPart;
                auto userDiff = user.QuadPart - previousUser.QuadPart;

                auto total = kernelDiff + userDiff;

                if (total > 0) {
                    usage = 100.0 * (total - std::min(idleDiff, total)) / total;
                }
            }

            previousIdle = idle;
            previousKernel = kernel;
            previousUser = user;

            initialized = true;
            lastUpdate = now;

            return usage;
        }

        // ==========================================
        // RAM information
        // ==========================================

        std::uint64_t totalRAM() {
            MEMORYSTATUSEX memory{};
            memory.dwLength = sizeof(memory);

            if (!GlobalMemoryStatusEx(&memory))
                return 0;

            return memory.ullTotalPhys;
        }

        std::uint64_t availableRAM() {
            MEMORYSTATUSEX memory{};
            memory.dwLength = sizeof(memory);

            if (!GlobalMemoryStatusEx(&memory))
                return 0;

            return memory.ullAvailPhys;
        }

        std::uint64_t usedRAM() {
            return totalRAM() - availableRAM();
        }

        double ramUsagePercent() {
            std::uint64_t total = totalRAM();

            if (total == 0)
                return 0.0;

            return 100.0 * usedRAM() / total;
        }

        // ==========================================
        // Storage information
        // ==========================================

        std::uint64_t totalStorage(const std::string& drive) {
            ULARGE_INTEGER total{}, free{};

            if (!GetDiskFreeSpaceExA(
                drive.c_str(),
                nullptr,
                &total,
                &free
            ))
                return 0;

            return total.QuadPart;
        }

        std::uint64_t availableStorage(const std::string& drive) {
            ULARGE_INTEGER available{}, total{}, free{};

            if (!GetDiskFreeSpaceExA(
                drive.c_str(),
                &available,
                &total,
                &free
            ))
                return 0;

            return available.QuadPart;
        }

        std::uint64_t usedStorage(const std::string& drive) {
            std::uint64_t total = totalStorage(drive);
            std::uint64_t available = availableStorage(drive);

            return total >= available ? total - available : 0;
        }

        double storageUsagePercent(const std::string& drive) {
            std::uint64_t total = totalStorage(drive);

            if (total == 0)
                return 0.0;

            return 100.0 * usedStorage(drive) / total;
        }

        // ==========================================
        // System uptime
        // ==========================================

        std::uint64_t uptimeMs() {
            return GetTickCount64();
        }

        std::uint64_t uptimeSeconds() {
            return uptimeMs() / 1000;
        }

        std::uint64_t uptimeMinutes() {
            return uptimeSeconds() / 60;
        }

        std::uint64_t uptimeHours() {
            return uptimeMinutes() / 60;
        }

        // ==========================================
        // Display information
        // ==========================================

        unsigned int screenWidth() {
            return GetSystemMetrics(SM_CXSCREEN);
        }

        unsigned int screenHeight() {
            return GetSystemMetrics(SM_CYSCREEN);
        }

        unsigned int screenRefreshRate() {
            DEVMODEA mode{};
            mode.dmSize = sizeof(mode);

            if (!EnumDisplaySettingsA(
                nullptr,
                ENUM_CURRENT_SETTINGS,
                &mode
            ))
                return 0;

            return mode.dmDisplayFrequency;
        }

        // ==========================================
        // Environment
        // ==========================================

        std::string currentDirectory() {
            DWORD size = GetCurrentDirectoryA(0, nullptr);

            if (size == 0)
                return "";

            std::vector<char> buffer(size);

            if (GetCurrentDirectoryA(size, buffer.data()) == 0)
                return "";

            return buffer.data();
        }

        std::string temporaryDirectory() {
            char buffer[MAX_PATH]{};

            DWORD size = GetTempPathA(MAX_PATH, buffer);

            if (size == 0 || size >= MAX_PATH)
                return "";

            return buffer;
        }

        std::string environmentVariable(const std::string& name) {
            DWORD size = GetEnvironmentVariableA(
                name.c_str(), nullptr, 0
            );

            if (size == 0)
                return "";

            std::vector<char> buffer(size);

            if (GetEnvironmentVariableA(
                name.c_str(), buffer.data(), size
            ) == 0)
                return "";

            return buffer.data();
        }

        // ==========================================
        // Battery
        // ==========================================

        bool hasBattery() {
            SYSTEM_POWER_STATUS status{};

            if (!GetSystemPowerStatus(&status))
                return false;

            return status.BatteryFlag != 128;
        }

        double batteryPercent() {
            SYSTEM_POWER_STATUS status{};

            if (!GetSystemPowerStatus(&status))
                return 0.0;

            if (status.BatteryLifePercent == 255)
                return 0.0;

            return status.BatteryLifePercent;
        }

        bool isCharging() {
            SYSTEM_POWER_STATUS status{};

            if (!GetSystemPowerStatus(&status))
                return false;

            return status.ACLineStatus == 1;
        }

        // ==========================================
        // Network
        // ==========================================

        std::string hostname() {
            return computerName();
        }

        std::string localIPAddress() {
            WSADATA wsa{};

            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
                return "";

            char host[256]{};

            if (gethostname(host, sizeof(host)) != 0) {
                WSACleanup();
                return "";
            }

            addrinfo hints{};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            addrinfo* result = nullptr;

            if (getaddrinfo(host, nullptr, &hints, &result) != 0) {
                WSACleanup();
                return "";
            }

            std::string address;

            for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
                auto addr = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);

                char ip[INET_ADDRSTRLEN]{};

                if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip))) {
                    if (std::string(ip) != "127.0.0.1") {
                        address = ip;
                        break;
                    }
                }
            }

            freeaddrinfo(result);
            WSACleanup();

            return address;
        }

        // ==========================================
        // System state
        // ==========================================

        bool is64Bit() {
            return sizeof(void*) == 8;
        }

        bool isVirtualMachine() {
            int info[4]{};

            __cpuid(info, 1);

            // Hypervisor-present bit (bit 31 of ECX)
            return (info[2] & (1 << 31)) != 0;
        }

    }
}