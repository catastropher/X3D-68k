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

#include <tilp2/ticables.h>
#include <tilp2/ticalcs.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "net/X_net.h"
#include "error/X_log.h"
#include "error/X_error.h"

#include "nsp_vpkt.h"
#include "nsp_cmd.h"

#define MAX_CONNECTIONS 4
#define SERVICE_ID 32767

typedef struct SendPacket
{
    char* data;
    int size;
    unsigned char type;
    
    struct SendPacket* next;
} SendPacket;

typedef enum ConnectionState
{
    CONNECTION_CLOSED,
    CONNECTION_ACTIVE,
    CONNECTION_WAITING_CONNECT,
    CONNECTION_REQUEST_CONNECT,
    CONNECTION_INITIALIZING
} ConnectionState;

typedef struct Connection
{
    int portId;
    CableHandle* cable;
    CalcHandle* calc;
    
    _Bool connected;
    
    X_Socket* socket;
    
    pthread_mutex_t sendLock;
    pthread_mutex_t recvLock;
    
    SendPacket* sendHead;
    SendPacket* sendTail;
    
    _Bool forceClose;
    
    pthread_t thread;
    
    ConnectionState state;
} Connection;

static Connection connections[MAX_CONNECTIONS];
static 

Connection* get_connection(int connectionId)
{
    return connections + connectionId;
}

static _Bool send_data(Connection* con, char* data, int size, unsigned char cmd)
{
    return nsp_cmd_s_generic_data(con->calc, size, (unsigned char*)data, SERVICE_ID, cmd) == 0;
}

static char* recv_data(Connection* con, int* size)
{
    uint32_t usize;
    unsigned char* data;
    nsp_cmd_r_generic_data(con->calc, &usize, &data);
    
    *size = usize;
    
    printf("Recv size: %d\n", *size);
    
    return (char*)data;
}

static _Bool perform_connection_handshake(Connection* con)
{
    char msg[] = "connect";
    if(!send_data(con, msg, sizeof(msg), 0))
    {
        x_log_error("   Couldn't send success message");
        return 0;
    }
    
    int size;
    char* data = recv_data(con, &size);
    if(!data)
    {
        x_log_error("   Couldn't receive success message");
        return 0;
    }
    
    if(strcmp(data, "success") != 0)
    {
        x_log_error("   Client did not return 'success' -> returned '%s'", data + 1);
        return 0;
    }
    
    return 1;
}

static _Bool socket_open(X_Socket* socket, const char* address)
{
//     if(port < 0 || port >= PORT_MAX)
//         return 0;
//     
//     Connection* con = get_connection(port);
// 
//     socket->socketInterfaceData = con;
//     con->socket = socket;
//     
//     
//     
//     x_log("Successfully connected to calc on port %d", port);
    
    return 1;
}

static _Bool open_connection(Connection* con, int port)
{
     // FIXME: what should happen if the port is already open?
    con->cable = ticables_handle_new(CABLE_USB, port);
    if(!con->cable)
    {
        x_log_error("Failed to open USB cable on port %d\n", port);
        return 0;
    }
    
    con->calc = ticalcs_handle_new(CALC_NSPIRE);
    ticables_options_set_timeout(con->cable, 50);
    
    if(!con->calc)
    {
        x_log_error("Failed to open calc on port %d\n", port);
        ticables_handle_del(con->cable);
        return 0;
    }
    
    if(ticalcs_cable_attach(con->calc, con->cable) != 0)
    {
        x_log_error("Failed to attach calc to cable\n", port);
        return 0;
    }
    
    printf("Ready: %d\n", ticalcs_calc_isready(con->calc));
    
    nsp_session_open(con->calc, SERVICE_ID);
    x_log("Opened connection to calc on port %d");
    
    return 1;
}

static Connection* connection_from_socket(X_Socket* socket)
{
    return (Connection*)socket->socketInterfaceData;
}

static _Bool send_packet(X_Socket* socket, X_Packet* packet)
{
     if(packet->size < 240)
         x_system_error("Packet too big");
    
    Connection* con = (Connection*)socket->socketInterfaceData;
    
    pthread_mutex_lock(&con->sendLock);
    
    SendPacket* sendPacket = malloc(sizeof(sendPacket));
    
    sendPacket->size = packet->size + 1;
    sendPacket->data = malloc(sendPacket->size);
    sendPacket->type = packet->type;
    sendPacket->next = NULL;
    
    sendPacket->data[0] = packet->size;
    memcpy(sendPacket->data + 1, packet->data, packet->size);
    
    if(!con->sendHead)
    {
        con->sendHead = sendPacket;
        con->sendTail = sendPacket;
    }
    else
    {
        con->sendTail->next = sendPacket;
        con->sendTail = sendPacket;
    }
    
    pthread_mutex_unlock(&con->sendLock);
    
    return 1;
    
}

