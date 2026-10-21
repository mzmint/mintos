#pragma once

#include <string>

namespace Mint {
	long long unixLocalTime();
	long long unixLocalTimeMs();
	std::string localTime(int format = 0);
	std::string localTimeMs(int format = 0);
	std::string localDate(int format = 0);
	std::string localDateMs(int format = 0);
	std::string localDateTime(int dateFormat = 0, int timeFormat = 0);
	std::string localDateTimeMs(int dateFormat = 0, int timeFormat = 0);
}