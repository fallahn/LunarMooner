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

#include <RDPlayerController.hpp>
#include <LMCollisionComponent.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/components/QuadTreeComponent.hpp>

using namespace rd;

namespace
{
    const float edgeDistance = 400.f; //distance in from left edge
    const float playerSpeed = 800.f;

    const float maxSpeed = 10490.f;
    const float maxRatio = 60.f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_velocity      (playerSpeed * 3.f, playerSpeed),
    m_active        (false)
{
    update = [this](xy::Entity& entity, float dt)
    {
        entity.move(m_velocity.x * dt, 0.f);
        m_velocity.x *= 0.925f;

        auto pos = entity.getWorldPosition();
        if (pos.x > edgeDistance)
        {
            m_velocity.x = 0.f;
            
            //idle state
            update = [](xy::Entity& entity, float dt) {};

            //send message to say we're ready top play
            auto msg = sendMessage<LMGameEvent>(GameEvent);
            msg->type = LMGameEvent::PlayerLanded;
            msg->posX = pos.x;
            msg->posY = pos.y;
        }
    };
}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    update(entity, dt);
}

void PlayerController::activate()
{
    if (!m_active && m_velocity.x == 0)
    {
        //enter player controlled update mode
        update = [this](xy::Entity& entity, float dt)
        {
            entity.move(m_velocity * dt);

            auto position = entity.getWorldPosition();
            if (position.y > xy::DefaultSceneSize.y)
            {
                position.y = -entity.getComponent<xy::QuadTreeComponent>()->localBounds().height;
                entity.setWorldPosition(position);
            }
            else if (position.y < -entity.getComponent<xy::QuadTreeComponent>()->localBounds().height)
            {
                position.y = xy::DefaultSceneSize.y;
                entity.setWorldPosition(position);
            }
        };
        m_active = true;

        auto msg = sendMessage<LMStateEvent>(StateEvent);
        msg->type = LMStateEvent::RoundBegin;
        msg->stateID = States::ID::RockDodging;
    }
    else
    {
        //IMPORTANT don't actually modify entity itself here
        //as we may be playing the end animation (which ignores
        //the current velocity property)
        m_velocity.y = -m_velocity.y;
    }
}

void PlayerController::end()
{
    m_velocity.x = playerSpeed;
    update = [this](xy::Entity& entity, float dt)
    {
        entity.move(m_velocity.x * dt, 0.f);
        if (entity.getWorldPosition().x > xy::DefaultSceneSize.x + 60.f)
        {
            //raise message to say we're at the end
            auto msg = sendMessage<LMStateEvent>(StateEvent);
            msg->type = LMStateEvent::RoundEnd;
            msg->stateID = States::ID::RockDodging;

            update = [](xy::Entity& entity, float dt) {};
            //LOG(std::to_string(m_velocity.x), xy::Logger::Type::Info);
        }
        m_velocity.x *= 1.1f; //get faster as we leave
    };
}

float PlayerController::getSpeedRatio() const
{
    return maxRatio - ((m_velocity.x / maxSpeed) * maxRatio);
}

void PlayerController::collisionCallback(lm::CollisionComponent* cc)
{
    if (!m_active) return;

    switch (cc->getID())
    {
    default: break;
    case lm::CollisionComponent::ID::Alien:
        //death by smashing
        break;
    case lm::CollisionComponent::ID::Body:
        //got human, get XP
        break;
    }
}

//private