static void recv_packet(X_Socket* socket)
{
    Connection* con = (Connection*)socket->socketInterfaceData;
    
    pthread_mutex_lock(&con->recvLock);
    
    int size;
    char* buf = recv_data(connection_from_socket(socket), &size);
    
    if(!buf || size < 1)
    {
        pthread_mutex_unlock(&con->recvLock);
        return;
    }
    
    int nextPacket = (socket->queueTail + 1) % socket->totalPackets;
    if(nextPacket == socket->queueHead)
    {
        socket->error = X_SOCKETERROR_OUT_OF_PACKETS;
        pthread_mutex_unlock(&con->recvLock);
        return;
    }
    
    X_Packet* packet = socket->packets + socket->queueTail;
    packet->data = packet->internalData;
    
    memcpy(packet->data, buf, size);
    
    socket->queueTail = nextPacket;
    
    pthread_mutex_unlock(&con->recvLock);
}

static void close_connection(Connection* con)
{
    
}

static void send_queued_packet(Connection* con)
{
    pthread_mutex_lock(&con->sendLock);
    
    if(con->sendHead)
    {
        SendPacket* packet = con->sendHead;
        con->sendHead = con->sendHead->next;
        
        send_data(con, packet->data, packet->size, packet->type);
        
        free(packet->data);
        free(packet);
    }
    
    pthread_mutex_unlock(&con->sendLock);
}

static _Bool wait_for_connect_packet(Connection* con)
{
    con->state = CONNECTION_WAITING_CONNECT;
    
    while(1)
    {
        int size;
        char* buf = recv_data(con, &size);
        
        if(buf)
        {
            if(buf[0] == X_PACKET_CONNECT)
            {
                x_log("Received connect packet!\n");
                con->state = CONNECTION_REQUEST_CONNECT;
                return 1;
            }
        }
        
        usleep(1000 * 1000);
    }
    
    return 0;
}

static void* connection_thread(void* data)
{
    Connection* con = (Connection*)data;
    
    x_log("Thread spawned!\n");
    
    open_connection(con, con->portId);
    
    if(!wait_for_connect_packet(con))
        return NULL;
    
    while(1)
    {
        if(con->state == CONNECTION_ACTIVE)
        {
            recv_packet(con->socket);
            send_queued_packet(con);
        }
    }
    
    x_log("Connection thread %d terminating\n", con->portId);
    
    return NULL;
}

static void init()
{
    ticables_library_init();
    ticalcs_library_init();
    
    for(int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        connections[i].portId = i;
        connections[i].cable = NULL;
        connections[i].calc = NULL;
        connections[i].connected = 0;
        connections[i].socket = NULL;
        connections[i].forceClose = 0;
        
        connections[i].sendHead = NULL;
        connections[i].sendTail = NULL;
        connections[i].state = CONNECTION_CLOSED;
        
        pthread_mutex_init(&connections[i].sendLock, NULL);
        pthread_mutex_init(&connections[i].recvLock, NULL);
    }
}

static _Bool match_address(const char* address)
{
    return 1;
}

static X_Packet* dequeue_packet(X_Socket* socket)
{
    return NULL;
}

static void socket_close(X_Socket* socket)
{
    
}

static _Bool get_connect_request(X_ConnectRequest* request)
{
    for(int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if(connections[i].state == CONNECTION_REQUEST_CONNECT)
        {
            sprintf(request->address, "calc:%d", i);
            return 1;
        }
    }
    
    return 0;
}

static X_SocketInterface g_socketInterface = 
{
    .matchAddress = match_address,
    .openSocket = socket_open,
    .closeSocket = socket_close,
    .sendPacket = send_packet,
    .dequeuePacket = dequeue_packet,
    .getConnectRequest = get_connect_request
};

static void* listen_for_connections_thread(void* data)
{
    while(1)
    {
        x_log("Waiting for connections...");
        
        for(int i = 0; i < MAX_CONNECTIONS; ++i)
        {
            if(connections[i].state != CONNECTION_CLOSED)
                continue;
            
            CalcModel model;
            if(ticalcs_probe(CABLE_USB, i, &model, 1) == 0)
            {
                connections[i].state = CONNECTION_INITIALIZING;
                pthread_create(&connections[i].thread, NULL, connection_thread, connections + i);
                
                printf("Found new device on port %d!\n", i);
            }
        }
        
        usleep(2 * 1000 * 1000);
    }
}

static pthread_t g_listenForConnectionsThread;

void test_socket()
{
    init();
    
    pthread_create(&g_listenForConnectionsThread, NULL, listen_for_connections_thread, NULL);
    
    //g_setenv("G_MESSAGES_DEBUG", "all", /* overwrite = */ 0);
    
    x_net_register_socket_interface(&g_socketInterface);
    
    do
    {
        X_ConnectRequest request;
        if(x_net_get_connect_request(&request))
        {
            x_log("Connect request from %s\n", request.address);
            exit(0);
        }
        
    } while(1);
    
    for(int i = 0; i < MAX_CONNECTIONS; ++i)
        pthread_join(connections[i].thread, NULL);

    X_Socket socket;
}
