#include "MQTT_client.h"
#include <stdio.h>
#include <windows.h>

int MQTT_callback(MQTT_msgTypedef *msg)
{
    if (msg->topic_len > 0) {
        printf("Message received, topic: ");
        for (int i = 0; i < *msg->topic_len; i++) {
            printf("%c", msg->topic[i]);
        }
        printf("; message: ");
        for (int i = 0; i < *msg->msg_len; i++) {
            printf("%c", msg->msg[i]);
        }
        printf(".\n");
    }

    return 0;
}

int main()
{
    MQTT_msgTypedef msg;

    MQTT_clientHandleTypedef client = {
        .status = DISCONNECTED,
        .addr = "127.0.0.1",
        .port = 1883,
        .keep_alive = 30,
        .client_ID = "BeastSenpai",
        .callback = MQTT_callback
    };
    printf("Start.. Addr=%s:%d\n", client.addr, client.port);
    if (MQTT_client_new(&client)) {
        printf("MQTT client create failed\n");
        return -1;
    }
    printf("Connected..\n");
    if (MQTT_client_subscribe(&client, "114514")) {
        printf("MQTT subscribe failed\n");
        return -1;
    }
    printf("Subscribed..\n");
    if (MQTT_client_publish(&client, "114514", "1919810")) {
        printf("MQTT publish failed\n");
        return -1;
    }
    printf("Published..\n");
    MQTT_client_loop(&client, &msg);

    return 0;
}