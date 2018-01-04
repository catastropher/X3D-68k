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

#include "X_net.h"

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

_Bool x_net_get_connect_request(X_ConnectRequest* dest)
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

_Bool x_socket_open(X_Socket* socket, const char* address)
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

_Bool x_socket_send_packet(X_Socket* socket, X_Packet* packet)
{
    return socket->interface->sendPacket(socket, packet);
}

static void process_connect_acknowledge(X_Socket* socket, X_Packet* packet)
{
    if(packet->data[0] != X_NET_SUCCESS)
        socket->error = X_SOCKETERROR_SERVER_REJECTED;
}

static void send_connect_acknowledge(X_Socket* socket, _Bool success)
{
    X_Packet packet;
    char buf[1] = { success };
    
    x_packet_init(&packet, X_PACKET_CONNECT_ACKNOWLEDGE, buf, 1);
    x_socket_send_packet(socket, &packet);
}

static _Bool process_system_packet(X_Socket* socket, X_Packet* packet)
{
    switch(packet->type)
    {
        case X_PACKET_CONNECT_ACKNOWLEDGE:
            process_connect_acknowledge(socket, packet);
            return 1;
            
        case X_PACKET_CONNECT:
            send_connect_acknowledge(socket, X_NET_ERROR);
            return 1;
            
        default:
            return 0;
    }
}

X_Packet* x_socket_receive_packet(X_Socket* socket)
{
    do
    {
        X_Packet* packet = socket->interface->dequeuePacket(socket);
        if(!packet)
            return NULL;
     
        if(!process_system_packet(socket, packet))
            return packet;
    } while(x_socket_connection_is_valid(socket));
    
    return NULL;
}

_Bool x_socket_connection_is_valid(X_Socket* socket)
{
    return socket->error == X_SOCKETERROR_NONE;
}

_Bool x_net_extract_address_and_port(const char* address, char* addressDest, int* portDest)
{
    char* t = addressDest;
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
    switch(socket->error)
    {
        case X_SOCKETERROR_NONE:                return "No error";
        case X_SOCKETERROR_CLOSED:              return "Socket closed";
        case X_SOCKETERROR_OUT_OF_PACKETS:      return "Out of packets";
        case X_SOCKETERROR_READ_FAILED:         return "Read failed";
        case X_SOCKETERROR_BAD_ADDRESS:         return "Bad address";
        case X_SOCKETERROR_NOT_OPENED:          return "Not opened";
        case X_SOCKETERROR_SEND_FAILED:         return "Send failed";
        case X_SOCKETERROR_SERVER_REJECTED:     return "Connection rejected by server";
        case X_SOCKETERROR_TIMED_OUT:           return "Connection timed out";
        case X_SOCKETERROR_ALREADY_OPENED:      return "Already opened";
        case X_SOCKETERROR_CONNECTION_FAILED:   return "Connection failed";
        
        default:                            return "Unknown error";
    }
}

