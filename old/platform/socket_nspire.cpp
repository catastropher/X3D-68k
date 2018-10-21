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

#if 0

#include <string.h>
#include <os.h>

#include "net/X_net.h"
#include "error/X_log.h"
#include "memory/X_alloc.h"

#define SERVICE_ID 32767

typedef struct Connection
{
    nn_ch_t handle;
    X_Socket* socket;
} Connection;

static Connection g_Connection;

static int recv_data(nn_ch_t handle, unsigned char* buf, int bufSize)
{    
    uint32_t size;
    int ret = TI_NN_Read(handle, 10, buf, bufSize, (uint32_t)&size);
    
    if(ret < 0)
        return 0;
    
    return size;
}

static _Bool send_data(X_Socket* socket, unsigned char* data, int size)
{
     return TI_NN_Write(g_Connection.handle, data, size) >= 0;
}

static void socket_callback(nn_ch_t handle, void* userData)
{
    Connection* con = (Connection*)userData;
    X_Socket* socket = con->socket;
    
    int nextPacket = (socket->queueTail + 1) % socket->totalPackets;
    
    if(nextPacket == socket->queueTail)
        socket->error = X_SOCKETERROR_OUT_OF_PACKETS;
    
    if(socket->error != X_SOCKETERROR_NONE)
        return;
    
    X_Packet* packet = socket->packets + socket->queueTail;
    
    int size = recv_data(handle, packet->internalData, 256);
    
    packet->data = packet->internalData + 2;
    packet->type = packet->internalData[0];
    packet->size = size - 2;

    socket->queueTail = nextPacket;
}

static nn_nh_t* probe_for_device()
{
    nn_oh_t handle = TI_NN_CreateOperationHandle();
    if(!handle)
    {
        x_log_error("Failed to create operation handle");
        return NULL;
    }
    
    if(TI_NN_NodeEnumInit(handle) < 0)
    {
        x_log_error("Failed to init node enum");
        return NULL;
    }
    
    nn_nh_t nodeHandle;
    if(TI_NN_NodeEnumNext(handle, &nodeHandle) < 0)
    {
        x_log_error("Failed to get next device in list");
        TI_NN_DestroyOperationHandle(handle);
        return NULL;
    }
    
    TI_NN_DestroyOperationHandle(handle);
    return nodeHandle;
}

static void socket_close(X_Socket* socket)
{
    socket->error = X_SOCKETERROR_CLOSED;
    TI_NN_StopService(SERVICE_ID);
    
    x_free(socket->packets);
    x_free(socket->packetData);
}

static _Bool send_packet(X_Socket* socket, X_Packet* packet)
{
    if(packet->size < 240)
    {
        unsigned char buf[1024];
        buf[0] = packet->type;
        buf[1] = packet->type;
        buf[2] = packet->size;
        memcpy(buf + 3, packet->data, packet->size);
        
        _Bool status = send_data(socket, buf, packet->size + 3);
        return status;
    }
    
    x_system_error("Packet too big");
}

static _Bool perform_connection_handshake(X_Socket* socket)
{
    X_Packet connectPacket;
    connectPacket.data = 0;
    connectPacket.type = X_PACKET_CONNECT;
    connectPacket.size = 0;
    if(!send_packet(socket, &connectPacket))
    {
        x_log_error("Failed to send connect packet");
        return 0;
    }
    
    return 1;
}

static _Bool socket_open(X_Socket* socket, const char* address)
{
    x_log("Attemping to open socket...");
    socket->error = X_SOCKETERROR_NOT_OPENED;
    
    if(g_Connection.socket)
    {
        x_log("USB Port already in use");
        return 0;
    }
    
    nn_nh_t device = probe_for_device();
    if(!device)
    {
        x_log_error("Failed to open socket: no device found");
        return 0;
    }
    
    int totalPackets = 32;
    const int MAX_PACKET_SIZE = 256;
    
    socket->packets = x_malloc(totalPackets * sizeof(X_Packet));
    socket->packetData = x_malloc(totalPackets * MAX_PACKET_SIZE);
    
    for(int i = 0; i < totalPackets; ++i)
        socket->packets[i].internalData = socket->packetData + i * MAX_PACKET_SIZE;
    
    socket->queueHead = 0;
    socket->queueTail = 0;
    socket->totalPackets = totalPackets;
    
    TI_NN_StopService(SERVICE_ID);
    int ret = TI_NN_StartService(SERVICE_ID, &g_Connection, socket_callback);
    if(ret < 0)
    {
        x_log_error("Failed to start service (error code %d)", ret);
        return 0;
    }
    
    nn_ch_t handle;
    if(TI_NN_Connect(device, SERVICE_ID, &handle) < 0 || !handle)
    {
        x_log_error("Failed to connect to handle");
        return 0;
    }
    
    g_Connection.socket = socket;
    g_Connection.handle = handle;
    
    x_log("Connect handle: %d", (int)handle);
    x_log("Address: %X", (unsigned int)&g_Connection);
    
    x_log("Opened connection");
    
    asm("mrs r1, cpsr \n\t"
        "bic r1, r1, #0x80 \n\t"
        "msr cpsr, r1"
        :
        :
    : "r1");
    
    if(!perform_connection_handshake(socket))
    {
        x_log_error("Failed to perform connection handshake");
        return 0;
    }
    
    socket->error = X_SOCKETERROR_NONE;
    gettimeofday(&socket->lastPacketRead, NULL);
    
    return 1;
}

static X_Packet* dequeue_packet(X_Socket* socket)
{
    if(socket->error != X_SOCKETERROR_NONE)
        return NULL;
    
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    
    if(socket->queueHead == socket->queueTail)
    {
//         if(currentTime.tv_sec - socket->lastPacketRead.tv_sec > 10)
//             socket->error = X_SOCKETERROR_TIMED_OUT;
        
        return NULL;
    }
    
    int nextPacket = (socket->queueHead + 1) % socket->totalPackets;
    X_Packet* packet = socket->packets + socket->queueHead;
    socket->lastPacketRead = currentTime;
    
    socket->queueHead = nextPacket;
    
    packet->readPos = 0;
    
    return packet;
}

static _Bool match_address(const char* address)
{
    return 1;
}

static X_SocketInterface g_socketInterface = 
{
    .matchAddress = match_address,
    .openSocket = socket_open,
    .closeSocket = socket_close,
    .sendPacket = send_packet,
    .dequeuePacket = dequeue_packet
};

void x_socket_nspire_register_interface(void)
{
    x_net_register_socket_interface(&g_socketInterface);
}

void test_socket()
{
//     X_Socket socket;
//     
//     //if(!socket_open(&socket))
//     //    return;
//     
//     do
//     {
//         X_Packet* packet = dequeue_packet(&socket);
//         
//         if(!packet)
//             continue;
//         
//         show_msgbox("Message", packet->data);
//         
//         char buf[32];
//         sprintf(buf, "%d", packet->size);
//         show_msgbox("Size", buf);
//                 
//     } while(socket.error == X_SOCKETERROR_NONE);
//     
//     char buf[32];
//     sprintf(buf, "%d", socket.error);
//     show_msgbox("Disconnect", buf);
//     
//     socket_close(&socket);
}

#endif
