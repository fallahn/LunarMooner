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

#ifndef LM_SHIELD_DRAWABLE_HPP_
#define LM_SHIELD_DRAWABLE_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <array>

namespace lm
{
    class ShieldDrawable final : public xy::Component, public sf::Drawable
    {
    public:
        ShieldDrawable(xy::MessageBus&, float);
        ~ShieldDrawable() = default;

        xy::Component::Type type()const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;
        sf::FloatRect globalBounds() const override;

        void setTexture(sf::Texture&);

    private:

        sf::Shader m_shader; //TODO we ought to resource manage this
        sf::Vector2f m_textureOffset;
        std::size_t m_waveTableIndex;
        std::vector<float> m_wavetable;

        xy::Entity* m_entity;

        float m_impactRadius;
        float m_impactIntensity;

        sf::FloatRect m_bounds;
        sf::Texture* m_texture;
        std::array<sf::Vertex, 22u> m_vertices;
        std::array<sf::Vertex, 11u> m_outline;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;


    };
}

#endif //LM_SHIELD_DRAWABLE_HPP_