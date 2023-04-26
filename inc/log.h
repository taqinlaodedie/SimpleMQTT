#ifndef __LOG_H__
#define __LOG_H__

#define MQTT_DEBUG
#if defined(MQTT_DEBUG)
    #define MQTT_LOG(fmt, ...) \
                MQTT_printf("%s:%d->%s", ##fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#elif
    #define MQTT_LOG(fmt, ...)
#endif

int MQTT_printf(const char *fmt, ...);

#endif