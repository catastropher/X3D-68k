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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "net/Net.hpp"
#include "error/Log.hpp"
#include "error/Error.hpp"
#include "memory/OldLink.hpp"
#include "memory/Alloc.h"

typedef struct ConnectionAddress
{
    unsigned int ipAddress;
    unsigned short port;
} ConnectionAddress;

typedef struct ConnectionRequest
{
    ConnectionAddress address;
    
    struct ConnectionRequest* next;
} ConnectionRequest;

typedef struct PcSocket
{
    OldLink connectionHead;
    OldLink connectionTail;
    
    int socketFd;
    struct sockaddr_in socketAddress;
    
    ConnectionRequest* connectRequestHead;
} PcSocket;

typedef struct Connection
{
    OldLink link;
    
    X_Socket* x3dSocket;
    ConnectionAddress recvAddress;
    ConnectionAddress sendAddress;
    
    PcSocket* pcSocket;
} Connection;

static PcSocket* get_pcsocket(void)
{
    static PcSocket socket;
    return &socket;
}

static void connectionaddress_to_inet_addr(ConnectionAddress* src, struct sockaddr_in* dest)
{
    dest->sin_family = AF_INET;
    dest->sin_addr.s_addr = src->ipAddress;
    dest->sin_port = src->port;
}

static void pcsocket_setup_address(PcSocket* sock, unsigned short port)
{
    sock->socketAddress.sin_family = AF_INET;
    sock->socketAddress.sin_port = htons(port);
    sock->socketAddress.sin_addr.s_addr = 0;    // Automatically pick IP address
}

static bool pcsocket_create_socket(PcSocket* sock)
{
    sock->socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock->socketFd < 0)
    {
        x_log_error("Failed to create socket");
        return 0;
    }
    
    return 1;
}

static bool pcsocket_bind(PcSocket* sock)
{
    if(bind(sock->socketFd, (struct sockaddr *)&sock->socketAddress, sizeof(struct sockaddr_in)) < 0)
    {
        x_log_error("Failed to bind socket");
        return 0;
    }
    
    return 1;
}

static bool pcsocket_disable_blocking(PcSocket* sock)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    
    if(setsockopt(sock->socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0)
    {
        x_log_error("Failed to disable socket blocking");
        return 0;
    }
    
    return 1;
}

static bool pcsocket_init(PcSocket* sock, unsigned short port)
{
    x_link_init(&sock->connectionHead, &sock->connectionTail);
    sock->connectRequestHead = NULL;
    
    pcsocket_setup_address(sock, port);
    
    bool success = pcsocket_create_socket(sock) &&
        pcsocket_bind(sock) &&
        pcsocket_disable_blocking(sock);
        
    if(!success)
        return 0;
    
    x_log("Initialized socket on port %d", port);
    
    return 1;
}

// static void pcsocket_cleanup(PcSocket* sock)
// {
//     if(sock->socketFd < 0)
//         return;
    
//     x_log("Closing socket...");
//     close(sock->socketFd);
// }

static void enqueue_packet(X_Socket* socket, unsigned char* data, int dataSize)
{
    X_Packet* packet = socket->packets + socket->queueTail;
    
    packet->type = (X_PacketType)data[0];
    packet->data = packet->internalData;
    memcpy(packet->data, data + 1, dataSize - 1);
    
    socket->queueTail = (socket->queueTail + 1) % socket->totalPackets;
}

static void forward_data_to_x3d_socket(PcSocket* sock, unsigned char* data, int dataSize, ConnectionAddress* address)
{
    for(OldLink* link = sock->connectionHead.next; link != &sock->connectionTail; link = link->next)
    {
        Connection* con = (Connection*)link;
        
        if(con->recvAddress.ipAddress == address->ipAddress && con->recvAddress.port == address->port)
        {
            enqueue_packet(con->x3dSocket, data, dataSize);
            return;
        }
    }
}

static void handle_connect_packet(PcSocket* sock, ConnectionAddress* address)
{
    // TODO: check whether the requested connection is already active
    ConnectionRequest* request = (ConnectionRequest*)x_malloc(sizeof(ConnectionRequest));
    
    request->address = *address;
    request->next = sock->connectRequestHead;
    sock->connectRequestHead = request;
}

