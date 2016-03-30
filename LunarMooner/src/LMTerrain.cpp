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

#include <LMTerrain.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

Terrain::Terrain(xy::MessageBus& mb, const std::array<std::pair<sf::Vector2f, sf::Vector2f>, 4u>& platforms)
    :xy::Component(mb, this)
{
    //create chain
    m_chain = 
    {
        {0.f, 600.f}, //TODO make this relative to world edge
        platforms[0].first + sf::Vector2f(2.f, 4.f),
        {platforms[0].first.x + platforms[0].second.x, platforms[0].first.y + 4.f}
    };

    //use chain for drawable
    for (const auto p : m_chain)
    {
        m_vertices.push_back({ p, sf::Color::Red });
    }
}

//public
void Terrain::entityUpdate(xy::Entity&, float)
{

}

//private
void Terrain::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::LinesStrip/*, states*/);
}