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

void test()
{
    EngineConfig config;
    EngineConfigBuilder builder;

    builder
        .memorySize(8 * 1024 * 1024, 64 * 1024)
        .defaultScreenSize()
        .build(config);

    Engine::init(config);

    auto keyboard = ServiceLocator::get<KeyboardDriver>();
    auto& state = keyboard->getKeyState();

    auto screen = ServiceLocator::get<Screen>();

    RasterFont font;
    FilePath path("font.xtex");
    font.loadFromFile(path);

    while(!state.keyIsSet(KEY_ESCAPE))
    {
        screen->fill(0);
        keyboard->update();

        if(state.keyIsSet(KEY_ENTER))
        {
            screen->drawString("Enter is pressed", font, { 0, 0 });
        }
        else
        {
            screen->drawString("Enter is not pressed", font, { 0, 0 });
        }

        screen->redraw();
    }
}

int main()
{
    test();

    System::cleanup();
}



