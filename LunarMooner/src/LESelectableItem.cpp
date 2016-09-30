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

#include <LESelectableItem.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace le;

namespace
{
    const sf::Vector2f pointSize(10.f, 10.f);
    const sf::Color outlineColour = sf::Color::Red;
    const sf::Color selectedColour = sf::Color::Red;
    const sf::Color deselectedColour = sf::Color::Transparent;
}

//----POINTS----//
PointItem::PointItem()
{
    m_shape.setSize(pointSize);
    m_shape.setOrigin(pointSize / 2.f);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(outlineColour);
    m_shape.setFillColor(deselectedColour);
}

//public
void PointItem::select()
{
    m_shape.setFillColor(selectedColour);
}

void PointItem::deselect()
{
    m_shape.setFillColor(deselectedColour);
}

sf::FloatRect PointItem::globalBounds() const
{
    return getTransform().transformRect(m_shape.getGlobalBounds());
}

//private
void PointItem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_shape, states);
}