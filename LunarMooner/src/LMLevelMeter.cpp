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

#include <LMLevelMeter.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace lm;

namespace
{
    const float pointerSize = 12.f;
    const float pointerY = 48.f;
    const float padLeft = 21.f;
    const float numSpace = 16.f;
}

LevelMeter::LevelMeter(sf::Texture& t)
    :   m_texture   (t),
    m_level         (0)
{
    sf::Vector2f size(t.getSize());

    //rear panel
    m_vertices[1].position.x = size.x;
    m_vertices[1].texCoords.x = size.x;
    m_vertices[2].position = { size.x, size.y / 2.f };
    m_vertices[2].texCoords = m_vertices[2].position;
    m_vertices[3].position.y = size.y / 2.f;
    m_vertices[3].texCoords.y = size.y / 2.f;

    //front panel
    m_vertices[8].texCoords.y = size.y / 2.f;
    m_vertices[9].position.x = size.x;
    m_vertices[9].texCoords = { size.x, size.y / 2.f };
    m_vertices[10].position = { size.x, size.y / 2.f };
    m_vertices[10].texCoords = size;
    m_vertices[11].position.y = size.y / 2.f;
    m_vertices[11].texCoords.y = size.y;

    //arrow
    m_pointer.vertices[1].position.x = pointerSize;
    m_pointer.vertices[1].texCoords.x = pointerSize;
    m_pointer.vertices[2].position = { pointerSize, pointerSize };
    m_pointer.vertices[2].texCoords = m_pointer.vertices[2].position;
    m_pointer.vertices[3].position.y = pointerSize;
    m_pointer.vertices[3].texCoords.y = pointerSize;

    for (auto i = 0u; i < 4; ++i)
    {
        m_vertices[i + 4].texCoords = m_pointer.vertices[i].texCoords;
    }

    m_pointer.setOrigin(pointerSize / 2.f, pointerSize);
    m_pointer.setPosition(180.f, pointerY);

    //calc positions of numbers on dial
    m_positions[0] = 21.f;
    for (auto i = 1u; i < m_positions.size(); ++i)
    {
        m_positions[i] = m_positions[i - 1] + numSpace;
    }
}

//public
void LevelMeter::update(float dt)
{
    //move to nearest level value
    const float dist = m_positions[m_level] - m_pointer.getPosition().x;
    m_pointer.move(dist * 0.1f, 0.f);

    //update varray for pointer
    const auto& tx = m_pointer.getTransform();
    for (auto i = 0u; i < 4; ++i)
    {
        m_vertices[i + 4].position = tx.transformPoint(m_pointer.vertices[i].position);
    }
}

//private
void LevelMeter::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_texture;
    states.transform *= getTransform();
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}