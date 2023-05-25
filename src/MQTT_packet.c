#include "MQTT_packet.h"
#include "log.h"
#include "stdlib.h"
#include <string.h>

#define PACKET_FIXED_HEADER_SIZE                2
#define PACKET_VARIABLE_HEADER_POSITION         2
#if defined(MQTT_VERSION_3_1)
#define CONNECT_PACKET_VARIABLE_HAEDER_SIZE     12
#elif defined(MQTT_VERSION_3_1_1)
#define CONNECT_PACKET_VARIABLE_HAEDER_SIZE     10
#endif
#define MAX_PACKET_CLIENT_ID_SIZE               23
#define MAX_PACKET_SIZE                         128

#if defined(MQTT_VERSION_3_1)
#define CONNECT_PACKET_PAYLOAD_POSITION         14   
#elif defined(MQTT_VERSION_3_1_1)
#define CONNECT_PACKET_PAYLOAD_POSITION         12
#endif

#define CONNECT_FLAG_USER_NAME                  (0x01 << 7)
#define CONNECT_FLAG_PASSWORD                   (0x01 << 6)
#define CONNECT_FLAG_WILL_REMAIN                (0x01 << 5)
#define CONNECT_FLAG_WILL_FLAG                  (0x01 << 2)
#define CONNECT_FLAG_CLEAN_SESSION              (0x01 << 1)

#define SUBSCRIBE_PACKET_VARIABLE_HAEDER_SIZE   2
#define SUBSCRIBE_PACKET_PAYLOAD_POSITION       4

#define UNSUBSCRIBE_PACKET_VARIABLE_HAEDER_SIZE 2
#define UNSUBSCRIBE_PACKET_PAYLOAD_POSITION     4

#define PACKET_UTF_STR_LEN(a) (strlen(a) + 2)

static int write_packet_utf_string(char *buf, const char *str)
{
    unsigned int len = strlen(str);
    buf[0] = 0x00;
    buf[1] = (char)len;
    memcpy(buf + 2, str, len);
    return (len + 2);
}

static int read_packet_utf_string(char *buf, const char *str)
{
    int len = ((unsigned char)str[0] << 8) | (unsigned char)str[1];
    if (len > MQTT_PACKET_BUFFER_SIZE) {
        MQTT_LOG("Message error!, length = %d\n", len);
        return 0;
    }
    memcpy(buf, str+2, len);
    return len;
}

int MQTT_create_connect_packet(MQTT_connectConfigTypedef *config, char *packet_buf)
{
    unsigned int packet_length, remaining_length;
    char payload_length = 0;
    char connect_flags = 0;
    char *variable_header_ptr = &packet_buf[PACKET_VARIABLE_HEADER_POSITION];
    char *payload_ptr = &packet_buf[CONNECT_PACKET_PAYLOAD_POSITION];

    // Verify packet length
    remaining_length = CONNECT_PACKET_VARIABLE_HAEDER_SIZE + PACKET_UTF_STR_LEN(config->client_ID);
    if (config->will_topic != NULL) {
        remaining_length += PACKET_UTF_STR_LEN(config->will_topic);
        remaining_length += PACKET_UTF_STR_LEN(config->will_msg);
    }
    if (config->will_topic != NULL) {
        remaining_length += PACKET_UTF_STR_LEN(config->user_name);
        if (config->password != NULL) {
            remaining_length += PACKET_UTF_STR_LEN(config->password);
        }
    }
    packet_length = PACKET_FIXED_HEADER_SIZE + remaining_length;
    if (packet_length > MQTT_PACKET_BUFFER_SIZE) {
        MQTT_LOG("Packet too large\n");
        return -1;
    }
    
    // Fixed header
    packet_buf[0] = COMMAND_TYPE_CONNECT << 4;
    packet_buf[1] = remaining_length;

    // Variable header
#if defined(MQTT_VERSION_3_1)
    variable_header_ptr += write_packet_utf_string(variable_header_ptr, "MQIsdp");
    variable_header_ptr[0] = 0x03;
#elif defined(MQTT_VERSION_3_1_1)
    variable_header_ptr += write_packet_utf_string(variable_header_ptr, "MQTT");
    variable_header_ptr[0] = 0x04;
#endif
    variable_header_ptr[1] = config->keep_alive >> 8;
    variable_header_ptr[2] = config->keep_alive & 0xFF;

    // Payload
    payload_ptr += write_packet_utf_string(payload_ptr, config->client_ID);
    if (config->will_topic != NULL) {
        payload_ptr += write_packet_utf_string(payload_ptr, config->will_topic);
        payload_ptr += write_packet_utf_string(payload_ptr, config->will_msg);
        connect_flags |= CONNECT_FLAG_WILL_FLAG;
        connect_flags |= config->will_QoS << 3;
    }
    if (config->user_name != NULL) {
        payload_ptr += write_packet_utf_string(payload_ptr, config->user_name);
        connect_flags |= CONNECT_FLAG_USER_NAME;
        if (config->password != NULL) {
            payload_ptr += write_packet_utf_string(payload_ptr, config->password);
            connect_flags |= CONNECT_FLAG_PASSWORD;
        }
    }
    if (config->will_retian == true) {
        connect_flags |= CONNECT_FLAG_WILL_REMAIN;
    }
    if (config->clean_session == true) {
        connect_flags |= CONNECT_FLAG_CLEAN_SESSION;
    }

    return packet_length;
}

