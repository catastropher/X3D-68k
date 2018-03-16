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

#pragma once

#include "net/X_net.h"
#include "system/X_File.h"

typedef struct X_Client
{
    X_Socket socket;
    bool connectedToServer;
    
    X_File currentTransfer;
    int transferSize;
    int transferTotalReceived;
} X_Client;

void x_client_init(X_Client* client);
bool x_client_connect(X_Client* client, const char* address);
void x_client_request_file(X_Client* client, const char* name);
void x_client_update(X_Client* client);

