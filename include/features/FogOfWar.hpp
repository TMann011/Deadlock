#pragma once

#include "source2/EntitySystem.hpp"

namespace Inkwell::Features
{
    class FogOfWarManager
    {
    public:
        static auto RevealMinimap( Source2::IGameEntitySystem* entity_system ) -> void;
    };
}
