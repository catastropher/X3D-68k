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

#include "X_ServerPackets.h"

static void send_echo_packet(X_Socket* socket)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
}

void x_server_handle_packets_for_player(X_Server* server, X_Player* player)
{
    X_Packet* packet;
    while((packet = x_socket_receive_packet(&player->socket)) != NULL)
    {
        switch(packet->type)
        {
            
        }
    }
}

