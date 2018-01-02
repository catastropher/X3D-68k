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

#include "X_net.h"

_Bool x_socket_open(X_Socket* socket, const char* address)
{
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

_Bool x_socket_connection_is_valid(X_Socket* socket)
{
    return socket->error == X_SOCKETERROR_NONE;
}



