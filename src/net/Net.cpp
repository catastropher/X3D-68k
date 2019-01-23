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

#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>

#include "Net.hpp"
#include "memory/Alloc.h"

static X_SocketInterface* g_interfaceHead;

void x_net_init()
{
    g_interfaceHead = NULL;
}

void x_net_register_socket_interface(X_SocketInterface* interface)
{
    interface->next = g_interfaceHead;
    g_interfaceHead = interface;
}

bool x_net_get_connect_request(X_ConnectRequest* dest)
{
    for(X_SocketInterface* interface = g_interfaceHead; interface != NULL; interface = interface->next)
    {
        if(interface->getConnectRequest(dest))
            return 1;
    }
    
    return 0;
}

static X_SocketInterface* get_socket_interface_from_address(const char* address)
{
    for(X_SocketInterface* interface = g_interfaceHead; interface != NULL; interface = interface->next)
    {
        if(interface->matchAddress(address))
            return interface;
    }
    
    return NULL;
}

bool x_socket_open(X_Socket* socket, const char* address)
{
    socket->interface = get_socket_interface_from_address(address);
    
    if(!socket->interface)
    {
        socket->error = X_SOCKETERROR_BAD_ADDRESS;
        return 0;
    }
    
    return socket->interface->openSocket(socket, address);
}

void x_socket_close(X_Socket* socket)
{
    socket->interface->closeSocket(socket);
}

bool x_socket_send_packet(X_Socket* socket, X_Packet* packet)
{
    return socket->interface->sendPacket(socket, packet);
}

// static void process_connect_acknowledge(X_Socket* socket, X_Packet* packet)
// {
//     if(packet->data[0] != X_NET_SUCCESS)
//         socket->error = X_SOCKETERROR_SERVER_REJECTED;
// }

// static void send_connect_acknowledge(X_Socket* socket, bool success)
// {
//     X_Packet packet;
//     char buf[1] = { success };
    
//     x_packet_init(&packet, X_PACKET_CONNECT_ACKNOWLEDGE, buf, 1);
//     x_socket_send_packet(socket, &packet);
// }

X_Packet* x_socket_receive_packet(X_Socket* socket)
{
    return socket->interface->dequeuePacket(socket);    
}

bool x_socket_connection_is_valid(X_Socket* socket)
{
    return socket->error == X_SOCKETERROR_NONE;
}

bool x_net_extract_address_and_port(const char* address, char* addressDest, int* portDest)
{
    while(*address && *address != ':')
        *addressDest++ = *address++;
    
    *addressDest = '\0';
    
    if(*address == '\0')
    {
        *portDest = '\0';
        return 0;
    }
    
    ++address;      // Skip ':'
    
    char port[X_NET_ADDRESS_MAX_LENGTH];
    char* portPos = port + 0;
    
    while(*address)
    {
        if(!isdigit(*address))
            return 0;
        
        *portPos++ = *address++;
    }
    
    *portPos = '\0';
    *portDest = atoi(port);
    
    return 1;
}

const char* x_socket_get_error_msg(X_Socket* socket)
{
    static const char* errorMessages[] = 
    {
        "No error",
        "Connection timed out",
        "Send failed",
        "Out of packets",
        "Read failed",
        "Not opened",
        "Connection rejected by server",
        "Bad address",
        "Closed",
        "Already opened",
        "Connection failed"
    };
    
    int error = (int)socket->error;
    
    if(error >= 0 && error < X_SOCKET_TOTAL_ERRORS)
        return errorMessages[socket->error];
    
    static const char unknownError[] = "Unknown error";
    return unknownError;
}

void x_socket_internal_init(X_Socket* socket, int totalPackets)
{
    const int MAX_PACKET_SIZE = 256;
    
    socket->packets = (X_Packet*)x_malloc(totalPackets * sizeof(X_Packet));
    socket->packetData = (unsigned char*)x_malloc(totalPackets * MAX_PACKET_SIZE);
    
    for(int i = 0; i < totalPackets; ++i)
        socket->packets[i].internalData = (char*)socket->packetData + i * MAX_PACKET_SIZE;
    
    socket->queueHead = 0;
    socket->queueTail = 0;
    socket->totalPackets = totalPackets;
    
    //gettimeofday(&socket->lastPacketRead, NULL);
}

X_Packet* x_socket_internal_dequeue(X_Socket* socket)
{
    if(socket->error != X_SOCKETERROR_NONE)
        return NULL;
    
    struct timeval currentTime;
    //gettimeofday(&currentTime, NULL);
    
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

