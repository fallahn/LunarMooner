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

#include <LMMothershipController.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Wavetable.hpp>

using namespace lm;

namespace
{
    //const auto wavetable = xy::Util::Wavetable::sine(0.05f);
}

MothershipController::MothershipController(xy::MessageBus& mb, sf::Vector2f travelAmount)
    : xy::Component (mb, this),
    m_bounds        (travelAmount),
    m_speed         (110.f),
    m_index         (/*wavetable.size() / 4*/0)
{
    m_velocity.x = (xy::Util::Random::value(0, 1) == 0) ? 1.f : -1.f;
    //m_speed *= 0.2f;
}

//public
void MothershipController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * m_speed /** wavetable[m_index]*/ * dt);
        
    //m_index = (m_index + 1) % wavetable.size();

    auto bounds = entity.globalBounds();
    if (bounds.left < m_bounds.x ||
        bounds.left + bounds.width > m_bounds.y)
    {
        m_speed = -m_speed;
    }
}