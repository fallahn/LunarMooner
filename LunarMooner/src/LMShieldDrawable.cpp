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

#include <LMShieldDrawable.hpp>

#include <xygine/util/Const.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace lm;

namespace
{
    const sf::Color shieldColour = sf::Color(0u, 255u, 255u, 120u);
    const float halfWidth = 960.f;

    const float thickness = 50.f;
}

ShieldDrawable::ShieldDrawable(xy::MessageBus& mb, float radius)
    : xy::Component(mb, this)
{
    XY_ASSERT(radius > halfWidth, "Needs a bigger radius");
    const float startAngle = std::asin(halfWidth / radius);
    const float endAngle = -startAngle;
    const float step = (startAngle - endAngle) / ((m_vertices.size() / 2) - 1);

    for (auto i = 0u; i < m_vertices.size() - 1; i+=2)
    {
        const float theta = startAngle - ((i / 2) * step);
        const sf::Vector2f position(sin(theta), cos(theta));

        m_vertices[i].position = position * (radius - thickness);
        m_vertices[i].color = sf::Color::Transparent;

        m_vertices[i+1].position = position * radius;
        m_vertices[i+1].color = shieldColour;
    }
}

//public
void ShieldDrawable::entityUpdate(xy::Entity&, float)
{
    //TODO some funky animation maybe
}

//private
void ShieldDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.blendMode = sf::BlendAdd;
    states.transform *= getTransform();
    rt.draw(m_vertices.data(), m_vertices.size(), sf::TrianglesStrip, states);
}