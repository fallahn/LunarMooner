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

#include <LMClockDisplay.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace lm;

namespace
{
    const int rows = 3;
    const int cols = 4;
}

ClockDisplay::ClockDisplay(const sf::Texture& t)
    : m_texture(t)
{
    //map our texture coords
    sf::Vector2f size(t.getSize());
    size.x /= cols;
    size.y /= rows;


}

//public 
void ClockDisplay::setTime(float time)
{

}

//private
void ClockDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}