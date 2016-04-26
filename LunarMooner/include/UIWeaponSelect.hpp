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

#ifndef UI_WEAPON_SELECT_HPP_
#define UI_WEAPON_SELECT_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <array>

namespace lm
{
    namespace ui
    {
        class WeaponSelect final : public xy::UI::Control
        {
        public:
            explicit WeaponSelect(sf::Texture&);
            ~WeaponSelect() = default;

            bool selectable() const override { return true; }
            void select() override;
            void deselect() override;

            void activate() override;
            void deactivate() override;

            void update(float) override;
            void handleEvent(const sf::Event&, const sf::Vector2f&) override;

            void setAlignment(xy::UI::Alignment) override;

            bool contains(const sf::Vector2f&)const override;

            std::size_t getSelectedIndex() const { return m_selectedIndex; }
            void setLockedFlags(sf::Uint8 flags);

        private:

            struct Item final : public sf::Transformable
            {
                std::array<sf::Vertex, 4u> vertices;
                float alpha = 255.f;
            };
            std::array<Item, 5u> m_items;

            sf::Texture& m_texture;
            std::size_t m_selectedIndex;
            std::size_t m_nextIndex;
            sf::Uint8 m_lockedFlags;

            std::array<sf::Vertex, 8u> m_vertices;
            std::size_t m_vertexCount;

            sf::FloatRect m_bounds;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
            void updateVertexArray();
        };
    }
}

#endif //UI_WEAPON_SELECT_HPP_