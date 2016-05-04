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

#include <LMCounterDisplay.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <xygine/Assert.hpp>

#include <functional>

using namespace lm;

namespace
{
    const float textSpacing = 34.f;
}

CounterDisplay::CounterDisplay(sf::Texture& texture, const sf::Font& font, const std::string& label, sf::Uint8 digitCount)
    : m_subRects(digitCount),
    m_texture   (texture),
    m_vertices  (digitCount * 8) //8 verts per digit
{
    texture.setRepeated(true);

    m_text.setFont(font);
    m_text.setString(label);
    m_text.setPosition(-(m_text.getLocalBounds().width + textSpacing), 0.f);

    sf::Vector2f size(texture.getSize());
    size.x /= 2.f;
    size.y /= 10.f;

    auto v = 0u;
    for (auto i = 0u; i < m_subRects.size(); ++i)
    {
        m_subRects[i].vertices[1].texCoords.x = size.x;
        m_subRects[i].vertices[2].texCoords = size;
        m_subRects[i].vertices[3].texCoords.y = size.y;

        m_subRects[i].vertices[0].position.x = size.x * i;
        m_subRects[i].vertices[1].position.x = (size.x * i) + size.x;
        m_subRects[i].vertices[2].position = { (size.x * i) + size.x , size.y };
        m_subRects[i].vertices[3].position = { size.x * i, size.y };

        m_subRects[i].size = size;

        m_vertices[v++] = m_subRects[i].vertices[0];
        m_vertices[v++] = m_subRects[i].vertices[1];
        m_vertices[v++] = m_subRects[i].vertices[2];
        m_vertices[v++] = m_subRects[i].vertices[3];
    }

    for (const auto& sr : m_subRects)
    {
        for (const auto vert : sr.vertices)
        {
            m_vertices[v].texCoords = vert.texCoords;
            m_vertices[v].texCoords.x += size.x;
            m_vertices[v++].position = vert.position;
        }
    }
}

//public
void CounterDisplay::update(float dt)
{
    auto i = 0u;
    for (auto& sr : m_subRects)
    {
        const auto& tx = sr.getTransform();
        for (const auto v : sr.vertices)
        {
            m_vertices[i++].texCoords = tx.transformPoint(v.texCoords);
        }

        //scrolls digit if not set
        if (sr.currentValue != sr.targetValue)
        {
            sr.update(dt);
        }
    }
}

void CounterDisplay::setValue(int value)
{
    XY_ASSERT(value < (std::pow(10.f, m_subRects.size()) - 1), "Value too large!");

    std::function<void(std::vector<sf::Uint8>&, int)> getDigits =
        [&getDigits](std::vector<sf::Uint8>& op, int number)
    {
        std::vector<sf::Uint8> retVal;
        if (number > 9)
        {
            getDigits(op, number / 10);
        }
        op.push_back(number % 10);
    };

    std::vector<sf::Uint8> digits;
    getDigits(digits, value);

    //may be fewer digits than display capable of
    std::size_t diff = m_subRects.size() - digits.size();
    for (std::size_t i = 0u; i < digits.size(); ++i, ++diff) 
    {
        m_subRects[diff].targetValue = digits[i];

        sf::Int8 distance = m_subRects[diff].targetValue - m_subRects[diff].currentValue;
        if (distance != 0)
        {
            if (std::abs(distance > 5))
            {
                distance = 10 - distance;
            }
            m_subRects[diff].targetPosition = (m_subRects[diff].currentValue * m_subRects[diff].size.y) + (distance * m_subRects[diff].size.y);
        }
    }
}

//private
void CounterDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();

    rt.draw(m_text, states);
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

//digit update
void CounterDisplay::SubRect::update(float dt)
{
    static const float moveSpeed = 100.f;

    auto pos = getPosition();
    if (pos.y > targetPosition)
    {
        move(0.f, -moveSpeed *dt);
    }
    else
    {
        move(0.f, moveSpeed * dt);
    }

    if (std::abs(pos.y - targetPosition) < 2.f)
    {
        setPosition(pos.x, targetPosition);
        currentValue = targetValue;
    }
}