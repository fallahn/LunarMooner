/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

LunarMooner - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#ifndef PH_BEAM_DRAWABLE_HPP_
#define PH_BEAM_DRAWABLE_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace ph
{
    class BeamDrawable final : public xy::Component, public sf::Drawable, private sf::Transformable
    {
    public:
        explicit BeamDrawable(xy::MessageBus&);
        ~BeamDrawable() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;

        void onDelayedStart(xy::Entity&) override;

    private:
        std::array<sf::Vertex, 3u> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //PH_BEAM_DRAWABLE_HPP_