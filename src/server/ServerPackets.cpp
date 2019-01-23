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

#include "ServerPackets.h"
#include "util/Util.hpp"
#include "engine/Engine.hpp"
#include "error/Log.hpp"

// static void send_echo_packet(X_Socket* socket)
// {
//     //struct timeval currentTime;
//     //gettimeofday(&currentTime, NULL);
// }

static void send_file_request_response(X_Socket* socket, bool success, int fileSize, const char* fileName)
{
    char packetBuf[256];
    X_Packet responsePacket;
    x_packet_init_empty(&responsePacket, X_PACKET_REQUEST_FILE_RESPONSE, packetBuf);
    
    x_packet_write_byte(&responsePacket, success);
    x_packet_write_int(&responsePacket, fileSize);
    
    char strippedFileName[X_PACKET_MAX_SIZE];
    x_filepath_extract_filename(fileName, strippedFileName);
    x_packet_write_str(&responsePacket, strippedFileName);
    
    x_socket_send_packet(socket, &responsePacket);
}

static void handle_file_request(X_Server* server, X_Player* player, X_Packet* fileRequestPacket)
{    
    char fileName[X_FILENAME_MAX_LENGTH];
    x_strncpy(fileName, fileRequestPacket->data, X_FILENAME_MAX_LENGTH);
    
    x_console_printf(x_engine_get_console(), "Received file request for file %s\n", fileName);
    
    if(strcmp(fileName, "update") == 0)
        strcpy(fileName, "../src/xtest.tns");
    
    bool success = !x_file_is_open(&player->currentTransfer) &&
        x_file_open_reading(&player->currentTransfer, fileName);
        
    
    x_log("Success: %d\n", (int)success);
    
    send_file_request_response(&player->socket, success, player->currentTransfer.size, fileName);
}

static bool transfer_is_active(X_Player* player)
{
    return x_file_is_open(&player->currentTransfer);
}

static void send_chunk_of_transfer_file(X_Player* player)
{
    x_log("Enter transfer");
    char buf[X_PACKET_MAX_SIZE];
    
    int amountToSend = X_MIN(X_PACKET_MAX_SIZE, (int)(player->currentTransfer.size - ftell(player->currentTransfer.file)));
    
    x_log("Amount to send: %d\n", amountToSend);
    
    int size = fread(buf, 1, amountToSend, player->currentTransfer.file);
    
    if(size == 0)
    {
        x_log("Size is 0");
        x_file_close(&player->currentTransfer);
        return;
    }
    
    x_log("Sending packet with file data");
    
    X_Packet packet;
    x_packet_init(&packet, X_PACKET_FILE_CHUNK, buf, size);
    x_socket_send_packet(&player->socket, &packet);
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
    
    if(transfer_is_active(player))
        send_chunk_of_transfer_file(player);
}

