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

#ifndef LM_CLOCK_DISPLAY_HPP_
#define LM_CLOCK_DISPLAY_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace sf
{
    class Texture;
}

namespace lm
{
    class ClockDisplay final : public sf::Drawable, public sf::Transformable
    {
    public:
        explicit ClockDisplay(const sf::Texture&);
        ~ClockDisplay() = default;

        ClockDisplay(const ClockDisplay&) = delete;
        ClockDisplay& operator = (const ClockDisplay&) = delete;

        ClockDisplay(ClockDisplay&&) noexcept = default;

        void setTime(float); //time in seconds

        const sf::FloatRect& getLocalBounds() const { return m_localBounds; }

    private:
        enum Digit
        {
            Zero,
            One,
            Two,
            Three,
            Four,
            Five,
            Size,
            Seven,
            Eight,
            Nine,
            Colon,
            Space,
            Count
        };
        struct Quad
        {
            std::array<sf::Vector2f, 4u> coords;
        };
        std::array<Quad, Digit::Count> m_digits;

        const sf::Texture& m_texture;
        sf::FloatRect m_localBounds;

        std::array<sf::Vertex, 20u> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_CLOCK_DISPLAY_HPP_