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

#ifndef BG_ROID_BELT_HPP_
#define BG_ROID_BELT_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <array>

namespace lm
{
    class RoidBelt final : public xy::Component, public sf::Drawable
    {
    public:
        RoidBelt(xy::MessageBus&, float, sf::Texture&, sf::Texture&);
        ~RoidBelt() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;
        sf::FloatRect globalBounds() const override { return m_bounds; }

        void flipDirection();
        void setShader(sf::Shader& s) { m_shader = &s; }
    private:

        struct Roid final : public sf::Transformable
        {
            std::array<sf::Vertex, 4u> vertices;
            float rotation = 0.f;
            float speed = 0.f;
            sf::Vector2f velocity;
            float alpha = 1.f;
            void update(float, const sf::FloatRect&);
        };
        std::vector<Roid> m_roids;

        sf::FloatRect m_bounds;
        sf::Texture& m_texture;
        sf::Texture& m_normalTexture;
        sf::Shader* m_shader;
        std::vector<sf::Vertex> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //NG_ROID_ROID_BELT_HPP_