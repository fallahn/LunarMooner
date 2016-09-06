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

#include <PHHeadsUp.hpp>

#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace ph;

HeadsUpDisplay::HeadsUpDisplay(xy::MessageBus& mb, ResourceCollection& rc)
    : xy::Component(mb, this),
    m_clock(rc.textureResource.get("assets/images/game/console/nixie_sheet.png")),
    m_time(90.f)
{
    //TODO position clock centre top
}

//public
void HeadsUpDisplay::entityUpdate(xy::Entity&, float dt)
{
    float lastTime = m_time;
    m_time = std::max(0.f, m_time - dt);

    //TODO raise event if time expired
    if (m_time == 0 && lastTime > 0)
    {

    }

    m_clock.setTime(m_time);
}

//private
void HeadsUpDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_clock);
}