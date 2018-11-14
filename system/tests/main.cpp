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

#include <X3D-System.hpp>
#include <X3D-graphics.hpp>
#include <X3D-engine.hpp>

using namespace X3D;

class PlayerEntity : public BaseEntity<PlayerEntity>
{
public:
    StringId getTypeId()
    {
        return "player"_sid;
    }
};

class EnemyEntity : public BaseEntity<EnemyEntity>
{
    StringId getTypeId()
    {
        return "enemy"_sid;
    }
};

class Game : public BaseGame
{
public:

private:
    void init()
    {
        FilePath path("font.xtex");
        font.loadFromFile(path);
    }

    void renderHud()
    {
        screen->drawString("Hello world!", font, { 100, 100 });
    }

    void beginFrame()
    {
        auto& keyState = keyboardDriver->getKeyState();

        if(keyState.keyIsSet(KEY_ESCAPE))
        {
            quit();
        }
    }

    RasterFont font;
};

void printTypeId(IEntity* entity)
{
    entity->getTypeId();
}

void test()
{
    EngineConfig config;
    EngineConfigBuilder builder;

    builder
        .memorySize(8 * 1024 * 1024, 64 * 1024)
        .defaultScreenSize()
        .build(config);

    Engine::init(config);

    Game game;
    game.run();
}

#include <typeinfo>

int main()
{
    test();

    System::cleanup();
}



