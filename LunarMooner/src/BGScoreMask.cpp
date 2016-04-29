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

#include <BGScoreboardMask.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const sf::Color borderColour(0u, 20u, 50u, 190u);
}

ScoreMask::ScoreMask(xy::MessageBus& mb, const sf::FloatRect& area)
    : xy::Component(mb, this)
{
    //ugh magic numbers for screen size again
    m_vertices = 
    {
        sf::Vector2f(),
        sf::Vector2f(area.left, 0.f),
        sf::Vector2f(area.left, xy::DefaultSceneSize.y),
        sf::Vector2f(0.f, xy::DefaultSceneSize.y),

        sf::Vector2f(area.left + area.width, 0.f),
        sf::Vector2f(xy::DefaultSceneSize.x, 0.f),
        xy::DefaultSceneSize,
        sf::Vector2f(area.left + area.width, xy::DefaultSceneSize.y)
    };

    m_vertices[0].color = sf::Color::Transparent;
    m_vertices[1].color = borderColour;
    m_vertices[2].color = borderColour;
    m_vertices[3].color = sf::Color::Transparent;
    m_vertices[4].color = borderColour;
    m_vertices[5].color = sf::Color::Transparent;
    m_vertices[6].color = sf::Color::Transparent;
    m_vertices[7].color = borderColour;
}

//public

//private
void ScoreMask::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}