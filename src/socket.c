#include "socket.h"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#endif
#include "log.h"

SOCKET socket_new(const char* addr, size_t addr_len, int port)
{
    SOCKET tcp_socket = -1;
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsadata;
    unsigned long iMode = 1; // Non blocking mode
    struct sockaddr_in sockAdrr;
    fd_set fs;
    int iErrorNo;
    int iLen;
    TIMEVAL tv = {
        .tv_sec = 1,
        .tv_usec = 0
    };
    int err = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (err) {
        MQTT_LOG("WSA startup failed with code %d\n", err);
        WSACleanup();
        return -1;
    }
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == INVALID_SOCKET) {
        MQTT_LOG("Failed to create socket\n");
        return -1;
    }

    sockAdrr.sin_family = AF_INET;
    sockAdrr.sin_addr.s_addr = inet_addr(addr);
    sockAdrr.sin_port = htons(port);
    err = connect(tcp_socket, (struct sockaddr *)&sockAdrr, sizeof(sockAdrr));
    if (err == SOCKET_ERROR) {
        MQTT_LOG("Failed to connect to server, err = %d\n", err);
        return -1;
    }
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.
    err = ioctlsocket(tcp_socket, FIONBIO, &iMode);
    if (err != NO_ERROR) {
        MQTT_LOG("ioctlsocket failed with error: %ld\n", err);
        return -1;
    }
    
    while (1) {
        iErrorNo = SOCKET_ERROR;
        iLen = sizeof(int);
        FD_ZERO(&fs);
        FD_SET(tcp_socket, &fs);
        err = select(1, NULL, &fs, NULL, &tv); // Check if the connection is ready
        if (err > 0) {
            MQTT_LOG("Successfully connected to server\n");
            break;
        }
    }
    
#endif
    return tcp_socket;
}

/* If no error occurs, the total number of bytes sent should be returned. */
int socket_send(SOCKET tcp_socket, const char *data, int len)
{
    int err;
#if defined(_WIN32) || defined(_WIN64)
    fd_set fs;
    TIMEVAL tv = {
        .tv_sec = 1,
        .tv_usec = 0
    };
    while(1) {
        FD_ZERO(&fs);
        FD_SET(tcp_socket, &fs);
        err = select(1, NULL, &fs, NULL, &tv);
        if (err > 0) {
            err = send(tcp_socket, data, len, 0);
            break;
        }
    }
    if (err == SOCKET_ERROR) {
        return -1;
    }
    if (err != len) {
        MQTT_LOG("%d bytes sent, expected %d\n");
        return -1;
    }
    err = 0;
#endif
    return err;
}

/* If no error occurs, the number of bytes received should be returned and the buffer pointed to by the data parameter will contain this data received. 
If there is nothing to read, return 0 */
int socket_recv(SOCKET tcp_socket, char *data, int buf_len)
{
    int packet_len = 0;
#if defined(_WIN32) || defined(_WIN64)
    fd_set fs;
    TIMEVAL tv = {
        .tv_sec = 1,
        .tv_usec = 0
    };
    FD_ZERO(&fs);
    FD_SET(tcp_socket, &fs);
    if (select(1, &fs, NULL, NULL, &tv) > 0)
        packet_len = recv(tcp_socket, data, buf_len, 0);
    if (packet_len == SOCKET_ERROR) {
        MQTT_LOG("recv error: %d\n", WSAGetLastError());
        closesocket(tcp_socket);
        WSACleanup();
    }
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