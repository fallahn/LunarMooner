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

#include <LMEarlyWarning.hpp>

#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const float radius = 20.f;
    const float initialScale = 15.f;
    const sf::Vector2f velocity(1.f, 0.f);
}

EarlyWarning::EarlyWarning(xy::MessageBus& mb, const sf::Vector2f& destination)
    : xy::Component     (mb, this),
    m_initialPosition   (0.f),
    m_speed             (0.f),
    m_scale             (initialScale),
    m_destination       (destination)
{
    m_shape.setRadius(radius);
    m_shape.setPointCount(3);
    m_shape.setFillColor(sf::Color::Transparent);
    m_shape.setOrigin(radius, 0.f);
    m_shape.setScale(m_scale, m_scale);
    m_shape.setOutlineColor(sf::Color::White);
    m_shape.setOutlineThickness(2.f);
}

//public
void EarlyWarning::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_scale > 1)
    {
        //we're zooming in
        m_scale = std::max(1.f, m_scale - (80.f * dt));
        m_shape.setScale(m_scale, m_scale);

        //and fading in
        const float ratio = 1.f - (m_scale - 1.f) / (initialScale - 1.f);
        const sf::Uint8 alpha = static_cast<sf::Uint8>(255.f * ratio);
        m_shape.setFillColor({ 255u, 0u, 0u, alpha });
        m_shape.setOutlineColor({ 255u, 255u, 255u, alpha });
    }
    else if (m_speed != 0)
    {
        //moving to dest
        auto position = entity.getPosition();
        entity.move(velocity * m_speed * dt);
        m_speed *= 0.97f;

        if (std::abs(position.x - m_destination.x) < 4.f)
        {
            //we've arrived!
            m_speed = 0.f;
        }
    }
    else
    {
        float alpha = static_cast<float>(m_shape.getFillColor().a);
        alpha = std::max(0.f, alpha - (255.f * dt));
        if (alpha > 0)
        {
            sf::Uint8 a = static_cast<sf::Uint8>(alpha);
            m_shape.setFillColor({ 255u, 0u, 0u,  a});
            m_shape.setOutlineColor({ 255u, 255u, 255u, a });
        }
        else
        {
            entity.destroy();
        }
    }
}

void EarlyWarning::onDelayedStart(xy::Entity& entity)
{
    m_initialPosition = entity.getPosition().x;
    m_speed = (m_destination.x - m_initialPosition) / 0.4f; //TODO time it should take to reach dest
}

//private
void EarlyWarning::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_shape, states);
}