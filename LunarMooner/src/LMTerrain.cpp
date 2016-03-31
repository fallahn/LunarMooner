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

Terrain::Terrain(xy::MessageBus& mb, const std::array<std::pair<sf::Vector2f, sf::Vector2f>, 4u>& platforms, const sf::FloatRect& bounds)
    :xy::Component(mb, this)
{
    //create chain
    m_chain = 
    {
        {bounds.left, 850.f},
        platforms[0].first + sf::Vector2f(2.f, 4.f),
        {platforms[0].first.x + platforms[0].second.x, platforms[0].first.y + 4.f},
        {513.f, 981.f},
        {536.f, 828.f},
        {628.f, 810.f},
        {703.f, 821.f},
        {768.f, 842.f},
        {790.f, 850.f},
        //{884.f, 823.f},
        platforms[1].first + sf::Vector2f(2.f, 4.f),
        { platforms[1].first.x + platforms[1].second.x, platforms[1].first.y + 4.f },
        {1072.f, 815.f},
        {1204.f, 787.f},
        {1278.f, 803.f},
        {1229.f, 850.f},
        {1207.f, 931.f},
        platforms[2].first + sf::Vector2f(2.f, 4.f),
        { platforms[2].first.x + platforms[2].second.x, platforms[2].first.y + 4.f },
        {1416.f, 970.f},
        {1462.f, 902.f},
        //{1541.f, 873.f},
        //{1566.f, 835.f},
        //{1573.f, 788.f},
        platforms[3].first + sf::Vector2f(2.f, 4.f),
        { platforms[3].first.x + platforms[3].second.x, platforms[3].first.y + 4.f },
        {bounds.left + bounds.width, 764.f}
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
    rt.draw(m_vertices.data(), m_vertices.size(), sf::LinesStrip, states);
}