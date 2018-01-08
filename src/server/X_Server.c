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

#include "X_Server.h"
#include "net/X_net.h"
#include "error/X_error.h"

void x_server_init(X_Server* server)
{
    
}

void x_server_cleanup(X_Server* server)
{
    
}

static X_Player* find_free_player_slot(X_Server* server)
{
    for(int i = 0; i < X_SERVER_MAX_PLAYERS; ++i)
    {
        if(!server->players[i].inUse)
            return server->players + i;
    }
    
    return NULL;
}

static void show_disconnect_message(X_Server* server, X_Player* player)
{
    
}

static void remove_disconnected_players(X_Server* server)
{
    for(int i = 0; i < X_SERVER_MAX_PLAYERS; ++i)
    {
        X_Player* player = server->players + i;
        
        if(player->inUse && !x_socket_connection_is_valid(&player->socket))
        {
            show_disconnect_message(server, player);
            player->inUse = 0;
        }
    }
}

static void handle_connect_requests(X_Server* server)
{
    X_ConnectRequest request;
    while(x_net_get_connect_request(&request))
    {
        X_Player* player = find_free_player_slot(server);
        
        // FIXME: send reject packet
        if(!player)
            x_system_error("No free player slots");
        
        if(!x_socket_open(&player->socket, request.address))
            return;
        
        player->inUse = 1;
    }
}

static void handle_network(X_Server* server)
{
    handle_connect_requests(server);
    remove_disconnected_players(server);
}
