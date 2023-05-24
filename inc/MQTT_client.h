#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "MQTT_packet.h"
#include "socket.h"

typedef int (*MQTT_client_callback)(MQTT_msgTypedef *);

typedef enum {
    DISCONNECTED = 0,
    CONNECTED,
    IN_LOOP
} MQTT_clientStatusTypedef;

typedef struct {
    MQTT_clientStatusTypedef status;
    const char *client_ID;
    const unsigned int keep_alive : 16;
    SOCKET socket;
    const char *addr;
    const int port;
    MQTT_client_callback callback;
} MQTT_clientHandleTypedef;

int MQTT_client_new(MQTT_clientHandleTypedef *hclient);
int MQTT_client_close(MQTT_clientHandleTypedef *hclient);
int MQTT_client_publish(MQTT_clientHandleTypedef *hclient, const char *topic, const char *content);
int MQTT_client_subscribe(MQTT_clientHandleTypedef *hclient, const char *topic);
int MQTT_client_unsubscribe(MQTT_clientHandleTypedef *hclient, const char *topic);
void MQTT_client_ping(MQTT_clientHandleTypedef *hclient);
int MQTT_client_decode_event(MQTT_clientHandleTypedef *hclient, MQTT_msgTypedef *msg);
void MQTT_client_loop(MQTT_clientHandleTypedef *hclient, MQTT_msgTypedef *msg);

#endif