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

#include <UIWeaponSelect.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace lm;
using namespace lm::ui;

namespace
{
    const float moveSpeed = -800.f;
}

WeaponSelect::WeaponSelect(sf::Texture& t)
    : m_texture     (t),
    m_selectedIndex (0u),
    m_nextIndex     (0u),
    m_vertexCount   (4u)
{
    sf::Vector2f size(t.getSize());
    size.x /= 5.f;
    size.y /= 2.f;

    m_bounds.width = size.x;
    m_bounds.height = size.y;

    for (auto i = 0u; i < m_items.size(); ++i)
    {
        m_items[i].vertices[1].position.x = size.x;
        m_items[i].vertices[2].position = size;
        m_items[i].vertices[3].position.y = size.y;

        m_items[i].vertices[0].texCoords = { i * m_bounds.width, m_bounds.height };
        m_items[i].vertices[1].texCoords = { (i * m_bounds.width) + m_bounds.width, m_bounds.height };
        m_items[i].vertices[2].texCoords = { (i * m_bounds.width) + m_bounds.width, m_bounds.height * 2.f };
        m_items[i].vertices[3].texCoords = { i * m_bounds.width, m_bounds.height * 2.f };
    }

    auto i = 0u;
    for (const auto& v : m_items[m_selectedIndex].vertices)
    {
        m_vertices[i++] = v;
    }
}

//public
void WeaponSelect::select()
{
    xy::UI::Control::select();

    //highlight control
    for (auto& v : m_vertices)
    {
        v.color = sf::Color::Cyan;
    }
}

void WeaponSelect::deselect()
{
    xy::UI::Control::deselect();

    //lowlight control
    for (auto& v : m_vertices)
    {
        v.color = sf::Color::White;
    }
}

void WeaponSelect::activate()
{
    xy::UI::Control::activate();

    //start animation
    m_nextIndex = (m_selectedIndex + 1) % m_items.size();
    m_items[m_nextIndex].setPosition(m_bounds.width, 0.f);
}

void WeaponSelect::deactivate()
{
    xy::UI::Control::deactivate();
    //TODO.... do we need this?
}

void WeaponSelect::update(float dt)
{
    //update any on going animatons
    if (m_selectedIndex != m_nextIndex)
    {
        m_items[m_selectedIndex].move(moveSpeed * dt, 0.f);
        m_items[m_nextIndex].move(moveSpeed * dt, 0.f);

        m_items[m_selectedIndex].alpha = (1.f - std::abs(m_items[m_selectedIndex].getPosition().x / m_bounds.width)) * 255.f;
        m_items[m_nextIndex].alpha = (1.f - (m_items[m_nextIndex].getPosition().x / m_bounds.width)) * 255.f;

        m_vertexCount = 8u;
        
        //we have reached our destination
        if (m_items[m_nextIndex].getPosition().x < 0.f)
        {
            m_selectedIndex = m_nextIndex;
            m_items[m_selectedIndex].setPosition(0.f, 0.f);
            m_items[m_selectedIndex].alpha = 255.f;

            m_vertexCount = 4u;
            deactivate(); //else we won't want to activate again
        }

        //update vertex array
        updateVertexArray();
    }
}

void WeaponSelect::handleEvent(const sf::Event& evt, const sf::Vector2f& mousePos)
{

}

void WeaponSelect::setAlignment(xy::UI::Alignment alignment)
{
    switch (alignment)
    {
    default: break;
    case xy::UI::Alignment::TopLeft:
        setOrigin(0.f, 0.f);
        break;
    case xy::UI::Alignment::BottomLeft:
        setOrigin(0.f, m_bounds.height);
        break;
    case xy::UI::Alignment::Centre:
        setOrigin(m_bounds.width / 2.f, m_bounds.height / 2.f);
        break;
    case xy::UI::Alignment::TopRight:
        setOrigin(m_bounds.width, 0.f);
        break;
    case xy::UI::Alignment::BottomRight:
        setOrigin(m_bounds.width, m_bounds.height);
        break;
    }
}

bool WeaponSelect::contains(const sf::Vector2f& mousePos) const
{
    return getTransform().transformRect(m_bounds).contains(mousePos);
}

void WeaponSelect::setLockedFlags(sf::Uint8 flags)
{
    for (auto i = 0u; i < m_items.size(); ++i)
    {
        if (flags & (1 << i))
        {
            m_items[i].vertices[0].texCoords.y = m_bounds.height;
            m_items[i].vertices[1].texCoords.y =  m_bounds.height;
            m_items[i].vertices[2].texCoords.y = m_bounds.height * 2.f;
            m_items[i].vertices[3].texCoords.y = m_bounds.height * 2.f;
        }
        else
        {
            m_items[i].vertices[0].texCoords.y = 0.f;
            m_items[i].vertices[1].texCoords.y = 0.f;
            m_items[i].vertices[2].texCoords.y = m_bounds.height - 1.f;
            m_items[i].vertices[3].texCoords.y = m_bounds.height - 1.f;
        }
    }

    updateVertexArray();
}

//private
void WeaponSelect::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_texture;
    states.transform *= getTransform();
    rt.draw(m_vertices.data(), m_vertexCount, sf::Quads, states);
}

void WeaponSelect::updateVertexArray()
{
    const auto& txA = m_items[m_selectedIndex].getTransform();
    const auto& txB = m_items[m_nextIndex].getTransform();
    for (auto i = 0u; i < 4u; ++i)
    {

        m_vertices[i] = m_items[m_selectedIndex].vertices[i];
        m_vertices[i].position = txA.transformPoint(m_vertices[i].position);
        m_vertices[i].color = sf::Color(255, 255, 255, static_cast<sf::Uint8>(m_items[m_selectedIndex].alpha));

        m_vertices[i + 4] = m_items[m_nextIndex].vertices[i];
        m_vertices[i + 4].position = txB.transformPoint(m_vertices[i + 4].position);
        m_vertices[i + 4].color = sf::Color(255, 255, 255, static_cast<sf::Uint8>(m_items[m_nextIndex].alpha));
    }
}