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

#include <RDDistanceDrawable.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace rd;

namespace
{
    const sf::Vector2f rectSize(xy::DefaultSceneSize.x, 20.f);
    const float arrowRadius = 20.f;
}

DistanceMeter::DistanceMeter(xy::MessageBus& mb)
    :xy::Component(mb, this)
{
    m_circleShape.setPointCount(3);
    m_circleShape.setRadius(arrowRadius);
    m_circleShape.setFillColor(sf::Color::Yellow);
    m_circleShape.setOutlineColor(sf::Color::Red);
    m_circleShape.setOutlineThickness(2.f);
    m_circleShape.setOrigin(arrowRadius, arrowRadius);
    m_circleShape.setPosition(0.f, arrowRadius);
    m_circleShape.setRotation(180.f);

    m_rectangleShape.setSize(rectSize);
    m_rectangleShape.setPosition(0.f, arrowRadius);
    m_rectangleShape.setFillColor(sf::Color::Transparent);
    m_rectangleShape.setOutlineColor(sf::Color::Yellow);
    m_rectangleShape.setOutlineThickness(-4.f);
}

//public
void DistanceMeter::entityUpdate(xy::Entity&, float)
{

}

void DistanceMeter::setDistance(float distance)
{
    //XY_ASSERT(distance >= 0 && distance <= 1.f, "Distance out of range");

    distance = std::min(1.f, std::max(0.f, distance));
    m_circleShape.setPosition(xy::DefaultSceneSize.x * distance, arrowRadius);
}

//private
void DistanceMeter::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_rectangleShape, states);
    rt.draw(m_circleShape, states);
}