// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <sys/time.h>
#include <string.h>

typedef enum X_PacketType
{
    X_PACKET_DATA = 0,
    X_PACKET_LARGE = 1,
    X_PACKET_CONNECT = 2,
    X_PACKET_CONNECT_ACKNOWLEDGE = 3,
    X_PACKET_ECHO = 4,
    X_PACKET_REQUEST_FILE = 5,
    X_PACKET_REQUEST_FILE_RESPONSE = 6,
    X_PACKET_FILE_CHUNK = 7
} X_PacketType;

#define X_NET_ERROR 0
#define X_NET_SUCCESS 1

#define X_PACKET_MAX_SIZE 238

typedef struct X_Packet
{
    unsigned int srcId;
    unsigned int destId;
    int size;
    char* data;
    unsigned int id;
    
    X_PacketType type;
    char* internalData;
    
    int readPos;
} X_Packet;

#define X_NET_ADDRESS_MAX_LENGTH 64

typedef struct X_ConnectRequest
{
    char address[X_NET_ADDRESS_MAX_LENGTH];
} X_ConnectRequest;

struct X_Socket;

typedef struct X_SocketInterface
{
    _Bool (*matchAddress)(const char* address);
    _Bool (*openSocket)(struct X_Socket* socket, const char* address);
    void (*closeSocket)(struct X_Socket* socket);
    _Bool (*sendPacket)(struct X_Socket* socket, X_Packet* packet);
    X_Packet* (*dequeuePacket)(struct X_Socket* socket);
    _Bool (*getConnectRequest)(X_ConnectRequest* dest);
    
    struct X_SocketInterface* next;
} X_SocketInterface;

typedef enum X_SocketError
{
    X_SOCKETERROR_NONE = 0,
    X_SOCKETERROR_TIMED_OUT = 1,
    X_SOCKETERROR_SEND_FAILED = 2,
    X_SOCKETERROR_OUT_OF_PACKETS = 3,
    X_SOCKETERROR_READ_FAILED = 4,
    X_SOCKETERROR_NOT_OPENED = 5,
    X_SOCKETERROR_SERVER_REJECTED = 6,
    X_SOCKETERROR_BAD_ADDRESS = 7,
    X_SOCKETERROR_CLOSED = 8,
    X_SOCKETERROR_ALREADY_OPENED = 9,
    X_SOCKETERROR_CONNECTION_FAILED = 10,
    X_SOCKET_TOTAL_ERRORS
} X_SocketError;

typedef struct X_Socket
{
    X_SocketInterface* interface;
    volatile X_SocketError error;
    void* socketInterfaceData;
    
    int queueHead;
    int queueTail;
    
    X_Packet* packets;
    int totalPackets;
    
    unsigned char* packetData;
    
    struct timeval lastPacketRead;
} X_Socket;

void x_net_init();
void x_net_register_socket_interface(X_SocketInterface* interface);
_Bool x_net_get_connect_request(X_ConnectRequest* dest);
_Bool x_net_extract_address_and_port(const char* address, char* addressDest, int* portDest);

_Bool x_socket_open(X_Socket* socket, const char* address);
void x_socket_close(X_Socket* socket);
_Bool x_socket_send_packet(X_Socket* socket, X_Packet* packet);
X_Packet* x_socket_receive_packet(X_Socket* socket);
_Bool x_socket_connection_is_valid(X_Socket* socket);
const char* x_socket_get_error_msg(X_Socket* socket);

void x_socket_internal_init(X_Socket* socket, int totalPackets);
X_Packet* x_socket_internal_dequeue(X_Socket* socket);


static inline void x_packet_init(X_Packet* packet, X_PacketType type, char* buf, int size)
{
    packet->type = type;
    packet->data = buf;
    packet->size = size;
}

static inline void x_packet_init_empty(X_Packet* packet, X_PacketType type, char* buf)
{
    packet->type = type;
    packet->data = buf;
    packet->size = 0;
}

static inline void x_packet_write_byte(X_Packet* packet, unsigned char byte)
{
    packet->data[packet->size++] = byte;
}

static inline void x_packet_write_short(X_Packet* packet, unsigned short s)
{
    packet->data[packet->size++] = s & 0xFF;
    packet->data[packet->size++] = s >> 8;
}

static inline void x_packet_write_int(X_Packet* packet, unsigned int val)
{
    for(int i = 0; i < 8 * 4; i += 8)
        packet->data[packet->size++] = (val >> i) & 0xFF;
}

static inline void x_packet_write_int64(X_Packet* packet, unsigned long long val)
{
    for(int i = 0; i < 8 * 8; i += 8)
        packet->data[packet->size++] = (val >> i) & 0xFF;
}

static inline void x_packet_write_str(X_Packet* packet, const char* str)
{
    // TODO: add bounds checking
    strcpy((char*)packet->data + packet->size, str);
    packet->size += strlen(str) + 1;
}

static inline unsigned char x_packet_read_byte(X_Packet* packet)
{
    return packet->data[packet->readPos++];
}

static inline unsigned short x_packet_read_short(X_Packet* packet)
{
    unsigned char* data = (unsigned char*)packet->data;
    
    unsigned short val = data[packet->readPos] |
        ((unsigned short)data[packet->readPos + 1] << 8);
        
    packet->readPos += 2;
    return val;
}

static inline unsigned int x_packet_read_int(X_Packet* packet)
{
    unsigned int res = 0;
    for(int i = 0; i < 8 * 4; i += 8)
        res |= ((unsigned int)packet->data[packet->readPos++]) << i;
    
    return res;
}

static inline unsigned long long x_packet_read_int64(X_Packet* packet)
{
    unsigned long long res = 0;
    for(int i = 0; i < 8 * 8; i += 8)
        res |= ((unsigned long long)packet->data[packet->readPos++]) << i;
    
    return res;
}

static inline void x_packet_read_str(X_Packet* packet, char* dest)
{
    while(packet->data[packet->readPos])
        *dest++ = packet->data[packet->readPos++];
    
    ++packet->readPos;
    *dest = '\0';
}

static inline X_SocketError x_socket_get_error(X_Socket* socket)
{
    return socket->error;
}

