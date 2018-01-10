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
#include "util/X_util.h"

static void send_echo_packet(X_Socket* socket)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
}

static void send_file_request_response(X_Socket* socket, _Bool success, int fileSize)
{
    char packetBuf[256];
    X_Packet responsePacket;
    x_packet_init_empty(&responsePacket, X_PACKET_REQUEST_FILE_RESPONSE, packetBuf);
    
    x_packet_write_byte(&responsePacket, success);
    x_packet_write_int(&responsePacket, fileSize);
    
    x_socket_send_packet(socket, &responsePacket);
}

static void handle_file_request(X_Server* server, X_Player* player, X_Packet* fileRequestPacket)
{
    char filename[X_FILENAME_MAX_LENGTH];
    x_strncpy(filename, fileRequestPacket->data, X_FILENAME_MAX_LENGTH);
    
    _Bool success = !x_file_is_open(&player->currentTransfer) &&
        x_file_open_reading(&player->currentTransfer, filename);
        
    send_file_request_response(&player->socket, success, player->currentTransfer.size);
}

void x_server_handle_packets_for_player(X_Server* server, X_Player* player)
{
    X_Packet* packet;
    while((packet = x_socket_receive_packet(&player->socket)) != NULL)
    {
        switch(packet->type)
        {
            case X_PACKET_REQUEST_FILE:
                handle_file_request(server, player, packet);
                break;
                
            default:
                break;
        }
    }
}

