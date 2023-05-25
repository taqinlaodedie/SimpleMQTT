# SimpleMQTT
## Overview
This is a light weight MQTT client library based on TCP IP socket for embedded platforms. The library supports commonly used functions like publish, subscribe and ping for the MQTT protocol.The library supports 3.1 and 3.1.1 version of MQTT for now. Once the TCP socket APIs are implemented, the client should be able to work.

## Porting
The followiing functions should be implemented to make the client work:

### In socket.c:
1. **socket_new()**: This function creates a TCP socket with the destination IP address, its length on bytes and the port number as parameters. The return value should be a positive integer which represents the socket ID if no error occurs, else a negative error code should be returned.
2. **socket_send()**: This function sends data to the server which takes the socket ID, the data buffer and its length as parameters. If no error occurs, the total number of bytes sent should be returned.
3. **socket_recv()**: This function receives data from the server which takes the socket ID, the data buffer and its maximum length as parameters. If no error occurs, the number of bytes received should be returned and the buffer pointed to by the data parameter will contain this data received. If there is nothing to read, return 0.

### In log.c:
1. **__write()**: This is the redirection function of *putc()*, which send a char to the output stream.

### In MQTT_osal.c:
1. **MQTT_timestamp()**: Returns the system timestamp in milliseconds. This timestamp will be used for the ping policy to keep the client alive.

## Usage
You can follow the sample code in the example repository. The client APIs are listed below:
1. **MQTT_client_new()**: Creates a MQTT client and connect to the server.
2. **MQTT_client_close()**: Close the connection.
3. **MQTT_client_publish()**: Publish a MQTT message.
4. **MQTT_client_subscribe()**: Subscribe a topic.
5. **MQTT_client_unsubscribe()**: Unsubscribe a topic.
6. **MQTT_client_unsubscribe()**: Send a ping request to the server.
7. **MQTT_client_decode_event()**: Receive data from the socket and decode the message.
8. **MQTT_client_loop()**: A blocking loop function which continues to receive data from socket and decode them. The *callback()* function in parameter *hclient* will be called in every iteration. Don't forget to add some delay time in the callback if you are using a RTOS, so the tasks with lower priority could be called.
You can refer to the sample code to understand how to use these APIs.

## To do list
1. Add MQTT 5.0 support.
2. Add SSL/TLS support.
3. Add more examples on different platforms.