int MQTT_handle_connack_packet(const char *packet, unsigned int len)
{
    if (len != 4) { // The packet size should be 4 bytes
        MQTT_LOG("Bad packet length %u", len);
        return -1;
    }
    if (packet[0] != (COMMAND_TYPE_CONNACK << 4)) {
        MQTT_LOG("Bad command type %u", packet[0]);
    }
    return (unsigned int)(packet[3]);
}

int MQTT_create_publish_packet(MQTT_publishConfigTypedef *config, char *packet_buf)
{
    unsigned int packet_length = 0;
    unsigned int remaining_length = 0;
    char *ptr;

    // Check packet length
    remaining_length = PACKET_UTF_STR_LEN(config->topic);
    if (config->QoS_level > 0) {
        remaining_length += 2; // Packet ID length
    }
    remaining_length += PACKET_UTF_STR_LEN(config->payload);
    packet_length = PACKET_FIXED_HEADER_SIZE + remaining_length;
    if (packet_length > MQTT_PACKET_BUFFER_SIZE) {
        MQTT_LOG("Packet too large\n");
        return -1;
    }

    // Fixed header
    packet_buf[0] = (COMMAND_TYPE_PUBLISH << 4) | (config->DUP_flag << 3) | (config->QoS_level << 1) | (config->retian);
    packet_buf[1] = (char)remaining_length;

    // Variable header
    ptr = &packet_buf[PACKET_VARIABLE_HEADER_POSITION];
    ptr += write_packet_utf_string(ptr, config->topic);
    if (config->QoS_level > 0) {
        ptr[0] = config->packet_ID >> 8;
        ptr[1] = config->packet_ID;
        ptr += 2;
    }

    // Payload
    write_packet_utf_string(ptr, config->payload);

    return packet_length;
}

int MQTT_handle_puback_packet(const char *packet, unsigned int len)
{
    if (len != 4) {
        MQTT_LOG("Bad packet length\n");
        return -1;
    }
    if (packet[0] != (COMMAND_TYPE_PUBACK << 4)) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }
    return 0;
}

int MQTT_handle_pubrec_packet(const char *packet, unsigned int len, unsigned int *packet_ID)
{
    if (len != 4) {
        MQTT_LOG("Bad packet length\n");
        return -1;
    }
    if (packet[0] != (COMMAND_TYPE_PUBREC << 4)) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }
    *packet_ID = (packet[2] << 8) | packet[3];
    return 0;
}

int MQTT_create_pubrel_packet(MQTT_pubrelConfigTypedef *config, char *packet_buf)
{
    packet_buf[0] = COMMAND_TYPE_PUBREL | 0x02;
    packet_buf[1] = 0x02;
    packet_buf[2] = config->packet_ID >> 8;
    packet_buf[3] = config->packet_ID | 0xFF;
    return 4;
}

int MQTT_handle_pubcomp_packet(const char *packet, unsigned int len)
{
    if (len != 4) {
        MQTT_LOG("Bad packet length\n");
        return -1;
    }
    if (packet[0] != (COMMAND_TYPE_PUBCOMP << 4)) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }
    return 0;
}

int MQTT_create_subscribe_packet(MQTT_subscribeConfigTypedef *config, char *packet_buf)
{
    unsigned int packet_length;
    unsigned int remaining_length;
    unsigned int payload_length;
    char *payload_ptr;

    // Check packet length
    payload_length = PACKET_UTF_STR_LEN(config->topic) + 1;
    remaining_length = SUBSCRIBE_PACKET_VARIABLE_HAEDER_SIZE + payload_length;
    packet_length = PACKET_FIXED_HEADER_SIZE + remaining_length;
    if (packet_length > MQTT_PACKET_BUFFER_SIZE) {
        MQTT_LOG("Packet too large\n");
        return -1;
    }

    // Fixed header
    packet_buf[0] = (char)(COMMAND_TYPE_SUCSCRIBE << 4) | (char)0x02;
    packet_buf[1] = remaining_length & 0xFF;

    // Variable header
    packet_buf[2] = config->packet_ID >> 8;
    packet_buf[3] = config->packet_ID & 0xFF;

    // payload
    payload_ptr = &packet_buf[SUBSCRIBE_PACKET_PAYLOAD_POSITION];
    payload_ptr += write_packet_utf_string(payload_ptr, config->topic);
    payload_ptr[0] = config->topic_QoS;
    
    return packet_length;
}

