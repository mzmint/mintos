#include "MintLibClock.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace Mint {

    namespace {

        // Get the current system clock time
        auto getNow() {
            return std::chrono::system_clock::now();
        }

        // Convert a timestamp to local calendar time
        std::tm getLocalTm(std::time_t timestamp) {
            std::tm result{};

#ifdef _WIN32
            localtime_s(&result, &timestamp);
#else
            localtime_r(&timestamp, &result);
#endif

            return result;
        }

        // Get Unix timestamp in milliseconds
        long long getUnixMs(
            std::chrono::system_clock::time_point now
        ) {
            return std::chrono::duration_cast<
                std::chrono::milliseconds
            >(now.time_since_epoch()).count();
        }

        // Format date
        std::string formatDate(
            const std::tm& t,
            int format
        ) {
            std::ostringstream out;

            switch (format) {
            case 0:
                out << std::put_time(&t, "%d/%m/%Y");
                break;

            case 1:
                out << std::put_time(&t, "%m/%d/%Y");
                break;

            case 2:
                out << std::put_time(&t, "%Y/%m/%d");
                break;

            default:
                throw std::invalid_argument(
                    "Invalid date format"
                );
            }

            return out.str();
        }

        // Format time
        std::string formatTime(
            const std::tm& t,
            int format
        ) {
            std::ostringstream out;

            switch (format) {
            case 0:
                out << std::put_time(&t, "%H:%M:%S");
                break;

            case 1:
                out << std::put_time(&t, "%I:%M:%S %p");
                break;

            default:
                throw std::invalid_argument(
                    "Invalid time format"
                );
            }

            return out.str();
        }

        // Get the millisecond fraction
        int getMilliseconds(long long timestampMs) {
            return static_cast<int>(
                (timestampMs % 1000 + 1000) % 1000
            );
        }

        // Format time with optional milliseconds
        std::string makeTime(
            bool milliseconds,
            int format
        ) {
            auto now = getNow();
            long long ms = getUnixMs(now);

            std::time_t timestamp =
                std::chrono::system_clock::to_time_t(now);

            std::tm t = getLocalTm(timestamp);
            std::string result = formatTime(t, format);

            if (milliseconds) {
                std::ostringstream out;

                out << result << '.'
                    << std::setw(3)
                    << std::setfill('0')
                    << getMilliseconds(ms);

                result = out.str();
            }

            return result;
        }

        // Format date
        std::string makeDate(int format) {
            auto now = getNow();

            std::time_t timestamp =
                std::chrono::system_clock::to_time_t(now);

            std::tm t = getLocalTm(timestamp);

            return formatDate(t, format);
        }

        // Format date and time together
        std::string makeDateTime(
            bool milliseconds,
            int dateFormat,
            int timeFormat
        ) {
            auto now = getNow();
            long long ms = getUnixMs(now);

            std::time_t timestamp =
                std::chrono::system_clock::to_time_t(now);

            std::tm t = getLocalTm(timestamp);

            std::ostringstream out;

            out << formatDate(t, dateFormat)
                << ' '
                << formatTime(t, timeFormat);

            if (milliseconds) {
                out << '.'
                    << std::setw(3)
                    << std::setfill('0')
                    << getMilliseconds(ms);
            }

            return out.str();
        }

    }

    // Unix time
    long long unixLocalTime() {
        return std::chrono::duration_cast<
            std::chrono::seconds
        >(getNow().time_since_epoch()).count();
    }

    long long unixLocalTimeMs() {
        return getUnixMs(getNow());
    }

    // Time
    std::string localTime(int format) {
        return makeTime(false, format);
    }

    std::string localTimeMs(int format) {
        return makeTime(true, format);
    }

    // Date
    std::string localDate(int format) {
        return makeDate(format);
    }

    std::string localDateMs(int format) {
        return makeDate(format);
    }

    // Date and time
    std::string localDateTime(
        int dateFormat,
        int timeFormat
    ) {
        return makeDateTime(
            false, dateFormat, timeFormat
        );
    }

    std::string localDateTimeMs(
        int dateFormat,
        int timeFormat
    ) {
        return makeDateTime(
            true, dateFormat, timeFormat
        );
    }

}
