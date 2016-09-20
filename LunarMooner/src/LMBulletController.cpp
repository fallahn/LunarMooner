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

#include <LMBulletController.hpp>
#include <LMCollisionComponent.hpp>

#include <xygine/Entity.hpp>

using namespace lm;

namespace
{
    const float speed = 600.f;
}

BulletController::BulletController(xy::MessageBus& mb, LMDirection direction)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_speed         (speed)
{
    switch (direction)
    {
    default: break;
    case LMDirection::Up:
        m_velocity.y = -1.f;
        break;
    case LMDirection::Down:
        m_velocity.y = 1.f;
        break;
    case LMDirection::Left:
        m_velocity.x = 1.f;
        break;
    case LMDirection::Right:
        m_velocity.x = -1.f;
        break;
    }
}

//public
void BulletController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * m_speed * dt);
}

void BulletController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void BulletController::collisionCallback(CollisionComponent* cc)
{
    switch (cc->getID())
    {    
    case CollisionComponent::ID::Player:
    case CollisionComponent::ID::Mothership:
    case CollisionComponent::ID::Ammo:
    case CollisionComponent::ID::Shield:
    case CollisionComponent::ID::Tower:
        //do nothing
        break;
    default: 
        //destroy becasue we hit something
        m_entity->destroy();
        break;
    }
}