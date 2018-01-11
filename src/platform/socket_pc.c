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

#include "net/X_net.h"
#include "error/X_log.h"

typedef struct Connection
{
    X_Socket* x3dSocket;
    unsigned int ipAddress;
    unsigned short port;
} Connection;

typedef struct PcSocket
{
    int socketFd;
    struct sockaddr_in socketAddress;
} PcSocket;

static void pcsocket_setup_address(PcSocket* sock, unsigned short port)
{
    sock->socketAddress.sin_family = AF_INET;
    sock->socketAddress.sin_port = htons(port);
    sock->socketAddress.sin_addr.s_addr = 0;    // Automatically pick IP address
}

static _Bool pcsocket_create_socket(PcSocket* sock)
{
    sock->socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock->socketFd < 0)
    {
        x_log_error("Failed to create socket");
        return 0;
    }
    
    return 1;
}

static _Bool pcsocket_bind(PcSocket* sock)
{
    if(bind(sock->socketFd, (struct sockaddr *)&sock->socketAddress, sizeof(struct sockaddr_in)) < 0)
    {
        x_log_error("Failed to bind socket");
        return 0;
    }
    
    return 1;
}

static _Bool pcsocket_init(PcSocket* sock, unsigned short port)
{
    pcsocket_setup_address(sock, port);
    
    if(!pcsocket_create_socket(sock))
        return 0;
    
    if(!pcsocket_bind(sock))
        return 0;
    
    x_log("Initialized socket on port %d", port);
    
    return 1;
}

static void pcsocket_cleanup(PcSocket* sock)
{
    if(sock->socketFd < 0)
        return;
    
    x_log("Closing socket...");
    close(sock->socketFd);
}

void test_pc_socket()
{
    PcSocket sock;
    pcsocket_init(&sock, 8000);
    pcsocket_cleanup(&sock);
}