static bool pcsocket_receive_packet(PcSocket* sock)
{
    struct sockaddr_in sourceAddress;
    unsigned int addressLength = sizeof(struct sockaddr_in);
    unsigned char buf[256];
    
    int size = recvfrom(sock->socketFd, buf, sizeof(buf), 0, (struct sockaddr *)&sourceAddress, &addressLength);
    if(size == -1)
    {
        if(errno != ETIMEDOUT && errno != EAGAIN)
            x_system_error("Error in receiving packet");    // FIXME: need a better way to handle this
        else
            return 0;
    }
    
    x_log("Has packet!");
    
    ConnectionAddress address;
    address.ipAddress = sourceAddress.sin_addr.s_addr;
    address.port = sourceAddress.sin_port;
    
    X_PacketType type = (X_PacketType)buf[0];
    
    if(type == X_PACKET_CONNECT)
        handle_connect_packet(sock, &address);
    else
        forward_data_to_x3d_socket(sock, buf, size, &address);
    
    return 1;
}

static bool extract_address(const char* addressXString, ConnectionAddress* dest)
{
    char address[X_NET_ADDRESS_MAX_LENGTH];
    int port;
    
    if(!x_net_extract_address_and_port(addressXString, address, &port))
        return 0;
    
    int ipAddress = inet_addr(address);
    if(ipAddress == -1)
        return 0;
    
    dest->ipAddress = ipAddress;
    dest->port = port;
    
    return 1;
}

static bool socket_open(X_Socket* socket, const char* addressXString)
{
    ConnectionAddress address;
    if(!extract_address(addressXString, &address))
    {
        socket->error = X_SOCKETERROR_BAD_ADDRESS;
        return 0;
    }
    
    Connection* con = (Connection*)x_malloc(sizeof(Connection));
    con->sendAddress = address;
    con->x3dSocket = socket;
    
    socket->socketInterfaceData = con;
    
    PcSocket* pcSocket = get_pcsocket();
    x_link_insert_after(&con->link, &pcSocket->connectionHead);
    x_socket_internal_init(socket, 512);
    
    con->pcSocket = pcSocket;
    
    return 1;
}

static void socket_close(X_Socket* socket)
{
    if(!socket->socketInterfaceData)
        return;
    
    Connection* con = (Connection*)socket->socketInterfaceData;
    x_link_unlink(&con->link);
    
    socket->socketInterfaceData = NULL;
    
    free(con);
}

static X_Packet* socket_dequeue_packet(X_Socket* socket)
{
    // TODO: is the right place for this?
    PcSocket* pcSocket = get_pcsocket();
    while(pcsocket_receive_packet(pcSocket)) ;
    
    return x_socket_internal_dequeue(socket);
}

static bool socket_match_address(const char* address)
{
    ConnectionAddress addr;
    return extract_address(address, &addr);
}

static bool socket_send_packet(X_Socket* socket, X_Packet* packet)
{
    if(packet->size > X_PACKET_MAX_SIZE)
        x_system_error("Bad packet size");
    
    char buf[256];
    buf[0] = packet->type;
    
    memcpy(buf + 1, packet->data, packet->size);
    
    struct sockaddr_in destAddress;
    unsigned int addressLength = sizeof(struct sockaddr_in);
    
    Connection* con = (Connection*)socket->socketInterfaceData;
    destAddress.sin_family = AF_INET;
    destAddress.sin_addr.s_addr = con->sendAddress.ipAddress;
    destAddress.sin_port = con->sendAddress.port;
    
    PcSocket* pcSocket = con->pcSocket;
    
    int totalSize = packet->size + 1;
    int sizeSent = sendto(pcSocket->socketFd, buf, totalSize, 0, (struct sockaddr *)&destAddress, addressLength);
    
    if(sizeSent != totalSize)
    {
        socket->error = X_SOCKETERROR_SEND_FAILED;
        return 0;
    }
    
    x_log("Packet sent");
    
    return 1;
}

static bool get_connect_request(X_ConnectRequest* dest)
{
    PcSocket* socket = get_pcsocket();
    if(!socket->connectRequestHead)
        return 0;
    
    ConnectionRequest* request = socket->connectRequestHead;
    socket->connectRequestHead = socket->connectRequestHead->next;
    
    struct sockaddr_in address;
    connectionaddress_to_inet_addr(&request->address, &address);
    
    const char* ipAddress = inet_ntoa(address.sin_addr);
    
    sprintf(dest->address, "%s:%d", ipAddress, request->address.port);
    
    x_free(request);
    return 1;
}

static X_SocketInterface g_socketInterface = 
{
    .matchAddress = socket_match_address,
    .openSocket = socket_open,
    .closeSocket = socket_close,
    .sendPacket = socket_send_packet,
    .dequeuePacket = socket_dequeue_packet,
    .getConnectRequest = get_connect_request
};

void x_socket_pc_register_interface(void)
{
    x_net_register_socket_interface(&g_socketInterface);
}

void test_pc_socket(int port)
{
    PcSocket* socket = get_pcsocket();
    pcsocket_init(socket, port);
}

void net_update()
{
    PcSocket* sock = get_pcsocket();
    pcsocket_receive_packet(sock);
}


