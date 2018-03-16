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

#define X_SERVER_MAX_PLAYERS 8

typedef struct X_Player
{
    X_Socket socket;
    bool inUse;
    X_File currentTransfer;
} X_Player;

typedef struct X_Server
{
    X_Player players[X_SERVER_MAX_PLAYERS];
} X_Server;


void x_server_init(X_Server* server);
void x_server_update(X_Server* server);

