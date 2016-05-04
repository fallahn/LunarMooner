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

#ifndef LM_COUNTER_DISPLAY_HPP_
#define LM_COUNTER_DISPLAY_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>
#include <vector>

namespace sf
{
    class Texture;
    class Font;
}

namespace lm
{
    class CounterDisplay final : public sf::Drawable, public sf::Transformable
    {
    public:
        CounterDisplay(sf::Texture&, const sf::Font&, const std::string&, sf::Uint8);
        ~CounterDisplay() = default;
        CounterDisplay(const CounterDisplay&) = delete;
        CounterDisplay& operator = (const CounterDisplay&) = delete;

        void update(float);
        void setValue(int);

    private:

        struct SubRect final : public sf::Transformable
        {
            std::array<sf::Vertex, 4u> vertices;
            sf::Int8 currentValue = 0;
            sf::Int8 targetValue = 0;
            float targetPosition = 0.f;
            sf::Vector2f size;
            void update(float);
        };
        std::vector<SubRect> m_subRects;

        const sf::Texture& m_texture;
        sf::Text m_text;

        std::vector<sf::Vertex> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_COUNTER_DISPLAY_HPP_