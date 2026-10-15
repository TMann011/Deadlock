#pragma once

#include "features/EntityCache.hpp"
#include "core/Math.hpp"
#include "core/Config.hpp"

namespace Inkwell::Features
{
    class Visuals
    {
    public:
        static auto Render( const VMatrix& view_matrix , float screen_w , float screen_h ) -> void;

    private:
        static auto RenderHeroESP( const CachedEntity& entry , const VMatrix& view_matrix , float screen_w , float screen_h ) -> void;
        static auto RenderTrooperESP( const CachedEntity& entry , const VMatrix& view_matrix , float screen_w , float screen_h ) -> void;
        static auto RenderThreatAlert( const CachedEntity& entry , const VMatrix& view_matrix , float screen_w , float screen_h ) -> void;
        static auto RenderOffScreenIndicator( const Vector3& enemy_origin , const VMatrix& view_matrix , float screen_w , float screen_h , float distance_m ) -> void;
        static auto ResolveHeroName( Source2::C_BaseEntity* ent ) -> const char*;
    };
}
