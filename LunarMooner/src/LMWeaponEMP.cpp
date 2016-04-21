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

#include <LMWeaponEMP.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const float startIncrease = 600.f;
    const float decreasePerSecond = 460.f;
}

WeaponEMP::WeaponEMP(xy::MessageBus& mb, const xy::Scene& scene)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_radiusIncrease(startIncrease)
{
    m_shape.setRadius(0.5f);
    m_shape.setOrigin(0.25f, 0.25f);
    m_shape.setFillColor(sf::Color::Transparent);
    m_shape.setOutlineColor(sf::Color::Magenta);
    m_shape.setOutlineThickness(2.f);
}

//public
void WeaponEMP::entityUpdate(xy::Entity& entity, float dt)
{
    m_radiusIncrease = std::max(0.f, m_radiusIncrease - (decreasePerSecond * dt));
    
    auto scale = m_shape.getScale();
    scale.x += m_radiusIncrease * dt;
    scale.y += m_radiusIncrease * dt;

    m_shape.setScale(scale);
    m_shape.setOutlineThickness(2.f * (1 / scale.x));

    if (m_radiusIncrease == 0)
    {
        entity.destroy();
    }

    const float alpha = std::max(0.f, m_radiusIncrease / startIncrease);
    sf::Color c(255u, 0u, 255u, static_cast<sf::Uint8>(alpha * 255.f));
    m_shape.setOutlineColor(c);

    //TODO collision detection
}


//private
void WeaponEMP::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.blendMode = sf::BlendAdd;
    rt.draw(m_shape, states);
}