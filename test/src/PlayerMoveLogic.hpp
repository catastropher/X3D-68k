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


#include "Player.hpp"

struct X_BspLevel;

class PlayerMoveLogic
{
public:
    PlayerMoveLogic(Player& player_, fp moveSpeed_)
        : player(player_),
        moveSpeed(moveSpeed_),
        enablePhysics(true)
    {
        
    }
    
    void applyMovement(PlayerKeyFlags keys, X_BspLevel* level);
    
private:
    static fp getForwardComponent(PlayerKeyFlags keys);
    static fp getStrafeComponent(PlayerKeyFlags keys);
    static Vec3fp getJumpVector(PlayerKeyFlags keys);
    Vec3fp getMovementKeyVector(PlayerKeyFlags keys, bool ignoreVerticalComponent);
    Vec3fp getMovementVector(PlayerKeyFlags keys);
    void handleAngleKeys(PlayerKeyFlags keys);
    void handleNormalMovement(PlayerKeyFlags flags, X_BspLevel* level);
    
    Player& player;
    fp moveSpeed;
    bool enablePhysics;
};
