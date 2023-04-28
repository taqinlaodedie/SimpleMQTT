#include "MQTT_client.h"
#include "string.h"
#include "log.h"

static unsigned char packet_buf[MQTT_PACKET_BUFFER_SIZE];

int MQTT_client_new(MQTT_clientHandleTypedef *hclient)
{
    if (hclient->status != DISCONNECTED) {
        MQTT_LOG("Already connected\n");
        return 0;
    }

    if (hclient->socket == 0) { // socket not initialized, do the initialization
        hclient->socket = socket_new(hclient->addr, strlen(hclient->addr), hclient->port);
        if (hclient->socket < 0) {
            MQTT_LOG("Failed to create socket\n");
            return -1;
        }
    }

    MQTT_connectConfigTypedef connect_config = {
        .clean_session = false,
        .client_ID = hclient->client_ID,
        .keep_alive = hclient->keep_alive,
        .user_name = NULL,
        .password = NULL,
        .will_QoS = 0,
        .will_topic = NULL,
        .will_msg = NULL,
        .will_retian = false
    };
    int packet_len = MQTT_create_connect_packet(&connect_config, packet_buf);
    if (packet_len < 0) {
        MQTT_LOG("Failed to create CONNECT packet\n");
        return -1;
    }
    if (socket_send(hclient->socket, packet_buf, packet_len) != 0) {
        MQTT_LOG("Failed to send CONNECT packet\n");
        return -1;
    }
    packet_len = socket_recv(hclient->socket, packet_buf, MQTT_PACKET_BUFFER_SIZE);
    if (packet_len <= 0) {
        MQTT_LOG("Socket receive err with %d", packet_len);
        return -1;
    }
    if (MQTT_handle_connack_packet(packet_buf, packet_len) != 0) {
        MQTT_LOG("Failed to get CONNACK packet\n");
        return -1;
    }

    return 0;
}

int MQTT_client_close(MQTT_clientHandleTypedef *hclient)
{
    int packet_len;
    if (hclient->status == DISCONNECTED) {
        MQTT_LOG("Already disconnected\n");
        return 0;
    }
    packet_len = MQTT_create_disconnect_packet(packet_buf);
    if (packet_len < 0) {
        MQTT_LOG("Failed to create DISCONNECT packet\n");
        return -1;
    }
    if (socket_send(hclient->socket, packet_buf, packet_buf) != 0) {
        MQTT_LOG("Failed to send DISCONNECT packet\n");
        return -1;
    }
    return 0;
}

int MQTT_client_publish(MQTT_clientHandleTypedef *hclient, const char *topic, const char *content)
{
    if (hclient->status == DISCONNECTED) {
        MQTT_LOG("Client not connected\n");
        return -1;
    }
    
    MQTT_publishConfigTypedef publish_config = {
        .DUP_flag = false,
        .QoS_level = 0,
        .retian = false,
        .packet_ID = 0,
        .topic = topic,
        .payload = content
    };
    int packet_len = MQTT_create_publish_packet(&publish_config, packet_buf);
    if (packet_len < 0) {
        MQTT_LOG("Failed to creat PUBLISH packet\n");
        return -1;
    }
    if (socket_send(hclient->socket, packet_buf, packet_len != 0)) {
        MQTT_LOG("Failed to send PUBLISH packet\n");
        return -1;
    }

    return 0;
}

int MQTT_client_subscribe(MQTT_clientHandleTypedef *hclient, const char *topic)
{
    if (hclient->status == DISCONNECTED) {
        MQTT_LOG("Client not connected\n");
        return -1;
    }

    MQTT_subscribeConfigTypedef subscribe_config = {
        .packet_ID = 0x10,
        .retain = false,
        .topic_QoS = 0,
        .topic = topic
    };
    int packet_len = MQTT_create_subscribe_packet(&subscribe_config, packet_buf);
    if (packet_len < 0) {
        MQTT_LOG("Failed to create SUBSCRIBE packet\n");
        return -1;
    }
    if (socket_send(hclient->socket, packet_buf, packet_len != 0)) {
        MQTT_LOG("Failed to send SUBSCRIBE packet\n");
        return -1;
    }
    packet_len = socket_recv(hclient->socket, packet_buf, MQTT_PACKET_BUFFER_SIZE);
    if (packet_len <= 0) {
        MQTT_LOG("Socket receive err with %d", packet_len);
        return -1;
    }
    if (MQTT_handle_suback_packet(packet_buf, packet_len) != 0) {
        MQTT_LOG("Failed to get SUBACK packet\n");
        return -1;
    }

    return 0;
}

int MQTT_client_unsubscribe(MQTT_clientHandleTypedef *hclient, const char *topic)
{
    if (hclient->status == DISCONNECTED) {
        MQTT_LOG("Client not connected\n");
        return -1;
    }

    MQTT_subscribeConfigTypedef unsubscribe_config = {
        .packet_ID = 0x10,
        .retain = false,
        .topic_QoS = 0,
        .topic = topic
    };
    int packet_len = MQTT_create_unsubscribe_packet(&unsubscribe_config, packet_buf);
    if (packet_len < 0) {
        MQTT_LOG("Failed to create UNSUBSCRIBE packet\n");
        return -1;
    }
    if (socket_send(hclient->socket, packet_buf, packet_len != 0)) {
        MQTT_LOG("Failed to send UNSUBSCRIBE packet\n");
        return -1;
    }
    packet_len = socket_recv(hclient->socket, packet_buf, MQTT_PACKET_BUFFER_SIZE);
    if (packet_len <= 0) {
        MQTT_LOG("Socket receive err with %d", packet_len);
        return -1;
    }
    if (MQTT_handle_unsuback_packet(packet_buf, packet_len) != 0) {
        MQTT_LOG("Failed to get UNSUBACK packet\n");
        return -1;
    }

    return 0;
}

int MQTT_client_decode_event(MQTT_clientHandleTypedef *hclient, MQTT_msgTypedef *msg)
{
    if (hclient->status == DISCONNECTED) {
        MQTT_LOG("Client not connected\n");
        return -1;
    }

    int packet_len = socket_recv(hclient->socket, packet_buf, MQTT_PACKET_BUFFER_SIZE);
    if (packet_len <= 0) {
        MQTT_LOG("Socket receive err with %d", packet_len);
        return -1;
    }

    if (MQTT_handle_publish_packet(packet_buf, packet_len, msg) != 0) {
        MQTT_LOG("Failed to decode message\n");
        return -1;
    }

    return 0;
}

void MQTT_client_loop(MQTT_clientHandleTypedef *hclient, MQTT_msgTypedef *msg)
{
    if (hclient->status == DISCONNECTED) {
        MQTT_LOG("Client not connected\n");
        return;
    }

    hclient->status = IN_LOOP;
    while (hclient->status == IN_LOOP) {
        MQTT_client_decode_event(hclient, msg);
        hclient->callback(hclient, msg);
    }
}