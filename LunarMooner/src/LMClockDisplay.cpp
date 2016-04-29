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

#include <LMClockDisplay.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <functional>

using namespace lm;

namespace
{
    const int rows = 3;
    const int cols = 4;
}

ClockDisplay::ClockDisplay(const sf::Texture& t)
    : m_texture(t)
{
    //map our texture coords
    sf::Vector2f size(t.getSize());
    size.x /= cols;
    size.y /= rows;

    auto i = 0;
    for (auto y = 0; y < rows; ++y)
    {
        for (auto x = 0; x < cols; ++x, ++i)
        {
            const sf::Vector2f offset(x * size.x, y * size.y);
            m_digits[i].coords[0] = offset;
            m_digits[i].coords[1] = { offset.x + size.x, y * size.y };
            m_digits[i].coords[2] = offset + size;
            m_digits[i].coords[3] = { offset.x, offset.y + size.y };
        }
    }

    //set clock quad positions
    for (auto i = 0; i < 5; ++i)
    {
        const sf::Vector2f offset(i * size.x, 0);
        std::size_t idx = i * 4;

        m_vertices[idx++].position = offset;
        m_vertices[idx++].position = { offset.x + size.x, offset.y };
        m_vertices[idx++].position = { offset.x + size.x, size.y };
        m_vertices[idx].position = { offset.x, size.y };
    }

    //middle quad is always colon
    std::size_t idx = 8;
    m_vertices[idx++].texCoords = m_digits[Digit::Colon].coords[0];
    m_vertices[idx++].texCoords = m_digits[Digit::Colon].coords[1];
    m_vertices[idx++].texCoords = m_digits[Digit::Colon].coords[2];
    m_vertices[idx].texCoords = m_digits[Digit::Colon].coords[3];
}

//public 
void ClockDisplay::setTime(float time)
{
    std::function<void(Digit, std::uint32_t)> setDigit = [this](Digit d, std::uint32_t idx)
    {
        idx *= 4;
        m_vertices[idx++].texCoords = m_digits[d].coords[0];
        m_vertices[idx++].texCoords = m_digits[d].coords[1];
        m_vertices[idx++].texCoords = m_digits[d].coords[2];
        m_vertices[idx].texCoords = m_digits[d].coords[3];
    };
    
    //minutes
    std::uint32_t minutes = static_cast<std::uint32_t>(time / 60.f);
    if (minutes > 9)
    {
        setDigit(static_cast<Digit>(minutes / 10), 0);
    }
    else
    {
        setDigit(Digit::Space, 0);
    }
    setDigit(static_cast<Digit>(minutes % 10), 1);

    //second
    std::uint32_t seconds = static_cast<std::uint32_t>(std::fmod(time, 60.f));
    if (seconds > 9)
    {
        setDigit(static_cast<Digit>(seconds / 10), 3);
    }
    else
    {
        setDigit(Digit::Zero, 3);
    }
    setDigit(static_cast<Digit>(seconds % 10), 4);
}

//private
void ClockDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}