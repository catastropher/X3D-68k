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

typedef enum X_PacketType
{
    X_PACKET_DATA = 0,
    X_PACKET_LARGE = 1,
    X_PACKET_CONNECT = 2
} X_PacketType;

typedef struct X_Packet
{
    unsigned int srcId;
    unsigned int destId;
    int size;
    unsigned char* data;
    unsigned int id;
    
    X_PacketType type;
    
    struct X_Packet* next;
} X_Packet;

typedef struct X_SocketInterface
{
    
} X_SocketInterface;

typedef enum X_SocketError
{
    X_SOCKETERROR_NONE = 0,
    X_SOCKETERROR_TIMED_OUT = 1,
    X_SOCKETERROR_SEND_FAILED = 2,
    X_SOCKETERROR_OUT_OF_PACKETS = 3
} X_SocketError;

typedef struct X_Socket
{
    X_SocketInterface* interface;
    X_SocketError error;
    void* socketInterfaceData;
    
    X_Packet* freeListHead;
    X_Packet* freeListTail;
    
    X_Packet* queueHead;
    X_Packet* queueTail;
} X_Socket;
