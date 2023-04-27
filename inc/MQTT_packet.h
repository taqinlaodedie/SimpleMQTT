#ifndef __MQTT_PACKET_H__
#define __MQTT_PQCKET_H__

#include <stdbool.h>

// #define MQTT_VERSION_3_1
#define MQTT_VERSION_3_1_1

// Command types
#define COMMAND_TYPE_RESERVED_0     0x0
#define COMMAND_TYPE_CONNECT        0x1
#define COMMAND_TYPE_CONNACK        0x2
#define COMMAND_TYPE_PUBLISH        0x3
#define COMMAND_TYPE_PUBACK         0x4
#define COMMAND_TYPE_PUBREC         0x5
#define COMMAND_TYPE_PUBREL         0x6
#define COMMAND_TYPE_PUBCOMP        0x7
#define COMMAND_TYPE_SUCSCRIBE      0x8
#define COMMAND_TYPE_SUBACK         0x9
#define COMMAND_TYPE_UNSUBSCRIBE    0xA
#define COMMAND_TYPE_UNSUBACK       0xB
#define COMMAND_TYPE_PINGREQ        0xC
#define COMMAND_TYPE_PINGRESP       0xD
#define COMMAND_TYPE_DISCONNECT     0xE
#define COMMAND_TYPE_RESERVED_15    0xF

typedef struct {
    const char *client_ID;
    const char *user_name;
    const char *password;
    const bool will_retian : 1;
    const bool clean_session : 1;
    unsigned int will_QoS : 2; 
    unsigned int keep_alive : 16;
    const char *will_topic;
    const char *will_msg;
} MQTT_connectConfigTypedef;

typedef struct {
    const bool DUP_flag : 1;
    const unsigned int QoS_level: 2;
    const bool retian : 1;
    const unsigned int packet_ID : 16;
    const char *topic;
    const char *payload;
} MQTT_publishConfigTypedef;

typedef struct {
    const unsigned int packet_ID : 16;
} MQTT_pubrelConfigTypedef;

typedef struct {
    const unsigned int packet_ID : 16;
    const bool retain : 1;
    const unsigned int topic_QoS : 2;
    const char *topic;
} MQTT_subscribeConfigTypedef;

typedef struct {
    const unsigned int packet_ID : 16;
    const char *topic;
} MQTT_unsubscribeConfigTypedef;

typedef struct {
    unsigned int packet_ID : 16;
} MQTT_pubackConfigTypedef;

typedef struct {
    char topic[128];
    char msg[128];
    unsigned int packet_ID : 16;
    unsigned int topic_len : 8;
    unsigned int msg_len : 8;
    unsigned int QoS : 2;
} MQTT_msgTypedef;

// Create packets to send, the packet_buf should be a 128 bytes array
int MQTT_create_connect_packet(MQTT_connectConfigTypedef *config, unsigned char *packet_buf);
int MQTT_create_publish_packet(MQTT_publishConfigTypedef *config, unsigned char *packet_buf);
int MQTT_create_pubrel_packet(MQTT_pubrelConfigTypedef *config, unsigned char *packet_buf);
int MQTT_create_subscribe_packet(MQTT_subscribeConfigTypedef *config, unsigned char *packet_buf);
int MQTT_create_unsubscribe_packet(MQTT_unsubscribeConfigTypedef *config, unsigned char *packet_buf);
int MQTT_create_pingreq_packet(unsigned char *packet_buf);
int MQTT_create_disconnect_packet(unsigned char *packet_buf);
int MQTT_create_puback_packet(MQTT_pubackConfigTypedef *config, unsigned char *packet_buf);

// Handle received packets, returns 0 if OK
int MQTT_handle_connack_packet(const unsigned char *packet, unsigned int len);
int MQTT_handle_puback_packet(const unsigned char *packet, unsigned int len);
int MQTT_handle_pubrec_packet(const unsigned char *packet, unsigned int len, unsigned int *packet_ID);
int MQTT_handle_pubcomp_packet(const unsigned char *packet, unsigned int len);
int MQTT_handle_suback_packet(const unsigned char *packet, unsigned int len);
int MQTT_handle_unsuback_packet(const unsigned char *packet, unsigned int len);
int MQTT_handle_pingresp_packet(const unsigned char *packet, unsigned int len);
int MQTT_handle_public_packet(const unsigned char *packet, unsigned len, MQTT_msgTypedef *msg);

#endif