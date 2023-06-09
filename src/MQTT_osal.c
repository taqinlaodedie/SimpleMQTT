#include "MQTT_osal.h"
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// Get timestamp in milliseconds
unsigned long long MQTT_timestamp()
{
    unsigned long timestamp;
#if defined(_WIN32) || defined(_WIN64)
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);
    timestamp = ((unsigned long long )file_time.dwHighDateTime << 32) + (unsigned long long)file_time.dwLowDateTime;
    timestamp /= 10000;
#endif
    return timestamp;
}