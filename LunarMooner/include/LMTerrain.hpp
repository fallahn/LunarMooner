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

        void init(const std::string&, xy::TextureResource&);
        
        const std::vector<sf::Vector2f>& getChain() const;
        const std::vector<Platform>& getPlatforms() const;

        void setLevel(sf::Uint8);
        bool valid() const { return !m_textures.empty(); }

    private:
        std::size_t m_level;
        
        mutable std::vector<std::pair<bool, std::vector<sf::Vector2f>>> m_chains;
        mutable std::vector<std::pair<bool, std::vector<Platform>>> m_platforms;

        sf::FloatRect m_bounds;
        xy::Entity* m_entity;

        std::array<sf::Vertex, 4u> m_vertices;
        std::vector<sf::Texture> m_textures;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        bool load(const std::string&, xy::TextureResource&);
    };
}

#endif //LM_TERRAIN_HPP_