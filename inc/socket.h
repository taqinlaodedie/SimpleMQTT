#ifndef __SOCKET_H__
#define __SOCKET_H__

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#endif
#include <stdio.h>

#define SOCKET int

SOCKET socket_new(const char* addr, size_t addr_len, int port);
int socket_send(SOCKET tcp_socket, const char *data, int len);
int socket_recv(SOCKET tcp_socket, char *data, int buf_len);
void socket_delete(SOCKET tcp_socket);

#endif