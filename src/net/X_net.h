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

#include <sys/time.h>

typedef enum X_PacketType
{
    X_PACKET_DATA = 0,
    X_PACKET_LARGE = 1,
    X_PACKET_CONNECT = 2,
    X_PACKET_CONNECT_ACKNOWLEDGE = 3
} X_PacketType;

#define X_NET_ERROR 0
#define X_NET_SUCCESS 1

typedef struct X_Packet
{
    unsigned int srcId;
    unsigned int destId;
    int size;
    unsigned char* data;
    unsigned int id;
    
    X_PacketType type;
    unsigned char* internalData;
} X_Packet;

struct X_Socket;

typedef struct X_SocketInterface
{
    _Bool (*matchAddress)(const char* address);
    _Bool (*openSocket)(struct X_Socket* socket, const char* address);
    void (*closeSocket)(struct X_Socket* socket);
    _Bool (*sendPacket)(struct X_Socket* socket, X_Packet* packet);
    struct X_Socket (*dequeuePacket)(struct X_Socket* socket);
} X_SocketInterface;

typedef enum X_SocketError
{
    X_SOCKETERROR_NONE = 0,
    X_SOCKETERROR_TIMED_OUT = 1,
    X_SOCKETERROR_SEND_FAILED = 2,
    X_SOCKETERROR_OUT_OF_PACKETS = 3,
    X_SOCKETERROR_READ_FAILED = 4,
    X_SOCKETERROR_NOT_OPENED = 5,
    X_SOCKETERROR_SERVER_REJECTED = 6
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

static inline void x_packet_init(X_Packet* packet, X_PacketType type, unsigned char* buf, int size)
{
    packet->type = type;
    packet->data = buf;
    packet->size = size;
}

