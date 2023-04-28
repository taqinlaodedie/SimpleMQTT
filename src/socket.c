#include "socket.h"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#endif
#include "log.h"

SOCKET socket_new(const char* addr, size_t addr_len, int port)
{
    SOCKET tcp_socket;
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsadata;
    struct sockaddr_in sockAdrr;
    int err = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (err) {
        MQTT_LOG("WSA startup failed with code %d\n", err);
        return -1;
    }
    tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_socket == INVALID_SOCKET) {
        MQTT_LOG("Failed to create socket\n");
        return -1;
    }
    sockAdrr.sin_family = AF_INET;
    sockAdrr.sin_addr.s_addr = inet_addr(addr);
    sockAdrr.sin_port = htons(port);
    err = connect(tcp_socket, (struct sockaddr *)&sockAdrr, sizeof(sockAdrr));
    if (err != 0) {
        MQTT_LOG("Failed to connect to server, err = %d\n", err);
        return -1;
    }
#endif
    return tcp_socket;
}

/* If no error occurs, the total number of bytes sent should be returned. */
int socket_send(SOCKET tcp_socket, const char *data, int len)
{
    int err;
#if defined(_WIN32) || defined(_WIN64)
    err = send(tcp_socket, data, len, 0);
#endif
    return err;
}

/* If no error occurs, the number of bytes received should be returned and the buffer pointed to by the data parameter will contain this data received. 
If the connection has been gracefully closed, the return value is zero. */
int socket_recv(SOCKET tcp_socket, char *data, int buf_len)
{
    int packet_len;
#if defined(_WIN32) || defined(_WIN64)
    packet_len = recv(tcp_socket, data, buf_len, 0);
#endif
    return packet_len;
}

void socket_delete(SOCKET tcp_socket)
{
#if defined(_WIN32) || defined(_WIN64)
    closesocket(tcp_socket);
    WSACleanup();
#endif
}