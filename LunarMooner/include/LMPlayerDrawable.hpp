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

#ifndef LM_PLAYER_DRAWABLE_HPP_
#define LM_PLAYER_DRAWABLE_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/ShaderProperty.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <array>

namespace sf
{
    class Texture;
}

namespace xy
{
    class TextureResource;
}

namespace lm
{
    class PlayerDrawable final : public xy::Component, public xy::ShaderProperty, public sf::Drawable
    {
    public:
        PlayerDrawable(xy::MessageBus&, xy::TextureResource&, const sf::Vector2f&);
        ~PlayerDrawable() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;

        void setScale(float);

    private:

        struct Quad final : public sf::Transformable
        {
            std::array<sf::Vertex, 4> vertices;
        };
        std::array<Quad, 2> m_legs;

        sf::Texture& m_diffuseTexture;
        sf::Texture& m_normalMap;
        //sf::Texture& m_maskMap;

        std::array<sf::Vertex, 12> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_PLAYER_DRAWBLE_HPP_