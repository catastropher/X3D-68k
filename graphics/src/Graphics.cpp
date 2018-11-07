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

#include "Graphics.hpp"
#include "Screen.hpp"
#include "LightingTable.hpp"

namespace X3D
{
    void Graphics::init(GraphicsConfig& config)
    {
        try
        {
            initService<LightingTable>(config.screen.palette);
            initService<Screen>(config.screen);

            Log::info("Init graphics library");
        }
        catch(const Exception& e)
        {
            Log::error(e, "Graphics startup failed");
            throw;
        }
    }

    // Calls the init() function of the given service type T with the given arguments
    template<typename T, typename ...Args>
    void Graphics::initService(Args&&... args)
    {
        auto service = ServiceLocator::get<T>();
        service->init(std::forward<Args>(args)...);
    }
}

