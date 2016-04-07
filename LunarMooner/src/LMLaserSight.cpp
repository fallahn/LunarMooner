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

#include <LMLaserSight.hpp>

#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const sf::Uint8 initialAlphaFill = 190u;
    const sf::Uint8 initialAlphaOutline = 100u;
}

LaserSight::LaserSight(xy::MessageBus& mb, float rotation)
    :xy::Component  (mb, this),
    m_alpha         (1.f)
{
    m_shape.setFillColor({ 255u, 10u, 10u, initialAlphaFill });
    m_shape.setOutlineColor({ 105u, 0u, 0u, initialAlphaOutline });
    m_shape.setOutlineThickness(1.f);
    m_shape.setSize({ 1600.f, 2.f });
    m_shape.setOrigin(0.f, 1.f);
    m_shape.setRotation(rotation);
}

//public
void LaserSight::entityUpdate(xy::Entity& entity, float dt)
{
    m_alpha = std::max(0.f, m_alpha - dt);
    
    auto colour = m_shape.getFillColor();
    colour.a = static_cast<sf::Uint8>(initialAlphaFill * m_alpha);
    m_shape.setFillColor(colour);

    colour = m_shape.getOutlineColor();
    colour.a = static_cast<sf::Uint8>(initialAlphaOutline * m_alpha);
    m_shape.setOutlineColor(colour);

    if (m_alpha == 0)
    {
        entity.destroy();
    }
}

//private
void LaserSight::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.blendMode = sf::BlendAdd;
    rt.draw(m_shape, states);
}