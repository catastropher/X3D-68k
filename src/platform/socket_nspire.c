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

#include <string.h>
#include <os.h>

#include "net/X_net.h"
#include "error/X_log.h"

// The Nspire can have at most one connection through the linkport

#define SERVICE_ID 32767

typedef struct Connection
{
    nn_ch_t handle;
    X_Socket* socket;
} Connection;

static Connection g_Connection;

static int recv_data(Connection* con, unsigned char* buf, int bufSize)
{
    unsigned int size;
    int ret = TI_NN_Read(con->handle, 0, buf, bufSize, (uint32_t)&size);
    
    if(ret < 0)
    {
        con->socket->error = X_SOCKETERROR_TIMED_OUT;
        return 0;
    }
    
    return size;
}

static _Bool send_data(X_Socket* socket, unsigned char* data, int size)
{
     return TI_NN_Write(g_Connection.handle, data, size) >= 0;
}

static void socket_callback(nn_ch_t handle, void* userData)
{
//     Connection* con = (Connection*)handle;
//     unsigned char buf[512];
//     
//     int size = recv_data(con, buf, 512);
//     
//     if(size == 0)
//         return;
//     
//     X_Socket* socket = con->socket;
//     X_Packet* packet = socket->freeListHead;
//     
//     if(!packet)
//     {
//         socket->error = X_SOCKETERROR_OUT_OF_PACKETS;
//         return;
//     }
//     
//     socket->freeListHead = socket->freeListHead->next;
//     
//     packet->type = buf[0];
//     packet->size = buf[1];
//     
//    memcpy(packet->data, buf + 2, packet->size);
}

static void enable_interrupts()
{
    asm("mrs r1, cpsr \n\t"
        "bic r1, r1, #0x80 \n\t"
        "msr cpsr, r1"
        :
        :
    : "r1");
}

static void* probe_for_device()
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
    TI_NN_StopService(32767);
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
        return send_data(socket, buf, packet->size + 3);
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

static _Bool socket_open(X_Socket* socket)
{
    x_log("Attemping to open socket...");
    
    nn_nh_t device = probe_for_device();
    if(!device)
    {
        x_log_error("Failed to open socket: no device found");
        return 0;
    }
    
    TI_NN_StopService(32767);
    int ret = TI_NN_StartService(32767, &g_Connection, socket_callback);
    if(ret < 0)
    {
        x_log_error("Failed to start service (error code %d)", ret);
        return 0;
    }
    
    nn_ch_t handle;
    if(TI_NN_Connect(device, SERVICE_ID, &handle) < 0)
    {
        x_log_error("Failed to connect to handle");
        return 0;
    }
    
    g_Connection.socket = socket;
    g_Connection.handle = handle;
    
    x_log("Opened connection");
    
    enable_interrupts();
    
    if(!perform_connection_handshake(socket))
    {
        x_log_error("Failed to perform connection handshake");
        return 0;
    }
    
    return 1;
}

void test_socket()
{
    X_Socket socket;
    socket_open(&socket);
    socket_close(&socket);
}

