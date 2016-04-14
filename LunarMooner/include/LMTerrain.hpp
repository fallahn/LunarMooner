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

#ifndef LM_TERRAIN_HPP_
#define LM_TERRAIN_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <array>

namespace xy
{
    class TextureResource;
}

namespace lm
{
    class Terrain final : public sf::Drawable, public xy::Component
    {
    public:
        struct Platform final
        {
            sf::Vector2f size;
            sf::Vector2f position;
            sf::Uint16 value = 0;
        };

        explicit Terrain(xy::MessageBus&);
        ~Terrain() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;
        sf::FloatRect globalBounds() const override { return m_bounds; }

        bool load(const std::string&, xy::TextureResource&);

        const std::vector<sf::Vector2f>& getChain() const;
        std::vector<Platform> getPlatforms() const;

    private:
        mutable bool m_transformed;
        mutable std::vector<sf::Vector2f> m_chain;
        std::vector<Platform> m_platforms;

        sf::FloatRect m_bounds;
        xy::Entity* m_entity;

        std::array<sf::Vertex, 4u> m_vertices;
        sf::Texture m_texture;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_TERRAIN_HPP_