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

#include <PHBeamDrawable.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace ph;

namespace
{
    const float beamWidth = 12.f;
    const float beamOffset = 0.f;
    const sf::Color beamColour = { 0, 250, 250, 140 };
}

BeamDrawable::BeamDrawable(xy::MessageBus& mb)
    : xy::Component(mb, this)
{
    m_vertices[0] = { {-beamWidth, beamOffset}, sf::Color::Transparent };
    m_vertices[1] = { {beamWidth, beamOffset}, sf::Color::Transparent };
    m_vertices[2] = { {0.f, 1.f}, beamColour };
}

//public
void BeamDrawable::entityUpdate(xy::Entity& entity, float dt)
{
   //TODO some funky animation or sth 
}

void BeamDrawable::onDelayedStart(xy::Entity& entity)
{
    auto origin = entity.getOrigin();
    setPosition(origin);
    setRotation(xy::Util::Vector::rotation(origin) + 90.f);
    setScale(1.f, xy::Util::Vector::length(origin));
}

//private
void BeamDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_vertices.data(), 3, sf::Triangles, states);
}