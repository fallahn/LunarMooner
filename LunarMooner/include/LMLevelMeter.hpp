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

#ifndef LM_LEVEL_METER_HPP_
#define LM_LEVEL_METER_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace lm
{
    class LevelMeter final : public sf::Drawable, public sf::Transformable
    {
    public:
        explicit LevelMeter(sf::Texture&);
        ~LevelMeter() = default;

        LevelMeter(const LevelMeter&) = delete;
        LevelMeter& operator = (const LevelMeter&) = delete;

        void update(float);
        void setLevel(sf::Uint8 level) { m_level = level - 1; }

    private:

        struct Pointer : public sf::Transformable
        {
            std::array<sf::Vertex, 4> vertices;
        }m_pointer;

        sf::Texture& m_texture;

        std::array<float, 10> m_positions;
        sf::Uint8 m_level;

        std::array<sf::Vertex, 12> m_vertices;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_LEVEL_METER_HPP_