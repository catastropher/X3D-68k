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

#include "net/X_net.h"
#include "error/X_log.h"
#include "error/X_error.h"

#include "nsp_vpkt.h"
#include "nsp_cmd.h"

#define MAX_CONNECTIONS 4
#define SERVICE_ID 32767

typedef struct Connection
{
    CableHandle* cable;
    CalcHandle* calc;
    _Bool connected;
} Connection;

Connection connections[MAX_CONNECTIONS];

static void init()
{
    ticables_library_init();
    ticalcs_library_init();
    
    for(int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        connections[i].cable = NULL;
        connections[i].calc = NULL;
        connections[i].connected = 0;
    }
}

Connection* get_connection(int connectionId)
{
    return connections + connectionId;
}

static _Bool send_data(Connection* con, char* data, int size, unsigned char cmd)
{
    return nsp_cmd_s_generic_data(con->calc, size, data, SERVICE_ID, cmd) == 0;
}

static unsigned char* recv_data(Connection* con, int* size)
{
    uint32_t usize;
    unsigned char* data;
    nsp_cmd_r_generic_data(con->calc, &usize, &data);
    
    *size = usize;
    
    printf("Recv size: %d\n", *size);
    
    return data;
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
    unsigned char* data = recv_data(con, &size);
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

static _Bool socket_open(X_Socket* socket, int port)
{
    if(port < 0 || port >= PORT_MAX)
        return 0;
    
    Connection* con = get_connection(port);

    if(!con->connected)
    {
        x_log_error("Attemped to open ");
    }
   
    
//     CalcModel model;
//     if(ticalcs_probe(CABLE_USB, port, &model, 1) != 0)
//     {
//         x_log_error("Failed to open calc on port %d: no calc found\n", port);
//         return -1;
//     }
    
    x_log("Attempting handshake...");
    
    if(!perform_connection_handshake(con))
    {
        x_log_error("Handshake failed");
        return 0;
    }
    
    x_log("Successfully connected to calc on port %d", port);
    
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

static void auto_connect_to_devices()
{
    for(int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if(connections[i].connected)
            continue;
        
        CalcModel model;
        if(ticalcs_probe(CABLE_USB, i, &model, 1) == 0)
        {
            connections[i].connected = 1;
            open_connection(connections + i, i);
            printf("Found new device on port %d!\n", i);
        }
    }
}

static Connection* connection_from_socket(X_Socket* socket)
{
    return (Connection*)socket->socketInterfaceData;
}

static void send_packet(X_Socket* socket, X_Packet* packet)
{
    if(packet->size < 240)
    {
        unsigned char buf[1024];
        buf[0] = packet->size;
        memcpy(buf + 1, packet->data, packet->size);
        send_data(connection_from_socket(socket), buf, packet->size + 1, packet->type);
        
        return;
    }
    
    x_system_error("Packet too big");
}

static void recv_packet(Connection* con)
{
    int size;
    unsigned char* buf = recv_data(con, &size);
    
    if(!buf || size < 1)
        return;
    
    if(buf[0] == X_PACKET_CONNECT)
        x_log("Received connect packed!\n");
}



void test_socket()
{
    init();
    
    g_setenv("G_MESSAGES_DEBUG", "all", /* overwrite = */ 0);
    
    while(1)//for(int i = 0; i < 50; ++i)
    {
        auto_connect_to_devices();
        
        for(int j = 0; j < 4; ++j)
        {
            if(connections[j].connected)
                recv_packet(connections + j);
        }
        
        //usleep( * 1000);
    }
    
//     X_Socket socket;
//     if(!socket_open(&socket, 1))
//     {
//         x_log_error("Socket opened failed");
//     }
}
