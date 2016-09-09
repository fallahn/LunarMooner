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

#ifndef LM_STARFIELD_HPP_
#define LM_STARFIELD_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <vector>
#include <array>

namespace xy
{
    class TextureResource;
}

namespace lm
{
    class Starfield final : public xy::Component, public sf::Drawable
    {
    public:
        Starfield(xy::MessageBus&, xy::TextureResource&);
        ~Starfield() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;
        sf::FloatRect globalBounds() const override { return m_bounds; }

        void setVelocity(const sf::Vector2f&);
        void setSpeedRatio(float v) { m_speedRatio = std::abs(v); }

    private:

        sf::FloatRect m_bounds;

        struct Star final : public sf::Transformable
        {
            std::array<sf::Vector2f, 4u> positions;
            float depth = 1.f;
            Star()
            {
                positions[1] = { 1.f, 0.f };
                positions[2] = { 1.f, 1.f };
                positions[3] = { 0.f, 1.f };
            }
        };
        std::vector<Star> m_stars;
        float m_speedRatio;

        sf::Vector2f m_position;
        sf::Vector2f m_velocity;
        mutable sf::Shader m_shader;
        sf::Sprite m_backgroundSprite;
        sf::Texture* m_starTexture;

        std::vector<sf::Vertex> m_vertices;       
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_STARFIELD_HPP_