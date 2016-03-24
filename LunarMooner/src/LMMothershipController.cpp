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

using namespace lm;

MothershipController::MothershipController(xy::MessageBus& mb, sf::Vector2f travelAmount)
    : xy::Component (mb, this),
    m_bounds        (travelAmount),
    m_speed         (140.f)
{
    m_velocity.x = 1.f;
}

//public
void MothershipController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * m_speed * dt);
        
    auto bounds = entity.globalBounds();
    if (bounds.left < m_bounds.x ||
        bounds.left + bounds.width > m_bounds.y)
    {
        m_speed = -m_speed;
    }
}