int MQTT_handle_suback_packet(const char *packet, unsigned int len)
{
    if (len != 5) {
        MQTT_LOG("Bad packet length\n");
        return -1;
    }
    if (packet[0] != (const char)(COMMAND_TYPE_SUBACK << 4)) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }
    
    return packet[4];
}

int MQTT_create_unsubscribe_packet(MQTT_unsubscribeConfigTypedef *config, char *packet_buf)
{
    unsigned int packet_length;
    unsigned int remaining_length;
    unsigned int payload_length;
    char *payload_ptr = &packet_buf[UNSUBSCRIBE_PACKET_PAYLOAD_POSITION];
    
    // Check packet length
    payload_length = PACKET_UTF_STR_LEN(config->topic);
    remaining_length = UNSUBSCRIBE_PACKET_VARIABLE_HAEDER_SIZE + payload_length;
    packet_length = PACKET_FIXED_HEADER_SIZE + remaining_length;
    if (packet_length > MQTT_PACKET_BUFFER_SIZE) {
        MQTT_LOG("Packet too large\n");
        return -1;
    }

    // Fixed header
    packet_buf[0] = (char)(COMMAND_TYPE_UNSUBSCRIBE << 4 | 0x02);
    packet_buf[1] = remaining_length;

    // Variable header
    packet_buf[2] = config->packet_ID >> 8;
    packet_buf[3] = config->packet_ID & 0xFF;

    // Payload
    write_packet_utf_string(payload_ptr, config->topic);

    return packet_length;
}

int MQTT_handle_unsuback_packet(const char *packet, unsigned int len)
{
    if (len != 4) {
        MQTT_LOG("Bad packet length\n");
        return -1;
    }
    if (packet[0] != (const char)(COMMAND_TYPE_UNSUBACK << 4)) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }
    return 0;
}

int MQTT_create_pingreq_packet(char *packet_buf)
{
    packet_buf[0] = (char)(COMMAND_TYPE_PINGREQ << 4);
    packet_buf[1] = 0x00;
    return 2;
}

int MQTT_create_pingresp_packet(char *packet_buf)
{
    packet_buf[0] = (char)(COMMAND_TYPE_PINGRESP << 4);
    packet_buf[1] = 0x00;
    return 2;
}

int MQTT_handle_pingresp_packet(const char *packet, unsigned int len)
{
    if (len != 4) {
        MQTT_LOG("Bad packet length\n");
        return -1;
    }
    if (packet[0] != (const char)(COMMAND_TYPE_PINGRESP << 4)) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }
    return 0;
}

int MQTT_create_disconnect_packet(char *packet_buf)
{
    packet_buf[0] = (char)(COMMAND_TYPE_DISCONNECT << 4);
    packet_buf[1] = 0x00;
    return 2;
}

int MQTT_handle_publish_packet(const char *packet, unsigned len, MQTT_msgTypedef *msg)
{
    char *packet_ptr = (char *)packet;

    // Verify header
    if (packet_ptr[0] >> 4 != COMMAND_TYPE_PUBLISH) {
        MQTT_LOG("Bad command type\n");
        return -1;
    }

    msg->QoS = (packet_ptr[0] & 0x06) >> 1;
    packet_ptr += 2;
    if (msg->QoS > 0) {
        msg->packet_ID = (packet_ptr[0] << 8) | packet_ptr[1];
        packet_ptr += 2;
    }
    
    // Decode payload
    msg->topic_len = read_packet_utf_string(msg->topic, packet_ptr);
    packet_ptr += msg->topic_len + 2;
    msg->msg_len = read_packet_utf_string(msg->msg, packet_ptr);

    return 0;
}

int MQTT_create_puback_packet(MQTT_pubackConfigTypedef *config, char *packet_buf)
{
    packet_buf[0] = COMMAND_TYPE_PUBACK << 4;
    packet_buf[1] = 0x20;
    packet_buf[2] = config->packet_ID >> 8;
    packet_buf[3] = config->packet_ID & 0xFF;
    return 4;
}