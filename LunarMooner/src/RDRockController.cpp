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

#include <RDRockController.hpp>
#include <LMCollisionComponent.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>

using namespace rd;

namespace
{
    const sf::Vector2f velocity(-800.f, 0.f);
}

RockController::RockController(xy::MessageBus& mb, const sf::Vector2f& homePosition)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_alive         (false),
    m_homePosition  (homePosition)
{

}

//public
void RockController::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_alive)
    {
        entity.move(velocity * dt);

        //check if we went off the edge of the screen
        if (entity.getWorldPosition().x < -100.f)
        {
            reset();
        }
    }
}

void RockController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void RockController::spawn(const sf::Vector2f& position)
{
    m_entity->setPosition(position);
    m_alive = true;
}

void RockController::reset()
{
    m_entity->setPosition(m_homePosition);
    m_alive = false;
}

void RockController::collisionCallback(lm::CollisionComponent* cc)
{
    LOG("Rock Collision " + std::to_string((int)cc->getID()), xy::Logger::Type::Info);
    if (m_alive)
    {
        switch (cc->getID())
        {
        default: break;
        case lm::CollisionComponent::ID::Bullet:
            reset();
            cc->getParentEntity().destroy();
            //TODO raise message
            break;
        case lm::CollisionComponent::ID::Player:
            cc->getParentEntity().destroy();
            
            {
                auto msg = sendMessage<LMGameEvent>(GameEvent);
                msg->type = LMGameEvent::PlayerDied;
                msg->posX = cc->getParentEntity().getWorldPosition().x;
                msg->posY = cc->getParentEntity().getWorldPosition().y;
            }

            break;
        case lm::CollisionComponent::ID::Tower:
            //we're using Tower for rocks because it won't get
            //killed by the bullet callback that way
            reset();
            break;
        }
    }
}