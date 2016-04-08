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

#ifndef LM_SPRITE_BATCH_HPP_
#define LM_SPRITE_BATCH_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <array>

namespace lm
{
    class Sprite;
    class SpriteBatch final : public sf::Drawable, public xy::Component
    {
    public:
        explicit SpriteBatch(xy::MessageBus&);
        ~SpriteBatch() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float);
        sf::FloatRect globalBounds() const override { return m_globalBounds; }

        std::unique_ptr<Sprite> addSprite(xy::MessageBus&);
        void setTexture(const sf::Texture*);

    private:
        
        const sf::Texture* m_texture;
        std::vector<sf::Vertex> m_vertices;
        std::vector<Sprite*> m_sprites;

        sf::FloatRect m_globalBounds;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

    class Sprite final : public xy::Component
    {
    public:
        explicit Sprite(xy::MessageBus&);
        ~Sprite() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void setTextureRect(const sf::FloatRect&);
        void setColour(const sf::Color&);

    private:
        friend class SpriteBatch;
        sf::Transform m_transform;
        std::array<sf::Vertex, 4u> m_vertices;
    };
}

#endif //LM_SPRITE_BATCH_HPP_