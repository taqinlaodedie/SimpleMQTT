#ifndef __LOG_H__
#define __LOG_H__

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define MQTT_DEBUG
#if defined(MQTT_DEBUG)
    #define MQTT_LOG(fmt, ...) \
                MQTT_printf("%s:%d->%s():", __FILE__, __LINE__, __FUNCTION__); \
                MQTT_printf(fmt, ##__VA_ARGS__);
#elif
    #define MQTT_LOG(fmt, ...)
#endif

int MQTT_printf(const char *fmt, ...);

#endif